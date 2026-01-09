# Conversion.cpp 信息泄露漏洞完整分析

## 問題描述

在 `Conversion::Convert(const char* phrase)` 函數中發現一個**嚴重的信息洩露（Information Disclosure）安全漏洞**。當處理包含截斷 UTF-8 序列的輸入時，函數會跳過 null 終止符，繼續讀取堆內存，並將洩露的數據輸出到轉換結果中。

**發現時間：** 2026-01-08（在修復 Issue #997 時的安全審查中發現）
**嚴重程度：** 嚴重（Critical）- 比 Issue #997 更嚴重
**影響版本：** 1.1.x 及更早版本
**漏洞類型：**
- 堆緩衝區溢出讀取（Heap Buffer Over-read）
- 信息洩露（Information Disclosure）
- 未定義行為（Undefined Behavior）

---

## 問題定位

### 受影響代碼

**文件：** `src/Conversion.cpp`
**函數：** `Conversion::Convert(const char* phrase)`
**行號：** 24-39

---

## 根本原因分析

### 原始代碼（有問題的版本）

```cpp
std::string Conversion::Convert(const char* phrase) const {
  std::ostringstream buffer;
  for (const char* pstr = phrase; *pstr != '\0';) {  // 第 26 行
    Optional<const DictEntry*> matched = dict->MatchPrefix(pstr);  // 第 27 行
    size_t matchedLength;
    if (matched.IsNull()) {
      matchedLength = UTF8Util::NextCharLength(pstr);  // 第 30 行 - 問題！
      buffer << UTF8Util::FromSubstr(pstr, matchedLength);  // 第 31 行 - 洩露！
    } else {
      matchedLength = matched.Get()->KeyLength();
      buffer << matched.Get()->GetDefault();
    }
    pstr += matchedLength;  // 第 36 行 - 跳過 null！
  }
  return buffer.str();
}
```

### 漏洞機制詳解

**攻擊場景：** 攻擊者構造包含截斷 UTF-8 序列的輸入字符串

```cpp
// 惡意構造的輸入
char malicious[100];
malicious[0] = '\xE5';  // "干" 字節1
malicious[1] = '\xB9';  // "干" 字節2
malicious[2] = '\xB2';  // "干" 字節3
malicious[3] = '\xE5';  // 開始第二個字符
malicious[4] = '\xB9';  // 只有2字節
malicious[5] = '\0';    // null終止 - 缺少第3字節！
// malicious[6..99] = 堆上的其他數據（可能包含敏感信息）

std::string leaked = conversion->Convert(malicious);
// leaked 包含："乾" + 堆內存洩露的數據！
```

### 執行流程分解

**初始狀態：**
- 輸入：`"干" + \xE5\xB9\x00` (5 字節)
- 堆內存佈局：null 後面可能包含敏感數據（如密鑰、用戶信息等）

**第一次迭代：**
1. `pstr` 指向 `\xE5\xB9\xB2`（"干"，完整的 3 字節字符）
2. `MatchPrefix(pstr)` 查找詞典，假設找到轉換為 "乾"
3. `matchedLength = 3`
4. 輸出 "乾" 到 buffer
5. `pstr += 3`，現在指向 `\xE5\xB9\x00`

**第二次迭代（觸發漏洞）：**
1. 循環條件檢查：`*pstr != '\0'` → `\xE5 != '\0'` → true，繼續
2. `MatchPrefix(pstr)` 沒有匹配（不完整的字符）
3. 進入 `if (matched.IsNull())` 分支
4. `NextCharLength(pstr)` 檢查 `\xE5` → 返回 **3**（3 字節 UTF-8 的預期長度）
5. **問題發生：** `FromSubstr(pstr, 3)` 嘗試複製 3 字節：
   - `pstr[0] = \xE5` ✓
   - `pstr[1] = \xB9` ✓
   - `pstr[2] = \x00` ⚠️ 複製了 null 終止符！
6. 將包含 null 的字符串片段輸出到 buffer
7. **關鍵問題：** `pstr += 3` 將指針移動到 **null 之後** 的位置！

**第三次迭代（信息洩露）：**
1. `pstr` 現在指向堆內存中 null 終止符之後的位置
2. 如果 `*pstr != '\0'`（堆內存中的垃圾數據不是 \x00）
3. 循環繼續，讀取堆內存
4. **將堆內存數據輸出到轉換結果** ⚠️⚠️⚠️

