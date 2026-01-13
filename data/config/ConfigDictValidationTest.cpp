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

class ConfigDictValidationTest : public ::testing::Test {
protected:
  void SetUp() override {
#ifdef BAZEL
    runfiles_.reset(Runfiles::CreateForTest());
    ASSERT_NE(nullptr, runfiles_);
    testcasesPath_ = runfiles_->Rlocation("_main/test/testcases/testcases.json");
    configDir_ = runfiles_->Rlocation("_main/data/config");
    dictDir_ = runfiles_->Rlocation("_main/data/dictionary");
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
    for (auto itr = expectedObj.MemberBegin(); itr != expectedObj.MemberEnd();
         ++itr) {
      const std::string config = itr->name.GetString();
      ASSERT_TRUE(itr->value.IsString());
      const std::string expected = itr->value.GetString();
      SimpleConverter& converter = GetConverter(config);
      EXPECT_EQ(expected, converter.Convert(input))
          << "config=" << config << " case=" << id;
    }
  }
}

} // namespace
} // namespace opencc

#endif // BAZEL
