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

#include <fstream>

#include "Config.hpp"
#include "ConfigTestBase.hpp"
#include "Converter.hpp"
#include "Exception.hpp"
#include "TestUtilsUTF8.hpp"

namespace opencc {

class ConfigTest : public ConfigTestBase {
protected:
  ConfigTest()
      : input(utf8("燕燕于飞差池其羽之子于归远送于野")),
        expected(utf8("燕燕于飛差池其羽之子于歸遠送於野")) {}

  virtual void SetUp() { converter = config.NewFromFile(CONFIG_TEST_PATH); }

  Config config;
  ConverterPtr converter;
  const std::string input;
  const std::string expected;
};

TEST_F(ConfigTest, Convert) {
  const std::string& converted = converter->Convert(input);
  EXPECT_EQ(expected, converted);
}

TEST_F(ConfigTest, ConvertBuffer) {
  char output[1024];
  const size_t length = converter->Convert(input.c_str(), output);
  EXPECT_EQ(expected.length(), length);
  EXPECT_EQ(expected, output);
}

TEST_F(ConfigTest, NonexistingPath) {
  const std::string path = "/opencc/no/such/file/or/directory";
  try {
    const ConverterPtr _ = config.NewFromFile(path);
  } catch (FileNotFound& e) {
    EXPECT_EQ(path + " not found or not accessible.", e.what());
  }
}

TEST_F(ConfigTest, NewFromStringWitoutTrailingSlash) {
  std::ifstream ifs(CONFIG_TEST_PATH);
  std::string content(std::istreambuf_iterator<char>(ifs),
                      (std::istreambuf_iterator<char>()));
  std::string pathWithoutTrailingSlash = CMAKE_SOURCE_DIR "/test/config_test";

  const ConverterPtr _ =
      config.NewFromString(content, pathWithoutTrailingSlash);
}

} // namespace opencc
