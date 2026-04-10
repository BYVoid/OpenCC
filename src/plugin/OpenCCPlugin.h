#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) && defined(OPENCC_PLUGIN_BUILD)
#define OPENCC_PLUGIN_EXPORT __declspec(dllexport)
#elif !defined(_WIN32)
#define OPENCC_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define OPENCC_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define OPENCC_SEGMENTATION_PLUGIN_ABI_MAJOR 1
#define OPENCC_SEGMENTATION_PLUGIN_ABI_MINOR 0

typedef struct {
  size_t struct_size;
  const char* key;
  const char* value;
} opencc_kv_pair_t;

typedef struct opencc_segmentation_handle opencc_segmentation_handle_t;

typedef struct {
  size_t struct_size;
  int code;
  const char* message;
} opencc_error_t;

typedef struct {
  size_t struct_size;
  const opencc_kv_pair_t* config;
  size_t config_size;
  opencc_segmentation_handle_t** out;
  opencc_error_t* error;
} opencc_segmentation_create_args_t;

typedef struct {
  size_t struct_size;
  opencc_segmentation_handle_t* handle;
  const char* utf8_text;
  char*** tokens;
  size_t* token_count;
  opencc_error_t* error;
} opencc_segmentation_segment_args_t;

typedef struct {
  size_t struct_size;
  uint16_t abi_major;
  uint16_t abi_minor;
  const char* plugin_name;
  const char* segmentation_type;
  int (*create)(opencc_segmentation_create_args_t* args);
  int (*segment)(opencc_segmentation_segment_args_t* args);
  void (*free_tokens)(char** tokens, size_t token_count);
  void (*destroy)(opencc_segmentation_handle_t* handle);
  void (*free_error)(opencc_error_t* error);
} opencc_segmentation_plugin_v1;

OPENCC_PLUGIN_EXPORT const opencc_segmentation_plugin_v1*
opencc_get_segmentation_plugin_v1(void);

#ifdef __cplusplus
}
#endif
