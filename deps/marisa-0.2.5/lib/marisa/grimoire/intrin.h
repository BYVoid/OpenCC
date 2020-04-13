#ifndef MARISA_GRIMOIRE_INTRIN_H_
#define MARISA_GRIMOIRE_INTRIN_H_

#include "marisa/base.h"

#if defined(__x86_64__) || defined(_M_X64)
 #define MARISA_X64
#elif defined(__i386__) || defined(_M_IX86)
 #define MARISA_X86
#else  // defined(__i386__) || defined(_M_IX86)
 #ifdef MARISA_USE_POPCNT
  #undef MARISA_USE_POPCNT
 #endif  // MARISA_USE_POPCNT
 #ifdef MARISA_USE_SSE4A
  #undef MARISA_USE_SSE4A
 #endif  // MARISA_USE_SSE4A
 #ifdef MARISA_USE_SSE4
  #undef MARISA_USE_SSE4
 #endif  // MARISA_USE_SSE4
 #ifdef MARISA_USE_SSE4_2
  #undef MARISA_USE_SSE4_2
 #endif  // MARISA_USE_SSE4_2
 #ifdef MARISA_USE_SSE4_1
  #undef MARISA_USE_SSE4_1
 #endif  // MARISA_USE_SSE4_1
 #ifdef MARISA_USE_SSSE3
  #undef MARISA_USE_SSSE3
 #endif  // MARISA_USE_SSSE3
 #ifdef MARISA_USE_SSE3
  #undef MARISA_USE_SSE3
 #endif  // MARISA_USE_SSE3
 #ifdef MARISA_USE_SSE2
  #undef MARISA_USE_SSE2
 #endif  // MARISA_USE_SSE2
#endif  // defined(__i386__) || defined(_M_IX86)

#ifdef MARISA_USE_POPCNT
 #ifndef MARISA_USE_SSE3
  #define MARISA_USE_SSE3
 #endif  // MARISA_USE_SSE3
 #ifdef _MSC_VER
  #include <intrin.h>
 #else  // _MSC_VER
  #include <popcntintrin.h>
 #endif  // _MSC_VER
#endif  // MARISA_USE_POPCNT

#ifdef MARISA_USE_SSE4A
 #ifndef MARISA_USE_SSE3
  #define MARISA_USE_SSE3
 #endif  // MARISA_USE_SSE3
 #ifndef MARISA_USE_POPCNT
  #define MARISA_USE_POPCNT
 #endif  // MARISA_USE_POPCNT
#endif  // MARISA_USE_SSE4A

#ifdef MARISA_USE_SSE4
 #ifndef MARISA_USE_SSE4_2
  #define MARISA_USE_SSE4_2
 #endif  // MARISA_USE_SSE4_2
#endif  // MARISA_USE_SSE4

#ifdef MARISA_USE_SSE4_2
 #ifndef MARISA_USE_SSE4_1
  #define MARISA_USE_SSE4_1
 #endif  // MARISA_USE_SSE4_1
 #ifndef MARISA_USE_POPCNT
  #define MARISA_USE_POPCNT
 #endif  // MARISA_USE_POPCNT
#endif  // MARISA_USE_SSE4_2

#ifdef MARISA_USE_SSE4_1
 #ifndef MARISA_USE_SSSE3
  #define MARISA_USE_SSSE3
 #endif  // MARISA_USE_SSSE3
#endif  // MARISA_USE_SSE4_1

#ifdef MARISA_USE_SSSE3
 #ifndef MARISA_USE_SSE3
  #define MARISA_USE_SSE3
 #endif  // MARISA_USE_SSE3
 #ifdef MARISA_X64
  #define MARISA_X64_SSSE3
 #else  // MARISA_X64
  #define MARISA_X86_SSSE3
 #endif  // MAIRSA_X64
 #include <tmmintrin.h>
#endif  // MARISA_USE_SSSE3

#ifdef MARISA_USE_SSE3
 #ifndef MARISA_USE_SSE2
  #define MARISA_USE_SSE2
 #endif  // MARISA_USE_SSE2
#endif  // MARISA_USE_SSE3

#ifdef MARISA_USE_SSE2
 #ifdef MARISA_X64
  #define MARISA_X64_SSE2
 #else  // MARISA_X64
  #define MARISA_X86_SSE2
 #endif  // MAIRSA_X64
 #include <emmintrin.h>
#endif  // MARISA_USE_SSE2

#ifdef _MSC_VER
 #if MARISA_WORD_SIZE == 64
  #include <intrin.h>
  #pragma intrinsic(_BitScanForward64)
 #else  // MARISA_WORD_SIZE == 64
  #include <intrin.h>
  #pragma intrinsic(_BitScanForward)
 #endif  // MARISA_WORD_SIZE == 64
#endif  // _MSC_VER

#endif  // MARISA_GRIMOIRE_INTRIN_H_
