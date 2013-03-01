{
  "includes": [
    "gypi/global.gypi",
    "gypi/configs.gypi",
    "gypi/dicts.gypi",
  ],
  "targets": [{
    "target_name": "binding",
    "sources": [
      "node/binding.cc",
      "src/config_reader.c",
      "src/converter.c",
      "src/dictionary_group.c",
      "src/dictionary_set.c",
      "src/encoding.c",
      "src/utils.c",
      "src/opencc.c",
      "src/dictionary/abstract.c",
      "src/dictionary/datrie.c",
      "src/dictionary/text.c"
    ],
    "dependencies": [
      "configs",
      "dicts",
    ]
  }]
}
