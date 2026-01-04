---
name: opencc-fix-translation-workflow
description: OpenCC 翻譯修正與完整發布流程（含 WASM 同步）
tags: [opencc, workflow, debugging, wasm]
---

# OpenCC 翻譯修正標準作業流程

此技能描述修復 OpenCC 轉換錯誤（如「方程式」變「方程序」）的完整生命週期，包含核心詞典修正、測試、驗證及 WASM 函式庫同步。

## 1. 問題診斷

當發現轉換錯誤（例如 A 被錯誤轉換為 B）時：

1.  **搜尋現有映射**：
    使用 `grep` 在 `data/dictionary` 中搜尋錯誤來源。
    ```bash
    grep "錯誤詞" data/dictionary/*.txt
    ```
2.  **確認干擾源**：
    通常是因為長度優先匹配（MaxMatch）中的「長詞」包含了目標詞，或者「短詞」映射導致了錯誤結果。
    *範例*：「方程式」被錯誤轉換為「方程序」，是因為「程式」->「程序」的映射存在，且「方程式」未被定義為專有名詞，導致被切分為「方」+「程式」。

## 2. 修正方案（Explicit Mapping）

若錯誤源自於分詞邏輯（如上述範例），最穩健的修復方式是**新增顯式映射（Explicit Mapping）**。

1.  **選擇正確的詞典檔**：
    - IT 術語：`TWPhrasesIT.txt`
    - 一般詞彙/其他：`TWPhrasesOther.txt`
    - 人名/地名：`TWPhrasesName.txt`
2.  **新增映射**：
    將詞彙映射到其自身，以防止被錯誤切割或轉換。
    ```text
    方程式	方程式
    ```
    *注意*：保持詞典字母排序（若有的話）。

## 3. 測試驅動（Test Cases）

在修改生效前，先建立測試案例以確保修復並防止回歸。

1.  **核心測試**：
    編輯 `test/testcases/testcases.json`。
    ```json
    {
      "id": "case_XXX",
      "input": "方程式",
      "expected": {
        "tw2sp": "方程式"
      }
    }
    ```

## 4. 建置與驗證

OpenCC 使用 CMake/Make 系統建置詞典。

1.  **重建詞典**：
    ```bash
    cd build/dbg  # 或你的建置目錄
    make Dictionaries
    ```
    此步驟會重新生成 `.ocd2` 二進位詞典。

2.  **手動驗證**：
    使用生成的 `opencc` 工具直接測試。
    ```bash
    echo "方程式" | ./src/tools/opencc -c root/share/opencc/tw2sp.json
    # 預期輸出：方程式
    ```

3.  **自動化測試**（可選但推薦）：
    執行 `make test` 或 `ctest`。

## 5. WASM 函式庫同步（自動化）

專案提供腳本自動同步核心詞典與測試案例到 `wasm-lib`。

1.  **執行同步腳本**：
    此腳本會呼叫 Bazel 建置詞典，並複製 `.ocd2` 與 `testcases.json` 到正確位置。
    ```bash
    ./wasm-lib/scripts/refresh_assets.sh
    ```

2.  **檢查並提交**：
    同步後檢查 `wasm-lib` 的變更狀態。
    ```bash
    git status wasm-lib
    # 預期看到 .ocd2 與 testcases.json 的變更
    ```


## 6. 提交 (Commit)

提交時建議明確分開或合併，但必須包含：
- 詞典文字檔變更 (`.txt`)
- 核心測試變更 (`test/testcases/testcases.json`)
- WASM 同步變更 (`wasm-lib/`)

```bash
git add data/dictionary/TWPhrasesOther.txt test/testcases/testcases.json
git add wasm-lib
git commit -m "Fix(Dictionary): correct conversion for 'XYZ' and sync wasm"
```
