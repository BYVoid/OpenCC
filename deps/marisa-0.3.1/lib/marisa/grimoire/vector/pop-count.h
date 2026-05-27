#ifndef MARISA_GRIMOIRE_VECTOR_POP_COUNT_H_
#define MARISA_GRIMOIRE_VECTOR_POP_COUNT_H_

#if __cplusplus >= 202002L
 #include <bit>
#endif

#include "marisa/grimoire/intrin.h"

namespace marisa::grimoire::vector {

#if defined(__cpp_lib_bitops) && __cpp_lib_bitops >= 201907L

inline std::size_t popcount(uint64_t x) {
  return static_cast<std::size_t>(std::popcount(x));
}

#else  // c++17

 #ifdef __has_builtin
  #define MARISA_HAS_BUILTIN(x) __has_builtin(x)
 #else
  #define MARISA_HAS_BUILTIN(x) 0
 #endif

 #if MARISA_WORD_SIZE == 64

inline std::size_t popcount(uint64_t x) {
  #if MARISA_HAS_BUILTIN(__builtin_popcountll)
  static_assert(sizeof(x) == sizeof(unsigned long long),
                "__builtin_popcountll does not take 64-bit arg");
  return __builtin_popcountll(x);
  #elif defined(MARISA_X64) && defined(MARISA_USE_POPCNT)
   #ifdef _MSC_VER
  return __popcnt64(x);
   #else   // _MSC_VER
  return static_cast<std::size_t>(_mm_popcnt_u64(x));
   #endif  // _MSC_VER
  #elif defined(MARISA_AARCH64)
  // Byte-wise popcount followed by horizontal add.
  return vaddv_u8(vcnt_u8(vcreate_u8(x)));
  #else   // defined(MARISA_AARCH64)
  x = (x & 0x5555555555555555ULL) + ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1);
  x = (x & 0x3333333333333333ULL) + ((x & 0xCCCCCCCCCCCCCCCCULL) >> 2);
  x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x & 0xF0F0F0F0F0F0F0F0ULL) >> 4);
  x *= 0x0101010101010101ULL;
  return x >> 56;
  #endif  // defined(MARISA_AARCH64)
}

 #else  // MARISA_WORD_SIZE == 64

inline std::size_t popcount(uint32_t x) {
  #if MARISA_HAS_BUILTIN(__builtin_popcount)
  static_assert(sizeof(x) == sizeof(unsigned int),
                "__builtin_popcount does not take 32-bit arg");
  return __builtin_popcount(x);
  #elif defined(MARISA_USE_POPCNT)
   #ifdef _MSC_VER
  return __popcnt(x);
   #else   // _MSC_VER
  return _mm_popcnt_u32(x);
   #endif  // _MSC_VER
  #else    // MARISA_USE_POPCNT
  x = (x & 0x55555555U) + ((x & 0xAAAAAAAAU) >> 1);
  x = (x & 0x33333333U) + ((x & 0xCCCCCCCCU) >> 2);
  x = (x & 0x0F0F0F0FU) + ((x & 0xF0F0F0F0U) >> 4);
  x *= 0x01010101U;
  return x >> 24;
  #endif   // MARISA_USE_POPCNT
}

 #endif  // MARISA_WORD_SIZE == 64

 #undef MARISA_HAS_BUILTIN

#endif  // c++17

}  // namespace marisa::grimoire::vector

#endif  // MARISA_GRIMOIRE_VECTOR_POP_COUNT_H_
