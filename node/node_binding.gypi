{
  "targets": [{
    "target_name": "binding",
    "sources": [
      "../node/binding.cc",
    ],
    "include_dirs": [
      "../src",
      "../deps/darts-clone",
      "../deps/rapidjson-0.11",
      "<!(node -e \"require('nan')\")"
    ]
  }]
}
