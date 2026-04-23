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

#include "test/PortableUtil.hpp"

#include "src/Common.hpp"
#include "rapidjson/document.h"
#include "gtest/gtest.h"

#ifdef BAZEL
#include "tools/cpp/runfiles/runfiles.h"
using bazel::tools::cpp::runfiles::Runfiles;
#endif

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
                          const std::string& outputFile,
                          const std::string& measuredResultFile = "",
                          const std::string& extraFlags = "") const {
    std::string cmd = QuotePath(OpenccCommand()) + " -i " +
                      QuotePath(inputFile) + " -o " +
                      QuotePath(outputFile) + " -c " +
                      QuotePath(ConfigurationDirectory() + config + ".json");
    if (!extraFlags.empty()) {
      cmd += " " + extraFlags;
    }
    if (!measuredResultFile.empty()) {
      cmd += " --measured_result " + QuotePath(measuredResultFile);
    }
#ifdef BAZEL
    const std::string dictFile =
        runfiles_->Rlocation("_main/data/dictionary/STCharacters.ocd2");
    const std::string dictDir = dictFile.substr(0, dictFile.find_last_of("/\\"));
    const std::string configFile =
        runfiles_->Rlocation("_main/data/config/s2t.json");
    const std::string configDir =
        configFile.substr(0, configFile.find_last_of("/\\"));
    cmd += " --path " + QuotePath(dictDir + "/") +
           " --path " + QuotePath(configDir + "/");
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

  std::string TestCommandWithFlags(const std::string& config,
                                   const std::string& inputFile,
                                   const std::string& outputFile,
                                   const std::string& extraFlags,
                                   const std::string& measuredResultFile = "")
      const {
    return TestCommand(config, inputFile, outputFile, measuredResultFile,
                       extraFlags);
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

TEST_F(CommandLineConvertTest, WritesMeasuredResultJson) {
  const std::string config = "s2t";
  const std::string inputFile = InputFile(config.c_str()) + ".measured";
  const std::string outputFile = OutputFile(config.c_str()) + ".measured";
  const std::string measuredResultFile =
      OutputDirectory() + config + ".measured_result.json";

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open()) << "Failed to open input file for writing: "
                               << inputFile;
    ofs << "开放中文转换" << std::endl;
  }

  ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile,
                                  measuredResultFile).c_str()));

  const std::string content = GetFileContents(measuredResultFile);
  rapidjson::Document doc;
  doc.Parse(content.c_str());
  ASSERT_FALSE(doc.HasParseError());
  ASSERT_TRUE(doc.IsObject());
  ASSERT_TRUE(doc.HasMember("config"));
  ASSERT_TRUE(doc["config"].IsString());
  EXPECT_EQ(ConfigurationDirectory() + config + ".json",
            std::string(doc["config"].GetString()));
  ASSERT_TRUE(doc.HasMember("mode"));
  EXPECT_STREQ("file", doc["mode"].GetString());
  ASSERT_TRUE(doc.HasMember("load_ms"));
  EXPECT_TRUE(doc["load_ms"].IsNumber());
  ASSERT_TRUE(doc.HasMember("convert_ms"));
  EXPECT_TRUE(doc["convert_ms"].IsNumber());
  ASSERT_TRUE(doc.HasMember("write_ms"));
  EXPECT_TRUE(doc["write_ms"].IsNumber());
  ASSERT_TRUE(doc.HasMember("total_ms"));
  EXPECT_TRUE(doc["total_ms"].IsNumber());
  ASSERT_TRUE(doc.HasMember("input_bytes"));
  EXPECT_TRUE(doc["input_bytes"].IsUint64());
  ASSERT_TRUE(doc.HasMember("output_bytes"));
  EXPECT_TRUE(doc["output_bytes"].IsUint64());
}

