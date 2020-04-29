#include "marisa/grimoire/vector/pop-count.h"
#include "marisa/grimoire/vector/bit-vector.h"

namespace marisa {
namespace grimoire {
namespace vector {
namespace {

const UInt8 SELECT_TABLE[8][256] = {
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

#if MARISA_WORD_SIZE == 64
const UInt64 MASK_55 = 0x5555555555555555ULL;
const UInt64 MASK_33 = 0x3333333333333333ULL;
const UInt64 MASK_0F = 0x0F0F0F0F0F0F0F0FULL;
const UInt64 MASK_01 = 0x0101010101010101ULL;
const UInt64 MASK_80 = 0x8080808080808080ULL;

std::size_t select_bit(std::size_t i, std::size_t bit_id, UInt64 unit) {
  UInt64 counts;
  {
 #if defined(MARISA_X64) && defined(MARISA_USE_SSSE3)
    __m128i lower_nibbles = _mm_cvtsi64_si128(unit & 0x0F0F0F0F0F0F0F0FULL);
    __m128i upper_nibbles = _mm_cvtsi64_si128(unit & 0xF0F0F0F0F0F0F0F0ULL);
    upper_nibbles = _mm_srli_epi32(upper_nibbles, 4);

    __m128i lower_counts =
        _mm_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0);
    lower_counts = _mm_shuffle_epi8(lower_counts, lower_nibbles);
    __m128i upper_counts =
        _mm_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0);
    upper_counts = _mm_shuffle_epi8(upper_counts, upper_nibbles);

    counts = _mm_cvtsi128_si64(_mm_add_epi8(lower_counts, upper_counts));
 #else  // defined(MARISA_X64) && defined(MARISA_USE_SSSE3)
    counts = unit - ((unit >> 1) & MASK_55);
    counts = (counts & MASK_33) + ((counts >> 2) & MASK_33);
    counts = (counts + (counts >> 4)) & MASK_0F;
 #endif  // defined(MARISA_X64) && defined(MARISA_USE_SSSE3)
    counts *= MASK_01;
  }

 #if defined(MARISA_X64) && defined(MARISA_USE_POPCNT)
  UInt8 skip;
  {
    __m128i x = _mm_cvtsi64_si128((i + 1) * MASK_01);
    __m128i y = _mm_cvtsi64_si128(counts);
    x = _mm_cmpgt_epi8(x, y);
    skip = (UInt8)PopCount::count(_mm_cvtsi128_si64(x));
  }
 #else  // defined(MARISA_X64) && defined(MARISA_USE_POPCNT)
  const UInt64 x = (counts | MASK_80) - ((i + 1) * MASK_01);
  #ifdef _MSC_VER
  unsigned long skip;
  ::_BitScanForward64(&skip, (x & MASK_80) >> 7);
  #else  // _MSC_VER
  const int skip = ::__builtin_ctzll((x & MASK_80) >> 7);
  #endif  // _MSC_VER
 #endif  // defined(MARISA_X64) && defined(MARISA_USE_POPCNT)

  bit_id += skip;
  unit >>= skip;
  i -= ((counts << 8) >> skip) & 0xFF;

  return bit_id + SELECT_TABLE[i][unit & 0xFF];
}
#else  // MARISA_WORD_SIZE == 64
 #ifdef MARISA_USE_SSE2
const UInt8 POPCNT_TABLE[256] = {
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
};

std::size_t select_bit(std::size_t i, std::size_t bit_id,
    UInt32 unit_lo, UInt32 unit_hi) {
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
    __m128i upper_nibbles = _mm_set1_epi8((UInt8)0xF0);
    upper_nibbles = _mm_and_si128(upper_nibbles, unit);
    upper_nibbles = _mm_srli_epi32(upper_nibbles, 4);

    __m128i lower_counts =
        _mm_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0);
    lower_counts = _mm_shuffle_epi8(lower_counts, lower_nibbles);
    __m128i upper_counts =
        _mm_set_epi8(4, 3, 3, 2, 3, 2, 2, 1, 3, 2, 2, 1, 2, 1, 1, 0);
    upper_counts = _mm_shuffle_epi8(upper_counts, upper_nibbles);

