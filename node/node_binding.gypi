{
  "targets": [{
    "target_name": "binding",
    "sources": [
      "../node/binding.cc",
      "../src/BinaryDict.cpp",
      "../src/Config.cpp",
      "../src/Conversion.cpp",
      "../src/ConversionChain.cpp",
      "../src/Converter.cpp",
      "../src/DartsDict.cpp",
      "../src/Dict.cpp",
      "../src/DictEntry.cpp",
      "../src/DictGroup.cpp",
      "../src/MaxMatchSegmentation.cpp",
      "../src/Segmentation.cpp",
      "../src/TextDict.cpp",
      "../src/UTF8Util.cpp",
    ],
    "include_dirs": [
      "../src",
      "../deps/darts-clone",
      "../deps/rapidjson-0.11",
      "<!(node -e \"require('nan')\")"
    ]
  }]
}
