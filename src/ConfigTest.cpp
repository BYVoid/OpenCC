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
#include <filesystem>
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "Config.hpp"
#include "ConfigTestBase.hpp"
#include "Converter.hpp"
#include "Exception.hpp"
#include "ResourceProvider.hpp"
#include "TestUtilsUTF8.hpp"

namespace opencc {
namespace {

namespace fs = std::filesystem;

std::string PathString(const fs::path& path) { return path.u8string(); }

fs::path MakeTempDir(const std::string& name) {
#if defined(_WIN32) || defined(_WIN64)
  const auto suffix = std::to_string(GetCurrentProcessId());
#else
  const auto suffix = std::to_string(getpid());
#endif
  fs::path dir = fs::temp_directory_path() / (name + "-" + suffix);
  fs::remove_all(dir);
  fs::create_directories(dir);
  return dir;
}

void WriteFile(const fs::path& path, const std::string& content) {
  std::ofstream ofs(path, std::ios::binary);
  ofs << content;
}

std::string SingleDictConfig(const std::string& dictFile) {
  return std::string("{\n"
                     "  \"name\": \"Resource Provider Test\",\n"
                     "  \"segmentation\": {\n"
                     "    \"type\": \"mmseg\",\n"
                     "    \"dict\": {\"type\": \"text\", \"file\": \"") +
         dictFile +
         "\"}\n"
         "  },\n"
         "  \"conversion_chain\": [{\n"
         "    \"dict\": {\"type\": \"text\", \"file\": \"" +
         dictFile +
         "\"}\n"
         "  }]\n"
         "}\n";
}

} // namespace

class ConfigTest : public ConfigTestBase {
protected:
  ConfigTest()
      : input(utf8("燕燕于飞差池其羽之子于归远送于野")),
        expected(utf8("燕燕于飛差池其羽之子于歸遠送於野")) {}

  virtual void SetUp() {
    converter = config.NewFromFile(CONFIG_TEST_JSON_PATH);
  }

