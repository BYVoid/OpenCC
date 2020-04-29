{
  "targets": [{
    "target_name": "opencc",
    "sources": [
      "../node/marisa.cc",
      "../node/opencc.cc",
    ],
    "include_dirs": [
      "../src",
      "../deps/rapidjson-1.1.0",
      "../deps/marisa-0.2.5/include",
      "../deps/marisa-0.2.5/lib",
      "<!(node -e \"require('nan')\")"
    ]
  }]
}
