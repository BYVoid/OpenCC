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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "test/PortableUtil.hpp"

#include "src/Common.hpp"
#include "rapidjson/document.h"
#include "gtest/gtest.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#ifdef BAZEL
#include "tools/cpp/runfiles/runfiles.h"
using bazel::tools::cpp::runfiles::Runfiles;
#endif

namespace opencc {

namespace fs = std::filesystem;

#ifdef _WIN32
std::wstring WideFromUtf8(const std::string& utf8) {
  if (utf8.empty()) {
    return L"";
  }
  const int required =
      MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
  if (required <= 1) {
    return L"";
  }
  std::wstring wide(static_cast<size_t>(required), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), required);
  wide.resize(static_cast<size_t>(required - 1));
  return wide;
}
#endif

class CommandLineConvertTest : public ::testing::Test {
protected:
  CommandLineConvertTest() { GetCurrentWorkingDirectory(); }

  virtual ~CommandLineConvertTest() { free(originalWorkingDirectory); }

  virtual void SetUp() {
#ifdef BAZEL
    runfiles_.reset(Runfiles::CreateForTest());
#else
    ASSERT_NE("", PROJECT_BINARY_DIR);
    ASSERT_NE("", PROJECT_SOURCE_DIR);
    ASSERT_EQ(0, portable_chdir(PROJECT_BINARY_DIR "/data"));
#endif
  }

  virtual void TearDown() { ASSERT_EQ(0, portable_chdir(originalWorkingDirectory)); }

  std::string GetFileContents(const std::string& fileName) const {
    std::ifstream fs(fileName, std::ios::binary);
    EXPECT_TRUE(fs.is_open()) << fileName;
    const std::string content((std::istreambuf_iterator<char>(fs)),
                              (std::istreambuf_iterator<char>()));
    fs.close();
    return content;
  }

