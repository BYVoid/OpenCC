/*
 * Open Chinese Convert
 *
 * Copyright 2026 Carbo Kuo and contributors
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
#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "gtest/gtest.h"
#include "rapidjson/document.h"

#ifndef BAZEL
#include "JiebaPluginIntegrationTestConfig.hpp"
#endif
#include "test/PortableUtil.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

std::wstring WideFromUtf8(const std::string& text) {
  if (text.empty()) {
    return L"";
  }
  const int size =
      MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
  if (size <= 1) {
    return L"";
  }
  std::wstring wide(static_cast<size_t>(size), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wide[0], size);
  wide.resize(static_cast<size_t>(size - 1));
  return wide;
}

void CopyRegularFiles(const std::filesystem::path& sourceDir,
                      const std::filesystem::path& targetDir) {
  for (const auto& entry : std::filesystem::directory_iterator(sourceDir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    std::filesystem::copy_file(entry.path(), targetDir / entry.path().filename(),
                               std::filesystem::copy_options::overwrite_existing);
  }
}

int RunProcessWide(const std::filesystem::path& application,
                   const std::wstring& arguments,
                   const std::filesystem::path& workingDirectory) {
  STARTUPINFOW startupInfo = {};
  startupInfo.cb = sizeof(startupInfo);
  PROCESS_INFORMATION processInfo = {};
  std::wstring commandLine = arguments;
  if (!CreateProcessW(WideFromUtf8(application.u8string()).c_str(),
                      commandLine.data(), nullptr, nullptr, FALSE, 0, nullptr,
                      WideFromUtf8(workingDirectory.u8string()).c_str(),
                      &startupInfo, &processInfo)) {
    return -1;
  }
  WaitForSingleObject(processInfo.hProcess, INFINITE);
  DWORD exitCode = 1;
  GetExitCodeProcess(processInfo.hProcess, &exitCode);
  CloseHandle(processInfo.hThread);
  CloseHandle(processInfo.hProcess);
  return static_cast<int>(exitCode);
}
#endif

#ifdef BAZEL
#include "tools/cpp/runfiles/runfiles.h"
using bazel::tools::cpp::runfiles::Runfiles;
#endif

namespace opencc {

#ifdef BAZEL
namespace {

std::string ParentDirectory(const std::string& path) {
  const std::string::size_type pos = path.find_last_of("/\\");
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(0, pos);
}

bool IsReadableFile(const std::string& path) {
  std::ifstream ifs(path.c_str());
  return ifs.is_open();
}

} // namespace
#else
namespace {

std::string ParentDirectory(const std::string& path) {
  const std::string::size_type pos = path.find_last_of("/\\");
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(0, pos);
}

bool IsReadableFile(const std::string& path) {
  std::ifstream ifs(path.c_str());
  return ifs.is_open();
}

} // namespace
#endif

struct CaseInput {
  std::string input;
  std::string expected;
};

using CasesByConfig = std::unordered_map<std::string, std::vector<CaseInput>>;

class JiebaPluginIntegrationTest : public ::testing::Test {
protected:
  JiebaPluginIntegrationTest() { originalWorkingDirectory_ = portable_getcwd(); }

  ~JiebaPluginIntegrationTest() override { free(originalWorkingDirectory_); }

  void SetUp() override {
#ifdef BAZEL
    runfiles_.reset(Runfiles::CreateForTest());
    ASSERT_TRUE(runfiles_ != nullptr);
    std::filesystem::create_directories(OutputDirectory());
#else
    ASSERT_EQ(0, portable_chdir(PROJECT_BINARY_DIR "/data"));
    std::filesystem::create_directories(OutputDirectory());
#endif
  }

  void TearDown() override {
    ASSERT_EQ(0, portable_chdir(originalWorkingDirectory_));
  }

  std::string OpenccCommand() const {
#ifdef BAZEL
#ifdef _WIN32
    std::string path = runfiles_->Rlocation("_main/src/tools/command_line.exe");
    if (path.empty()) {
      path = runfiles_->Rlocation("_main/src/tools/command_line");
    }
    return path;
#else
    return runfiles_->Rlocation("_main/src/tools/command_line");
#endif
#else
    return OPENCC_TEST_COMMAND;
#endif
  }

  static std::string QuotePath(const std::string& path) {
    return "\"" + path + "\"";
  }

  static std::string JsonString(const std::string& value) {
    std::string escaped = "\"";
    for (char ch : value) {
      if (ch == '\\' || ch == '"') {
        escaped.push_back('\\');
      }
      escaped.push_back(ch);
    }
    escaped.push_back('"');
    return escaped;
  }

  std::string OutputDirectory() const {
#ifdef BAZEL
    return ::testing::TempDir() + "opencc-jieba/";
#else
    return PROJECT_BINARY_DIR "/plugins/jieba/tests/";
#endif
  }

  std::string InputFile(const std::string& config) const {
    return OutputDirectory() + config + ".in";
  }

  std::string OutputFile(const std::string& config) const {
    return OutputDirectory() + config + ".out";
  }

  std::string ConfigPath(const std::string& config) const {
#ifdef BAZEL
    return runfiles_->Rlocation("_main/plugins/jieba/data/config/" + config +
                                ".json");
#else
    if (config == "s2twp_jieba" || config == "tw2sp_jieba") {
      return std::string(CMAKE_SOURCE_DIR) + "/plugins/jieba/data/config/" +
             config + ".json";
    }
    return std::string(CMAKE_SOURCE_DIR) + "/data/config/" + config + ".json";
#endif
  }

  std::string DictionaryDirectory() const {
#ifdef BAZEL
    const std::string dictFile =
        runfiles_->Rlocation("_main/data/dictionary/STCharacters.ocd2");
    return ParentDirectory(dictFile);
#else
    return std::string(PROJECT_BINARY_DIR) + "/data";
#endif
  }

  std::string PluginDirectory() const {
#ifdef BAZEL
    const std::vector<std::string> candidates = {
#ifdef _WIN32
        "_main/plugins/jieba/opencc-jieba.dll",
        "_main/plugins/jieba/opencc-jieba.exe",
#elif defined(__APPLE__)
        "_main/plugins/jieba/libopencc-jieba.dylib",
        "_main/plugins/jieba/opencc-jieba.dylib",
        "_main/plugins/jieba/libopencc-jieba.so",
#else
        "_main/plugins/jieba/libopencc-jieba.so",
        "_main/plugins/jieba/opencc-jieba.so",
#endif
    };
    for (const std::string& candidate : candidates) {
      const std::string path = runfiles_->Rlocation(candidate);
      if (!path.empty() && IsReadableFile(path)) {
        return ParentDirectory(path);
      }
    }
    return "";
#else
    return OPENCC_PLUGIN_TEST_DIR;
#endif
  }

  std::string MergedJiebaDictPath() const {
#ifdef BAZEL
    return runfiles_->Rlocation("_main/plugins/jieba/jieba_dict/jieba_merged.ocd2");
#else
    const std::string pluginDir = PluginDirectory();
    const std::string pluginParent = ParentDirectory(pluginDir);

    std::vector<std::string> candidates;
    candidates.push_back(OPENCC_JIEBA_MERGED_DICT_PATH);
    candidates.push_back(pluginDir + "/jieba_dict/jieba_merged.ocd2");
    if (!pluginParent.empty()) {
      candidates.push_back(pluginParent + "/jieba_dict/jieba_merged.ocd2");
    }
#if defined(_WIN32) || defined(_WIN64)
    const char* const configs[] = {"Debug", "Release", "RelWithDebInfo",
                                   "MinSizeRel"};
    for (const char* config : configs) {
      candidates.push_back(pluginDir + "/" + config +
                           "/jieba_dict/jieba_merged.ocd2");
      if (!pluginParent.empty()) {
        candidates.push_back(pluginParent + "/" + config +
                             "/jieba_dict/jieba_merged.ocd2");
      }
    }
#endif

    for (const std::string& candidate : candidates) {
      if (IsReadableFile(candidate)) {
        return candidate;
      }
    }
    return candidates.front();
#endif
  }

  std::string JiebaModelPath() const {
#ifdef BAZEL
    return runfiles_->Rlocation(
        "_main/plugins/jieba/deps/cppjieba/dict/hmm_model.utf8");
#else
    return std::string(CMAKE_SOURCE_DIR) +
           "/plugins/jieba/deps/cppjieba/dict/hmm_model.utf8";
#endif
  }

  std::string EnvPrefix() const {
    const std::string pluginDir = PluginDirectory();
#ifdef _WIN32
    return "set \"OPENCC_SEGMENTATION_PLUGIN_PATH=" + pluginDir + "\" && ";
#else
    return "OPENCC_SEGMENTATION_PLUGIN_PATH=" + QuotePath(pluginDir) + " ";
#endif
  }

  std::string BuildCommand(const std::string& config,
                           const std::string& inputFile,
                           const std::string& outputFile) const {
    std::string cmd =
        BuildCommandWithPaths(ConfigPath(config), inputFile, outputFile,
                              DictionaryDirectory() + "/", PluginDirectory());
#ifdef _WIN32
    return "\"" + cmd + "\"";
#else
    return cmd;
#endif
  }

  std::string BuildCommandWithPaths(const std::string& configPath,
                                    const std::string& inputFile,
                                    const std::string& outputFile,
                                    const std::string& dictionaryDir,
                                    const std::string& pluginDir) const {
#ifdef _WIN32
    std::string envPrefix =
        "set \"OPENCC_SEGMENTATION_PLUGIN_PATH=" + pluginDir + "\" && ";
#else
    std::string envPrefix =
        "OPENCC_SEGMENTATION_PLUGIN_PATH=" + QuotePath(pluginDir) + " ";
#endif
    return envPrefix + QuotePath(OpenccCommand()) + " -i " + QuotePath(inputFile) +
           " -o " + QuotePath(outputFile) + " -c " + QuotePath(configPath) +
           " --path " + QuotePath(dictionaryDir);
  }

#if defined(_WIN32) || defined(_WIN64)
  static int RunCommandUtf8(const std::string& command) {
    const int size =
        MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, nullptr, 0);
    if (size <= 1) {
      return -1;
    }
    std::wstring wide(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wide[0], size);
    return _wsystem(wide.c_str());
  }
#else
  static int RunCommandUtf8(const std::string& command) {
    return system(command.c_str());
  }
#endif

  static CasesByConfig LoadCases(const std::string& jsonPath) {
    CasesByConfig cases;
    std::ifstream ifs(jsonPath);
    if (!ifs.is_open()) {
      throw std::runtime_error("Cannot open " + jsonPath);
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    rapidjson::Document doc;
    std::string content = buffer.str();
    doc.Parse(content.c_str());
    if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember("cases")) {
      throw std::runtime_error("Invalid testcase format");
    }
    for (auto& entry : doc["cases"].GetArray()) {
      const std::string input = entry["input"].GetString();
      for (auto itr = entry["expected"].MemberBegin();
           itr != entry["expected"].MemberEnd(); ++itr) {
        if (!itr->value.IsString()) {
          continue;
        }
        const std::string config = itr->name.GetString();
        if (config != "s2twp_jieba" && config != "tw2sp_jieba") {
          continue;
        }
        cases[config].push_back({input, itr->value.GetString()});
      }
    }
    return cases;
  }

  char* originalWorkingDirectory_ = nullptr;

#ifdef BAZEL
  std::unique_ptr<Runfiles> runfiles_;
#endif
};

TEST_F(JiebaPluginIntegrationTest, ConvertFromJsonCases) {
  const CasesByConfig cases = LoadCases(
#ifdef BAZEL
      runfiles_->Rlocation(
          "_main/plugins/jieba/tests/data/jieba_comparison_testcases.json")
#else
      std::string(CMAKE_SOURCE_DIR) +
      "/plugins/jieba/tests/data/jieba_comparison_testcases.json"
#endif
  );

  for (const auto& entry : cases) {
    const std::string& config = entry.first;
    const std::string inputFile = InputFile(config);
    const std::string outputFile = OutputFile(config);

    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open());
      for (const auto& item : entry.second) {
        ofs << item.input << "\n";
      }
    }

    ASSERT_EQ(0, RunCommandUtf8(BuildCommand(config, inputFile, outputFile)));

    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string line;
    size_t idx = 0;
    while (std::getline(ifs, line)) {
      if (!line.empty() && line.back() == '\r') {
        line.pop_back();
      }
      ASSERT_LT(idx, entry.second.size());
      EXPECT_EQ(entry.second[idx].expected, line)
          << "config=" << config << " index=" << idx;
      idx++;
    }
    EXPECT_EQ(idx, entry.second.size()) << "config=" << config;
  }
}

TEST_F(JiebaPluginIntegrationTest, ConvertWithMergedOcd2JiebaDictionary) {
  const std::string inputFile = InputFile("s2twp_jieba_merged");
  const std::string outputFile = OutputFile("s2twp_jieba_merged");
  const std::string configFile = OutputDirectory() + "s2twp_jieba_merged.json";

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "生活着名为正敏的少女\n";
  }

  {
    std::ofstream ofs(configFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "{\n"
        << "  \"name\": \"merged jieba dict test\",\n"
        << "  \"segmentation\": {\n"
        << "    \"type\": \"jieba\",\n"
        << "    \"resources\": {\n"
        << "      \"dict_path\": " << JsonString(MergedJiebaDictPath()) << ",\n"
        << "      \"model_path\": " << JsonString(JiebaModelPath()) << "\n"
        << "    }\n"
        << "  },\n"
        << "  \"conversion_chain\": [{\n"
        << "    \"dict\": {\n"
        << "      \"type\": \"group\",\n"
        << "      \"dicts\": [{\n"
        << "        \"type\": \"ocd2\",\n"
        << "        \"file\": \"STPhrases.ocd2\"\n"
        << "      }, {\n"
        << "        \"type\": \"ocd2\",\n"
        << "        \"file\": \"STCharacters.ocd2\"\n"
        << "      }]\n"
        << "    }\n"
        << "  }, {\n"
        << "    \"dict\": {\n"
        << "      \"type\": \"ocd2\",\n"
        << "      \"file\": \"TWPhrases.ocd2\"\n"
        << "    }\n"
        << "  }, {\n"
        << "    \"dict\": {\n"
        << "      \"type\": \"ocd2\",\n"
        << "      \"file\": \"TWVariants.ocd2\"\n"
        << "    }\n"
        << "  }]\n"
        << "}\n";
  }

  ASSERT_EQ(0, RunCommandUtf8(BuildCommandWithPaths(
                   configFile, inputFile, outputFile, DictionaryDirectory() + "/",
                   PluginDirectory())));

  std::ifstream ifs(outputFile, std::ios::binary);
  ASSERT_TRUE(ifs.is_open());
  std::string line;
  ASSERT_TRUE(std::getline(ifs, line));
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
  EXPECT_EQ("生活著名為正敏的少女", line);
}

#if defined(_WIN32) || defined(_WIN64)
TEST_F(JiebaPluginIntegrationTest, ConvertFromUnicodePluginPath) {
  namespace fs = std::filesystem;

  const fs::path tempRoot =
      fs::temp_directory_path() /
      fs::u8path("opencc-jieba-中文路径-" +
                 std::to_string(GetCurrentProcessId()));
  const fs::path tempBin = tempRoot / "bin";
  const fs::path tempShareOpencc = tempRoot / "share" / "opencc";
  const fs::path tempJiebaDict = tempShareOpencc / "jieba_dict";
  const fs::path tempPlugins = tempBin / "plugins";
  const fs::path inputFile = tempBin / "input.txt";
  const fs::path outputFile = tempBin / "output.txt";
  const fs::path configFile = tempShareOpencc / "s2twp_jieba.json";
  const fs::path sourceConfig = fs::u8path(ConfigPath("s2twp_jieba"));
  const fs::path sourcePluginRoot =
      sourceConfig.parent_path().parent_path().parent_path();
  const fs::path sourceJiebaDict =
      sourcePluginRoot / "deps" / "cppjieba" / "dict";
  const fs::path sourceMergedDict = fs::u8path(MergedJiebaDictPath());
  const fs::path sourcePluginDir = fs::u8path(PluginDirectory());
  const fs::path sourceOpenccExe = fs::u8path(OpenccCommand());
  const fs::path sourceBinDir = sourceOpenccExe.parent_path();

  fs::remove_all(tempRoot);
  fs::create_directories(tempBin);
  fs::create_directories(tempJiebaDict);
  fs::create_directories(tempPlugins);
  CopyRegularFiles(sourceBinDir, tempBin);
  CopyRegularFiles(fs::u8path(DictionaryDirectory()), tempShareOpencc);
  fs::copy_file(sourceConfig, configFile, fs::copy_options::overwrite_existing);
  CopyRegularFiles(sourceJiebaDict, tempJiebaDict);
  fs::copy_file(sourceMergedDict, tempJiebaDict / sourceMergedDict.filename(),
                fs::copy_options::overwrite_existing);
  CopyRegularFiles(sourcePluginDir, tempPlugins);

  try {
    {
      std::ofstream ofs(inputFile, std::ios::binary);
      ASSERT_TRUE(ofs.is_open());
      ofs << "生活着名为正敏的少女\n";
    }

    const fs::path tempOpenccExe = tempBin / sourceOpenccExe.filename();
    const std::wstring args =
        WideFromUtf8(sourceOpenccExe.filename().u8string()) +
        L" -i input.txt -o output.txt -c s2twp_jieba.json";
    ASSERT_EQ(0, RunProcessWide(tempOpenccExe, args, tempBin));

    std::ifstream ifs(outputFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::string line;
    ASSERT_TRUE(std::getline(ifs, line));
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    EXPECT_EQ("生活著名為正敏的少女", line);
  } catch (...) {
    fs::remove_all(tempRoot);
    throw;
  }

  fs::remove_all(tempRoot);
}
#endif

} // namespace opencc
