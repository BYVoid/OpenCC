#include "marisa/grimoire/vector/bit-vector.h"

#include <algorithm>
#if __cplusplus >= 202002L
 #include <bit>
#endif
#include <cassert>

#include "marisa/grimoire/vector/pop-count.h"

namespace marisa::grimoire::vector {
namespace {

#if defined(__cpp_lib_bitops) && __cpp_lib_bitops >= 201907L

inline std::size_t countr_zero(uint64_t x) {
  return static_cast<std::size_t>(std::countr_zero(x));
}

#else  // c++17

inline std::size_t countr_zero(uint64_t x) {
 #ifdef _MSC_VER
  unsigned long pos;
  ::_BitScanForward64(&pos, x);
  return pos;
 #else   // _MSC_VER
  return __builtin_ctzll(x);
 #endif  // _MSC_VER
}

#endif  // c++17

#ifdef MARISA_USE_BMI2
inline std::size_t select_bit(std::size_t i, std::size_t bit_id,
                              uint64_t unit) {
  return bit_id + countr_zero(_pdep_u64(1ULL << i, unit));
}
#else  // MARISA_USE_BMI2
// clang-format off
const uint8_t SELECT_TABLE[8][256] = {
  {
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
  },
  {
    7, 7, 7, 1, 7, 2, 2, 1, 7, 3, 3, 1, 3, 2, 2, 1,
    7, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1,
    5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 6, 6, 1, 6, 2, 2, 1, 6, 3, 3, 1, 3, 2, 2, 1,
    6, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    6, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1,
    5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 7, 7, 1, 7, 2, 2, 1, 7, 3, 3, 1, 3, 2, 2, 1,
    7, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1,
    5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 6, 6, 1, 6, 2, 2, 1, 6, 3, 3, 1, 3, 2, 2, 1,
    6, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    6, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1,
    5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1
  },
  {
    7, 7, 7, 7, 7, 7, 7, 2, 7, 7, 7, 3, 7, 3, 3, 2,
    7, 7, 7, 4, 7, 4, 4, 2, 7, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 5, 7, 5, 5, 2, 7, 5, 5, 3, 5, 3, 3, 2,
    7, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 6, 7, 6, 6, 2, 7, 6, 6, 3, 6, 3, 3, 2,
    7, 6, 6, 4, 6, 4, 4, 2, 6, 4, 4, 3, 4, 3, 3, 2,
    7, 6, 6, 5, 6, 5, 5, 2, 6, 5, 5, 3, 5, 3, 3, 2,
    6, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 7, 7, 7, 7, 2, 7, 7, 7, 3, 7, 3, 3, 2,
    7, 7, 7, 4, 7, 4, 4, 2, 7, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 5, 7, 5, 5, 2, 7, 5, 5, 3, 5, 3, 3, 2,
    7, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 6, 7, 6, 6, 2, 7, 6, 6, 3, 6, 3, 3, 2,
    7, 6, 6, 4, 6, 4, 4, 2, 6, 4, 4, 3, 4, 3, 3, 2,
    7, 6, 6, 5, 6, 5, 5, 2, 6, 5, 5, 3, 5, 3, 3, 2,
    6, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 3,
    7, 7, 7, 7, 7, 7, 7, 4, 7, 7, 7, 4, 7, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 5, 7, 7, 7, 5, 7, 5, 5, 3,
    7, 7, 7, 5, 7, 5, 5, 4, 7, 5, 5, 4, 5, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 3,
    7, 7, 7, 6, 7, 6, 6, 4, 7, 6, 6, 4, 6, 4, 4, 3,
    7, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 3,
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 3,
    7, 7, 7, 7, 7, 7, 7, 4, 7, 7, 7, 4, 7, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 5, 7, 7, 7, 5, 7, 5, 5, 3,
    7, 7, 7, 5, 7, 5, 5, 4, 7, 5, 5, 4, 5, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 3,
    7, 7, 7, 6, 7, 6, 6, 4, 7, 6, 6, 4, 6, 4, 4, 3,
    7, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 3,
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 5, 7, 7, 7, 5, 7, 5, 5, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 4,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 5,
    7, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 5, 7, 7, 7, 5, 7, 5, 5, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 4,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 5,
    7, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 5,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 5
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
  }
};
// clang-format on