  std::string GetFileContents(const fs::path& fileName) const {
    std::ifstream stream(fileName, std::ios::binary);
    EXPECT_TRUE(stream.is_open()) << fileName.u8string();
    const std::string content((std::istreambuf_iterator<char>(stream)),
                              (std::istreambuf_iterator<char>()));
    stream.close();
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

#if defined(BAZEL) && defined(OPENCC_ENABLE_RESOURCE_ZIP_TEST)
  std::string ResourceZipFile() const {
    return runfiles_->Rlocation("_main/data/opencc-resources.zip");
  }

  std::string ResourceOcd2ZipFile() const {
    return runfiles_->Rlocation("_main/data/opencc-resources-ocd2.zip");
  }
#endif

  std::string ConfigurationDirectory() const {
#ifdef BAZEL
    return "";
#else
    return PROJECT_SOURCE_DIR "/data/config/";
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
    const std::string dictDir =
        dictFile.substr(0, dictFile.find_last_of("/\\"));
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

  static int RunCommand(const std::string& cmd) {
#ifdef _WIN32
    return _wsystem(WideFromUtf8(cmd).c_str());
#else
    return system(cmd.c_str());
#endif
  }

#if defined(BAZEL) && defined(OPENCC_ENABLE_RESOURCE_ZIP_TEST)
  std::string TestResourceZipCommand(const std::string& config,
                                     const std::string& inputFile,
                                     const std::string& outputFile) const {
    std::string cmd = QuotePath(OpenccCommand()) + " -i " +
                      QuotePath(inputFile) + " -o " + QuotePath(outputFile) +
                      " -c " + QuotePath(config + ".json") +
                      " --resource-zip " + QuotePath(ResourceZipFile());
#ifdef _WIN32
    return "\"" + cmd + "\"";
#else
    return cmd;
#endif
  }
#endif

  std::string TestCommandWithFlags(const std::string& config,
                                   const std::string& inputFile,
                                   const std::string& outputFile,
                                   const std::string& extraFlags,
                                   const std::string& measuredResultFile = "")
      const {
    return TestCommand(config, inputFile, outputFile, measuredResultFile,
                       extraFlags);
  }

  std::string TestStdinCommand(const std::string& config,
                               const std::string& inputFile,
                               const std::string& outputFile) const {
    std::string cmd = QuotePath(OpenccCommand()) + " -c " +
                      QuotePath(ConfigurationDirectory() + config + ".json");
#ifdef BAZEL
    const std::string dictFile =
        runfiles_->Rlocation("_main/data/dictionary/STCharacters.ocd2");
    const std::string dictDir =
        dictFile.substr(0, dictFile.find_last_of("/\\"));
    const std::string configFile =
        runfiles_->Rlocation("_main/data/config/s2t.json");
    const std::string configDir =
        configFile.substr(0, configFile.find_last_of("/\\"));
    cmd += " --path " + QuotePath(dictDir + "/") +
           " --path " + QuotePath(configDir + "/");
#endif
    cmd += " < " + QuotePath(inputFile) + " > " + QuotePath(outputFile);
#ifdef _WIN32
    return "\"" + cmd + "\"";
#else
    return cmd;
#endif
  }

#ifndef _WIN32
  std::string TestPipeCommand(const std::string& config,
                              const std::string& outputFile) const {
    std::string cmd = "(printf '后台'; sleep 0.1; printf '老板') | " +
                      QuotePath(OpenccCommand()) + " -c " +
                      QuotePath(ConfigurationDirectory() + config + ".json");
#ifdef BAZEL
    const std::string dictFile =
        runfiles_->Rlocation("_main/data/dictionary/STCharacters.ocd2");
    const std::string dictDir =
        dictFile.substr(0, dictFile.find_last_of("/\\"));
    const std::string configFile =
        runfiles_->Rlocation("_main/data/config/s2t.json");
    const std::string configDir =
        configFile.substr(0, configFile.find_last_of("/\\"));
    cmd += " --path " + QuotePath(dictDir + "/") +
           " --path " + QuotePath(configDir + "/");
#endif
    cmd += " > " + QuotePath(outputFile);
    return cmd;
  }
#endif

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
  doc.Parse<rapidjson::kParseCommentsFlag |
            rapidjson::kParseTrailingCommasFlag>(content.c_str());
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
  const std::string casesPath = PROJECT_SOURCE_DIR "/test/testcases/testcases.json";
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

    ASSERT_EQ(0,
              system(TestCommandWithFlags(
                         config, inputFile, outputFile,
                         "--include-tofu-risk-dictionaries")
                         .c_str()));

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

TEST_F(CommandLineConvertTest, SkipsTofuRiskDictionariesByDefault) {
  const std::string inputFile = InputFile("tofu_risk_default");
  const std::string outputFile = OutputFile("tofu_risk_default");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "㑮";
  }

  ASSERT_EQ(0, system(TestCommand("t2s", inputFile, outputFile).c_str()));
  EXPECT_EQ("㑮", GetFileContents(outputFile));
}

TEST_F(CommandLineConvertTest, IncludeTofuRiskDictionariesFlagRestoresLegacy) {
  const std::string inputFile = InputFile("tofu_risk_include");
  const std::string outputFile = OutputFile("tofu_risk_include");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "㑮";
  }

  ASSERT_EQ(0, system(TestCommandWithFlags(
                   "t2s", inputFile, outputFile,
                   "--include-tofu-risk-dictionaries")
                   .c_str()));
  EXPECT_EQ("𫝈", GetFileContents(outputFile));
}

#if defined(BAZEL) && defined(OPENCC_ENABLE_RESOURCE_ZIP_TEST)
TEST_F(CommandLineConvertTest, ResourceZipConvertsWithoutResourcePaths) {
  const std::string inputFile = InputFile("resource_zip");
  const std::string outputFile = OutputFile("resource_zip");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "打印机和鼠标";
  }

  ASSERT_EQ(0,
            RunCommand(TestResourceZipCommand("s2twp", inputFile, outputFile)));
  EXPECT_EQ("印表機和滑鼠", GetFileContents(outputFile));
}

TEST_F(CommandLineConvertTest, ResourceOcd2ZipConvertsWithoutResourcePaths) {
  const std::string inputFile = InputFile("resource_ocd2_zip");
  const std::string outputFile = OutputFile("resource_ocd2_zip");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "打印机和鼠标";
  }

  std::string cmd = QuotePath(OpenccCommand()) + " -i " +
                    QuotePath(inputFile) + " -o " + QuotePath(outputFile) +
                    " -c s2twp.json" +
                    " --resource-zip " + QuotePath(ResourceOcd2ZipFile());
#ifdef _WIN32
  cmd = "\"" + cmd + "\"";
