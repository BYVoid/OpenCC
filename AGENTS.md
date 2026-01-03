# OpenCC 專案速覽

本文檔彙整目前代理掌握的 Open Chinese Convert（OpenCC）專案資訊，協助快速熟悉程式碼結構、資料組織與配套工具。

## 專案概述
- OpenCC 是一套開源的中文簡繁體與地區用詞轉換工具，支援簡↔繁、港澳臺差異、日文新舊字形等多種轉換方案。
- 專案同時提供 C++ 核心程式庫、C 語言介面、命令列工具，以及 Python、Node.js 等語言綁定，詞庫與程式解耦，方便自訂與擴充。
- 主要相依：`rapidjson` 解析設定，`marisa-trie` 處理高效能詞典（`.ocd2`），可選 `Darts` 以支援舊版 `.ocd`。

## 核心模組與流程
1. **設定載入 (`src/Config.cpp`)**
   - 讀取 JSON 設定（位於 `data/config/*.json`），解析分詞器定義與轉換鏈。
   - 依 `type` 欄位載入不同格式的詞典（純文字、`ocd2`、詞典組），並支援附加搜尋路徑。
   - 建立 `Converter` 物件，持有分詞器與轉換鏈。

2. **分詞 (`src/MaxMatchSegmentation.cpp`)**
   - 預設分詞型態為 `mmseg`，即最大正向匹配。
   - 以詞典做最長前綴匹配，將輸入切成 `Segments`；無法匹配的 UTF-8 片段依字元長度保留。

3. **轉換鏈 (`src/ConversionChain.cpp`, `src/Conversion.cpp`)**
   - 轉換鏈是有序的 `Conversion` 清單，每個節點依賴一個詞典，透過最長前綴匹配把片段替換為目標值。
   - 支援詞組優先、異體字替換、多階段組合等進階情境。

4. **詞典系統**
   - 抽象介面 `Dict` 統一前綴匹配、全前綴匹配與詞典遍歷。
   - `TextDict` (`.txt`) 由制表符純文字建構詞典；`MarisaDict` (`.ocd2`) 提供高效能字典樹；`DictGroup` 可將多個詞典依序組成集合。
   - `SerializableDict` 定義序列化與檔案載入邏輯，命令列工具據此在不同格式間互轉。

5. **API 封裝**
   - `SimpleConverter`（高階 C++ 介面）封裝 `Config + Converter`，提供字串、指標緩衝、部分長度轉換等多種多載。
   - `opencc.h` 暴露 C API：`opencc_open`、`opencc_convert_utf8` 等，供語言綁定與命令列重用。
   - 命令列程式 `opencc`（`src/tools/CommandLine.cpp`）示範批次轉換、串流讀取、自動刷新與同檔案輸入輸出處理。

## 資料與設定
- 詞庫維護在 `data/dictionary/*.txt`，涵蓋短語、單字、地區差異、日文新字等專題檔；建置時轉成 `.ocd2` 加速。
- 預設設定位於 `data/config/`，如 `s2t.json`、`t2s.json`、`s2tw.json` 等，定義分詞器型態、使用的詞典與組合方式。
- `data/scheme` 與 `data/scripts` 提供詞庫編譯腳本與規範校驗工具。

### 詞典二進位格式：`.ocd` 與 `.ocd2`
- `.ocd`（舊格式）以 `OPENCCDARTS1` 為檔頭，主體為 Darts double-array trie 的序列化資料，搭配 `BinaryDict` 結構保存鍵值偏移與拼接緩衝，載入流程見 `src/DartsDict.cpp` 與 `src/BinaryDict.cpp`。常用於需要 `ENABLE_DARTS` 的相容環境。
- `.ocd2`（預設格式）以 `OPENCC_MARISA_0.2.5` 為檔頭，接著寫入 `marisa::Trie` 資料，然後用 `SerializedValues` 模組保存所有候選值列表，詳見 `src/MarisaDict.cpp`、`src/SerializedValues.cpp`。此格式體積更小、載入更快（例如 `NEWS.md` 記錄 `STPhrases` 從 4.3MB 縮減至 924KB）。
- 命令列工具 `opencc_dict` 支援 `text ↔ ocd2`（以及可選 `ocd`）互轉，新增或調整詞典時先編輯 `.txt`，再執行工具產生目標格式。

## 開發與測試
- 頂層建置系統支援 CMake、Bazel、Node.js 的 `binding.gyp`、Python `pyproject.toml`，跨平台整合 CI。
- `src/*Test.cpp`、`test/` 目錄包含 Google Test 風格的單元測試，涵蓋詞典匹配、轉換鏈、分詞等關鍵邏輯。
- 工具 `opencc_dict`、`opencc_phrase_extract`（`src/tools/`）協助開發者轉換詞庫格式、抽取短語。

## 生態綁定
- Python 模組位於 `python/`，透過 C API 提供 `OpenCC` 類別。
- Node.js 擴充在 `node/` 目錄，使用 N-API/Node-API 呼叫核心程式庫。
- README 列出第三方 Swift、Java、Go、WebAssembly 等移植專案，展示生態廣度。

## 常見自訂步驟
1. 編輯或新增 `data/dictionary/*.txt` 詞條。
2. 使用 `opencc_dict` 轉換為 `.ocd2`。
3. 在 `data/config` 複製／修改設定 JSON 並指定新的詞典檔案。
4. 透過 `SimpleConverter`、命令列工具或語言綁定載入自訂設定驗證效果。

> 若需更深入，可閱讀 `src/README.md` 的模組說明，或參考 `test/` 下的案例理解轉換鏈組合。

## 瀏覽器與第三方實作注意事項
- 官方未直接支援純前端執行，社群方案（如 `opencc-js`、`wasm-opencc`）可供參考。
- 若自行編譯 WebAssembly，可用 Emscripten 將 `.ocd2` 寫入虛擬檔案系統，在 Web Worker 中呼叫轉換以避免阻塞 UI，並搭配 gzip/brotli 與 Service Worker 快取降低首次載入成本。
- 純 JavaScript 查表可先將詞典預處理為 JSON/Trie 結構，手寫最長前綴匹配；請留意控制資源體積，並在轉換長文本時避免多餘字串拷貝。

### 第三方實作常見偏差（推測）
- **缺少分詞與轉換鏈順序**：若未還原 `group` 設定或詞典優先級，複合詞可能被拆開或被單字覆蓋。
- **最長前綴邏輯缺失**：只按字元替換會遺漏成語、多字詞結果。
- **UTF-8 處理不嚴謹**：疏漏多位元組字元或 surrogate pair 處理，容易位移或截斷。
- **詞典／設定不完整**：缺少分詞詞典、地區差異等 `.ocd2`，輸出會缺詞。
- **路徑與載入流程差異**：若未遵循 OpenCC 的路徑搜尋與設定解析細節，實際載入的資源與官方不同，結果自然偏差。

## 延伸閱讀

### 技術文件
- **[演算法與理論局限性分析](doc/ALGORITHM_AND_LIMITATIONS.md)** - 深入探討 OpenCC 的核心演算法（最大正向匹配分詞）、轉換鏈機制、詞典系統，以及在中文簡繁轉換中面臨的理論局限性（一對多歧義、缺乏上下文理解、維護負擔等）。

### 貢獻指南
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - 如何為 OpenCC 貢獻詞典條目、撰寫測試案例、執行測試流程的完整說明。

### 專案文件
- **[src/README.md](src/README.md)** - 核心模組的詳細技術說明。
- **[README.md](README.md)** - 專案總覽、安裝與使用指南。