 #if MARISA_WORD_SIZE == 64
constexpr uint64_t MASK_01 = 0x0101010101010101ULL;

// Pre-computed lookup table trick from Gog, Simon and Matthias Petri.
// "Optimized succinct data structures for massive data."  Software:
// Practice and Experience 44 (2014): 1287 - 1314.
// PREFIX_SUM_OVERFLOW[i] = (0x7F - i) * MASK_01.
const uint64_t PREFIX_SUM_OVERFLOW[64] = {
    // clang-format off
  0x7F * MASK_01, 0x7E * MASK_01, 0x7D * MASK_01, 0x7C * MASK_01,
  0x7B * MASK_01, 0x7A * MASK_01, 0x79 * MASK_01, 0x78 * MASK_01,
  0x77 * MASK_01, 0x76 * MASK_01, 0x75 * MASK_01, 0x74 * MASK_01,
  0x73 * MASK_01, 0x72 * MASK_01, 0x71 * MASK_01, 0x70 * MASK_01,

  0x6F * MASK_01, 0x6E * MASK_01, 0x6D * MASK_01, 0x6C * MASK_01,
  0x6B * MASK_01, 0x6A * MASK_01, 0x69 * MASK_01, 0x68 * MASK_01,
  0x67 * MASK_01, 0x66 * MASK_01, 0x65 * MASK_01, 0x64 * MASK_01,
  0x63 * MASK_01, 0x62 * MASK_01, 0x61 * MASK_01, 0x60 * MASK_01,

  0x5F * MASK_01, 0x5E * MASK_01, 0x5D * MASK_01, 0x5C * MASK_01,
  0x5B * MASK_01, 0x5A * MASK_01, 0x59 * MASK_01, 0x58 * MASK_01,
  0x57 * MASK_01, 0x56 * MASK_01, 0x55 * MASK_01, 0x54 * MASK_01,
  0x53 * MASK_01, 0x52 * MASK_01, 0x51 * MASK_01, 0x50 * MASK_01,

  0x4F * MASK_01, 0x4E * MASK_01, 0x4D * MASK_01, 0x4C * MASK_01,
  0x4B * MASK_01, 0x4A * MASK_01, 0x49 * MASK_01, 0x48 * MASK_01,
  0x47 * MASK_01, 0x46 * MASK_01, 0x45 * MASK_01, 0x44 * MASK_01,
  0x43 * MASK_01, 0x42 * MASK_01, 0x41 * MASK_01, 0x40 * MASK_01
    // clang-format on
};

std::size_t select_bit(std::size_t i, std::size_t bit_id, uint64_t unit) {
  uint64_t counts;
  {
  #if defined(MARISA_X64) && defined(MARISA_USE_SSSE3)
    __m128i lower_nibbles =
        _mm_cvtsi64_si128(static_cast<long long>(unit & 0x0F0F0F0F0F0F0F0FULL));
    __m128i upper_nibbles =
        _mm_cvtsi64_si128(static_cast<long long>(unit & 0xF0F0F0F0F0F0F0F0ULL));
    upper_nibbles = _mm_srli_epi32(upper_nibbles, 4);

    __m128i lower_counts =
        _mm_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0);
    lower_counts = _mm_shuffle_epi8(lower_counts, lower_nibbles);
    __m128i upper_counts =
        _mm_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0);
    upper_counts = _mm_shuffle_epi8(upper_counts, upper_nibbles);

    counts = static_cast<uint64_t>(
        _mm_cvtsi128_si64(_mm_add_epi8(lower_counts, upper_counts)));
  #elif defined(MARISA_AARCH64)
    // Byte-wise popcount using CNT (plus a lot of conversion noise).
    // This actually only requires NEON, not AArch64, but we are already
    // in a 64-bit `#ifdef`.
    counts = vget_lane_u64(vreinterpret_u64_u8(vcnt_u8(vcreate_u8(unit))), 0);
  #else   // defined(MARISA_AARCH64)
    constexpr uint64_t MASK_0F = 0x0F0F0F0F0F0F0F0FULL;
    constexpr uint64_t MASK_33 = 0x3333333333333333ULL;
    constexpr uint64_t MASK_55 = 0x5555555555555555ULL;
    counts = unit - ((unit >> 1) & MASK_55);
    counts = (counts & MASK_33) + ((counts >> 2) & MASK_33);
    counts = (counts + (counts >> 4)) & MASK_0F;
  #endif  // defined(MARISA_AARCH64)
    counts *= MASK_01;
  }