#endif

  ASSERT_EQ(0, RunCommand(cmd));
  EXPECT_EQ("印表機和滑鼠", GetFileContents(outputFile));
}
#endif

TEST_F(CommandLineConvertTest, AmbiguitiesEmitsDefineOnFirstUseRecords) {
  const std::string inputFile = InputFile("ambiguities_records");
  const std::string recordsFile = OutputFile("ambiguities_records");
  const std::string convertFile = OutputFile("ambiguities_convert");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    // 文丑 (STPhrases 文丑 -> 文丑 文醜) appears twice so the second
    // reference must reuse the first definition; 头发/干燥 are single-value
    // phrase conversions and must not be flagged.
    ofs << "文丑与文丑相争，头发干燥";
  }

  ASSERT_EQ(0, system(TestCommand("s2t", inputFile, convertFile).c_str()));
  ASSERT_EQ(0, system(TestCommandWithFlags("s2t", inputFile, recordsFile,
                                           "--ambiguities")
                          .c_str()));

  std::istringstream records(GetFileContents(recordsFile));
  std::string line;
  std::string reconstructed;
  std::vector<std::string> defs;
  std::vector<uint64_t> ambIndexes;
  bool sawEnd = false;
  while (std::getline(records, line)) {
    if (line.empty()) {
      continue;
    }
    ASSERT_FALSE(sawEnd) << "record after end record: " << line;
    rapidjson::Document doc;
    doc.Parse(line.c_str());
    ASSERT_FALSE(doc.HasParseError()) << line;
    ASSERT_TRUE(doc.IsObject()) << line;
    if (doc.HasMember("def")) {
      ASSERT_TRUE(doc["def"].IsString());
      defs.push_back(doc["def"].GetString());
    } else if (doc.HasMember("lit")) {
      ASSERT_TRUE(doc["lit"].IsString());
      reconstructed += doc["lit"].GetString();
    } else if (doc.HasMember("amb")) {
      ASSERT_TRUE(doc["amb"].IsObject());
      ASSERT_TRUE(doc["amb"].HasMember("t") && doc["amb"]["t"].IsString())
          << line;
      ASSERT_TRUE(doc["amb"].HasMember("s") && doc["amb"]["s"].IsUint64())
          << line;
      // Define-on-first-use contract: every reference points at an
      // already-defined source index.
      ASSERT_LT(doc["amb"]["s"].GetUint64(), defs.size()) << line;
      reconstructed += doc["amb"]["t"].GetString();
      ambIndexes.push_back(doc["amb"]["s"].GetUint64());
    } else if (doc.HasMember("end")) {
      sawEnd = true;
      EXPECT_EQ(reconstructed.size(),
                doc["end"]["output_bytes"].GetUint64());
      EXPECT_EQ(ambIndexes.size(), doc["end"]["ambiguities"].GetUint64());
      EXPECT_EQ(defs.size(), doc["end"]["sources"].GetUint64());
    } else {
      FAIL() << "unknown record: " << line;
    }
  }
  EXPECT_TRUE(sawEnd);
  // Interleaved records reconstruct exactly the plain conversion output.
  EXPECT_EQ(GetFileContents(convertFile), reconstructed);
  // 文丑 appears twice in the input: dedup means exactly one def, and both
  // occurrences reference it.  Invariant-style, so unrelated dictionary
  // updates cannot break this test.
  ASSERT_EQ(1, std::count(defs.begin(), defs.end(), "文丑"));
  const uint64_t wenchouIndex = static_cast<uint64_t>(
      std::find(defs.begin(), defs.end(), "文丑") - defs.begin());
  EXPECT_GE(std::count(ambIndexes.begin(), ambIndexes.end(), wenchouIndex),
            2);
}