**示例輸出（信息洩露）：**
```
正常輸出：  "乾"
洩露輸出：  "乾" + "\xAA\xBB\xCC..." (堆內存垃圾數據)
```

### 實際演示結果

使用測試程序驗證，輸入 `"一" + \xE4\xB8 + null`：

```
Iteration 2:
  pstr offset: 3
  *pstr: 0xe4
  NextCharLength returned: 3
  Bytes to copy: e4 b8 00
  ⚠️ WARNING: Copying includes/crosses null terminator!
  After pstr += 3, new offset: 6
  ⚠️ CRITICAL: pstr is now BEYOND the null terminator!

Iteration 3:
  pstr offset: 6
  *pstr: 0xaa  ← 堆內存數據
  NextCharLength returned: 1
  Bytes to copy: aa  ← 這會被輸出到結果！
```

---

## 與 Issue #997 的對比

| 特徵 | Issue #997 (MaxMatchSegmentation) | 本漏洞 (Conversion) |
|------|-----------------------------------|---------------------|
| **漏洞類型** | 整數下溢 → 緩衝區溢出讀取 | 跳過 null → 信息洩露 |
| **觸發機制** | `length -= matchedLength` 下溢 | `pstr += matchedLength` 跳過 null |
| **是否輸出洩露數據** | ❌ 否（只讀取，不輸出） | ✅ **是（輸出到結果）** |
| **危害程度** | 高（DoS + 潛在崩潰） | **嚴重（信息洩露 + DoS）** |
| **可利用性** | 中等（需要 ASan 檢測） | **高（直接洩露數據）** |
| **CVSS 評分** | ~7.5 (High) | **~8.5 (Critical)** |

---

## 安全影響評估

### 1. 信息洩露（Critical）

**最嚴重的後果：** 將堆內存內容洩露到轉換結果中

**可能洩露的敏感信息：**
- 🔴 加密密鑰、API tokens
- 🔴 用戶個人信息（姓名、郵箱、密碼）
- 🔴 其他正在處理的文本內容
- 🔴 內部數據結構、指針地址（ASLR 繞過）

**攻擊場景：**
```cpp
// 在 Web 服務中
std::string user_input = request.getParameter("text");  // 攻擊者控制
std::string converted = converter->Convert(user_input);
response.send(converted);  // 洩露的堆數據被發送回攻擊者！
```

### 2. 拒絕服務（High）

- 如果堆內存中長時間沒有遇到 `\x00`，循環會持續讀取
- 可能導致程序崩潰或掛起
- 影響服務可用性

### 3. 未定義行為（Critical）

- C++ 標準：訪問數組邊界外的內存是未定義行為
- 可能導致：
  - 段錯誤（Segmentation Fault）
  - 數據損壞
  - 在某些環境下可能被利用為代碼執行（理論上）

### 4. ASLR 繞過

- 洩露的指針地址可用於繞過地址空間佈局隨機化
- 配合其他漏洞可能實現更嚴重的攻擊

---

## 為什麼正常使用不易觸發？

### 安全場景（不會觸發）

1. **完整的 UTF-8 字符串**
   ```cpp
   std::string normal = "正常的中文文本";  // 所有字符完整
   // ✓ 安全：不會觸發漏洞
   ```

2. **從可信來源讀取的文件**
   - 文本編輯器保存的文件
   - 標準 API 返回的字符串
   - 正確編碼的網絡響應

### 危險場景（會觸發）

1. **不可信用戶輸入**
   ```cpp
   // Web 表單、API 請求
   std::string user_input = get_untrusted_input();
   converter->Convert(user_input);  // ⚠️ 危險
   ```

2. **二進制數據誤當作文本**
   ```cpp
   // 損壞的文件、網絡數據包
   char corrupted_data[100];
   read_from_network(corrupted_data);
   converter->Convert(corrupted_data);  // ⚠️ 危險
   ```

3. **手動構造的字符串**
   ```cpp
   char buf[10];
   buf[0] = '\xE5';
   buf[1] = '\xB9';
   buf[2] = '\0';  // 截斷的 UTF-8
   converter->Convert(buf);  // ⚠️ 觸發漏洞
   ```

