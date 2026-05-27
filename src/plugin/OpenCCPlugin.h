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

/*
 * struct_size rules:
 *
 * - Caller must initialize struct_size to sizeof(struct) before passing
 *   any structure across the ABI boundary.
 *
 * - Callee must validate struct_size is sufficient for fields it reads.
 *
 * - Future ABI-compatible extensions may append fields to the end.
 *   Implementations must ignore unknown trailing fields.
 */

#define OPENCC_SEGMENTATION_PLUGIN_ABI_MAJOR 2
#define OPENCC_SEGMENTATION_PLUGIN_ABI_MINOR 0

enum {
  OPENCC_ERROR_UNKNOWN = 1,
  OPENCC_ERROR_INVALID_ARGUMENT = 2,
  OPENCC_ERROR_PLUGIN_NOT_FOUND = 3,
  OPENCC_ERROR_PLUGIN_LOAD_FAILED = 4,
  OPENCC_ERROR_PLUGIN_SYMBOL_MISSING = 5,
  OPENCC_ERROR_PLUGIN_ABI_MISMATCH = 6,
  OPENCC_ERROR_PLUGIN_TYPE_MISMATCH = 7,
  OPENCC_ERROR_PLUGIN_DESCRIPTOR_INVALID = 8,
  OPENCC_ERROR_PLUGIN_RESOURCE_MISSING = 9,
  OPENCC_ERROR_PLUGIN_RUNTIME_FAILURE = 10,
};

typedef struct {
  size_t struct_size;
  const char* key;
  const char* value;
} opencc_kv_pair_t;

typedef struct opencc_segmentation_handle opencc_segmentation_handle_t;

typedef struct {
  size_t struct_size;
  int code;
  /*
   * On return, error->message may point either to:
   * - a static constant string, or
   * - plugin-owned dynamically allocated memory.
   * free_error(error) must release any plugin-owned resources associated with
   * the error object and leave it in a safely destructible state.
   */
  const char* message;
} opencc_error_t;

typedef struct {
  size_t struct_size;
  /*
   * One positive length per segment, measured in Unicode code points.
   * The sequence must cover the full input text in order.
   */
  uint32_t* codepoint_lengths;
  size_t segment_count;
} opencc_segment_length_array_t;

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
  opencc_segment_length_array_t* segment_lengths;
  opencc_error_t* error;
} opencc_segmentation_segment_args_t;

/*
 * ABI Layout & Alignment Rules:
 * 1. Uses natural C struct alignment (no #pragma pack).
 * 2. Field order must not be altered in future versions.
 * 3. Extensions must strictly append fields to the end.
 */
typedef struct {
  size_t struct_size;

  /*
   * Versioning semantics:
   * - abi_major:
   *   Breaking ABI changes increment this value.
   *   Host must require exact major match.
   *
   * - abi_minor:
   *   Backward-compatible ABI additions increment this value.
   *   Host may require plugin->abi_minor >= minimum_supported_minor.
   *   Host should not reject a plugin solely because plugin->abi_minor is
   *   newer.
   */
  uint16_t abi_major;
  uint16_t abi_minor;

  /*
   * Must point to static, null-terminated constant strings.
   * Host will not attempt to free() or delete[] these pointers.
   */
  const char* plugin_name;
  const char* segmentation_type;

  /*
   * [ABI CONTRACT]
   * create(args):
   * On success:
   * - returns 0
   * - *args->out must be set to a valid handle
   * - args->error->code should be 0 (or unchanged)
   * - args->error->message may be NULL
   *
   * On failure:
   * - returns non-zero
   * - *args->out must be NULL
   * - args->error must remain in a state safe for free_error()
   */
  int (*create)(opencc_segmentation_create_args_t* args);

  /*
   * [ABI CONTRACT]
   * segment(args):
   * On success:
   * - returns 0
   * - args->utf8_text is interpreted as null-terminated UTF-8 input
   * - segment_lengths contains plugin-owned segment length storage until
   *   free_segment_lengths() is called.
   * - each returned length must be > 0 and measured in Unicode code points
   * - the returned length sequence must cover the full input in order
   *
   * On failure:
   * - returns non-zero
   * - segment_lengths may be partially populated, but must remain safe for
   *   free_segment_lengths().
   */
  int (*segment)(opencc_segmentation_segment_args_t* args);

  /*
   * Cleanup Functions:
   * 1. Must gracefully handle null pointers
   *    (segment_count = 0, codepoint_lengths = null).
   * 2. Host promises to call these at most once per returned object.
   * 3. free_segment_lengths() must also accept segment length array
   *    structures that were
   *    partially initialized by segment() on failure.
   */
  void (*free_segment_lengths)(opencc_segment_length_array_t* segment_lengths);
  void (*destroy)(opencc_segmentation_handle_t* handle);
  void (*free_error)(opencc_error_t* error);
} opencc_segmentation_plugin_v2;

OPENCC_PLUGIN_EXPORT const opencc_segmentation_plugin_v2*
opencc_get_segmentation_plugin_v2(void);

#ifdef __cplusplus
}
#endif
