/*
 * Open Chinese Convert
 *
 * Copyright 2015 Carbo Kuo <byvoid@byvoid.com>
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

#include <thread>

#include "ConfigTestBase.hpp"
#include "SimpleConverter.hpp"
#include "TestUtilsUTF8.hpp"
#include "opencc.h"

namespace opencc {

class SimpleConverterTest : public ConfigTestBase {
protected:
  SimpleConverterTest() {}

  void TestConverter(const std::string& config) const {
    const SimpleConverter converter(config);
    const std::string& converted =
        converter.Convert(utf8("燕燕于飞差池其羽之子于归远送于野"));
    EXPECT_EQ(utf8("燕燕于飛差池其羽之子于歸遠送於野"), converted);
  }
};

TEST_F(SimpleConverterTest, Convert) { TestConverter(CONFIG_TEST_JSON_PATH); }

TEST_F(SimpleConverterTest, Multithreading) {
  const auto& routine = [this](const std::string& config) {
    TestConverter(config);
  };
  std::thread thread1(routine, CONFIG_TEST_JSON_PATH);
  std::thread thread2(routine, CONFIG_TEST_JSON_PATH);
  routine(CONFIG_TEST_JSON_PATH);
  thread1.join();
  thread2.join();
}

TEST_F(SimpleConverterTest, CInterface) {
  const std::string& text = utf8("燕燕于飞差池其羽之子于归远送于野");
  const std::string& expected = utf8("燕燕于飛差池其羽之子于歸遠送於野");
  {
    opencc_t od = opencc_open(CONFIG_TEST_JSON_PATH.c_str());
    char* converted = opencc_convert_utf8(od, text.c_str(), (size_t)-1);
    EXPECT_EQ(expected, converted);
    opencc_convert_utf8_free(converted);
    EXPECT_EQ(0, opencc_close(od));
  }
  {
    char output[1024];
    opencc_t od = opencc_open(CONFIG_TEST_JSON_PATH.c_str());
    size_t length =
        opencc_convert_utf8_to_buffer(od, text.c_str(), (size_t)-1, output);
    EXPECT_EQ(expected.length(), length);
    EXPECT_EQ(expected, output);
    EXPECT_EQ(0, opencc_close(od));
  }
  {
    std::string path = "/opencc/no/such/file/or/directory";
    opencc_t od = opencc_open(path.c_str());
    EXPECT_EQ(reinterpret_cast<opencc_t>(-1), od);
    EXPECT_EQ(path + " not found or not accessible.", opencc_error());
  }
}

} // namespace opencc