  #if defined(MARISA_X64) && defined(MARISA_USE_POPCNT)
  uint8_t skip;
  {
    __m128i x = _mm_cvtsi64_si128(static_cast<long long>((i + 1) * MASK_01));
    __m128i y = _mm_cvtsi64_si128(static_cast<long long>(counts));
    x = _mm_cmpgt_epi8(x, y);
    skip = (uint8_t)popcount(static_cast<uint64_t>(_mm_cvtsi128_si64(x)));
  }
  #else   // defined(MARISA_X64) && defined(MARISA_USE_POPCNT)
  constexpr uint64_t MASK_80 = 0x8080808080808080ULL;
  const uint64_t x = (counts + PREFIX_SUM_OVERFLOW[i]) & MASK_80;
  // We masked with `MASK_80`, so the first bit set is the high bit in the
  // byte, therefore `num_trailing_zeros == 8 * byte_nr + 7` and the byte
  // number is the number of trailing zeros divided by 8.  We just shift off
  // the low 7 bits, so `CTZ` gives us the `skip` value we want for the
  // number of bits of `counts` to shift.
  const int skip = countr_zero(x >> 7);
  #endif  // defined(MARISA_X64) && defined(MARISA_USE_POPCNT)

  bit_id += static_cast<std::size_t>(skip);
  unit >>= skip;
  i -= ((counts << 8) >> skip) & 0xFF;

  return bit_id + SELECT_TABLE[i][unit & 0xFF];
}
 #else    // MARISA_WORD_SIZE == 64
  #ifdef MARISA_USE_SSE2
// Popcount of the byte times eight.
const uint8_t POPCNT_X8_TABLE[256] = {
    // clang-format off
   0,  8,  8, 16,  8, 16, 16, 24,  8, 16, 16, 24, 16, 24, 24, 32,
   8, 16, 16, 24, 16, 24, 24, 32, 16, 24, 24, 32, 24, 32, 32, 40,
   8, 16, 16, 24, 16, 24, 24, 32, 16, 24, 24, 32, 24, 32, 32, 40,
  16, 24, 24, 32, 24, 32, 32, 40, 24, 32, 32, 40, 32, 40, 40, 48,
   8, 16, 16, 24, 16, 24, 24, 32, 16, 24, 24, 32, 24, 32, 32, 40,
  16, 24, 24, 32, 24, 32, 32, 40, 24, 32, 32, 40, 32, 40, 40, 48,
  16, 24, 24, 32, 24, 32, 32, 40, 24, 32, 32, 40, 32, 40, 40, 48,
  24, 32, 32, 40, 32, 40, 40, 48, 32, 40, 40, 48, 40, 48, 48, 56,
   8, 16, 16, 24, 16, 24, 24, 32, 16, 24, 24, 32, 24, 32, 32, 40,
  16, 24, 24, 32, 24, 32, 32, 40, 24, 32, 32, 40, 32, 40, 40, 48,
  16, 24, 24, 32, 24, 32, 32, 40, 24, 32, 32, 40, 32, 40, 40, 48,
  24, 32, 32, 40, 32, 40, 40, 48, 32, 40, 40, 48, 40, 48, 48, 56,
  16, 24, 24, 32, 24, 32, 32, 40, 24, 32, 32, 40, 32, 40, 40, 48,
  24, 32, 32, 40, 32, 40, 40, 48, 32, 40, 40, 48, 40, 48, 48, 56,
  24, 32, 32, 40, 32, 40, 40, 48, 32, 40, 40, 48, 40, 48, 48, 56,
  32, 40, 40, 48, 40, 48, 48, 56, 40, 48, 48, 56, 48, 56, 56, 64
    // clang-format on
};

