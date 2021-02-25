#ifndef MARISA_BASE_H_
#define MARISA_BASE_H_

// Old Visual C++ does not provide stdint.h.
#ifndef _MSC_VER
 #include <stdint.h>
#endif  // _MSC_VER

#ifdef __cplusplus
 #include <cstddef>
#else  // __cplusplus
 #include <stddef.h>
#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#ifdef _MSC_VER
typedef unsigned __int8  marisa_uint8;
typedef unsigned __int16 marisa_uint16;
typedef unsigned __int32 marisa_uint32;
typedef unsigned __int64 marisa_uint64;
#else  // _MSC_VER
typedef uint8_t  marisa_uint8;
typedef uint16_t marisa_uint16;
typedef uint32_t marisa_uint32;
typedef uint64_t marisa_uint64;
#endif  // _MSC_VER

#if defined(_WIN64) || defined(__amd64__) || defined(__x86_64__) || \
    defined(__ia64__) || defined(__ppc64__) || defined(__powerpc64__) || \
    defined(__sparc64__) || defined(__mips64__) || defined(__aarch64__) || \
    defined(__s390x__)
 #define MARISA_WORD_SIZE 64
#else  // defined(_WIN64), etc.
 #define MARISA_WORD_SIZE 32
#endif  // defined(_WIN64), etc.

//#define MARISA_WORD_SIZE  (sizeof(void *) * 8)

#define MARISA_UINT8_MAX  ((marisa_uint8)~(marisa_uint8)0)
#define MARISA_UINT16_MAX ((marisa_uint16)~(marisa_uint16)0)
#define MARISA_UINT32_MAX ((marisa_uint32)~(marisa_uint32)0)
#define MARISA_UINT64_MAX ((marisa_uint64)~(marisa_uint64)0)
#define MARISA_SIZE_MAX   ((size_t)~(size_t)0)

#define MARISA_INVALID_LINK_ID MARISA_UINT32_MAX
#define MARISA_INVALID_KEY_ID  MARISA_UINT32_MAX
#define MARISA_INVALID_EXTRA   (MARISA_UINT32_MAX >> 8)

// Error codes are defined as members of marisa_error_code. This library throws
// an exception with one of the error codes when an error occurs.
typedef enum marisa_error_code_ {
  // MARISA_OK means that a requested operation has succeeded. In practice, an
  // exception never has MARISA_OK because it is not an error.
  MARISA_OK           = 0,

  // MARISA_STATE_ERROR means that an object was not ready for a requested
  // operation. For example, an operation to modify a fixed vector throws an
  // exception with MARISA_STATE_ERROR.
  MARISA_STATE_ERROR  = 1,

  // MARISA_NULL_ERROR means that an invalid NULL pointer has been given.
  MARISA_NULL_ERROR   = 2,

  // MARISA_BOUND_ERROR means that an operation has tried to access an out of
  // range address.
  MARISA_BOUND_ERROR  = 3,

  // MARISA_RANGE_ERROR means that an out of range value has appeared in
  // operation.
  MARISA_RANGE_ERROR  = 4,

  // MARISA_CODE_ERROR means that an undefined code has appeared in operation.
  MARISA_CODE_ERROR   = 5,

  // MARISA_RESET_ERROR means that a smart pointer has tried to reset itself.
  MARISA_RESET_ERROR  = 6,

  // MARISA_SIZE_ERROR means that a size has exceeded a library limitation.
  MARISA_SIZE_ERROR   = 7,

  // MARISA_MEMORY_ERROR means that a memory allocation has failed.
  MARISA_MEMORY_ERROR = 8,

  // MARISA_IO_ERROR means that an I/O operation has failed.
  MARISA_IO_ERROR     = 9,

  // MARISA_FORMAT_ERROR means that input was in invalid format.
  MARISA_FORMAT_ERROR = 10,
} marisa_error_code;

// Min/max values, flags and masks for dictionary settings are defined below.
// Please note that unspecified settings will be replaced with the default
// settings. For example, 0 is equivalent to (MARISA_DEFAULT_NUM_TRIES |
// MARISA_DEFAULT_TRIE | MARISA_DEFAULT_TAIL | MARISA_DEFAULT_ORDER).

