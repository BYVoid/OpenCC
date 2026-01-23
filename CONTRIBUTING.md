# 貢獻指南

感謝您對 OpenCC 專案的貢獻！本文件說明如何為 OpenCC 貢獻詞典條目、撰寫測試並確保程式碼品質。

## 目錄

- [新增詞典條目](#新增詞典條目)
- [排序詞典](#排序詞典)
- [執行測試](#執行測試)
- [撰寫測試案例](#撰寫測試案例)
- [簡轉繁轉換的特殊注意事項](#簡轉繁轉換的特殊注意事項)

## 新增詞典條目

### 1. 選擇正確的詞典檔案

詞典檔案位於 `data/dictionary/` 目錄下，根據轉換類型選擇對應的檔案：

- **簡繁轉換**
  - `STCharacters.txt` - 簡體到繁體（單字）
  - `STPhrases.txt` - 簡體到繁體（詞組）
  - `TSCharacters.txt` - 繁體到簡體（單字）
  - `TSPhrases.txt` - 繁體到簡體（詞組）

- **臺灣正體用詞**
  - `TWVariants.txt` - 臺灣異體字
  - `TWPhrases.txt` - 臺灣慣用詞

- **香港繁體用詞**
  - `HKVariants.txt` - 香港異體字
  - `HKVariantsRevPhrases.txt` - 香港異體字反向詞組

- **日文新舊字形**
  - `JPShinjitaiCharacters.txt` - 日文新字體（單字）
  - `JPShinjitaiPhrases.txt` - 日文新字體（詞組）
  - `JPVariants.txt` - 日文異體字

### 2. 詞典格式規範

詞典檔案使用 **Tab 字元**（`\t`）分隔來源詞與目標詞，**請勿使用空格**。

格式：`來源詞<TAB>目標詞`

範例：

```
虚伪叹息	虛偽嘆息
潮湿灶台	潮濕灶台
赞叹	讚歎
```

如果一個來源詞對應多個可能的目標詞，使用空格分隔：

```
一出	一齣 一出
```

### 3. 編輯詞典

使用文字編輯器開啟對應的 `.txt` 檔案，新增您的詞條。請確保：

1. 使用 **Tab 字元**（`\t`）分隔來源詞與目標詞
2. 每行一個條目
3. 檔案使用 UTF-8 編碼

## 排序詞典

**重要**：詞典檔案必須按字典序排序，否則測試會失敗。

### 使用排序工具

專案提供了自動排序工具，位於 `data/scripts/` 目錄：

#### 排序單一檔案

```bash
python3 data/scripts/sort.py data/dictionary/STPhrases.txt
```

這會直接排序並覆蓋原檔案。如果想輸出到其他檔案：

```bash
python3 data/scripts/sort.py data/dictionary/STPhrases.txt data/dictionary/STPhrases_sorted.txt
```

#### 排序所有詞典檔案

```bash
python3 data/scripts/sort_all.py data/dictionary
```

這會自動排序 `data/dictionary/` 目錄下所有 `.txt` 檔案。

### 排序檢查

排序是否正確會在測試時自動檢查。如果詞典未排序或包含重複的鍵，`DictionaryTest` 會報錯：

```
[ FAILED ] DictionaryTest/STPhrases.UniqueSortedTest
STPhrases is not sorted.
```

遇到此錯誤時，請執行排序工具重新排序。

## 執行測試

OpenCC 使用 [Bazel](https://bazel.build/) 作為建置系統。

### 安裝 Bazel

#### macOS

```bash
brew install bazel
```

#### Ubuntu/Debian

```bash
sudo apt install bazel
```

或參考 [Bazel 官方安裝指南](https://bazel.build/install)。

#### 其他作業系統

請參考 [Bazel 安裝文件](https://bazel.build/install) 獲取適合您系統的安裝方式。

### 執行所有測試

```bash
bazel test --test_output=all //src/... //data/... //test/... //python/...
```

### 執行特定測試

僅測試詞典：

```bash
bazel test //data/dictionary:dictionary_test
```

僅測試轉換案例：

```bash
bazel test //data/config:config_dict_validation_test
```

### 測試輸出

- `--test_output=all`：顯示所有測試輸出
- `--test_output=errors`：僅顯示失敗的測試

## 撰寫測試案例

### 測試驅動開發流程

在修改詞典前，建議先撰寫測試案例，遵循測試驅動開發（TDD）流程：

1. **先寫測試**：在 `test/testcases/testcases.json` 新增測試案例
2. **確認測試失敗**：執行測試，確認新案例因為詞典未更新而失敗
3. **修改詞典**：新增或修改詞典條目
4. **測試通過**：再次執行測試，確認修改後測試通過

這樣可以確保您的修改確實達到預期效果。

### 測試案例格式

測試案例定義於 `test/testcases/testcases.json`，格式如下：

```json
{
  "cases": [
    {
      "id": "英文簡要描述這個測試是修復或解決什麼問題的",
      "input": "輸入內容",
      "expected": {
        "s2t": "預期的簡轉繁輸出",
        "s2tw": "預期的簡轉臺灣正體輸出",
        "t2s": "預期的繁轉簡輸出",
        ...
      }
    }
  ]
}
```

### 欄位說明

- `id`：唯一的測試案例識別碼，可使用 `case_` 前綴加流水號，或 `OpenCC_Issue_1234` 等
- `input`：輸入文字
- `expected`：各種轉換配置的預期輸出
  - 僅需包含您要測試的轉換配置
  - 可以同時測試多種配置

### 可用的轉換配置

- `s2t` - 簡體到 OpenCC 標準繁體
- `s2tw` - 簡體到臺灣正體
- `s2twp` - 簡體到臺灣正體（含地域用詞轉換）
- `s2hk` - 簡體到香港繁體
- `tw2s` - OpenCC 標準臺灣正體到簡體
- `tw2sp` - 臺灣正體到簡體（含地域用詞轉換）
- `tw2t` - 臺灣正體到 OpenCC 標準繁體
- `hk2s` - 香港繁體到簡體
- `hk2t` - 香港繁體到臺灣正體
- `t2s` - OpenCC 標準繁體到簡體
- `t2tw` - OpenCC 標準繁體到臺灣正體
- `t2hk` - OpenCC 標準繁體到香港繁體
- `jp2t` - 日文新字體到舊字體
- `t2jp` - 日文舊字體到新字體

### 範例

```json
{
  "id": "case_050",
  "input": "這個軟體裡有一套軟體動物的資料庫",
  "expected": {
    "tw2sp": "这个软件里有一套软体动物的数据库"
  }
}
```

## 簡轉繁轉換的特殊注意事項

當您修改簡轉繁相關詞典時，需要特別注意不同地區的轉換配置可能都會受到影響。

### 關聯修改

如需修改 `TWPhrases.txt`，需要同時修改 `TWPhrasesRev.txt`，反之亦然。否則測試會失敗。

### 涉及的配置檔案

簡轉繁轉換主要涉及以下配置：

1. **`s2t.json`** - 基本簡轉繁
   - 使用 `STPhrases.txt` 和 `STCharacters.txt`

2. **`s2tw.json`** - 簡體轉臺灣正體
   - 使用 `STPhrases.txt`、`STCharacters.txt`
   - 額外使用 `TWVariantsPhrases.txt`、`TWVariants.txt`

3. **`s2twp.json`** - 簡體轉臺灣正體（含慣用詞）
   - 使用 `STPhrases.txt`、`STCharacters.txt`
   - 額外使用 `TWPhrases.txt`、`TWVariantsPhrases.txt`、`TWVariants.txt`

4. **`s2hk.json`** - 簡體轉香港繁體
   - 使用 `STPhrases.txt`、`STCharacters.txt`
   - 額外使用 `HKVariantsPhrases.txt`、`HKVariants.txt`

### 測試建議

修改 `STPhrases.txt` 或 `STCharacters.txt` 時，建議在 `testcases.json` 中同時測試多個相關配置：

```json
{
  "id": "case_example",
  "input": "简体文字",
  "expected": {
    "s2t": "繁體文字",
    "s2tw": "繁體文字",
    "s2twp": "臺灣慣用詞",
    "s2hk": "香港繁體"
  }
}
```

這樣可以確保您的修改在各種轉換情境下都能正確運作。

### 常見情況

- **僅修改基本簡繁對應**：修改 `STCharacters.txt`，測試至少包含 `s2t`
- **修改詞組轉換**：修改 `STPhrases.txt`，測試包含 `s2t`、`s2tw`、`s2twp`、`s2hk`
- **臺灣特有用詞**：修改 `TWPhrases*.txt` 或 `TWVariantsPhrases.txt`、`TWVariants.txt`，測試包含 `s2tw`、`s2twp`
- **香港特有用詞**：修改 `HKVariantsPhrases.txt`、`HKVariants*.txt`，測試包含 `s2hk`

## 提交變更

完成修改後，請確認：

- [ ] 詞典檔案已使用 Tab 字元分隔
- [ ] 詞典檔案已正確排序（執行 `sort.py` 或 `sort_all.py`）
- [ ] 已新增對應的測試案例到 `testcases.json`
- [ ] 修改前測試案例失敗，修改後測試通過
- [ ] 所有測試通過（`bazel test --test_output=all //src/... //data/... //test/...`）

符合以上條件後，即可提交 Pull Request。

## 需要協助？

如有任何問題，歡迎：

- 在 [GitHub Issues](https://github.com/BYVoid/OpenCC/issues) 提問
- 加入 [Telegram 討論群組](https://t.me/open_chinese_convert)

感謝您的貢獻！