TEST_F(CommandLineConvertTest, SegmentationOutputIsJson) {
  const std::string config = "s2t";
  const std::string inputFile = InputFile("segmentation_test");
  const std::string outputFile = OutputFile("segmentation_test");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "\xe5\xbc\x80\xe6\x94\xbe\xe4\xb8\xad\xe6\x96\x87\xe8\xbd\xac\xe6"
           "\x8d\xa2"  // 开放中文转换
        << "\n";
  }

  ASSERT_EQ(0, system(TestCommandWithFlags(config, inputFile, outputFile,
                                           "--segmentation").c_str()));

  const std::string content = GetFileContents(outputFile);
  rapidjson::Document doc;
  doc.Parse(content.c_str());
  ASSERT_FALSE(doc.HasParseError()) << "Output is not valid JSON: " << content;
  ASSERT_TRUE(doc.IsObject());
  ASSERT_TRUE(doc.HasMember("input"));
  ASSERT_TRUE(doc["input"].IsString());
  ASSERT_TRUE(doc.HasMember("segments"));
  ASSERT_TRUE(doc["segments"].IsArray());
  EXPECT_FALSE(doc["segments"].GetArray().Empty());
  // Should NOT have 'stages' or 'output' keys
  EXPECT_FALSE(doc.HasMember("stages"));
  EXPECT_FALSE(doc.HasMember("output"));
}

TEST_F(CommandLineConvertTest, InspectOutputIsJson) {
  const std::string config = "s2t";
  const std::string inputFile = InputFile("inspect_test");
  const std::string outputFile = OutputFile("inspect_test");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "\xe5\xbc\x80\xe6\x94\xbe\xe4\xb8\xad\xe6\x96\x87\xe8\xbd\xac\xe6"
           "\x8d\xa2"  // 开放中文转换
        << "\n";
  }

  ASSERT_EQ(0, system(TestCommandWithFlags(config, inputFile, outputFile,
                                           "--inspect").c_str()));

  const std::string content = GetFileContents(outputFile);
  rapidjson::Document doc;
  doc.Parse(content.c_str());
  ASSERT_FALSE(doc.HasParseError()) << "Output is not valid JSON: " << content;
  ASSERT_TRUE(doc.IsObject());
  ASSERT_TRUE(doc.HasMember("input"));
  ASSERT_TRUE(doc["input"].IsString());
  ASSERT_TRUE(doc.HasMember("segments"));
  ASSERT_TRUE(doc["segments"].IsArray());
  ASSERT_TRUE(doc.HasMember("stages"));
  ASSERT_TRUE(doc["stages"].IsArray());
  ASSERT_TRUE(doc.HasMember("output"));
  ASSERT_TRUE(doc["output"].IsString());
  // Validate stage schema
  for (auto& stage : doc["stages"].GetArray()) {
    ASSERT_TRUE(stage.IsObject());
    ASSERT_TRUE(stage.HasMember("index"));
    ASSERT_TRUE(stage["index"].IsUint64());
    ASSERT_TRUE(stage.HasMember("segments"));
    ASSERT_TRUE(stage["segments"].IsArray());
  }
}

TEST_F(CommandLineConvertTest, SegmentationAndInspectAreMutuallyExclusive) {
  const std::string config = "s2t";
  const std::string inputFile = InputFile("mutex_test");
  const std::string outputFile = OutputFile("mutex_test");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "test\n";
  }

  // Should fail with non-zero exit code
  const int exitCode =
      system(TestCommandWithFlags(config, inputFile, outputFile,
                                  "--segmentation --inspect")
                 .c_str());
  EXPECT_NE(0, exitCode);
}

TEST_F(CommandLineConvertTest, MeasuredResultIncludesOutputMode) {
  const std::string config = "s2t";
  const std::string inputFile = InputFile("inspect_measured_test");
  const std::string outputFile = OutputFile("inspect_measured_test");
  const std::string measuredResultFile =
      OutputDirectory() + "inspect_measured_result.json";

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "\xe5\xbc\x80\xe6\x94\xbe\xe4\xb8\xad\xe6\x96\x87\xe8\xbd\xac\xe6"
           "\x8d\xa2"  // 开放中文转换
        << "\n";
  }

  ASSERT_EQ(0,
            system(TestCommandWithFlags(config, inputFile, outputFile,
                                        "--inspect", measuredResultFile)
                       .c_str()));

  const std::string content = GetFileContents(measuredResultFile);
  rapidjson::Document doc;
  doc.Parse(content.c_str());
  ASSERT_FALSE(doc.HasParseError());
  ASSERT_TRUE(doc.IsObject());
  ASSERT_TRUE(doc.HasMember("output_mode"));
  ASSERT_TRUE(doc["output_mode"].IsString());
  EXPECT_STREQ("inspect", doc["output_mode"].GetString());
}

