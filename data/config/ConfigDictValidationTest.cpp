/*
 * Open Chinese Convert
 *
 * End-to-end validation of all configs against consolidated testcases.json.
 */

#ifndef BAZEL
// This test is Bazel-only; CMake builds should skip compiling it.
static_assert(false, "ConfigDictValidationTest is only supported under Bazel");
#else

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "gtest/gtest.h"
#include "rapidjson/document.h"
#include "src/SimpleConverter.hpp"

#include "tools/cpp/runfiles/runfiles.h"
using bazel::tools::cpp::runfiles::Runfiles;

namespace opencc {
namespace {

#ifndef OPENCC_CONFIG_DICT_TEST_FILE
#error "OPENCC_CONFIG_DICT_TEST_FILE must be defined"
#endif

class ConfigDictValidationTest : public ::testing::Test {
protected:
  void SetUp() override {
#ifdef BAZEL
    runfiles_.reset(Runfiles::CreateForTest());
    ASSERT_NE(nullptr, runfiles_);
    testcasesPath_ = runfiles_->Rlocation("_main/test/testcases/testcases.json");
    configName_ = OPENCC_CONFIG_DICT_TEST_FILE;
    const std::string suffix = ".json";
    if (configName_.size() >= suffix.size() &&
        configName_.compare(configName_.size() - suffix.size(), suffix.size(),
                            suffix) == 0) {
      configName_.erase(configName_.size() - suffix.size());
    }
    const std::string configFile = runfiles_->Rlocation(
        "_main/data/config/" + std::string(OPENCC_CONFIG_DICT_TEST_FILE));
    configDir_ = configFile.substr(0, configFile.find_last_of("/\\"));
    const std::string dictFile =
        runfiles_->Rlocation("_main/data/dictionary/STCharacters.ocd2");
    dictDir_ = dictFile.substr(0, dictFile.find_last_of("/\\"));
#else
    FAIL() << "This test expects Bazel runfiles.";
#endif
  }

  std::string ReadFile(const std::string& path) {
    std::ifstream ifs(path);
    EXPECT_TRUE(ifs.is_open()) << path;
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
  }

  SimpleConverter& GetConverter(const std::string& config) {
    auto it = converters_.find(config);
    if (it != converters_.end()) {
      return *it->second;
    }
    const std::string configPath = configDir_ + "/" + config + ".json";
    auto inserted = converters_.emplace(
        config,
        std::make_unique<SimpleConverter>(configPath,
                                          std::vector<std::string>{
                                              configDir_, dictDir_}));
    return *inserted.first->second;
  }

  std::unique_ptr<Runfiles> runfiles_;
  std::string configName_;
  std::string testcasesPath_;
  std::string configDir_;
  std::string dictDir_;
  std::unordered_map<std::string, std::unique_ptr<SimpleConverter>>
      converters_;
};

TEST_F(ConfigDictValidationTest, ConvertExpectedOutputs) {
  const std::string json = ReadFile(testcasesPath_);
  rapidjson::Document doc;
  doc.Parse(json.c_str());
  ASSERT_FALSE(doc.HasParseError());
  ASSERT_TRUE(doc.IsObject());
  ASSERT_TRUE(doc.HasMember("cases"));
  const auto& cases = doc["cases"];
  ASSERT_TRUE(cases.IsArray());

  size_t checkedCases = 0;
  for (auto& testcase : cases.GetArray()) {
    ASSERT_TRUE(testcase.IsObject());
    ASSERT_TRUE(testcase.HasMember("input"));
    ASSERT_TRUE(testcase["input"].IsString());
    const std::string input = testcase["input"].GetString();
    const std::string id =
        testcase.HasMember("id") && testcase["id"].IsString()
            ? testcase["id"].GetString()
            : "(unknown id)";
    ASSERT_TRUE(testcase.HasMember("expected"));
    const auto& expectedObj = testcase["expected"];
    ASSERT_TRUE(expectedObj.IsObject());
    if (!expectedObj.HasMember(configName_.c_str())) {
      continue;
    }
    const auto& expectedValue = expectedObj[configName_.c_str()];
    ASSERT_TRUE(expectedValue.IsString())
        << "config=" << configName_ << " case=" << id;
    const std::string expected = expectedValue.GetString();
    SimpleConverter& converter = GetConverter(configName_);
    EXPECT_EQ(expected, converter.Convert(input))
        << "config=" << configName_ << " case=" << id;
    ++checkedCases;
  }
  EXPECT_GT(checkedCases, 0) << "No testcases found for config=" << configName_;
}

} // namespace
} // namespace opencc

#endif // BAZEL
