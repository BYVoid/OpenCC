{
  "targets": [
    {
      "target_name": "opencc",
      "sources": [
        "../node/marisa.cc",
        "../node/opencc.cc"
      ],
      "include_dirs": [
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
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ]
    }
  ]
}
