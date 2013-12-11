{
  "includes": [
    "gypi/global.gypi",
    "gypi/libopencc.gypi"
  ],
  "targets": [{
    "target_name": "opencc",
    "type": "executable",
    "sources": [
      "src/CommandLine.cpp"
    ],
    "dependencies": [
      "libopencc"
    ]
  }, {
    "target_name": "opencc_dict",
    "type": "executable",
    "sources": [
      "src/DictConverter.cpp"
    ],
    "dependencies": [
      "libopencc"
    ]
  }]
}
