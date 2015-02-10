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
#include "Common.hpp"

namespace opencc {

class CommandLineConvertTest : public ::testing::Test {
protected:
  CommandLineConvertTest() { GetCurrentWorkingDirectory(); }

  virtual ~CommandLineConvertTest() { free(originalWorkingDirectory); }

  virtual void SetUp() {
    ASSERT_NE("", PROJECT_BINARY_DIR);
    ASSERT_NE("", CMAKE_SOURCE_DIR);
    ASSERT_EQ(0, chdir(PROJECT_BINARY_DIR "/data"));
  }

  virtual void TearDown() { ASSERT_EQ(0, chdir(originalWorkingDirectory)); }

  string GetFileContents(const string& fileName) const {
    std::ifstream fs(fileName);
    EXPECT_TRUE(fs.is_open());
    const string content((std::istreambuf_iterator<char>(fs)),
                         (std::istreambuf_iterator<char>()));
    fs.close();
    return content;
  }

  void GetCurrentWorkingDirectory() {
    originalWorkingDirectory = getcwd(nullptr, 0);
  }

  const char* OpenccCommand() const {
    return PROJECT_BINARY_DIR "/src/tools/opencc";
  }

  const char* InputDirectory() const {
    return CMAKE_SOURCE_DIR "/test/testcases/";
  }

  const char* OutputDirectory() const { return PROJECT_BINARY_DIR "/test/"; }

  const char* AnswerDirectory() const {
    return CMAKE_SOURCE_DIR "/test/testcases/";
  }

  const char* ConfigurationDirectory() const {
    return CMAKE_SOURCE_DIR "/data/config/";
  }

  string OutputFile(const char* config) const {
    return string(OutputDirectory()) + config + ".out";
  }

  string AnswerFile(const char* config) const {
    return string(AnswerDirectory()) + config + ".ans";
  }

  string TestCommand(const char* config) const {
    return OpenccCommand() + string("") + " -i " + InputDirectory() + config +
           ".in" + " -o " + OutputFile(config) + " -c " +
           ConfigurationDirectory() + config + ".json";
  }

  char* originalWorkingDirectory;
};

class ConfigurationTest : public CommandLineConvertTest,
                          public ::testing::WithParamInterface<const char*> {};

TEST_P(ConfigurationTest, Convert) {
  const char* config = GetParam();
  ASSERT_EQ(0, system(TestCommand(config).c_str()));
  const string& output = GetFileContents(OutputFile(config));
  const string& answer = GetFileContents(AnswerFile(config));
  ASSERT_EQ(answer, output);
}

INSTANTIATE_TEST_CASE_P(CommandLine, ConfigurationTest,
                        ::testing::Values("s2t", "t2s", "s2tw", "s2twp", "tw2s",
                                          "tw2sp", "s2hk", "hk2s"));

} // namespace opencc
