/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 Carbo Kuo <byvoid@byvoid.com>
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

#include "gtest/gtest.h"
#include <string>

namespace opencc {

using std::string;

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif // ifdef _MSC_VER

#define EXPECT_VECTOR_EQ(expected, actual)                                     \
  {                                                                            \
    const auto& a1 = (expected);                                               \
    const auto& a2 = (actual);                                                 \
    EXPECT_EQ(a1.size(), a2.size());                                           \
    for (size_t i = 0; i < a1.size(); i++) {                                   \
      EXPECT_EQ(a1[i], a2[i]) << "Where i = " << i;                            \
    }                                                                          \
  }

} // namespace opencc
