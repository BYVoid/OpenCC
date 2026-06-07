# OpenCC 配置範例

[English](README.md)

本目錄收錄供學習與自訂使用的配置範例。這些檔案不是 OpenCC 官方內建配置，
也不會出現在 `opencc --help` 的內建配置列表中。

使用時請明確傳入配置檔路徑，並用 `--path` 指向已編譯詞典檔所在目錄：

```sh
opencc -c examples/config/s2twp_taiwan_word.json --path /path/to/opencc/dictionary/
```

若使用 Bazel 建置產物：

```sh
bazel-bin/src/tools/command_line \
  -c examples/config/s2twp_taiwan_word.json \
  --path bazel-bin/data/dictionary/
```

這些範例以 `data/config/s2twp.json` 為基礎，並加入小型 inline dictionary，
示範如何在不修改官方詞典檔的情況下加入本地覆寫規則。使用者可在此基礎上酌情修改，
以適應自身轉換需求。

詞組範例也將對應 inline 條目加入 `segmentation.dict`，並放在
`STPhrases.ocd2` 之前。這樣更適合作為使用者自行修改的模板：若新增多字詞覆寫，
分詞條目可避免來源詞組在進入轉換鏈之前被切開，導致後續規則無法命中。

引號範例沒有修改 `segmentation`，因為它只轉換單字元標點符號。

分詞位置示例：

```json
"segmentation": {
  "type": "mmseg",
  "dict": {
    "type": "group",
    "dicts": [
      {
        "type": "inline",
        "entries": {
          "自訂來源詞組": "自訂來源詞組"
        }
      },
      { "type": "ocd2", "file": "STPhrases.ocd2" }
    ]
  }
}
```
