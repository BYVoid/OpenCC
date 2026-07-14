# 貢獻指南

感謝您對 OpenCC 專案的貢獻！本文件說明如何為 OpenCC 貢獻詞典條目、撰寫測試並確保程式碼品質。

## 授權

OpenCC 以 [Apache License 2.0](LICENSE) 釋出。提交 Pull Request、issue/comment、郵件 patch、詞典條目、測試案例或其他形式的貢獻，即表示您確認自己有權提交這些內容，並同意您的貢獻也依 Apache License 2.0 授權釋出。

## 目錄

- [授權](#授權)
- [新增詞典條目](#新增詞典條目)
- [排序詞典](#排序詞典)
- [執行測試](#執行測試)
- [撰寫測試案例](#撰寫測試案例)
- [Jieba 插件測試](#jieba-插件測試)
- [簡轉繁轉換的特殊注意事項](#簡轉繁轉換的特殊注意事項)
- [字級規則與詞表依賴](#字級規則與詞表依賴)

## 新增詞典條目

新增地區慣用詞前，請先閱讀 [OpenCC 地區詞收錄標準](doc/regional-phrase-criteria.md)。該標準說明哪些地區詞適合收錄、需要提供哪些依據，以及應如何避免高風險誤轉換。

### 1. 選擇正確的詞典檔案

詞典檔案位於 `data/dictionary/` 目錄下，根據轉換類型選擇對應的檔案：

- **簡繁轉換**
  - `STCharacters.txt` - 簡體到繁體（單字）
  - `STPhrases.txt` - 簡體到繁體（詞組）
  - `TSCharacters.txt` - 繁體到簡體（單字）
  - `TSPhrases.txt` - 繁體到簡體（詞組）

- **臺灣正體用詞**
  - `TWVariants.txt` - 臺灣異體字
  - `TWVariantsPhrases.txt` - 轉入臺灣字形（如 `s2tw`、`s2twp`、`t2tw`）時使用的臺灣異體字詞組例外
  - `TWVariantsRevPhrases.txt` - 從臺灣字形轉出（如 `tw2s`、`tw2sp`、`tw2t`）時使用的臺灣異體字詞組例外
  - `TWPhrases.txt` - 臺灣慣用詞

- **香港繁體用詞**
  - `HKVariants.txt` - 香港異體字
  - `HKVariantsPhrases.txt` - 轉入香港字形（如 `s2hk`、`t2hk`）時使用的香港異體字詞組例外
  - `HKVariantsRevPhrases.txt` - 從香港字形轉出（如 `hk2s`、`hk2t`）時使用的香港異體字詞組例外

- **日文新舊字形**（僅供探索性研究，不建議用於生產環境）
  - `JPShinjitaiCharacters.txt` - 日文新舊字體對照（單字）
  - `JPShinjitaiPhrases.txt` - 日文新字體到舊字體（詞組，亦包含少量和製漢語詞匯轉換）

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

### 4. 檢查字級規則與詞表依賴

詞組級詞表如果仍保留逐字的 `A -> B` 轉換，對應的字級詞表也必須顯式
保留 `A` 的 `B` 候選。若一般情況下不應再把 `A` 預設轉為 `B`，請不要
直接刪除字級關係；應改為非預設候選，例如 `A<TAB>A B`。這樣單字預設
仍保留為 `A`，但詞表中的 `A -> B` 依賴不會變成隱式規則。

修改 `STCharacters.txt`、`TSCharacters.txt`、`TWVariants.txt`、
`HKVariants.txt` 或其對應詞組詞表時，請同步檢查詞組中逐字對齊的
替換是否仍在字表候選中。可執行以下測試檢查這類字級依賴：

```bash
bazel test //data/dictionary:dictionary_phrase_character_dependency_test
```

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

OpenCC 同時維護 CMake、Bazel 與 Node.js 建置/測試流程。詞典與轉換行為修改通常至少需要跑 Bazel 測試；C++ CLI、插件或包裝相關修改也應跑對應的 CMake 或 npm 測試。

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

### Bazel 測試

```bash
bazel test --test_output=errors //src/... //data/... //test/... //python/...
```

如果修改了 jieba 插件或 golden 輸出，也請加入相關目標：

```bash
bazel test --test_output=errors //plugins/jieba/... //test/golden:golden_convert_test
```

常用的特定測試：

```bash
bazel test //data/dictionary/...
bazel test //data/config:config_dict_validation_test
bazel test //test:command_line_converter_test
```

### CMake 測試

本地 C++ 開發可使用 CMake：

```bash
cmake -S . -B build -DENABLE_GTEST=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

如需測試 optional jieba plugin：

```bash
cmake -S . -B build -DENABLE_GTEST=ON -DBUILD_OPENCC_JIEBA_PLUGIN=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

### Node.js 測試

Node.js binding 與 npm CLI 修改請執行（addon 以 Bazel 構建，源碼倉庫內
`npm install` 不會再觸發 node-gyp 編譯；如需舊的 node-gyp 構建，可用
`npm install --build-from-source`）：

```bash
npm install --omit=optional
./scripts/build-node-prebuild-bazel.sh
npm test
```

`build-node-prebuild-bazel.sh` 會產生 `prebuilds/<platform>-<arch>/opencc.node`
與共用資源 `prebuilds/assets/`，與 npm 發佈使用的佈局一致。如需確保測試只使用
prebuild 佈局（忽略本地 `build/Release`）：

```bash
PREBUILDS_ONLY=1 npm test
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

### 使用腳本生成測試案例

專案提供 `scripts/add_testcase.py`，可根據目前建置出的 OpenCC 與詞典，自動為
`test/testcases/testcases.json` 生成一筆測試案例。此工具適合在已完成詞典修改後，
快速補齊多個常用配置的 `expected` 輸出；提交前仍需人工檢查輸出是否符合預期。

基本用法：

```bash
python3 scripts/add_testcase.py \
  --kind Issue \
  --number 1234 \
  --brief-description taiwan_regional_phrase \
  --input "这个软件里有一套软体动物的数据库"
```

腳本預設會先執行 Bazel build，然後使用 `bazel-bin/src/tools/command_line` 與
建置出的詞典生成結果。測試案例 ID 會組成類似
`BYVoid_OpenCC_Issue_1234_taiwan_regional_phrase` 的格式。

常用參數：

- `--kind`：來源類型，使用 `Issue` 或 `PR`
- `--number`：issue 或 PR 編號
- `--brief-description`：英文簡短描述，會作為測試案例 ID 的後綴
- `--input`：測試輸入文字
- `--dry-run`：只輸出將生成的 JSON，不寫入檔案
- `--replace`：若同 ID 測試案例已存在，覆蓋原案例
- `--no-build`：跳過 Bazel build，直接使用既有 build artifacts
- `--opencc`、`--config-dir`、`--dict-dir`、`--testcases`：指定自訂執行檔、配置目錄、
  詞典目錄或測試案例檔案

例如先預覽生成內容：

```bash
python3 scripts/add_testcase.py \
  --kind PR \
  --number 5678 \
  --brief-description hk_variant_phrase \
  --input "测试文字" \
  --dry-run
```

此腳本目前生成以下配置的結果：`s2t`、`s2tw`、`s2twp`、`s2hk`、`t2s`、
`t2tw`、`t2hk`、`tw2s`、`tw2sp`、`tw2t`、`hk2s`、`hk2t`。若需要測試
`jp2t`、`t2jp`，請手動在 `expected` 中加入對應結果。

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

下列模式目前缺少大量詞組，歡迎貢獻新詞組：
- `s2hkp` - 簡體到香港繁體（含地域用詞轉換）
- `hk2sp` - 香港繁體到簡體（含地域用詞轉換）

下列模式僅供探索性研究，不建議用於生產環境：
- `jp2t` - 日文新字體到舊字體
- `t2jp` - 日文舊字體到新字體

### Golden 測試

較長文本或多配置輸出可放在 `test/golden/input/` 與 `test/golden/output/`。更新 golden 輸出時可使用：

```bash
python3 test/golden/golden_convert_test.py --update
```

提交前請檢查 golden diff，確認變更符合預期。

## Jieba 插件測試

`opencc-jieba` 是可選插件，配置位於 `plugins/jieba/data/config/`，測試案例位於：

- `plugins/jieba/tests/data/jieba_comparison_testcases.json`
- `test/golden/` 中的 `_jieba` 配置輸出

修改 jieba 分詞、插件載入、或 `*_jieba.json` 配置時，請至少執行：

```bash
bazel test --test_output=errors //plugins/jieba/... //test/golden:golden_convert_test
```

npm 插件包位於 `plugins/jieba/node/`。若修改 npm integration，請同時安裝測試 `opencc` 與 `opencc-jieba`，確認 `s2twp_jieba` 等模式可由 JavaScript API 與 npm CLI 載入。

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

`TWVariantsPhrases.txt` 與 `TWVariantsRevPhrases.txt`、`HKVariantsPhrases.txt` 與
`HKVariantsRevPhrases.txt` 則不是 `TWPhrases.txt` / `TWPhrasesRev.txt` 這種
逐條互逆的詞彙對照表。`*VariantsPhrases.txt` 是正向地區異體字轉換的詞組級例外，
當詞組命中時，該片段會按詞組條目轉換，不再受 `TWVariants.txt` 或 `HKVariants.txt`
的字級映射影響；`*VariantsRevPhrases.txt` 是反向異體字轉換的詞組級例外，
命中後同樣不再受產生的 `TWVariantsRev.txt` 或 `HKVariantsRev.txt` 的字級映射影響。
兩者有關聯，但應依各自轉換方向的需要維護，不要求一組詞反向後必須出現在另一個檔案中。

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

5. **`s2hkp.json`** - 簡體轉香港繁體（含慣用詞）
   - 使用 `STPhrases.txt`、`STCharacters.txt`
   - 額外使用 `HKPhrases.txt`、`HKVariantsPhrases.txt`、`HKVariants.txt`

### 地區慣用詞與簡繁階段

`TWPhrases.txt` / `HKPhrases.txt` 第一欄會自動生成
`STPhrases_GeneratedFromRegionalPhrases`，用來降低 `STPhrases.txt` 的維護負擔。
`s2t`、`s2tw`、`s2hk`、`s2twp`、`s2hkp` 等標準簡轉繁配置會在分詞階段和第一段
簡繁轉換鏈使用這份生成詞典。分詞階段可避免簡體輸入在地區詞彙階段前被錯誤切分；
第一段轉換鏈則可把 `t2s(地區詞表第一欄)` 投射回地區詞表 key，避免整個詞因分詞
命中而保留為簡體。

`TWPhrases.txt` / `HKPhrases.txt` 的第一欄可能是地區詞彙轉換前的另一種繁體形式，
不一定總是 OpenCC Standard Traditional 的最佳輸出。因此，如果生成結果不應成為
普通 `s2t` / `s2tw` / `s2hk` 的輸出，應在 `STPhrases.txt` 明確加入普通簡繁需要的
條目或測試來固定預期，而不是只依賴地區詞表 key。

因此，新增臺灣或香港慣用詞時，通常只需維護 `TWPhrases.txt` / `TWPhrasesRev.txt`
或 `HKPhrases.txt` / `HKPhrasesRev.txt`。當 `t2s(地區詞表第一欄)` 與實際簡體
輸入不一致，或普通簡繁輸出需要不同於地區詞表第一欄時，才需要額外在
`STPhrases.txt` 補簡繁階段的條目。

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
- [ ] 所有相關 Bazel/CMake/npm 測試通過
- [ ] 如修改 release packaging，已檢查相關腳本（例如 `scripts/release-windows-winget.ps1`、npm prebuild/package 腳本）

符合以上條件後，即可提交 Pull Request。

## 需要協助？

如有任何問題，歡迎：

- 在 [GitHub Issues](https://github.com/BYVoid/OpenCC/issues) 提問
- 加入 [Telegram 討論群組](https://t.me/open_chinese_convert)

感謝您的貢獻！