TEST_F(CommandLineConvertTest, AmbiguitiesCoversMultiStageChain) {
  const std::string inputFile = InputFile("ambiguities_s2twp");
  const std::string recordsFile = OutputFile("ambiguities_s2twp");
  const std::string convertFile = OutputFile("ambiguities_s2twp_convert");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    // s2twp is a multi-stage chain: 下面 is one-to-many in stage one
    // (STPhrases 下面 -> 下面 下麪) while 信号 becomes one-to-many in the
    // regional stage (TWPhrases 信號 -> 訊號 信號), so its span must map
    // back through the stage alignment to the Simplified input slice.
    // 软件 converts unambiguously (軟件 -> 軟體) and must not be flagged.
    ofs << "下面是软件的信号";
  }

  ASSERT_EQ(0, system(TestCommand("s2twp", inputFile, convertFile).c_str()));
  ASSERT_EQ(0, system(TestCommandWithFlags("s2twp", inputFile, recordsFile,
                                           "--ambiguities")
                          .c_str()));

  std::istringstream records(GetFileContents(recordsFile));
  std::string line;
  std::string reconstructed;
  std::vector<std::string> defs;
  std::vector<std::string> ambSources;
  while (std::getline(records, line)) {
    if (line.empty()) {
      continue;
    }
    rapidjson::Document doc;
    doc.Parse(line.c_str());
    ASSERT_FALSE(doc.HasParseError()) << line;
    if (doc.HasMember("def")) {
      defs.push_back(doc["def"].GetString());
    } else if (doc.HasMember("lit")) {
      reconstructed += doc["lit"].GetString();
    } else if (doc.HasMember("amb")) {
      const size_t index = doc["amb"]["s"].GetUint64();
      ASSERT_LT(index, defs.size()) << line;
      ambSources.push_back(defs[index]);
      reconstructed += doc["amb"]["t"].GetString();
    }
  }
  EXPECT_EQ(GetFileContents(convertFile), reconstructed);
  // Contains-style assertions so unrelated dictionary updates (new
  // multi-value entries elsewhere in the sentence) cannot break the test.
  EXPECT_NE(defs.end(), std::find(defs.begin(), defs.end(), "下面"));
  EXPECT_NE(defs.end(), std::find(defs.begin(), defs.end(), "信号"));
  EXPECT_NE(ambSources.end(),
            std::find(ambSources.begin(), ambSources.end(), "下面"));
  EXPECT_NE(ambSources.end(),
            std::find(ambSources.begin(), ambSources.end(), "信号"));
}

TEST_F(CommandLineConvertTest, AmbiguitiesRejectsInvalidUtf8) {
  // Length-based walking tolerates a multi-byte lead byte with invalid
  // continuation bytes; the record writer validates encoding so such input
  // aborts the stream (non-zero exit, no end record) instead of emitting
  // JSON that strict consumers reject. Plain conversion stays
  // byte-transparent and is unaffected.
  const std::string inputFile = InputFile("ambiguities_invalid_utf8");
  const std::string recordsFile = OutputFile("ambiguities_invalid_utf8");
  const std::string convertFile = OutputFile("ambiguities_invalid_utf8_conv");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "abc\xE4\x41\x41zzz";
  }

  // The same input converts fine in byte-transparent convert mode, pinning
  // the failure below to the record writer's encoding validation rather
  // than some earlier stage.
  ASSERT_EQ(0, RunCommand(TestCommand("s2t", inputFile, convertFile)));
  ASSERT_NE(0, RunCommand(TestCommandWithFlags("s2t", inputFile, recordsFile,
                                               "--ambiguities")));
  EXPECT_EQ(std::string::npos, GetFileContents(recordsFile).find("\"end\""));
}