// A dictionary consists of 3 tries in default. Usually more tries make a
// dictionary space-efficient but time-inefficient.
typedef enum marisa_num_tries_ {
  MARISA_MIN_NUM_TRIES     = 0x00001,
  MARISA_MAX_NUM_TRIES     = 0x0007F,
  MARISA_DEFAULT_NUM_TRIES = 0x00003,
} marisa_num_tries;

// This library uses a cache technique to accelerate search functions. The
// following enumerated type marisa_cache_level gives a list of available cache
// size options. A larger cache enables faster search but takes a more space.
typedef enum marisa_cache_level_ {
  MARISA_HUGE_CACHE        = 0x00080,
  MARISA_LARGE_CACHE       = 0x00100,
  MARISA_NORMAL_CACHE      = 0x00200,
  MARISA_SMALL_CACHE       = 0x00400,
  MARISA_TINY_CACHE        = 0x00800,
  MARISA_DEFAULT_CACHE     = MARISA_NORMAL_CACHE
} marisa_cache_level;

// This library provides 2 kinds of TAIL implementations.
typedef enum marisa_tail_mode_ {
  // MARISA_TEXT_TAIL merges last labels as zero-terminated strings. So, it is
  // available if and only if the last labels do not contain a NULL character.
  // If MARISA_TEXT_TAIL is specified and a NULL character exists in the last
  // labels, the setting is automatically switched to MARISA_BINARY_TAIL.
  MARISA_TEXT_TAIL         = 0x01000,

  // MARISA_BINARY_TAIL also merges last labels but as byte sequences. It uses
  // a bit vector to detect the end of a sequence, instead of NULL characters.
  // So, MARISA_BINARY_TAIL requires a larger space if the average length of
  // labels is greater than 8.
  MARISA_BINARY_TAIL       = 0x02000,

  MARISA_DEFAULT_TAIL      = MARISA_TEXT_TAIL,
} marisa_tail_mode;

// The arrangement of nodes affects the time cost of matching and the order of
// predictive search.
typedef enum marisa_node_order_ {
  // MARISA_LABEL_ORDER arranges nodes in ascending label order.
  // MARISA_LABEL_ORDER is useful if an application needs to predict keys in
  // label order.
  MARISA_LABEL_ORDER       = 0x10000,

  // MARISA_WEIGHT_ORDER arranges nodes in descending weight order.
  // MARISA_WEIGHT_ORDER is generally a better choice because it enables faster
  // matching.
  MARISA_WEIGHT_ORDER      = 0x20000,

  MARISA_DEFAULT_ORDER     = MARISA_WEIGHT_ORDER,
} marisa_node_order;

typedef enum marisa_config_mask_ {
  MARISA_NUM_TRIES_MASK    = 0x0007F,
  MARISA_CACHE_LEVEL_MASK  = 0x00F80,
  MARISA_TAIL_MODE_MASK    = 0x0F000,
  MARISA_NODE_ORDER_MASK   = 0xF0000,
  MARISA_CONFIG_MASK       = 0xFFFFF
} marisa_config_mask;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#ifdef __cplusplus

// `std::swap` is in <utility> since C++ 11 but in <algorithm> in C++ 98:
#if __cplusplus >= 201103L
 #include <utility>
#else
 #include <algorithm>
#endif
namespace marisa {

typedef ::marisa_uint8  UInt8;
typedef ::marisa_uint16 UInt16;
typedef ::marisa_uint32 UInt32;
typedef ::marisa_uint64 UInt64;

typedef ::marisa_error_code ErrorCode;

typedef ::marisa_cache_level CacheLevel;
typedef ::marisa_tail_mode TailMode;
typedef ::marisa_node_order NodeOrder;

using std::swap;

}  // namespace marisa
#endif  // __cplusplus

#ifdef __cplusplus
 #include "marisa/exception.h"
 #include "marisa/scoped-ptr.h"
 #include "marisa/scoped-array.h"
#endif  // __cplusplus

#endif  // MARISA_BASE_H_
