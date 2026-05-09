/*
 * Open Chinese Convert
 *
 * Validate bundled JSON configurations against opencc_config.schema.json.
 */

#ifndef BAZEL
// This test is Bazel-only; CMake builds should skip compiling it.
static_assert(false, "ConfigSchemaValidationTest is only supported under Bazel");
#else

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/schema.h"
#include "rapidjson/stringbuffer.h"

#include "tools/cpp/runfiles/runfiles.h"
using bazel::tools::cpp::runfiles::Runfiles;

namespace opencc {
namespace {

#ifndef OPENCC_CONFIG_SCHEMA_TEST_FILE
#error "OPENCC_CONFIG_SCHEMA_TEST_FILE must be defined"
#endif

std::string ReadFile(const std::string& path) {
  std::ifstream ifs(path);
  EXPECT_TRUE(ifs.is_open()) << path;
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  return buffer.str();
}

std::string StringifyPointer(const rapidjson::Pointer& pointer) {
  rapidjson::StringBuffer buffer;
  pointer.Stringify(buffer);
  return buffer.GetString();
}

TEST(ConfigSchemaValidationTest, BundledConfigsMatchSchema) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  ASSERT_NE(nullptr, runfiles);

  const std::string configFile = OPENCC_CONFIG_SCHEMA_TEST_FILE;
  const std::string schemaPath =
      runfiles->Rlocation("_main/data/config/opencc_config.schema.json");
  const std::string schemaJson = ReadFile(schemaPath);
  rapidjson::Document schemaDoc;
  schemaDoc.Parse(schemaJson.c_str());
  ASSERT_FALSE(schemaDoc.HasParseError())
      << "schema parse error at offset " << schemaDoc.GetErrorOffset() << ": "
      << rapidjson::GetParseError_En(schemaDoc.GetParseError());
  ASSERT_TRUE(schemaDoc.IsObject());
  rapidjson::SchemaDocument schema(schemaDoc);

  const std::string configPath =
      runfiles->Rlocation("_main/data/config/" + configFile);
  const std::string configJson = ReadFile(configPath);
  rapidjson::Document configDoc;
  configDoc.Parse(configJson.c_str());
  ASSERT_FALSE(configDoc.HasParseError())
      << configFile << " parse error at offset " << configDoc.GetErrorOffset()
      << ": " << rapidjson::GetParseError_En(configDoc.GetParseError());

  rapidjson::SchemaValidator validator(schema);
  EXPECT_TRUE(configDoc.Accept(validator))
      << configFile << " violates schema at document path "
      << StringifyPointer(validator.GetInvalidDocumentPointer())
      << ", schema path "
      << StringifyPointer(validator.GetInvalidSchemaPointer()) << ", keyword "
      << validator.GetInvalidSchemaKeyword();
}

} // namespace
} // namespace opencc

#endif // BAZEL