    counts = _mm_add_epi8(lower_counts, upper_counts);
  #else  // MARISA_USE_SSSE3
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

  UInt8 skip;
  {
    __m128i x = _mm_set1_epi8((UInt8)(i + 1));
    x = _mm_cmpgt_epi8(x, accumulated_counts);
    skip = POPCNT_TABLE[_mm_movemask_epi8(x)];
  }

  UInt8 byte;
  {
  #ifdef _MSC_VER
    __declspec(align(16)) UInt8 unit_bytes[16];
    __declspec(align(16)) UInt8 accumulated_counts_bytes[16];
  #else  // _MSC_VER
    UInt8 unit_bytes[16] __attribute__ ((aligned (16)));
    UInt8 accumulated_counts_bytes[16] __attribute__ ((aligned (16)));
  #endif  // _MSC_VER
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
 #endif  // MARISA_USE_SSE2
#endif  // MARISA_WORD_SIZE == 64

}  // namespace

#if MARISA_WORD_SIZE == 64

std::size_t BitVector::rank1(std::size_t i) const {
  MARISA_DEBUG_IF(ranks_.empty(), MARISA_STATE_ERROR);
  MARISA_DEBUG_IF(i > size_, MARISA_BOUND_ERROR);

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
  offset += PopCount::count(units_[i / 64] & ((1ULL << (i % 64)) - 1));
  return offset;
}

std::size_t BitVector::select0(std::size_t i) const {
  MARISA_DEBUG_IF(select0s_.empty(), MARISA_STATE_ERROR);
  MARISA_DEBUG_IF(i >= num_0s(), MARISA_BOUND_ERROR);

  const std::size_t select_id = i / 512;
  MARISA_DEBUG_IF((select_id + 1) >= select0s_.size(), MARISA_BOUND_ERROR);
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
  MARISA_DEBUG_IF(select1s_.empty(), MARISA_STATE_ERROR);
  MARISA_DEBUG_IF(i >= num_1s(), MARISA_BOUND_ERROR);

  const std::size_t select_id = i / 512;
  MARISA_DEBUG_IF((select_id + 1) >= select1s_.size(), MARISA_BOUND_ERROR);
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
  MARISA_DEBUG_IF(ranks_.empty(), MARISA_STATE_ERROR);
  MARISA_DEBUG_IF(i > size_, MARISA_BOUND_ERROR);

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
    offset += PopCount::count(units_[(i / 32) - 1]);
  }
  offset += PopCount::count(units_[i / 32] & ((1U << (i % 32)) - 1));
  return offset;
}

std::size_t BitVector::select0(std::size_t i) const {
  MARISA_DEBUG_IF(select0s_.empty(), MARISA_STATE_ERROR);
  MARISA_DEBUG_IF(i >= num_0s(), MARISA_BOUND_ERROR);

  const std::size_t select_id = i / 512;
  MARISA_DEBUG_IF((select_id + 1) >= select0s_.size(), MARISA_BOUND_ERROR);
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

#ifdef MARISA_USE_SSE2
  return select_bit(i, unit_id * 32, ~units_[unit_id], ~units_[unit_id + 1]);
#else  // MARISA_USE_SSE2
  UInt32 unit = ~units_[unit_id];
  PopCount count(unit);
  if (i >= count.lo32()) {
    ++unit_id;
    i -= count.lo32();
    unit = ~units_[unit_id];
    count = PopCount(unit);
  }

  std::size_t bit_id = unit_id * 32;
  if (i < count.lo16()) {
    if (i >= count.lo8()) {
      bit_id += 8;
      unit >>= 8;
      i -= count.lo8();
    }
  } else if (i < count.lo24()) {
    bit_id += 16;
    unit >>= 16;
    i -= count.lo16();
  } else {
    bit_id += 24;
    unit >>= 24;
    i -= count.lo24();
  }
  return bit_id + SELECT_TABLE[i][unit & 0xFF];
#endif  // MARISA_USE_SSE2
}

std::size_t BitVector::select1(std::size_t i) const {
  MARISA_DEBUG_IF(select1s_.empty(), MARISA_STATE_ERROR);
  MARISA_DEBUG_IF(i >= num_1s(), MARISA_BOUND_ERROR);

  const std::size_t select_id = i / 512;
  MARISA_DEBUG_IF((select_id + 1) >= select1s_.size(), MARISA_BOUND_ERROR);
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

#ifdef MARISA_USE_SSE2
  return select_bit(i, unit_id * 32, units_[unit_id], units_[unit_id + 1]);
#else  // MARISA_USE_SSE2
  UInt32 unit = units_[unit_id];
  PopCount count(unit);
  if (i >= count.lo32()) {
    ++unit_id;
    i -= count.lo32();
    unit = units_[unit_id];
    count = PopCount(unit);
  }

  std::size_t bit_id = unit_id * 32;
  if (i < count.lo16()) {
    if (i >= count.lo8()) {
      bit_id += 8;
      unit >>= 8;
      i -= count.lo8();
    }
  } else if (i < count.lo24()) {
    bit_id += 16;
    unit >>= 16;
    i -= count.lo16();
  } else {
    bit_id += 24;
    unit >>= 24;
    i -= count.lo24();
  }
  return bit_id + SELECT_TABLE[i][unit & 0xFF];
#endif  // MARISA_USE_SSE2
}

#endif  // MARISA_WORD_SIZE == 64

void BitVector::build_index(const BitVector &bv,
    bool enables_select0, bool enables_select1) {
  ranks_.resize((bv.size() / 512) + (((bv.size() % 512) != 0) ? 1 : 0) + 1);

  std::size_t num_0s = 0;
  std::size_t num_1s = 0;

  for (std::size_t i = 0; i < bv.size(); ++i) {
    if ((i % 64) == 0) {
      const std::size_t rank_id = i / 512;
      switch ((i / 64) % 8) {
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

    if (bv[i]) {
      if (enables_select1 && ((num_1s % 512) == 0)) {
        select1s_.push_back(static_cast<UInt32>(i));
      }
      ++num_1s;
    } else {
      if (enables_select0 && ((num_0s % 512) == 0)) {
        select0s_.push_back(static_cast<UInt32>(i));
      }
      ++num_0s;
    }
  }

  if ((bv.size() % 512) != 0) {
    const std::size_t rank_id = (bv.size() - 1) / 512;
    switch (((bv.size() - 1) / 64) % 8) {
      case 0: {
        ranks_[rank_id].set_rel1(num_1s - ranks_[rank_id].abs());
      }
      case 1: {
        ranks_[rank_id].set_rel2(num_1s - ranks_[rank_id].abs());
      }
      case 2: {
        ranks_[rank_id].set_rel3(num_1s - ranks_[rank_id].abs());
      }
      case 3: {
        ranks_[rank_id].set_rel4(num_1s - ranks_[rank_id].abs());
      }
      case 4: {
        ranks_[rank_id].set_rel5(num_1s - ranks_[rank_id].abs());
      }
      case 5: {
        ranks_[rank_id].set_rel6(num_1s - ranks_[rank_id].abs());
      }
      case 6: {
        ranks_[rank_id].set_rel7(num_1s - ranks_[rank_id].abs());
        break;
      }
    }
  }

  size_ = bv.size();
  num_1s_ = bv.num_1s();

  ranks_.back().set_abs(num_1s);
  if (enables_select0) {
    select0s_.push_back(static_cast<UInt32>(bv.size()));
    select0s_.shrink();
  }
  if (enables_select1) {
    select1s_.push_back(static_cast<UInt32>(bv.size()));
    select1s_.shrink();
  }
}

}  // namespace vector
}  // namespace grimoire
}  // namespace marisa
