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
      "src/dict_group.c",
      "src/dict_chain.c",
      "src/encoding.c",
      "src/utils.c",
      "src/opencc.c",
      "src/dict.c",
      "src/dictionary/datrie.c",
      "src/dictionary/text.c"
    ],
    "dependencies": [
      "configs",
      "dicts",
    ]
  }]
}
