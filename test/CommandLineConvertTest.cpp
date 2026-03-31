/*
 * Open Chinese Convert
 *
 * Copyright 2015-2026 Carbo Kuo and contributors
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
#include <map>
#include <sstream>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "src/Common.hpp"
#include "rapidjson/document.h"
#include "gtest/gtest.h"

#ifdef BAZEL
#include "tools/cpp/runfiles/runfiles.h"
using bazel::tools::cpp::runfiles::Runfiles;
#endif

namespace {
// Cross-platform wrappers for POSIX directory functions
inline char* portable_getcwd() {
#ifdef _WIN32
  return _getcwd(nullptr, 0);
#else
  return getcwd(nullptr, 0);
#endif
}
inline int portable_chdir(const char* path) {
#ifdef _WIN32
  return _chdir(path);
#else
  return chdir(path);
#endif
}
} // namespace

namespace opencc {

class CommandLineConvertTest : public ::testing::Test {
protected:
  CommandLineConvertTest() { GetCurrentWorkingDirectory(); }

  virtual ~CommandLineConvertTest() { free(originalWorkingDirectory); }

  virtual void SetUp() {
#ifdef BAZEL
    runfiles_.reset(Runfiles::CreateForTest());
#else
    ASSERT_NE("", PROJECT_BINARY_DIR);
    ASSERT_NE("", CMAKE_SOURCE_DIR);
    ASSERT_EQ(0, portable_chdir(PROJECT_BINARY_DIR "/data"));
#endif
  }

  virtual void TearDown() { ASSERT_EQ(0, portable_chdir(originalWorkingDirectory)); }

  std::string GetFileContents(const std::string& fileName) const {
    std::ifstream fs(fileName);
    EXPECT_TRUE(fs.is_open()) << fileName;
    const std::string content((std::istreambuf_iterator<char>(fs)),
                              (std::istreambuf_iterator<char>()));
    fs.close();
    return content;
  }

  void GetCurrentWorkingDirectory() {
    originalWorkingDirectory = portable_getcwd();
  }

  std::string OpenccCommand() const {
#ifdef BAZEL
#ifdef _WIN32
    // On Windows, cc_binary executables have .exe extension in the runfiles manifest
    std::string path = runfiles_->Rlocation("_main/src/tools/command_line.exe");
    if (path.empty()) {
      path = runfiles_->Rlocation("_main/src/tools/command_line");
    }
    return path;
#else
    return runfiles_->Rlocation("_main/src/tools/command_line");
#endif
#else
#ifndef _MSC_VER
    return PROJECT_BINARY_DIR "/src/tools/opencc";
#else
#ifdef NDEBUG
    return PROJECT_BINARY_DIR "/src/tools/Release/opencc.exe";
#else
    return PROJECT_BINARY_DIR "/src/tools/Debug/opencc.exe";
#endif
#endif
#endif
  }

  // Quote a path for use in a shell command, in case the path contains spaces.
  static std::string QuotePath(const std::string& path) {
    return "\"" + path + "\"";
  }

  std::string OutputDirectory() const {
#ifdef BAZEL
    return ::testing::TempDir() + "/";
#else
    return PROJECT_BINARY_DIR "/test/";
#endif
  }

  std::string ConfigurationDirectory() const {
#ifdef BAZEL
    return "";
#else
    return CMAKE_SOURCE_DIR "/data/config/";
#endif
  }

  std::string InputFile(const char* config) const {
    return OutputDirectory() + config + ".in";
  }

  std::string OutputFile(const char* config) const {
    return OutputDirectory() + config + ".out";
  }

  std::string TestCommand(const std::string& config,
                          const std::string& inputFile,
                          const std::string& outputFile) const {
    std::string cmd = QuotePath(OpenccCommand()) + " -i " +
                      QuotePath(inputFile) + " -o " +
                      QuotePath(outputFile) + " -c " +
                      QuotePath(ConfigurationDirectory() + config + ".json");
#ifdef BAZEL
    cmd += " --path " + QuotePath(runfiles_->Rlocation("_main/data/dictionary") + "/") +
           " --path " + QuotePath(runfiles_->Rlocation("_main/data/config") + "/");
#endif
#ifdef _WIN32
    // On Windows, cmd.exe /C strips the first and last quote characters when
    // the command starts with a quoted path, corrupting the command. Wrapping
    // the entire command in an extra pair of outer quotes causes cmd.exe to
    // strip only those outer quotes, leaving the inner quoted paths intact.
    return "\"" + cmd + "\"";
#else
    return cmd;
#endif
  }

  char* originalWorkingDirectory;

#ifdef BAZEL
  std::unique_ptr<Runfiles> runfiles_;
#endif
};

struct CaseInput {
  std::string input;
  std::string expected;
};

using CasesByConfig = std::unordered_map<std::string, std::vector<CaseInput>>;

CasesByConfig LoadCases(const std::string& jsonPath) {
  CasesByConfig cases;
  std::string content;
  {
    std::ifstream ifs(jsonPath);
    if (!ifs.is_open()) {
      throw std::runtime_error("Cannot open " + jsonPath);
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    content = buffer.str();
  }

  rapidjson::Document doc;
  doc.Parse(content.c_str());
  if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember("cases") ||
      !doc["cases"].IsArray()) {
    throw std::runtime_error("Invalid testcases.json format");
  }

  for (auto& entry : doc["cases"].GetArray()) {
    if (!entry.IsObject() || !entry.HasMember("input") ||
        !entry["input"].IsString() || !entry.HasMember("expected") ||
        !entry["expected"].IsObject()) {
      continue;
    }
    const std::string input = entry["input"].GetString();
    for (auto itr = entry["expected"].MemberBegin();
         itr != entry["expected"].MemberEnd(); ++itr) {
      if (!itr->value.IsString()) {
        continue;
      }
      const std::string config = itr->name.GetString();
      cases[config].push_back({input, itr->value.GetString()});
    }
  }
  return cases;
}

TEST_F(CommandLineConvertTest, ConvertFromJson) {
#ifdef BAZEL
  const std::string casesPath =
      runfiles_->Rlocation("_main/test/testcases/testcases.json");
#else
  const std::string casesPath = CMAKE_SOURCE_DIR "/test/testcases/testcases.json";
#endif
  const CasesByConfig cases = LoadCases(casesPath);

  for (const auto& entry : cases) {
    const std::string& config = entry.first;
    const std::string inputFile = InputFile(config.c_str());
    const std::string outputFile = OutputFile(config.c_str());

    // Write inputs into a temp file (one per line).
    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open()) << "Failed to open input file for writing: "
                                 << inputFile;
      for (const auto& item : entry.second) {
        ofs << item.input << "\n";
      }
    }

    ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile).c_str()));

    // Read outputs and compare line by line.
    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string line;
    size_t idx = 0;
    while (std::getline(ifs, line)) {
      if (!line.empty() && line.back() == '\r') {
        line.pop_back(); // normalize Windows CRLF
      }
      ASSERT_LT(idx, entry.second.size());
      EXPECT_EQ(entry.second[idx].expected, line)
          << "config=" << config << " index=" << idx;
      idx++;
    }
    EXPECT_EQ(idx, entry.second.size()) << "config=" << config;
  }
}

} // namespace opencc