  Config config;
  ConverterPtr converter;
  const std::string input;
  const std::string expected;
};

TEST_F(ConfigTest, Convert) {
  const std::string& converted = converter->Convert(input);
  EXPECT_EQ(expected, converted);
}

TEST_F(ConfigTest, ConvertBuffer) {
  char output[1024];
  const size_t length = converter->Convert(input.c_str(), output);
  EXPECT_EQ(expected.length(), length);
  EXPECT_EQ(expected, output);
}

TEST_F(ConfigTest, NonexistingPath) {
  const std::string path = "/opencc/no/such/file/or/directory";
  try {
    const ConverterPtr _ = config.NewFromFile(path);
  } catch (FileNotFound& e) {
    EXPECT_EQ(path + " not found or not accessible.", e.what());
  }
}

TEST_F(ConfigTest, NewFromStringWitoutTrailingSlash) {
  std::ifstream ifs(CONFIG_TEST_JSON_PATH);
  std::string content(std::istreambuf_iterator<char>(ifs),
                      (std::istreambuf_iterator<char>()));

  const ConverterPtr _ = config.NewFromString(content, CONFIG_TEST_DIR_PATH);
}

TEST_F(ConfigTest, DefaultConfigPathFindsAdjacentResources) {
  const fs::path tempDir = MakeTempDir("opencc-adjacent-resource-test");
  fs::copy_file(fs::u8path(CONFIG_TEST_DIR_PATH) / "config_test.json",
                tempDir / "config_test.json");
  fs::copy_file(fs::u8path(CONFIG_TEST_DIR_PATH) / "config_test_phrases.txt",
                tempDir / "config_test_phrases.txt");
  fs::copy_file(fs::u8path(CONFIG_TEST_DIR_PATH) / "config_test_characters.txt",
                tempDir / "config_test_characters.txt");

  try {
    const ConverterPtr tempConverter =
        config.NewFromFile(PathString(tempDir / "config_test.json"));
    EXPECT_EQ(expected, tempConverter->Convert(input));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, ExplicitProviderFindsResources) {
  const fs::path tempDir = MakeTempDir("opencc-explicit-provider-test");
  const fs::path configDir = tempDir / "config";
  const fs::path resourceDir = tempDir / "resources";
  fs::create_directories(configDir);
  fs::create_directories(resourceDir);
  WriteFile(configDir / "config.json", SingleDictConfig("dict.txt"));
  WriteFile(resourceDir / "dict.txt", utf8("鼠标\t滑鼠\n"));

  try {
    std::shared_ptr<ResourceProvider> provider(
        new FilesystemResourceProvider({PathString(resourceDir)}));
    const ConverterPtr tempConverter =
        config.NewFromFile(PathString(configDir / "config.json"), provider);
    EXPECT_EQ(utf8("滑鼠"), tempConverter->Convert(utf8("鼠标")));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, ExplicitProviderFindsConfigNameAndResources) {
  const fs::path tempDir = MakeTempDir("opencc-provider-config-test");
  const fs::path resourceDir = tempDir / "resources";
  fs::create_directories(resourceDir);
  WriteFile(resourceDir / "provider_config.json", SingleDictConfig("dict.txt"));
  WriteFile(resourceDir / "dict.txt", utf8("鼠标\t滑鼠\n"));

  try {
    std::shared_ptr<ResourceProvider> provider(
        new FilesystemResourceProvider({PathString(resourceDir)}));
    const ConverterPtr tempConverter =
        config.NewFromFile("provider_config.json", provider);
    EXPECT_EQ(utf8("滑鼠"), tempConverter->Convert(utf8("鼠标")));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, ExplicitProviderConfigOverridesInstalledOrCwdConfigName) {
  const fs::path tempDir = MakeTempDir("opencc-provider-config-override-test");
  const fs::path cwdDir = tempDir / "cwd";
  const fs::path resourceDir = tempDir / "resources";
  fs::create_directories(cwdDir);
  fs::create_directories(resourceDir);
  WriteFile(cwdDir / "config.json", SingleDictConfig("dict_a.txt"));
  WriteFile(resourceDir / "config.json", SingleDictConfig("dict_b.txt"));
  WriteFile(resourceDir / "dict_a.txt", utf8("鼠标\t甲\n"));
  WriteFile(resourceDir / "dict_b.txt", utf8("鼠标\t乙\n"));

  const fs::path originalCwd = fs::current_path();
  try {
    fs::current_path(cwdDir);
    std::shared_ptr<ResourceProvider> provider(
        new FilesystemResourceProvider({PathString(resourceDir)}));
    const ConverterPtr tempConverter =
        config.NewFromFile("config.json", provider);
    EXPECT_EQ(utf8("乙"), tempConverter->Convert(utf8("鼠标")));
    fs::current_path(originalCwd);
  } catch (...) {
    fs::current_path(originalCwd);
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, RelativeParentDictionaryPathStillWorks) {
  const fs::path tempDir = MakeTempDir("opencc-relative-parent-resource-test");
  const fs::path configDir = tempDir / "config";
  const fs::path dictionaryDir = tempDir / "dictionary";
  fs::create_directories(configDir);
  fs::create_directories(dictionaryDir);
  WriteFile(configDir / "config.json",
            SingleDictConfig("../dictionary/dict.txt"));
  WriteFile(dictionaryDir / "dict.txt", utf8("鼠标\t滑鼠\n"));

  try {
    const ConverterPtr tempConverter =
        config.NewFromFile(PathString(configDir / "config.json"));
    EXPECT_EQ(utf8("滑鼠"), tempConverter->Convert(utf8("鼠标")));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, MultipleSearchPathsUseFirstMatch) {
  const fs::path tempDir = MakeTempDir("opencc-provider-order-test");
  const fs::path configDir = tempDir / "config";
  const fs::path firstDir = tempDir / "first";
  const fs::path secondDir = tempDir / "second";
  fs::create_directories(configDir);
  fs::create_directories(firstDir);
  fs::create_directories(secondDir);
  WriteFile(configDir / "config.json", SingleDictConfig("dict.txt"));
  WriteFile(firstDir / "dict.txt", utf8("鼠标\t第一\n"));
  WriteFile(secondDir / "dict.txt", utf8("鼠标\t第二\n"));

  try {
    std::shared_ptr<ResourceProvider> provider(
        new FilesystemResourceProvider(
            {PathString(firstDir), PathString(secondDir)}));
    const ConverterPtr tempConverter =
        config.NewFromFile(PathString(configDir / "config.json"), provider);
    EXPECT_EQ(utf8("第一"), tempConverter->Convert(utf8("鼠标")));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, MissingResourceListsSearchedPaths) {
  const fs::path tempDir = MakeTempDir("opencc-provider-missing-test");
  const fs::path firstDir = tempDir / "first";
  const fs::path secondDir = tempDir / "second";
  fs::create_directories(firstDir);
  fs::create_directories(secondDir);

  try {
    FilesystemResourceProvider provider(
        {PathString(firstDir), PathString(secondDir)});
    provider.Resolve("missing.ocd2");
    FAIL() << "Expected FileNotFound";
  } catch (const FileNotFound& e) {
    const std::string message = e.what();
    EXPECT_NE(std::string::npos, message.find("missing.ocd2"));
    EXPECT_NE(std::string::npos, message.find(PathString(firstDir)));
    EXPECT_NE(std::string::npos, message.find(PathString(secondDir)));
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, PluginLikeResourcePathSupplementsMainPath) {
  const fs::path tempDir = MakeTempDir("opencc-plugin-resource-test");
  const fs::path configDir = tempDir / "config";
  const fs::path mainDir = tempDir / "main";
  const fs::path pluginDir = tempDir / "plugin";
  fs::create_directories(configDir);
  fs::create_directories(mainDir);
  fs::create_directories(pluginDir);
  WriteFile(configDir / "config.json", SingleDictConfig("plugin_dict.txt"));
  WriteFile(pluginDir / "plugin_dict.txt", utf8("服务器\t伺服器\n"));

  try {
    std::shared_ptr<ResourceProvider> provider(
        new FilesystemResourceProvider(
            {PathString(mainDir), PathString(pluginDir)}));
    const ConverterPtr tempConverter =
        config.NewFromFile(PathString(configDir / "config.json"), provider);
    EXPECT_EQ(utf8("伺服器"), tempConverter->Convert(utf8("服务器")));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

#if defined(_MSC_VER)
// This case exists only to verify Windows Unicode path handling in OpenCC
// itself. Other platforms do not have this specific regression, and MinGW-like
// Windows environments do not provide a stable enough std::filesystem Unicode
// behavior for this check to be reliable.
TEST_F(ConfigTest, LoadConfigFromUnicodePath) {
  namespace fs = std::filesystem;

  const fs::path sourceDir = fs::u8path(CONFIG_TEST_DIR_PATH);
  const fs::path tempDir =
      fs::temp_directory_path() /
      fs::u8path("opencc-中文路径-config-test-" +
                 std::to_string(GetCurrentProcessId()));

  fs::remove_all(tempDir);
  fs::create_directories(tempDir);
  fs::copy_file(sourceDir / "config_test.json", tempDir / "config_test.json",
                fs::copy_options::overwrite_existing);
  fs::copy_file(sourceDir / "config_test_phrases.txt",
                tempDir / "config_test_phrases.txt",
                fs::copy_options::overwrite_existing);
  fs::copy_file(sourceDir / "config_test_characters.txt",
                tempDir / "config_test_characters.txt",
                fs::copy_options::overwrite_existing);
  try {
    const ConverterPtr unicodeConverter =
        config.NewFromFile(tempDir.u8string() + "/config_test.json");
    EXPECT_EQ(expected, unicodeConverter->Convert(input));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }

  fs::remove_all(tempDir);
}
#endif

} // namespace opencc