TEST_F(CommandLineConvertTest, AmbiguitiesInPlaceInvalidUtf8KeepsOriginal) {
  // The record writer throws on invalid UTF-8; with --in-place the
  // exception must abort before the temp-file replacement so the user's
  // original file survives intact, and the temp file must be cleaned up.
  // A dedicated directory makes the no-residue assertion exact.
  const fs::path dir =
      fs::u8path(OutputDirectory()) / fs::u8path("ambiguities_inplace_bad");
  fs::create_directories(dir);
  const fs::path file = dir / fs::u8path("input.txt");
  const std::string original = "abc\xE4\x41\x41zzz";

  {
    std::ofstream ofs(file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << original;
  }

  ASSERT_NE(0, RunCommand(TestCommand("s2t", file.u8string(),
                                      file.u8string(), "",
                                      "--ambiguities --in-place")));
  EXPECT_EQ(original, GetFileContents(file));
  size_t entries = 0;
  for (const auto& entry : fs::directory_iterator(dir)) {
    (void)entry;
    entries++;
  }
  EXPECT_EQ(1u, entries) << "temporary file left behind";
}

TEST_F(CommandLineConvertTest, AmbiguitiesInPlaceRewritesFile) {
  // Regression: the --ambiguities branch used to early-return from
  // ConvertFileStreams, skipping the fclose epilogue; --in-place then
  // replaced the output file while both streams were still open (fails on
  // Windows, risks unflushed data elsewhere).
  const std::string file = OutputFile("ambiguities_inplace");
  {
    std::ofstream ofs(file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "文丑";
  }

  ASSERT_EQ(0, RunCommand(
                   TestCommand("s2t", file, file, "", "--ambiguities --in-place")));
  const std::string contents = GetFileContents(file);
  EXPECT_NE(std::string::npos, contents.find("{\"def\":\"文丑\"}")) << contents;
  EXPECT_NE(std::string::npos, contents.find("\"end\"")) << contents;
}

TEST_F(CommandLineConvertTest, StdinPreservesLineEndingsAndUnknownCharacters) {
  const std::string config = "s2t";
  const std::string inputFile = InputFile("stdin_line_endings");
  const std::string outputFile = OutputFile("stdin_line_endings");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "鼠标=mouse\r\n123\n未登录";
  }

  ASSERT_EQ(0, system(TestStdinCommand(config, inputFile, outputFile).c_str()));
  EXPECT_EQ("鼠標=mouse\r\n123\n未登錄", GetFileContents(outputFile));
}

#ifndef _WIN32
TEST_F(CommandLineConvertTest, WarnsWhenInputContainsVariationSelector) {
  const std::string inputFile = InputFile("ivs_warning");
  const std::string outputFile = OutputFile("ivs_warning");
  const std::string stderrFile = OutputFile("ivs_warning.stderr");
  const std::string variationSelector = "\xF3\xA0\x84\x80";

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "汉禰" << variationSelector;
  }

  const std::string command =
      TestCommand("s2t", inputFile, outputFile) + " 2> " +
      QuotePath(stderrFile);
  ASSERT_EQ(0, system(command.c_str()));
  EXPECT_EQ("漢禰" + variationSelector, GetFileContents(outputFile));
  EXPECT_NE(std::string::npos,
            GetFileContents(stderrFile).find(
                "warning: input contains Unicode variation selectors"));
}

TEST_F(CommandLineConvertTest,
       WarnsWhenSupplementaryVariationSelectorCrossesChunkBoundary) {
  const std::string inputFile = InputFile("ivs_warning_chunk_boundary");
  const std::string outputFile = OutputFile("ivs_warning_chunk_boundary");
  const std::string stderrFile =
      OutputFile("ivs_warning_chunk_boundary.stderr");

  std::string input(1048576 - 1, 'a');
  input.push_back(static_cast<char>(0xF3));
  input.push_back(static_cast<char>(0xA0));
  input.push_back(static_cast<char>(0x84));
  input.push_back(static_cast<char>(0x80));

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << input;
  }

  const std::string command =
      TestCommand("s2t", inputFile, outputFile) + " 2> " +
      QuotePath(stderrFile);
  ASSERT_EQ(0, system(command.c_str()));
  EXPECT_EQ(input, GetFileContents(outputFile));
  EXPECT_NE(std::string::npos,
            GetFileContents(stderrFile).find(
                "warning: input contains Unicode variation selectors"));
}
#endif

#ifndef _WIN32
TEST_F(CommandLineConvertTest, PipeShortReadContinuesUntilEof) {
  const std::string outputFile = OutputFile("pipe_short_read");

  ASSERT_EQ(0, system(TestPipeCommand("s2t", outputFile).c_str()));

  ASSERT_EQ("後臺老闆", GetFileContents(outputFile));
}
#endif