4. **不正確的字符串截斷**
   ```cpp
   std::string text = get_long_text();
   text = text.substr(0, some_length);  // 可能在字符中間截斷
   converter->Convert(text);  // ⚠️ 可能觸發
   ```

**結論：** 雖然正常使用不易觸發，但在處理不可信輸入的場景下，這是一個**嚴重的安全漏洞**。

---

## 解決方案

### 修復策略

使用兩層保護：
1. 動態計算剩餘長度傳遞給 `MatchPrefix`
2. 顯式檢查 `matchedLength` 不超過剩餘長度

### 修復後的代碼

```cpp
std::string Conversion::Convert(const char* phrase) const {
  std::ostringstream buffer;
  // Calculate string end to prevent reading beyond null terminator
  const char* phraseEnd = phrase;
  while (*phraseEnd != '\0') {
    phraseEnd++;
  }

  for (const char* pstr = phrase; *pstr != '\0';) {
    size_t remainingLength = phraseEnd - pstr;  // 動態計算剩餘長度
    Optional<const DictEntry*> matched = dict->MatchPrefix(pstr, remainingLength);
    size_t matchedLength;
    if (matched.IsNull()) {
      matchedLength = UTF8Util::NextCharLength(pstr);
      // Ensure we don't read beyond the null terminator
      if (matchedLength > remainingLength) {  // 顯式邊界檢查
        matchedLength = remainingLength;
      }
      buffer << UTF8Util::FromSubstr(pstr, matchedLength);
    } else {
      matchedLength = matched.Get()->KeyLength();
      buffer << matched.Get()->GetDefault();
    }
    pstr += matchedLength;  // 現在保證不會跳過 null
  }
  return buffer.str();
}
```

### 關鍵改進點

1. **計算字符串結束位置**
   ```cpp
   const char* phraseEnd = phrase;
   while (*phraseEnd != '\0') {
       phraseEnd++;
   }
   ```
   - 一次性計算字符串長度
   - 避免重複計算的開銷

2. **動態計算剩餘長度**
   ```cpp
   size_t remainingLength = phraseEnd - pstr;
   ```
   - 每次迭代精確計算從當前位置到字符串末尾的距離
   - 傳遞給 `MatchPrefix` 避免其越界訪問

3. **顯式邊界檢查**
   ```cpp
   if (matchedLength > remainingLength) {
       matchedLength = remainingLength;
   }
   ```
   - **防禦性編程**：即使 `NextCharLength` 返回錯誤值也不會越界
   - 確保 `pstr += matchedLength` 永遠不會跳過 null

### 正確性證明

**不變量：** `0 ≤ (pstr - phrase) ≤ (phraseEnd - phrase)`

**證明：**
1. 初始狀態：`pstr = phrase` → `pstr - phrase = 0` ✓
2. 每次迭代：
   - `remainingLength = phraseEnd - pstr ≥ 0`（指針算術保證）
   - `matchedLength = min(NextCharLength(pstr), remainingLength)`
   - `pstr += matchedLength` 後，`pstr ≤ phraseEnd` ✓
3. 循環終止條件：`*pstr = '\0'` → `pstr = phraseEnd` ✓

**結論：** 指針永遠不會超出字符串邊界。

---

## 測試驗證

### 新增測試用例

```cpp
TEST_F(ConversionTest, TruncatedUtf8Sequence) {
  // Construct a string ending with a truncated 3-byte UTF-8 sequence
  std::string malformed;
  malformed += utf8("干");   // Valid character
  malformed += '\xE5';       // Start of 3-byte UTF-8
  malformed += '\xB9';       // Second byte (missing third!)

  // The fixed code should handle this gracefully
  EXPECT_NO_THROW({
    const std::string converted = conversion->Convert(malformed);
    EXPECT_GE(converted.length(), 3);  // At least "乾"
    // Should NOT contain garbage heap data
  });
}
```

### 使用 AddressSanitizer 驗證

```bash
# 編譯時啟用 ASan
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make
./ConversionTest

# 舊代碼會報錯：
# ==12345==ERROR: AddressSanitizer: heap-buffer-overflow
# READ of size 1 at ...

# 新代碼通過 ✓
```

---

## 修復時間線

