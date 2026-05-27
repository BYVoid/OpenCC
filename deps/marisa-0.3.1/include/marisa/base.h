#ifndef MARISA_BASE_H_
#define MARISA_BASE_H_

#include <cstddef>
#include <cstdint>
#include <exception>
#include <system_error>
#include <utility>

// These aliases are left for backward compatibility.
using marisa_uint8 [[deprecated]] = std::uint8_t;
using marisa_uint16 [[deprecated]] = std::uint16_t;
using marisa_uint32 [[deprecated]] = std::uint32_t;
using marisa_uint64 [[deprecated]] = std::uint64_t;

#if UINTPTR_MAX == UINT64_MAX
 #define MARISA_WORD_SIZE 64
#elif UINTPTR_MAX == UINT32_MAX
 #define MARISA_WORD_SIZE 32
#else
 #error Failed to detect MARISA_WORD_SIZE
#endif

// These constant variables are left for backward compatibility.
[[deprecated]] constexpr auto MARISA_UINT8_MAX = UINT8_MAX;
[[deprecated]] constexpr auto MARISA_UINT16_MAX = UINT16_MAX;
[[deprecated]] constexpr auto MARISA_UINT32_MAX = UINT32_MAX;
[[deprecated]] constexpr auto MARISA_UINT64_MAX = UINT64_MAX;
[[deprecated]] constexpr auto MARISA_SIZE_MAX = SIZE_MAX;

#define MARISA_INVALID_LINK_ID UINT32_MAX
#define MARISA_INVALID_KEY_ID  UINT32_MAX
#define MARISA_INVALID_EXTRA   (UINT32_MAX >> 8)

// Error codes are defined as members of marisa_error_code. This library throws
// an exception with one of the error codes when an error occurs.
enum marisa_error_code {
  // MARISA_OK means that a requested operation has succeeded. In practice, an
  // exception never has MARISA_OK because it is not an error.
  MARISA_OK = 0,

  // MARISA_STATE_ERROR means that an object was not ready for a requested
  // operation. For example, an operation to modify a fixed vector throws an
  // exception with MARISA_STATE_ERROR.
  MARISA_STATE_ERROR = 1,

  // MARISA_NULL_ERROR means that an invalid nullptr has been given.
  MARISA_NULL_ERROR = 2,

  // MARISA_BOUND_ERROR means that an operation has tried to access an out of
  // range address.
  MARISA_BOUND_ERROR = 3,

  // MARISA_RANGE_ERROR means that an out of range value has appeared in
  // operation.
  MARISA_RANGE_ERROR = 4,

  // MARISA_CODE_ERROR means that an undefined code has appeared in operation.
  MARISA_CODE_ERROR = 5,

  // MARISA_RESET_ERROR means that a smart pointer has tried to reset itself.
  MARISA_RESET_ERROR = 6,

  // MARISA_SIZE_ERROR means that a size has exceeded a library limitation.
  MARISA_SIZE_ERROR = 7,

  // MARISA_MEMORY_ERROR means that a memory allocation has failed.
  MARISA_MEMORY_ERROR = 8,

  // MARISA_IO_ERROR means that an I/O operation has failed.
  MARISA_IO_ERROR = 9,

  // MARISA_FORMAT_ERROR means that input was in invalid format.
  MARISA_FORMAT_ERROR = 10,
};

// Flags for memory mapping are defined as members of marisa_map_flags.
// Trie::open() accepts a combination of these flags.
enum marisa_map_flags {
  // MARISA_MAP_POPULATE specifies MAP_POPULATE.
  MARISA_MAP_POPULATE = 1 << 0,
};

// Min/max values, flags and masks for dictionary settings are defined below.
// Please note that unspecified settings will be replaced with the default
// settings. For example, 0 is equivalent to (MARISA_DEFAULT_NUM_TRIES |
// MARISA_DEFAULT_TRIE | MARISA_DEFAULT_TAIL | MARISA_DEFAULT_ORDER).

// A dictionary consists of 3 tries in default. Usually more tries make a
// dictionary space-efficient but time-inefficient.
enum marisa_num_tries {
  MARISA_MIN_NUM_TRIES = 0x00001,
  MARISA_MAX_NUM_TRIES = 0x0007F,
  MARISA_DEFAULT_NUM_TRIES = 0x00003,
};

// This library uses a cache technique to accelerate search functions. The
// following enumerated type marisa_cache_level gives a list of available cache
// size options. A larger cache enables faster search but takes a more space.
enum marisa_cache_level {
  MARISA_HUGE_CACHE = 0x00080,
  MARISA_LARGE_CACHE = 0x00100,
  MARISA_NORMAL_CACHE = 0x00200,
  MARISA_SMALL_CACHE = 0x00400,
  MARISA_TINY_CACHE = 0x00800,
  MARISA_DEFAULT_CACHE = MARISA_NORMAL_CACHE
};