std::size_t select_bit(std::size_t i, std::size_t bit_id, uint32_t unit_lo,
                       uint32_t unit_hi) {
  __m128i unit;
  {
    __m128i lower_dword = _mm_cvtsi32_si128(unit_lo);
    __m128i upper_dword = _mm_cvtsi32_si128(unit_hi);
    upper_dword = _mm_slli_si128(upper_dword, 4);
    unit = _mm_or_si128(lower_dword, upper_dword);
  }

  __m128i counts;
  {
   #ifdef MARISA_USE_SSSE3
    __m128i lower_nibbles = _mm_set1_epi8(0x0F);
    lower_nibbles = _mm_and_si128(lower_nibbles, unit);
    __m128i upper_nibbles = _mm_set1_epi8((uint8_t)0xF0);
    upper_nibbles = _mm_and_si128(upper_nibbles, unit);
    upper_nibbles = _mm_srli_epi32(upper_nibbles, 4);

    __m128i lower_counts =
        _mm_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0);
    lower_counts = _mm_shuffle_epi8(lower_counts, lower_nibbles);
    __m128i upper_counts =
        _mm_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0);
    upper_counts = _mm_shuffle_epi8(upper_counts, upper_nibbles);

    counts = _mm_add_epi8(lower_counts, upper_counts);
   #else   // MARISA_USE_SSSE3
    __m128i x = _mm_srli_epi32(unit, 1);
    x = _mm_and_si128(x, _mm_set1_epi8(0x55));
    x = _mm_sub_epi8(unit, x);

    __m128i y = _mm_srli_epi32(x, 2);
    y = _mm_and_si128(y, _mm_set1_epi8(0x33));
    x = _mm_and_si128(x, _mm_set1_epi8(0x33));
    x = _mm_add_epi8(x, y);

    y = _mm_srli_epi32(x, 4);
    x = _mm_add_epi8(x, y);
    counts = _mm_and_si128(x, _mm_set1_epi8(0x0F));
   #endif  // MARISA_USE_SSSE3
  }

  __m128i accumulated_counts;
  {
    __m128i x = counts;
    x = _mm_slli_si128(x, 1);
    __m128i y = counts;
    y = _mm_add_epi32(y, x);

    x = y;
    y = _mm_slli_si128(y, 2);
    x = _mm_add_epi32(x, y);

    y = x;
    x = _mm_slli_si128(x, 4);
    y = _mm_add_epi32(y, x);

    accumulated_counts = _mm_set_epi32(0x7F7F7F7FU, 0x7F7F7F7FU, 0, 0);
    accumulated_counts = _mm_or_si128(accumulated_counts, y);
  }

  uint8_t skip;
  {
    __m128i x = _mm_set1_epi8((uint8_t)(i + 1));
    x = _mm_cmpgt_epi8(x, accumulated_counts);
    // Since we use `_mm_movemask_epi8`, to move the top bit of every byte,
    // popcount times eight gives the original popcount of `x` before the
    // movemask.  (`_mm_cmpgt_epi8` sets all bits in a byte to 0 or 1.)
    skip = POPCNT_X8_TABLE[_mm_movemask_epi8(x)];
  }

  uint8_t byte;
  {
    alignas(16) uint8_t unit_bytes[16];
    alignas(16) uint8_t accumulated_counts_bytes[16];
    accumulated_counts = _mm_slli_si128(accumulated_counts, 1);
    _mm_store_si128(reinterpret_cast<__m128i *>(unit_bytes), unit);
    _mm_store_si128(reinterpret_cast<__m128i *>(accumulated_counts_bytes),
                    accumulated_counts);

    bit_id += skip;
    byte = unit_bytes[skip / 8];
    i -= accumulated_counts_bytes[skip / 8];
  }

  return bit_id + SELECT_TABLE[i][byte];
}
  #else    // MARISA_USE_SSE2
const uint8_t POPCNT_TABLE[256] = {
    // clang-format off
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    // clang-format on
};

