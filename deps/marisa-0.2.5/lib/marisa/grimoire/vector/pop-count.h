#ifndef MARISA_GRIMOIRE_VECTOR_POP_COUNT_H_
#define MARISA_GRIMOIRE_VECTOR_POP_COUNT_H_

#include "marisa/grimoire/intrin.h"

namespace marisa {
namespace grimoire {
namespace vector {

#if MARISA_WORD_SIZE == 64

class PopCount {
 public:
  explicit PopCount(UInt64 x) : value_() {
    x = (x & 0x5555555555555555ULL) + ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1);
    x = (x & 0x3333333333333333ULL) + ((x & 0xCCCCCCCCCCCCCCCCULL) >> 2);
    x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x & 0xF0F0F0F0F0F0F0F0ULL) >> 4);
    x *= 0x0101010101010101ULL;
    value_ = x;
  }

  std::size_t lo8() const {
    return (std::size_t)(value_ & 0xFFU);
  }
  std::size_t lo16() const {
    return (std::size_t)((value_ >> 8) & 0xFFU);
  }
  std::size_t lo24() const {
    return (std::size_t)((value_ >> 16) & 0xFFU);
  }
  std::size_t lo32() const {
    return (std::size_t)((value_ >> 24) & 0xFFU);
  }
  std::size_t lo40() const {
    return (std::size_t)((value_ >> 32) & 0xFFU);
  }
  std::size_t lo48() const {
    return (std::size_t)((value_ >> 40) & 0xFFU);
  }
  std::size_t lo56() const {
    return (std::size_t)((value_ >> 48) & 0xFFU);
  }
  std::size_t lo64() const {
    return (std::size_t)((value_ >> 56) & 0xFFU);
  }

  static std::size_t count(UInt64 x) {
#if defined(MARISA_X64) && defined(MARISA_USE_POPCNT)
 #ifdef _MSC_VER
    return __popcnt64(x);
 #else  // _MSC_VER
    return _mm_popcnt_u64(x);
 #endif  // _MSC_VER
#else  // defined(MARISA_X64) && defined(MARISA_USE_POPCNT)
    return PopCount(x).lo64();
#endif  // defined(MARISA_X64) && defined(MARISA_USE_POPCNT)
  }

 private:
  UInt64 value_;
};

#else  // MARISA_WORD_SIZE == 64

class PopCount {
 public:
  explicit PopCount(UInt32 x) : value_() {
    x = (x & 0x55555555U) + ((x & 0xAAAAAAAAU) >> 1);
    x = (x & 0x33333333U) + ((x & 0xCCCCCCCCU) >> 2);
    x = (x & 0x0F0F0F0FU) + ((x & 0xF0F0F0F0U) >> 4);
    x *= 0x01010101U;
    value_ = x;
  }

  std::size_t lo8() const {
    return value_ & 0xFFU;
  }
  std::size_t lo16() const {
    return (value_ >> 8) & 0xFFU;
  }
  std::size_t lo24() const {
    return (value_ >> 16) & 0xFFU;
  }
  std::size_t lo32() const {
    return (value_ >> 24) & 0xFFU;
  }

  static std::size_t count(UInt32 x) {
#ifdef MARISA_USE_POPCNT
 #ifdef _MSC_VER
    return __popcnt(x);
 #else  // _MSC_VER
    return _mm_popcnt_u32(x);
 #endif  // _MSC_VER
#else  // MARISA_USE_POPCNT
    return PopCount(x).lo32();
#endif  // MARISA_USE_POPCNT
  }

 private:
  UInt32 value_;
};

#endif  // MARISA_WORD_SIZE == 64

}  // namespace vector
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_VECTOR_POP_COUNT_H_