// Verify --segmentation does NOT run the conversion chain: the segments in the
// output must be the raw segmenter tokens, not post-conversion text.
// With s2t.json on "开放中文转换", segmentation splits into simplified-Chinese
// tokens; none of those tokens should already be Traditional Chinese unless
// the conversion chain ran.
TEST_F(CommandLineConvertTest, SegmentationDoesNotRunConversionChain) {
  const std::string config = "s2t";
  const std::string inputFile = InputFile("seg_no_convert_test");
  const std::string outputFile = OutputFile("seg_no_convert_test");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    // 开放中文转换 — all simplified; after s2t conversion it would become
    // 開放中文轉換.  --segmentation must return the simplified tokens.
    ofs << "\xe5\xbc\x80\xe6\x94\xbe\xe4\xb8\xad\xe6\x96\x87\xe8\xbd\xac\xe6"
           "\x8d\xa2"  // 开放中文转换
        << "\n";
  }

  ASSERT_EQ(0, system(TestCommandWithFlags(config, inputFile, outputFile,
                                           "--segmentation").c_str()));

  const std::string content = GetFileContents(outputFile);
  rapidjson::Document doc;
  doc.Parse(content.c_str());
  ASSERT_FALSE(doc.HasParseError());
  ASSERT_TRUE(doc.IsObject());
  ASSERT_TRUE(doc.HasMember("segments"));
  ASSERT_TRUE(doc["segments"].IsArray());

  // Reconstruct the concatenation of all segments.
  std::string joined;
  for (auto& seg : doc["segments"].GetArray()) {
    ASSERT_TRUE(seg.IsString());
    joined += seg.GetString();
  }
  // The joined segments must equal the *original* simplified input,
  // not the converted traditional output.
  const std::string simplified =
      "\xe5\xbc\x80\xe6\x94\xbe\xe4\xb8\xad\xe6\x96\x87\xe8\xbd\xac\xe6"
      "\x8d\xa2";  // 开放中文转换
  EXPECT_EQ(simplified, joined)
      << "Segments were converted — conversion chain must not run in "
         "--segmentation mode";
}

// Verify that a multi-line input in --segmentation mode produces exactly one
// JSON object per input line with no spurious extra record at EOF.
TEST_F(CommandLineConvertTest, SegmentationNoSpuriousEofRecord) {
  const std::string config = "s2t";
  const std::string inputFile = InputFile("seg_eof_test");
  const std::string outputFile = OutputFile("seg_eof_test");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    // Two lines, each terminated by \n (file ends with a newline).
    ofs << "\xe5\xbc\x80\xe6\x94\xbe\xe4\xb8\xad\xe6\x96\x87\xe8\xbd\xac\xe6"
           "\x8d\xa2\n"  // 开放中文转换
        << "\xe6\xb1\x89\xe5\xad\x97\n";  // 汉字
  }

  ASSERT_EQ(0, system(TestCommandWithFlags(config, inputFile, outputFile,
                                           "--segmentation").c_str()));

  // Output should be exactly two JSON objects separated by a newline.
  const std::string content = GetFileContents(outputFile);
  // Count newlines — the format is "obj\nobj" so there should be exactly 1
  // newline separator for 2 records.
  size_t newlines = 0;
  for (char c : content) {
    if (c == '\n') {
      ++newlines;
    }
  }
  // Allow at most one trailing newline; the important thing is that we have
  // exactly two JSON objects.
  const size_t separators = (newlines > 0 && content.back() == '\n')
                                ? newlines - 1
                                : newlines;
  EXPECT_EQ(1u, separators) << "Expected exactly 2 JSON records (1 separator),"
                                " got extra records. Content: "
                             << content;

  // Also parse both records to confirm they're valid JSON.
  // Records are separated by exactly one '\n' with no trailing newline in the
  // payload itself.
  size_t sep = content.find('\n');
  ASSERT_NE(std::string::npos, sep) << "No separator found";
  std::string first = content.substr(0, sep);
  // skip trailing newline if present
  std::string rest = content.substr(sep + 1);
  if (!rest.empty() && rest.back() == '\n') {
    rest.pop_back();
  }

  rapidjson::Document d1, d2;
  d1.Parse(first.c_str());
  EXPECT_FALSE(d1.HasParseError()) << "First record invalid JSON: " << first;
  EXPECT_TRUE(d1.IsObject());
  d2.Parse(rest.c_str());
  EXPECT_FALSE(d2.HasParseError()) << "Second record invalid JSON: " << rest;
  EXPECT_TRUE(d2.IsObject());
}
} // namespace opencc