std::size_t select_bit(std::size_t i, std::size_t bit_id, uint32_t unit_lo,
                       uint32_t unit_hi) {
  uint32_t next_byte = unit_lo & 0xFF;
  uint32_t byte_popcount = POPCNT_TABLE[next_byte];
  // Assuming the desired bit is in a random byte, branches are not
  // taken 7/8 of the time, so this is branch-predictor friendly,
  // unlike binary search.
  if (i < byte_popcount) return bit_id + SELECT_TABLE[i][next_byte];
  i -= byte_popcount;
  next_byte = (unit_lo >> 8) & 0xFF;
  byte_popcount = POPCNT_TABLE[next_byte];
  if (i < byte_popcount) return bit_id + 8 + SELECT_TABLE[i][next_byte];
  i -= byte_popcount;
  next_byte = (unit_lo >> 16) & 0xFF;
  byte_popcount = POPCNT_TABLE[next_byte];
  if (i < byte_popcount) return bit_id + 16 + SELECT_TABLE[i][next_byte];
  i -= byte_popcount;
  next_byte = unit_lo >> 24;
  byte_popcount = POPCNT_TABLE[next_byte];
  if (i < byte_popcount) return bit_id + 24 + SELECT_TABLE[i][next_byte];
  i -= byte_popcount;

  next_byte = unit_hi & 0xFF;
  byte_popcount = POPCNT_TABLE[next_byte];
  if (i < byte_popcount) return bit_id + 32 + SELECT_TABLE[i][next_byte];
  i -= byte_popcount;
  next_byte = (unit_hi >> 8) & 0xFF;
  byte_popcount = POPCNT_TABLE[next_byte];
  if (i < byte_popcount) return bit_id + 40 + SELECT_TABLE[i][next_byte];
  i -= byte_popcount;
  next_byte = (unit_hi >> 16) & 0xFF;
  byte_popcount = POPCNT_TABLE[next_byte];
  if (i < byte_popcount) return bit_id + 48 + SELECT_TABLE[i][next_byte];
  i -= byte_popcount;
  next_byte = unit_hi >> 24;
  // Assume `i < POPCNT_TABLE[next_byte]`.
  return bit_id + 56 + SELECT_TABLE[i][next_byte];
}
  #endif   // MARISA_USE_SSE2

// This is only used by build_index, so don't worry about the small performance
// penalty from not having version taking only a uint32_t.
inline std::size_t select_bit(std::size_t i, std::size_t bit_id,
                              uint32_t unit) {
  return select_bit(i, bit_id, /*unit_lo=*/unit, /*unit_hi=*/0);
}

 #endif  // MARISA_WORD_SIZE == 64
#endif   // MARISA_USE_BMI2

}  // namespace

#if MARISA_WORD_SIZE == 64

std::size_t BitVector::rank1(std::size_t i) const {
  assert(!ranks_.empty());
  assert(i <= size_);

  const RankIndex &rank = ranks_[i / 512];
  std::size_t offset = rank.abs();
  switch ((i / 64) % 8) {
    case 1: {
      offset += rank.rel1();
      break;
    }
    case 2: {
      offset += rank.rel2();
      break;
    }
    case 3: {
      offset += rank.rel3();
      break;
    }
    case 4: {
      offset += rank.rel4();
      break;
    }
    case 5: {
      offset += rank.rel5();
      break;
    }
    case 6: {
      offset += rank.rel6();
      break;
    }
    case 7: {
      offset += rank.rel7();
      break;
    }
  }
  offset += popcount(units_[i / 64] & ((1ULL << (i % 64)) - 1));
  return offset;
}

