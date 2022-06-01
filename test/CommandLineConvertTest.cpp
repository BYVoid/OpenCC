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
#include <iostream>

#include "Common.hpp"
#include "gtest/gtest.h"

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

  std::string GetFileContents(const std::string& fileName) const {
    std::ifstream fs(fileName);
    EXPECT_TRUE(fs.is_open());
    const std::string content((std::istreambuf_iterator<char>(fs)),
                              (std::istreambuf_iterator<char>()));
    fs.close();
    return content;
  }

  void GetCurrentWorkingDirectory() {
    originalWorkingDirectory = getcwd(nullptr, 0);
  }

  const char* OpenccCommand() const {
#ifndef _MSC_VER
    return PROJECT_BINARY_DIR "/src/tools/opencc";
#else
#ifdef NDEBUG
    return PROJECT_BINARY_DIR "/src/tools/Release/opencc.exe";
#else
    return PROJECT_BINARY_DIR "/src/tools/Debug/opencc.exe";
#endif
#endif
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

  std::string InputFile(const char* config) const {
    return std::string(InputDirectory()) + config + ".in";
  }

  std::string OutputFile(const char* config) const {
    return std::string(OutputDirectory()) + config + ".out";
  }

  std::string AnswerFile(const char* config) const {
    return std::string(AnswerDirectory()) + config + ".ans";
  }

  std::string TestCommand(const char* config, const std::string& inputFile,
                          const std::string& outputFile) const {
    return OpenccCommand() + std::string("") + " -i " + inputFile + " -o " +
           outputFile + " -c " + ConfigurationDirectory() + config + ".json";
  }

  char* originalWorkingDirectory;
};

class ConfigurationTest : public CommandLineConvertTest,
                          public ::testing::WithParamInterface<const char*> {};

TEST_P(ConfigurationTest, Convert) {
  const char* config = GetParam();
  const std::string inputFile = InputFile(config);
  const std::string outputFile = OutputFile(config);
  ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile).c_str()));
  const std::string output = GetFileContents(OutputFile(config));
  const std::string answer = GetFileContents(AnswerFile(config));
  ASSERT_EQ(answer, output);
}

TEST_P(ConfigurationTest, InPlaceConvert) {
  const char* config = GetParam();
  // Copy input to output
  const std::string inputFile = InputFile(config);
  const std::string outputFile = OutputFile(config);
  std::ifstream source(inputFile, std::ios::binary);
  std::ofstream dest(outputFile, std::ios::binary);
  dest << source.rdbuf();
  source.close();
  dest.close();
  // Test in-place convert (same file)
  ASSERT_EQ(0, system(TestCommand(config, outputFile, outputFile).c_str()));
  const std::string output = GetFileContents(OutputFile(config));
  const std::string answer = GetFileContents(AnswerFile(config));
  ASSERT_EQ(answer, output);
}

INSTANTIATE_TEST_SUITE_P(CommandLine, ConfigurationTest,
                         ::testing::Values("hk2s", "hk2t", "jp2t", "s2hk",
                                           "s2t", "s2tw", "s2twp", "t2hk",
                                           "t2jp", "t2s", "tw2s", "tw2sp",
                                           "tw2t"));

} // namespace opencc
