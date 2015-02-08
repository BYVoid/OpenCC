/*
 * Open Chinese Convert
 *
 * Copyright 2015 BYVoid <byvoid@byvoid.com>
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

#include "gtest/gtest.h"
#include "UTF8StringSlice.hpp"

namespace opencc {

class UTF8StringSliceTest : public ::testing::Test {
protected:
  UTF8StringSliceTest()
      : text("天行健，君子以自強不息。地勢坤，君子以厚德載物。"){};

  const UTF8StringSlice text;
};

TEST_F(UTF8StringSliceTest, UTF8Length) { EXPECT_EQ(24, text.UTF8Length()); }

TEST_F(UTF8StringSliceTest, ByteLength) { EXPECT_EQ(72, text.ByteLength()); }

TEST_F(UTF8StringSliceTest, Left) {
  EXPECT_EQ(UTF8StringSlice("天行健"), text.Left(3));
}

TEST_F(UTF8StringSliceTest, Right) {
  EXPECT_EQ(UTF8StringSlice("厚德載物。"), text.Right(5));
}

TEST_F(UTF8StringSliceTest, SubString) {
  EXPECT_EQ(UTF8StringSlice("自強不息"), text.SubString(7, 4));
}

TEST_F(UTF8StringSliceTest, ToString) {
  EXPECT_EQ("地勢坤", text.SubString(12, 3).ToString());
}

TEST_F(UTF8StringSliceTest, Compare) {
  EXPECT_TRUE(text.SubString(12, 3) > UTF8StringSlice("一"));
  EXPECT_TRUE(text.SubString(12, 3) == UTF8StringSlice("地勢坤"));
}

} // namespace opencc