std::size_t BitVector::select0(std::size_t i) const {
  assert(!select0s_.empty());
  assert(i < num_0s());

  const std::size_t select_id = i / 512;
  assert((select_id + 1) < select0s_.size());
  if ((i % 512) == 0) {
    return select0s_[select_id];
  }
  std::size_t begin = select0s_[select_id] / 512;
  std::size_t end = (select0s_[select_id + 1] + 511) / 512;
  if (begin + 10 >= end) {
    while (i >= ((begin + 1) * 512) - ranks_[begin + 1].abs()) {
      ++begin;
    }
  } else {
    while (begin + 1 < end) {
      const std::size_t middle = (begin + end) / 2;
      if (i < (middle * 512) - ranks_[middle].abs()) {
        end = middle;
      } else {
        begin = middle;
      }
    }
  }
  const std::size_t rank_id = begin;
  i -= (rank_id * 512) - ranks_[rank_id].abs();

  const RankIndex &rank = ranks_[rank_id];
  std::size_t unit_id = rank_id * 8;
  if (i < (256U - rank.rel4())) {
    if (i < (128U - rank.rel2())) {
      if (i >= (64U - rank.rel1())) {
        unit_id += 1;
        i -= 64 - rank.rel1();
      }
    } else if (i < (192U - rank.rel3())) {
      unit_id += 2;
      i -= 128 - rank.rel2();
    } else {
      unit_id += 3;
      i -= 192 - rank.rel3();
    }
  } else if (i < (384U - rank.rel6())) {
    if (i < (320U - rank.rel5())) {
      unit_id += 4;
      i -= 256 - rank.rel4();
    } else {
      unit_id += 5;
      i -= 320 - rank.rel5();
    }
  } else if (i < (448U - rank.rel7())) {
    unit_id += 6;
    i -= 384 - rank.rel6();
  } else {
    unit_id += 7;
    i -= 448 - rank.rel7();
  }

  return select_bit(i, unit_id * 64, ~units_[unit_id]);
}

std::size_t BitVector::select1(std::size_t i) const {
  assert(!select1s_.empty());
  assert(i < num_1s());

  const std::size_t select_id = i / 512;
  assert((select_id + 1) < select1s_.size());
  if ((i % 512) == 0) {
    return select1s_[select_id];
  }
  std::size_t begin = select1s_[select_id] / 512;
  std::size_t end = (select1s_[select_id + 1] + 511) / 512;
  if (begin + 10 >= end) {
    while (i >= ranks_[begin + 1].abs()) {
      ++begin;
    }
  } else {
    while (begin + 1 < end) {
      const std::size_t middle = (begin + end) / 2;
      if (i < ranks_[middle].abs()) {
        end = middle;
      } else {
        begin = middle;
      }
    }
  }
  const std::size_t rank_id = begin;
  i -= ranks_[rank_id].abs();

  const RankIndex &rank = ranks_[rank_id];
  std::size_t unit_id = rank_id * 8;
  if (i < rank.rel4()) {
    if (i < rank.rel2()) {
      if (i >= rank.rel1()) {
        unit_id += 1;
        i -= rank.rel1();
      }
    } else if (i < rank.rel3()) {
      unit_id += 2;
      i -= rank.rel2();
    } else {
      unit_id += 3;
      i -= rank.rel3();
    }
  } else if (i < rank.rel6()) {
    if (i < rank.rel5()) {
      unit_id += 4;
      i -= rank.rel4();
    } else {
      unit_id += 5;
      i -= rank.rel5();
    }
  } else if (i < rank.rel7()) {
    unit_id += 6;
    i -= rank.rel6();
  } else {
    unit_id += 7;
    i -= rank.rel7();
  }

  return select_bit(i, unit_id * 64, units_[unit_id]);
}

#else  // MARISA_WORD_SIZE == 64

std::size_t BitVector::rank1(std::size_t i) const {
  assert(!ranks_.empty());
  assert(i <= size_);

  const RankIndex &rank = ranks_[i / 512];
  std::size_t offset = rank.abs();
  switch ((i / 64) % 8) {
    case 1: {
      offset += rank.rel1();
      break;
    }
    case 2: {
      offset += rank.rel2();
      break;
    }
    case 3: {
      offset += rank.rel3();
      break;
    }
    case 4: {
      offset += rank.rel4();
      break;
    }
    case 5: {
      offset += rank.rel5();
      break;
    }
    case 6: {
      offset += rank.rel6();
      break;
    }
    case 7: {
      offset += rank.rel7();
      break;
    }
  }
  if (((i / 32) & 1) == 1) {
    offset += popcount(units_[(i / 32) - 1]);
  }
  offset += popcount(units_[i / 32] & ((1U << (i % 32)) - 1));
  return offset;
}

