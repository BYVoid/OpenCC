/*
 * Open Chinese Convert
 *
 * Copyright 2024-2026 Carbo Kuo and contributors
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

#ifdef BAZEL
#include "tools/cpp/runfiles/runfiles.h"
using bazel::tools::cpp::runfiles::Runfiles;
#endif

#include "test/PortableUtil.hpp"

namespace opencc {

class BazelOpenccTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
#ifdef BAZEL
    runfiles_.reset(Runfiles::CreateForTest());
    const std::string configFile =
        runfiles_->Rlocation("_main/data/config/s2t.json");
    configDir_ = configFile.substr(0, configFile.find_last_of("/\\"));
    const std::string dictFile =
        runfiles_->Rlocation("_main/data/dictionary/STCharacters.ocd2");
    dictDir_ = dictFile.substr(0, dictFile.find_last_of("/\\"));
#endif
  }

  void SetUp() override {
    // opencc_open() has no paths argument; set cwd to dictDir so
    // LoadDictWithPaths finds .ocd2 files, and OPENCC_DATA_DIR to configDir
    // so FindConfigFile finds .json files.
    const char* prev = std::getenv("OPENCC_DATA_DIR");
    savedOpenccDataDir_ = prev ? prev : "";
    portable_chdir(dictDir_.c_str());
    portable_putenv("OPENCC_DATA_DIR", configDir_.c_str());
  }

  void TearDown() override {
    if (savedOpenccDataDir_.empty()) {
      portable_unsetenv("OPENCC_DATA_DIR");
    } else {
      portable_putenv("OPENCC_DATA_DIR", savedOpenccDataDir_.c_str());
    }
  }

  static std::unique_ptr<Runfiles> runfiles_;
  static std::string configDir_;
  static std::string dictDir_;
  std::string savedOpenccDataDir_;
};

std::unique_ptr<Runfiles> BazelOpenccTest::runfiles_;
std::string BazelOpenccTest::configDir_;
std::string BazelOpenccTest::dictDir_;

TEST_F(BazelOpenccTest, SimpleConverter_s2t) {
  SimpleConverter converter(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD,
                            {configDir_, dictDir_});
  EXPECT_EQ(converter.Convert("简化字测试"), "簡化字測試");
}

TEST_F(BazelOpenccTest, SimpleConverter_t2s) {
  SimpleConverter converter(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP,
                            {configDir_, dictDir_});
  EXPECT_EQ(converter.Convert("簡化字測試"), "简化字测试");
}

TEST_F(BazelOpenccTest, CInterface_s2t) {
  std::string text = "简化字测试";
  opencc_t od = opencc_open(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
  ASSERT_NE(od, reinterpret_cast<opencc_t>(-1));
  char* converted = opencc_convert_utf8(od, text.c_str(), (size_t)-1);
  EXPECT_STREQ("簡化字測試", converted);
  opencc_convert_utf8_free(converted);
  EXPECT_EQ(0, opencc_close(od));
}

TEST_F(BazelOpenccTest, CInterface_t2s) {
  std::string text = "簡化字測試";
  opencc_t od = opencc_open(OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP);
  ASSERT_NE(od, reinterpret_cast<opencc_t>(-1));
  char* converted = opencc_convert_utf8(od, text.c_str(), (size_t)-1);
  EXPECT_STREQ("简化字测试", converted);
  opencc_convert_utf8_free(converted);
  EXPECT_EQ(0, opencc_close(od));
}

} // namespace opencc
