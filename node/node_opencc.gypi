{
  "targets": [
    {
      "target_name": "opencc",
      "sources": [
        "../node/opencc.cc",
        "../src/BinaryDict.cpp",
        "../src/Config.cpp",
        "../src/Conversion.cpp",
        "../src/ConversionChain.cpp",
        "../src/Converter.cpp",
        "../src/Dict.cpp",
        "../src/DictConverter.cpp",
        "../src/DictEntry.cpp",
        "../src/DictGroup.cpp",
        "../src/Lexicon.cpp",
        "../src/MarisaDict.cpp",
        "../src/MaxMatchSegmentation.cpp",
        "../src/PluginSegmentation.cpp",
        "../src/PrefixMatch.cpp",
        "../src/Segmentation.cpp",
        "../src/SerializableDict.cpp",
        "../src/SerializedValues.cpp",
        "../src/TextDict.cpp",
        "../src/UTF8Util.cpp",
        "../deps/marisa-0.3.1/lib/marisa/agent.cc",
        "../deps/marisa-0.3.1/lib/marisa/grimoire/io/mapper.cc",
        "../deps/marisa-0.3.1/lib/marisa/grimoire/io/reader.cc",
        "../deps/marisa-0.3.1/lib/marisa/grimoire/io/writer.cc",
        "../deps/marisa-0.3.1/lib/marisa/grimoire/trie/louds-trie.cc",
        "../deps/marisa-0.3.1/lib/marisa/grimoire/trie/tail.cc",
        "../deps/marisa-0.3.1/lib/marisa/grimoire/vector/bit-vector.cc",
        "../deps/marisa-0.3.1/lib/marisa/keyset.cc",
        "../deps/marisa-0.3.1/lib/marisa/trie.cc"
      ],
      "include_dirs": [
        "..",
        "../node",
        "../src",
        "../deps/rapidjson-1.1.0",
        "../deps/marisa-0.3.1/include",
        "../deps/marisa-0.3.1/lib",
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "Opencc_BUILT_AS_STATIC",
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ]
    }
  ]
}