std::size_t BitVector::select0(std::size_t i) const {
  assert(!select0s_.empty());
  assert(i < num_0s());

  const std::size_t select_id = i / 512;
  assert((select_id + 1) < select0s_.size());
  if ((i % 512) == 0) {
    return select0s_[select_id];
  }
  std::size_t begin = select0s_[select_id] / 512;
  std::size_t end = (select0s_[select_id + 1] + 511) / 512;
  if (begin + 10 >= end) {
    while (i >= ((begin + 1) * 512) - ranks_[begin + 1].abs()) {
      ++begin;
    }
  } else {
    while (begin + 1 < end) {
      const std::size_t middle = (begin + end) / 2;
      if (i < (middle * 512) - ranks_[middle].abs()) {
        end = middle;
      } else {
        begin = middle;
      }
    }
  }
  const std::size_t rank_id = begin;
  i -= (rank_id * 512) - ranks_[rank_id].abs();

  const RankIndex &rank = ranks_[rank_id];
  std::size_t unit_id = rank_id * 16;
  if (i < (256U - rank.rel4())) {
    if (i < (128U - rank.rel2())) {
      if (i >= (64U - rank.rel1())) {
        unit_id += 2;
        i -= 64 - rank.rel1();
      }
    } else if (i < (192U - rank.rel3())) {
      unit_id += 4;
      i -= 128 - rank.rel2();
    } else {
      unit_id += 6;
      i -= 192 - rank.rel3();
    }
  } else if (i < (384U - rank.rel6())) {
    if (i < (320U - rank.rel5())) {
      unit_id += 8;
      i -= 256 - rank.rel4();
    } else {
      unit_id += 10;
      i -= 320 - rank.rel5();
    }
  } else if (i < (448U - rank.rel7())) {
    unit_id += 12;
    i -= 384 - rank.rel6();
  } else {
    unit_id += 14;
    i -= 448 - rank.rel7();
  }

  return select_bit(i, unit_id * 32, ~units_[unit_id], ~units_[unit_id + 1]);
}

std::size_t BitVector::select1(std::size_t i) const {
  assert(!select1s_.empty());
  assert(i < num_1s());

  const std::size_t select_id = i / 512;
  assert((select_id + 1) < select1s_.size());
  if ((i % 512) == 0) {
    return select1s_[select_id];
  }
  std::size_t begin = select1s_[select_id] / 512;
  std::size_t end = (select1s_[select_id + 1] + 511) / 512;
  if (begin + 10 >= end) {
    while (i >= ranks_[begin + 1].abs()) {
      ++begin;
    }
  } else {
    while (begin + 1 < end) {
      const std::size_t middle = (begin + end) / 2;
      if (i < ranks_[middle].abs()) {
        end = middle;
      } else {
        begin = middle;
      }
    }
  }
  const std::size_t rank_id = begin;
  i -= ranks_[rank_id].abs();

  const RankIndex &rank = ranks_[rank_id];
  std::size_t unit_id = rank_id * 16;
  if (i < rank.rel4()) {
    if (i < rank.rel2()) {
      if (i >= rank.rel1()) {
        unit_id += 2;
        i -= rank.rel1();
      }
    } else if (i < rank.rel3()) {
      unit_id += 4;
      i -= rank.rel2();
    } else {
      unit_id += 6;
      i -= rank.rel3();
    }
  } else if (i < rank.rel6()) {
    if (i < rank.rel5()) {
      unit_id += 8;
      i -= rank.rel4();
    } else {
      unit_id += 10;
      i -= rank.rel5();
    }
  } else if (i < rank.rel7()) {
    unit_id += 12;
    i -= rank.rel6();
  } else {
    unit_id += 14;
    i -= rank.rel7();
  }

  return select_bit(i, unit_id * 32, units_[unit_id], units_[unit_id + 1]);
}

#endif  // MARISA_WORD_SIZE == 64

