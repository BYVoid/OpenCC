#include <string>
#include <unordered_map>

#include <emscripten/emscripten.h>
#ifdef OPENCC_WASM_WITH_OPENCC
#include "../src/opencc.h"
#endif

struct Converter {
  opencc_t oc;
  std::string out;
};

static std::unordered_map<int, Converter> converters;
static int next_id = 1;

static const char* throw_error(const char* msg) {
  emscripten_throw_string(msg);
  return msg;
}

extern "C" {

int opencc_create(const char* configPath) {
#ifdef OPENCC_WASM_WITH_OPENCC
  if (configPath == nullptr) {
    throw_error("opencc_create: null configPath");
    return -1;
  }
  opencc_t oc = opencc_open(configPath);
  if (oc == (opencc_t)-1) {
    throw_error("opencc_create: opencc_open failed");
    return -1;
  }
  int id = next_id++;
  converters.emplace(id, Converter{oc, std::string()});
  return id;
#else
  (void)configPath;
  throw_error("opencc_create: OPENCC_WASM_WITH_OPENCC not enabled");
  return -1;
#endif
}

const char* opencc_convert(int handle, const char* input) {
  static std::string err;
#ifdef OPENCC_WASM_WITH_OPENCC
  if (input == nullptr) {
    return throw_error("opencc_convert: null input");
  }
  auto it = converters.find(handle);
  if (it == converters.end()) {
    return throw_error("opencc_convert: invalid handle");
  }
  char* converted = opencc_convert_utf8(it->second.oc, input, (size_t)-1);
  if (converted != nullptr) {
    it->second.out.assign(converted);
    opencc_convert_utf8_free(converted);
    return it->second.out.c_str();
  }
  return throw_error("opencc_convert: conversion returned null");
#else
  (void)handle;
  (void)input;
  return throw_error("opencc_convert: OPENCC_WASM_WITH_OPENCC not enabled");
#endif
}

void opencc_destroy(int handle) {
#ifdef OPENCC_WASM_WITH_OPENCC
  auto it = converters.find(handle);
  if (it != converters.end()) {
    opencc_close(it->second.oc);
    converters.erase(it);
  }
#else
  (void)handle;
#endif
}

}  // extern "C"
