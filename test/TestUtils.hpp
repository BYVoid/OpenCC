/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Common.hpp"
#include "Segments.hpp"

namespace opencc {

#ifdef _MSC_VER
# define __func__ __FUNCTION__
#endif // ifdef _MSC_VER

#define stringize(s) # s
#define Assert(condition, msg) {                                                  \
    if (!(condition)) {                                                           \
      std::ostringstream __buffer;                                                \
      __buffer << "Assertion failed: " << stringize(condition) << ", function "   \
               << __func__ << ", " << __FILE__ << ":" << __LINE__ << "\n" << msg; \
      throw AssertionFailure(__buffer.str());                                     \
    }                                                                             \
}
#define AssertTrue(condition) Assert(condition, "")
#define AssertEquals(expected, actual) {               \
    if (!((expected) == (actual))) {                   \
      std::ostringstream __buffer0;                    \
      __buffer0 << "Expected: " << (expected) << "\n"; \
      __buffer0 << "Actual: " << (actual) << "\n";     \
      Assert((expected) == (actual), __buffer0.str()); \
    }                                                  \
}

class AssertionFailure : public std::runtime_error {
public:
  AssertionFailure(string msg) : std::runtime_error(msg) {
  }
};

static inline void SegmentsAssertEquals(const Segments& expected,
                                        const Segments& actual) {
  size_t length = expected.Length();
  AssertTrue(length == actual.Length());
  for (size_t i = 0; i < length; i++) {
    AssertEquals(string(expected.At(i)), string(actual.At(i)));
  }
}

class TestUtils {
public:
  static void RunTest(const string name, void (* func)(void)) {
    clock_t start = clock();
    std::cout << "[" << name << "]" << "...";
    try {
      func();
      clock_t end = clock();
      double duration = (end - start) * 1000.0 / CLOCKS_PER_SEC;
      std::cout << "Success" << " (" << duration << "ms)" << std::endl;
    } catch (AssertionFailure e) {
      std::cout << "Failed" << std::endl;
      std::cout << e.what() << std::endl;
    }
  }
};
}