TEST_F(CommandLineConvertTest, ConvertsFilesWithUnicodePaths) {
  const fs::path unicodeDir =
      fs::u8path(OutputDirectory()) / fs::u8path("opencc-中文路径-cli-😀-𠮷");
  fs::create_directories(unicodeDir);

  const fs::path inputFile = unicodeDir / fs::u8path("输入 文件.txt");
  const fs::path outputFile = unicodeDir / fs::u8path("輸出 文件.txt");
  const fs::path measuredResultFile =
      unicodeDir / fs::u8path("測量 結果.json");

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open()) << inputFile.u8string();
    ofs << "开放中文转换";
  }

  ASSERT_EQ(0, RunCommand(TestCommand("s2t", inputFile.u8string(),
                                      outputFile.u8string(),
                                      measuredResultFile.u8string())));
  EXPECT_EQ("開放中文轉換", GetFileContents(outputFile));

  const std::string measured = GetFileContents(measuredResultFile);
  rapidjson::Document doc;
  doc.Parse(measured.c_str());
  ASSERT_FALSE(doc.HasParseError()) << measured;
  ASSERT_TRUE(doc.IsObject());
  ASSERT_TRUE(doc.HasMember("input"));
  EXPECT_EQ(inputFile.u8string(), std::string(doc["input"].GetString()));
}

TEST_F(CommandLineConvertTest, ConvertsInPlaceWithUnicodePath) {
  const fs::path unicodeDir =
      fs::u8path(OutputDirectory()) / fs::u8path("opencc-就地转换-cli-😀");
  fs::create_directories(unicodeDir);

  const fs::path file = unicodeDir / fs::u8path("输入输出同一文件.txt");

  {
    std::ofstream ofs(file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open()) << file.u8string();
    ofs << "开放中文转换";
  }

  ASSERT_EQ(0, RunCommand(TestCommand("s2t", file.u8string(), file.u8string(),
                                      "", "--in-place")));
  EXPECT_EQ("開放中文轉換", GetFileContents(file));
}

TEST_F(CommandLineConvertTest, RejectsInPlaceConversionWithoutFlag) {
  const fs::path unicodeDir =
      fs::u8path(OutputDirectory()) / fs::u8path("opencc-拒绝就地转换-cli");
  fs::create_directories(unicodeDir);

  const fs::path file = unicodeDir / fs::u8path("输入输出同一文件.txt");

  {
    std::ofstream ofs(file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open()) << file.u8string();
    ofs << "开放中文转换";
  }

  EXPECT_NE(0, RunCommand(TestCommand("s2t", file.u8string(), file.u8string())));
  EXPECT_EQ("开放中文转换", GetFileContents(file));
}

TEST_F(CommandLineConvertTest, MissingInputDoesNotTruncateOutput) {
  const fs::path unicodeDir =
      fs::u8path(OutputDirectory()) / fs::u8path("opencc-缺失输入-cli");
  fs::create_directories(unicodeDir);

  const fs::path inputFile = unicodeDir / fs::u8path("不存在.txt");
  const fs::path outputFile = unicodeDir / fs::u8path("已有输出.txt");

  {
    std::ofstream ofs(outputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open()) << outputFile.u8string();
    ofs << "existing output";
  }

  EXPECT_NE(0, RunCommand(TestCommand("s2t", inputFile.u8string(),
                                      outputFile.u8string())));
  EXPECT_EQ("existing output", GetFileContents(outputFile));
}

TEST_F(CommandLineConvertTest, ConvertsInPlaceWithDifferentPathSpellings) {
  const fs::path unicodeDir =
      fs::u8path(OutputDirectory()) / fs::u8path("opencc-同文件不同路径-cli");
  fs::create_directories(unicodeDir);

  const fs::path file = unicodeDir / fs::u8path("输入输出同一文件.txt");

  {
    std::ofstream ofs(file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open()) << file.u8string();
    ofs << "开放中文转换";
  }

  const fs::path spelledDifferently = unicodeDir / fs::u8path(".") /
                                      fs::u8path("输入输出同一文件.txt");
  ASSERT_EQ(0, RunCommand(TestCommand("s2t", file.u8string(),
                                      spelledDifferently.u8string(), "",
                                      "--in-place")));
  EXPECT_EQ("開放中文轉換", GetFileContents(file));
}

#ifndef _WIN32
TEST_F(CommandLineConvertTest, InPlaceConversionPreservesFileMode) {
  const fs::path unicodeDir =
      fs::u8path(OutputDirectory()) / fs::u8path("opencc-保留权限-cli");
  fs::create_directories(unicodeDir);

  const fs::path file = unicodeDir / fs::u8path("输入输出同一文件.txt");

  {
    std::ofstream ofs(file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open()) << file.u8string();
    ofs << "开放中文转换";
  }
  ASSERT_EQ(0, chmod(file.c_str(), 0644));

  ASSERT_EQ(0, RunCommand(TestCommand("s2t", file.u8string(), file.u8string(),
                                      "", "--in-place")));
  EXPECT_EQ("開放中文轉換", GetFileContents(file));

  struct stat info;
  ASSERT_EQ(0, stat(file.c_str(), &info));
  EXPECT_EQ(0644, info.st_mode & 07777);
}

