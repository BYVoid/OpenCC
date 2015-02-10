{
  "targets": [{
    "target_name": "opencc_dict",
    "type": "executable",
    "sources": [
      "../src/BinaryDict.cpp",
      "../src/DartsDict.cpp",
      "../src/Dict.cpp",
      "../src/DictEntry.cpp",
      "../src/DictGroup.cpp",
      "../src/TextDict.cpp",
      "../src/UTF8Util.cpp",
      "../src/tools/DictConverter.cpp",
    ],
    "include_dirs": [
      "../src",
      "../deps/darts-clone",
      "../deps/tclap-1.2.1"
    ]
  }]
}
