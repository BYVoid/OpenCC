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

#include "test/PortableUtil.hpp"

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

    ASSERT_EQ(0, system(BuildCommand(config, inputFile, outputFile).c_str()));

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

#if defined(_WIN32) || defined(_WIN64)
TEST_F(JiebaPluginIntegrationTest, ConvertFromUnicodePluginPath) {
  namespace fs = std::filesystem;

  const fs::path tempRoot =
      fs::temp_directory_path() /
      fs::u8path("opencc-jieba-中文路径-" +
                 std::to_string(GetCurrentProcessId()));
  const fs::path tempShareOpencc = tempRoot / "share" / "opencc";
  const fs::path tempJiebaDict = tempShareOpencc / "jieba_dict";
  const fs::path tempPlugins = tempRoot / "bin" / "plugins";
  const fs::path inputFile = tempRoot / "input.txt";
  const fs::path outputFile = tempRoot / "output.txt";
  const fs::path configFile = tempShareOpencc / "s2twp_jieba.json";
  const fs::path sourceConfig = fs::u8path(ConfigPath("s2twp_jieba"));
  const fs::path sourceJiebaDict = sourceConfig.parent_path().parent_path() /
                                   "deps" / "cppjieba" / "dict";
  const fs::path sourcePluginDir = fs::u8path(PluginDirectory());

  fs::remove_all(tempRoot);
  fs::create_directories(tempJiebaDict);
  fs::create_directories(tempPlugins);
  fs::copy_file(sourceConfig, configFile, fs::copy_options::overwrite_existing);
  for (const auto& entry : fs::directory_iterator(sourceJiebaDict)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    fs::copy_file(entry.path(), tempJiebaDict / entry.path().filename(),
                  fs::copy_options::overwrite_existing);
  }
  for (const auto& entry : fs::directory_iterator(sourcePluginDir)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    fs::copy_file(entry.path(), tempPlugins / entry.path().filename(),
                  fs::copy_options::overwrite_existing);
  }

  {
    std::ofstream ofs(inputFile, std::ios::binary);
    ASSERT_TRUE(ofs.is_open());
    ofs << "生活着名为正敏的少女\n";
  }

  const std::string cmd = BuildCommandWithPaths(
      configFile.u8string(), inputFile.u8string(), outputFile.u8string(),
      DictionaryDirectory() + "/", tempPlugins.u8string());
  ASSERT_EQ(0, system(("\"" + cmd + "\"").c_str()));

  std::ifstream ifs(outputFile, std::ios::binary);
  ASSERT_TRUE(ifs.is_open());
  std::string line;
  ASSERT_TRUE(std::getline(ifs, line));
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
  EXPECT_EQ("生活著名為正敏的少女", line);

  fs::remove_all(tempRoot);
}
#endif

} // namespace opencc