TEST_F(CommandLineConvertTest, InPlaceConversionRejectsHardLinks) {
  const fs::path unicodeDir =
      fs::u8path(OutputDirectory()) / fs::u8path("opencc-拒绝硬链接-cli");
  fs::create_directories(unicodeDir);

  const fs::path file = unicodeDir / fs::u8path("真实输入输出.txt");
  const fs::path hardLink = unicodeDir / fs::u8path("硬链接输出.txt");

  {
    std::ofstream ofs(file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open()) << file.u8string();
    ofs << "开放中文转换";
  }

  std::error_code error;
  fs::remove(hardLink, error);
  fs::create_hard_link(file, hardLink, error);
  if (error) {
    GTEST_SKIP() << "Hard-link creation failed: " << error.message();
  }

  EXPECT_NE(0, RunCommand(TestCommand("s2t", file.u8string(),
                                      hardLink.u8string(), "",
                                      "--in-place")));
  EXPECT_EQ("开放中文转换", GetFileContents(file));
  EXPECT_EQ("开放中文转换", GetFileContents(hardLink));
}
#endif

TEST_F(CommandLineConvertTest, ConvertsInPlaceThroughSymlink) {
  const fs::path unicodeDir =
      fs::u8path(OutputDirectory()) / fs::u8path("opencc-符号链接-cli");
  fs::create_directories(unicodeDir);

  const fs::path file = unicodeDir / fs::u8path("真实输入输出.txt");
  const fs::path symlink = unicodeDir / fs::u8path("符号链接输出.txt");

  {
    std::ofstream ofs(file, std::ios::binary);
    ASSERT_TRUE(ofs.is_open()) << file.u8string();
    ofs << "开放中文转换";
  }

  std::error_code error;
  fs::remove(symlink, error);
  fs::create_symlink(file.filename(), symlink, error);
  if (error) {
    GTEST_SKIP() << "Symlink creation failed: " << error.message();
  }

  ASSERT_EQ(0, RunCommand(TestCommand("s2t", file.u8string(),
                                      symlink.u8string(), "", "--in-place")));
  EXPECT_EQ("開放中文轉換", GetFileContents(file));
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
  const std::string config = "s2twp";
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

TEST_F(CommandLineConvertTest, SegmentationFailsWhenConfigHasNoSegmentation) {
  // s2t has no segmentation step; --segmentation should exit non-zero.
  const std::string config = "s2t";
  const std::string inputFile = InputFile("seg_no_seg_test");
  const std::string outputFile = OutputFile("seg_no_seg_test");
  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "开放中文转换\n";
  }
  EXPECT_NE(0, system(TestCommandWithFlags(config, inputFile, outputFile,
                                           "--segmentation").c_str()));
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
  const std::string config = "s2twp";
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
  const std::string config = "s2twp";
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
  TEST_F(CommandLineConvertTest, PreservesLineEnding_LF) {
    const std::string config = "s2t";
    const std::string inputFile = InputFile("lf_input");
    const std::string outputFile = OutputFile("lf_output");

    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open());
      ofs << "第一行\n第二行\n第三行\n";
    }

    ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile).c_str()));

    // Read output in binary mode to check line endings
    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                              (std::istreambuf_iterator<char>()));
    ifs.close();

    // Should contain LF (0a) but not CR (0d)
    ASSERT_NE(std::string::npos, content.find('\n'));
    ASSERT_EQ(std::string::npos, content.find('\r'));
  }

  TEST_F(CommandLineConvertTest, PreservesLineEnding_CRLF) {
    const std::string config = "s2t";
    const std::string inputFile = InputFile("crlf_input");
    const std::string outputFile = OutputFile("crlf_output");

    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open());
      ofs << "第一行\r\n第二行\r\n第三行\r\n";
    }

    ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile).c_str()));

    // Read output in binary mode to check line endings
    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                              (std::istreambuf_iterator<char>()));
    ifs.close();

    // Should contain both CR (0d) and LF (0a)
    ASSERT_NE(std::string::npos, content.find('\r'));
    ASSERT_NE(std::string::npos, content.find('\n'));
  }

  TEST_F(CommandLineConvertTest, PreservesLineEnding_CRLF_ASCII) {
    const std::string config = "s2t";
    const std::string inputFile = InputFile("crlf_ascii_input");
    const std::string outputFile = OutputFile("crlf_ascii_output");

    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open());
      ofs << "hello\r\nworld\r\n";
    }

    ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile).c_str()));

    // Read output in binary mode
    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                              (std::istreambuf_iterator<char>()));
    ifs.close();

    // Should preserve CRLF
    ASSERT_NE(std::string::npos, content.find('\r'));
    ASSERT_NE(std::string::npos, content.find('\n'));
  }

  TEST_F(CommandLineConvertTest, PreservesLineEnding_NoTrailingNewline_LF) {
    const std::string config = "s2t";
    const std::string inputFile = InputFile("no_trailing_lf_input");
    const std::string outputFile = OutputFile("no_trailing_lf_output");

    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open());
      ofs << "第一行\n第二行";  // No trailing newline
    }

    ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile).c_str()));

    // Read output in binary mode
    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                              (std::istreambuf_iterator<char>()));
    ifs.close();

    // Should have LF but no CR, and no trailing newline
    ASSERT_NE(std::string::npos, content.find('\n'));
    ASSERT_EQ(std::string::npos, content.find('\r'));
    ASSERT_NE('\n', content.back());
  }

  TEST_F(CommandLineConvertTest, PreservesLineEnding_NoTrailingNewline_CRLF) {
    const std::string config = "s2t";
    const std::string inputFile = InputFile("no_trailing_crlf_input");
    const std::string outputFile = OutputFile("no_trailing_crlf_output");

    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open());
      ofs << "第一行\r\n第二行";  // No trailing newline
    }

    ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile).c_str()));

    // Read output in binary mode
    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                              (std::istreambuf_iterator<char>()));
    ifs.close();

    // Should preserve CRLF but no trailing newline
    ASSERT_NE(std::string::npos, content.find('\r'));
    ASSERT_NE(std::string::npos, content.find('\n'));
    ASSERT_NE('\r', content.back());
    ASSERT_NE('\n', content.back());
  }

  TEST_F(CommandLineConvertTest, ChunkingBoundaryTruncationFix) {
    const std::string config = "s2t";
    const std::string inputFile = InputFile("chunk_boundary_test");
    const std::string outputFile = OutputFile("chunk_boundary_test");

    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open());
      // Create exactly 1MB - 3 bytes of padding. 
      // This forces the "头" character (3 bytes) to end exactly at the 1MB 
      // chunk boundary, while "发尾巴" falls into the next read chunk.
      std::string padding(1048576 - 3, 'a');
      ofs << padding << "头发尾巴";
    }

    ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile).c_str()));

    // Read output in binary mode
    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    ifs.close();

    // 1. Verify EOF truncation bug is fixed (the output shouldn't be truncated)
    // 2. Verify Semantic-safe chunking (the word shouldn't be split into 頭發)
    const std::string expectedEnd = "頭髮尾巴";
    ASSERT_GE(content.size(), expectedEnd.size());
    EXPECT_EQ(expectedEnd, content.substr(content.size() - expectedEnd.size()));
  }
  
  TEST_F(CommandLineConvertTest, ExactChunkSizeDoesNotTruncate) {
    const std::string config = "s2t";
    const std::string inputFile = InputFile("exact_chunk_test");
    const std::string outputFile = OutputFile("exact_chunk_test");

    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open());
      // Create EXACTLY 1MB of data (1048576 bytes).
      // This triggers the bug where fread reads exactly bufferSizeAvailable,
      // defers MAX_KEEP_CHARS to the next iteration, and then the next fread
      // returns 0. If the loop breaks immediately on fread returning 0, the
      // deferred tail is lost.
      std::string padding(1048576, 'a');
      ofs << padding;
    }

    ASSERT_EQ(0, system(TestCommand(config, inputFile, outputFile).c_str()));

    // Read output in binary mode
    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    ifs.close();

    // Verify output length is exactly 1MB.
    EXPECT_EQ(1048576, content.size());
  }
} // namespace opencc