// This library provides 2 kinds of TAIL implementations.
enum marisa_tail_mode {
  // MARISA_TEXT_TAIL merges last labels as zero-terminated strings. So, it is
  // available if and only if the last labels do not contain a NULL character.
  // If MARISA_TEXT_TAIL is specified and a NULL character exists in the last
  // labels, the setting is automatically switched to MARISA_BINARY_TAIL.
  MARISA_TEXT_TAIL = 0x01000,

  // MARISA_BINARY_TAIL also merges last labels but as byte sequences. It uses
  // a bit vector to detect the end of a sequence, instead of NULL characters.
  // So, MARISA_BINARY_TAIL requires a larger space if the average length of
  // labels is greater than 8.
  MARISA_BINARY_TAIL = 0x02000,

  MARISA_DEFAULT_TAIL = MARISA_TEXT_TAIL,
};

// The arrangement of nodes affects the time cost of matching and the order of
// predictive search.
enum marisa_node_order {
  // MARISA_LABEL_ORDER arranges nodes in ascending label order.
  // MARISA_LABEL_ORDER is useful if an application needs to predict keys in
  // label order.
  MARISA_LABEL_ORDER = 0x10000,

  // MARISA_WEIGHT_ORDER arranges nodes in descending weight order.
  // MARISA_WEIGHT_ORDER is generally a better choice because it enables faster
  // matching.
  MARISA_WEIGHT_ORDER = 0x20000,

  MARISA_DEFAULT_ORDER = MARISA_WEIGHT_ORDER,
};

enum marisa_config_mask {
  MARISA_NUM_TRIES_MASK = 0x0007F,
  MARISA_CACHE_LEVEL_MASK = 0x00F80,
  MARISA_TAIL_MODE_MASK = 0x0F000,
  MARISA_NODE_ORDER_MASK = 0xF0000,
  MARISA_CONFIG_MASK = 0xFFFFF
};

namespace marisa {

// These aliases are left for backward compatibility.
using UInt8 [[deprecated]] = std::uint8_t;
using UInt16 [[deprecated]] = std::uint16_t;
using UInt32 [[deprecated]] = std::uint32_t;
using UInt64 [[deprecated]] = std::uint64_t;

using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;

using ErrorCode = marisa_error_code;
using CacheLevel = marisa_cache_level;
using TailMode = marisa_tail_mode;
using NodeOrder = marisa_node_order;

// This is left for backward compatibility.
using std::swap;

// This is left for backward compatibility.
using Exception = std::exception;

}  // namespace marisa

// These macros are used to convert a line number to a string constant.
#define MARISA_INT_TO_STR(value) #value
#define MARISA_LINE_TO_STR(line) MARISA_INT_TO_STR(line)
#define MARISA_LINE_STR          MARISA_LINE_TO_STR(__LINE__)

// MARISA_THROW throws an exception with a filename, a line number, an error
// code and an error message. The message format is as follows:
//  "__FILE__:__LINE__: error_code: error_message"
#define MARISA_THROW(error_type, error_message)                     \
  (throw (error_type)(__FILE__ ":" MARISA_LINE_STR ": " #error_type \
                               ": " error_message))

// MARISA_THROW_IF throws an exception if `condition' is true.
#define MARISA_THROW_IF(condition, error_type) \
  (void)((!(condition)) || (MARISA_THROW(error_type, #condition), 0))

// MARISA_THROW_SYSTEM_ERROR_IF throws an exception if `condition` is true.
// ::GetLastError() or errno should be passed as `error_value`.
#define MARISA_THROW_SYSTEM_ERROR_IF(condition, error_value, error_category,   \
                                     function_name)                            \
  (void)((!(condition)) ||                                                     \
         (throw std::system_error(                                             \
              std::error_code(error_value, error_category),                    \
              __FILE__ ":" MARISA_LINE_STR                                     \
                       ": std::system_error: " function_name ": " #condition), \
          false))

// #ifndef MARISA_USE_EXCEPTIONS
//  #if defined(__GNUC__) && !defined(__EXCEPTIONS)
//   #define MARISA_USE_EXCEPTIONS 0
//  #elif defined(__clang__) && !defined(__cpp_exceptions)
//   #define MARISA_USE_EXCEPTIONS 0
//  #elif defined(_MSC_VER) && !_HAS_EXCEPTIONS
//   #define MARISA_USE_EXCEPTIONS 0
//  #else
//   #define MARISA_USE_EXCEPTIONS 1
//  #endif
// #endif

// #if MARISA_USE_EXCEPTIONS
//  #define MARISA_TRY      try
//  #define MARISA_CATCH(x) catch (x)
// #else
//  #define MARISA_TRY      if (true)
//  #define MARISA_CATCH(x) if (false)
// #endif

#endif  // MARISA_BASE_H_