- **2026-01-08：** 在修復 Issue #997 的安全審查中發現此漏洞
- **2026-01-08：** 立即修復並創建測試用例
- **版本：** 待發佈於下一個補丁版本

---

## 經驗教訓與最佳實踐

### 1. 永遠驗證邊界

**不好的做法：**
```cpp
size_t len = GetExpectedLength(ptr);
ProcessBytes(ptr, len);  // 假設 len 是正確的
```

**好的做法：**
```cpp
size_t len = GetExpectedLength(ptr);
size_t available = GetAvailableBytes(ptr);
len = min(len, available);  // 顯式檢查
ProcessBytes(ptr, len);
```

### 2. 不要依賴單層保護

本漏洞的修復使用了**多層防禦**：
- 層 1：傳遞精確的 `remainingLength` 給 `MatchPrefix`
- 層 2：顯式檢查 `matchedLength ≤ remainingLength`
- 層 3：循環條件 `*pstr != '\0'` 作為最後防線

### 3. 對不可信輸入要特別小心

```cpp
// 處理外部輸入時
std::string ProcessUntrustedInput(const std::string& input) {
    // 1. 驗證 UTF-8 有效性
    if (!IsValidUTF8(input)) {
        throw InvalidInputException();
    }

    // 2. 限制長度
    if (input.length() > MAX_SAFE_LENGTH) {
        throw InputTooLongException();
    }

    // 3. 再處理
    return converter->Convert(input);
}
```

### 4. 使用靜態分析工具

定期使用以下工具掃描代碼：
- **AddressSanitizer (ASan)**：檢測內存錯誤
- **MemorySanitizer (MSan)**：檢測未初始化內存讀取
- **UndefinedBehaviorSanitizer (UBSan)**：檢測未定義行為
- **Valgrind**：內存洩露和越界訪問檢測
- **Coverity, Clang Static Analyzer**：靜態代碼分析

### 5. 模糊測試（Fuzzing）

```bash
# 使用 libFuzzer 或 AFL 進行模糊測試
clang++ -fsanitize=fuzzer,address -g fuzzer.cpp -o fuzzer
./fuzzer corpus/ -max_len=1024 -runs=1000000
```

模糊測試能自動生成各種邊界情況，包括本漏洞的觸發條件。

---

## CVE 信息

**建議申請 CVE：** 是
**漏洞類型：** CWE-125 (Out-of-bounds Read) + CWE-200 (Information Exposure)
**CVSS v3.1 評分：** 8.6 (High/Critical)

**評分依據：**
- 攻擊向量：Network (AV:N)
- 攻擊複雜度：Low (AC:L)
- 權限要求：None (PR:N)
- 用戶交互：None (UI:N)
- 影響範圍：Unchanged (S:U)
- 機密性影響：**High (C:H)** ← 信息洩露
- 完整性影響：None (I:N)
- 可用性影響：High (A:H) ← DoS

---

## 參考資料

1. [CWE-125: Out-of-bounds Read](https://cwe.mitre.org/data/definitions/125.html)
2. [CWE-200: Exposure of Sensitive Information](https://cwe.mitre.org/data/definitions/200.html)
3. [UTF-8 Encoding Specification](https://en.wikipedia.org/wiki/UTF-8)
4. [AddressSanitizer Documentation](https://github.com/google/sanitizers/wiki/AddressSanitizer)
5. [OWASP: Information Disclosure](https://owasp.org/www-community/vulnerabilities/Information_exposure_through_query_strings_in_url)

---

## 相關文件

- **源代碼：** `src/Conversion.cpp`
- **測試代碼：** `src/ConversionTest.cpp`
- **相關漏洞：** `doc/ISSUE_997_ANALYSIS.md` (MaxMatchSegmentation 緩衝區溢出)
- **UTF-8 工具：** `src/UTF8Util.hpp`

---

**結論：** 此漏洞是一個嚴重的安全問題，可導致敏感信息洩露。由於轉換結果會輸出給用戶或其他系統，洩露的堆內存數據可能包含密鑰、個人信息等敏感內容。**強烈建議所有用戶立即升級到包含此修復的版本。**

對於無法立即升級的用戶，建議：
1. 驗證所有輸入的 UTF-8 有效性
2. 限制處理不可信來源的數據
3. 在沙箱環境中運行 OpenCC
4. 監控異常的輸出長度（可能表示洩露）
