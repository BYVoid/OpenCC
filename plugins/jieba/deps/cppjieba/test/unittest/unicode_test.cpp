#include "cppjieba/Unicode.hpp"
#include "limonp/StdExtension.hpp"
#include "gtest/gtest.h"

using namespace cppjieba;
using namespace std;

TEST(UnicodeTest, Test1) {
  string s = "你好世界";
  RuneStrArray runes;
  ASSERT_TRUE(DecodeUTF8RunesInString(s, runes));
  string actual;
  string expected = "[{\"rune\": \"20320\", \"offset\": 0, \"len\": 3}, {\"rune\": \"22909\", \"offset\": 3, \"len\": 3}, {\"rune\": \"19990\", \"offset\": 6, \"len\": 3}, {\"rune\": \"30028\", \"offset\": 9, \"len\": 3}]";
  actual << runes;
  ASSERT_EQ(expected, actual);
}

TEST(UnicodeTest, Illegal) {
  string s = "123\x80";
  RuneStrArray runes;
  ASSERT_FALSE(DecodeUTF8RunesInString(s, runes));
  string actual;
  string expected = "[]";
  actual << runes;
  ASSERT_EQ(expected, actual);
}

TEST(UnicodeTest, Rand) {
  const size_t ITERATION = 1024;
  const size_t MAX_LEN = 256;
  string s;
  srand(time(NULL));

  for (size_t i = 0; i < ITERATION; i++) {
    size_t len = rand() % MAX_LEN;
    s.resize(len);
    for (size_t j = 0; j < len; j++) {
      s[rand() % len] = rand();
    }
    RuneStrArray runes;
    DecodeUTF8RunesInString(s, runes);
  }
}
