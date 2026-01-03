---
name: opencc-dict-edit
description: 編輯 OpenCC 詞典並執行相關的測試流程
tags: [dictionary, testing, opencc]
---

# OpenCC 詞典編輯技能

此技能協助編輯 OpenCC 詞典檔案，並確保正確執行測試驅動開發流程。

## 使用時機

當用戶要求：
- 新增或修改詞典條目
- 修正轉換錯誤
- 新增地區用詞

## 執行步驟

### 1. 確認詞典檔案

根據轉換類型選擇正確的詞典：

**簡繁轉換基礎：**
- `data/dictionary/STCharacters.txt` - 簡→繁（單字）
- `data/dictionary/STPhrases.txt` - 簡→繁（詞組）
- `data/dictionary/TSCharacters.txt` - 繁→簡（單字）
- `data/dictionary/TSPhrases.txt` - 繁→簡（詞組）

**臺灣地區用詞：**
- `data/dictionary/TWVariants.txt` - 臺灣異體字
- `data/dictionary/TWPhrasesIT.txt` - 資訊科技用語
- `data/dictionary/TWPhrasesName.txt` - 人名地名
- `data/dictionary/TWPhrasesOther.txt` - 其他用語

**香港地區用詞：**
- `data/dictionary/HKVariants.txt`

**日文：**
- `data/dictionary/JPShinjitaiCharacters.txt`
- `data/dictionary/JPShinjitaiPhrases.txt`

### 2. 撰寫測試案例（TDD）

**重要**：先寫測試，確保修改前測試會失敗。

編輯 `test/testcases/testcases.json`：

```json
{
  "id": "case_XXX",
  "input": "輸入文字",
  "expected": {
    "s2t": "預期輸出",
    "s2tw": "預期輸出（臺灣）",
    "s2twp": "預期輸出（臺灣含慣用詞）"
  }
}
```

**配置選擇**：
- 修改 `STPhrases.txt` → 測試 `s2t`, `s2tw`, `s2twp`, `s2hk`
- 修改 `TWPhrases*.txt` → 測試 `s2tw`, `s2twp`
- 修改 `HKVariants.txt` → 測試 `s2hk`

### 3. 執行測試（確認失敗）

```bash
bazel test //test:opencc_test
```

應該看到新測試案例失敗。

### 4. 編輯詞典

**格式**：`來源<TAB>目標`

**注意事項**：
- 使用 Tab 字元（`\t`），不是空格
- 一行一個條目
- UTF-8 編碼

範例：
```
虚伪叹息	虛偽嘆息
```

### 5. 排序詞典

**重要**：詞典必須排序，否則測試會失敗。

單一檔案：
```bash
python3 data/scripts/sort.py data/dictionary/STPhrases.txt
```

所有檔案：
```bash
python3 data/scripts/sort_all.py data/dictionary
```

### 6. 執行測試（確認成功）

```bash
bazel test //test:opencc_test
bazel test //data/dictionary:dictionary_test
```

所有測試應該通過。

### 7. 提交變更

```bash
git add data/dictionary/*.txt test/testcases/testcases.json
git commit -m "新增詞典條目：[簡要描述]"
```

## 常見陷阱

- ❌ 使用空格而非 Tab → 格式錯誤
- ❌ 忘記排序 → `DictionaryTest` 失敗
- ❌ 未先寫測試 → 無法驗證修改效果
- ❌ 測試配置不完整 → 遺漏某些轉換場景

## 檢查清單

- [ ] 選擇了正確的詞典檔案
- [ ] 在 `testcases.json` 新增測試案例
- [ ] 執行測試確認失敗
- [ ] 編輯詞典（使用 Tab 分隔）
- [ ] 執行排序腳本
- [ ] 執行測試確認成功
- [ ] 提交變更
