/*
 * Open Chinese Convert
 *
 * Copyright 2024-2024 Carbo Kuo <byvoid@byvoid.com>
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

#include "opencc.h"
#include "gtest/gtest.h"

namespace opencc {

class BazelOpenccTest : public ::testing::Test {};

TEST_F(BazelOpenccTest, SimpleConverter_s2t) {
  SimpleConverter converter(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
  EXPECT_EQ(converter.Convert("简化字测试"), "簡化字測試");
}

TEST_F(BazelOpenccTest, SimpleConverter_t2s) {
  SimpleConverter converter(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP);
  EXPECT_EQ(converter.Convert("簡化字測試"), "简化字测试");
}

TEST_F(BazelOpenccTest, CInterface_s2t) {
  std::string text = "简化字测试";
  opencc_t od = opencc_open(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
  char* converted = opencc_convert_utf8(od, text.c_str(), (size_t)-1);
  EXPECT_STREQ("簡化字測試", converted);
  opencc_convert_utf8_free(converted);
  EXPECT_EQ(0, opencc_close(od));
}

TEST_F(BazelOpenccTest, CInterface_t2s) {
  std::string text = "簡化字測試";
  opencc_t od = opencc_open(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP);
  char* converted = opencc_convert_utf8(od, text.c_str(), (size_t)-1);
  EXPECT_STREQ("简化字测试", converted);
  opencc_convert_utf8_free(converted);
  EXPECT_EQ(0, opencc_close(od));
}

} // namespace opencc
