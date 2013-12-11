{
  "includes": [
    "global.gypi",
  ],
  "targets": [{
    "target_name": "libopencc",
    "type": "<(library)",
    "sources": [
      "../src/Config.cpp",
      "../src/Conversion.cpp",
      "../src/ConversionChain.cpp",
      "../src/DartsDict.cpp",
      "../src/Dict.cpp",
      "../src/DictGroup.cpp",
      "../src/MaxMatchSegmentation.cpp",
      "../src/Segmentation.cpp",
      "../src/TextDict.cpp",
      "../src/UTF8Util.cpp",
    ],
    "include_dirs": [
      "../deps/darts-clone",
      "../deps/rapidjson-0.11"
    ],
    "conditions": [
      ["OS=='linux'", {
        "cflags": [
          "-fPIC"
        ]
      }]
    ]
  }]
}
