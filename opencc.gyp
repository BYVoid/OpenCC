{
  "includes": [
    "gypi/global.gypi",
    "gypi/configs.gypi",
    "gypi/dicts.gypi",
  ],
  "targets": [{
    "target_name": "libopencc",
    "type": "<(library)",
    "sources": [
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
    "conditions": [
      ["OS=='linux'", {
        "cflags": [
          "-fPIC"
        ]
      }]
    ]
  }, {
    "target_name": "opencc",
    "type": "executable",
    "sources": [
      "src/tools/opencc.c"
    ],
    "dependencies": [
      "libopencc"
    ]
  }]
}