void BitVector::build_index(const BitVector &bv, bool enables_select0,
                            bool enables_select1) {
  const std::size_t num_bits = bv.size();
  ranks_.resize((num_bits / 512) + (((num_bits % 512) != 0) ? 1 : 0) + 1);

  std::size_t num_0s = 0;  // Only updated if enables_select0 is true.
  std::size_t num_1s = 0;

  const std::size_t num_units = bv.units_.size();
  for (std::size_t unit_id = 0; unit_id < num_units; ++unit_id) {
    const std::size_t bit_id = unit_id * MARISA_WORD_SIZE;

    if ((bit_id % 64) == 0) {
      const std::size_t rank_id = bit_id / 512;
      switch ((bit_id / 64) % 8) {
        case 0: {
          ranks_[rank_id].set_abs(num_1s);
          break;
        }
        case 1: {
          ranks_[rank_id].set_rel1(num_1s - ranks_[rank_id].abs());
          break;
        }
        case 2: {
          ranks_[rank_id].set_rel2(num_1s - ranks_[rank_id].abs());
          break;
        }
        case 3: {
          ranks_[rank_id].set_rel3(num_1s - ranks_[rank_id].abs());
          break;
        }
        case 4: {
          ranks_[rank_id].set_rel4(num_1s - ranks_[rank_id].abs());
          break;
        }
        case 5: {
          ranks_[rank_id].set_rel5(num_1s - ranks_[rank_id].abs());
          break;
        }
        case 6: {
          ranks_[rank_id].set_rel6(num_1s - ranks_[rank_id].abs());
          break;
        }
        case 7: {
          ranks_[rank_id].set_rel7(num_1s - ranks_[rank_id].abs());
          break;
        }
      }
    }

    const Unit unit = bv.units_[unit_id];
    // push_back resizes with 0, so the high bits of the last unit are 0 and
    // do not affect the 1s count.
    const std::size_t unit_num_1s = popcount(unit);

    if (enables_select0) {
      // num_0s is somewhat move involved to compute, so only do it if we
      // need it.  The last word has zeros in the high bits, so that needs
      // to be accounted for when computing the unit_num_0s from unit_num_1s.
      const std::size_t bits_remaining = num_bits - bit_id;
      const std::size_t unit_num_0s =
          std::min<std::size_t>(bits_remaining, MARISA_WORD_SIZE) - unit_num_1s;

      // Note: MSVC rejects unary minus operator applied to unsigned type.
      const std::size_t zero_bit_id = (0 - num_0s) % 512;
      if (unit_num_0s > zero_bit_id) {
        // select0s_ is uint32_t, but select_bit returns size_t, so cast to
        // suppress narrowing conversion warning.  push_back checks the
        // size, so there is no truncation here.
        select0s_.push_back(
            static_cast<uint32_t>(select_bit(zero_bit_id, bit_id, ~unit)));
      }

      num_0s += unit_num_0s;
    }

    if (enables_select1) {
      // Note: MSVC rejects unary minus operator applied to unsigned type.
      const std::size_t one_bit_id = (0 - num_1s) % 512;
      if (unit_num_1s > one_bit_id) {
        select1s_.push_back(
            static_cast<uint32_t>(select_bit(one_bit_id, bit_id, unit)));
      }
    }

    num_1s += unit_num_1s;
  }

  if ((num_bits % 512) != 0) {
    const std::size_t rank_id = (num_bits - 1) / 512;
    switch (((num_bits - 1) / 64) % 8) {
      case 0: {
        ranks_[rank_id].set_rel1(num_1s - ranks_[rank_id].abs());
      }
        [[fallthrough]];
      case 1: {
        ranks_[rank_id].set_rel2(num_1s - ranks_[rank_id].abs());
      }
        [[fallthrough]];
      case 2: {
        ranks_[rank_id].set_rel3(num_1s - ranks_[rank_id].abs());
      }
        [[fallthrough]];
      case 3: {
        ranks_[rank_id].set_rel4(num_1s - ranks_[rank_id].abs());
      }
        [[fallthrough]];
      case 4: {
        ranks_[rank_id].set_rel5(num_1s - ranks_[rank_id].abs());
      }
        [[fallthrough]];
      case 5: {
        ranks_[rank_id].set_rel6(num_1s - ranks_[rank_id].abs());
      }
        [[fallthrough]];
      case 6: {
        ranks_[rank_id].set_rel7(num_1s - ranks_[rank_id].abs());
        break;
      }
    }
  }

  size_ = num_bits;
  num_1s_ = bv.num_1s();

  ranks_.back().set_abs(num_1s);
  if (enables_select0) {
    select0s_.push_back(static_cast<uint32_t>(num_bits));
    select0s_.shrink();
  }
  if (enables_select1) {
    select1s_.push_back(static_cast<uint32_t>(num_bits));
    select1s_.shrink();
  }
}

}  // namespace marisa::grimoire::vector
