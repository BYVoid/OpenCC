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
#include <string_view>
#include <utility>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "Config.hpp"
#include "ConfigTestBase.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "Segments.hpp"
#include "Exception.hpp"
#include "ResourceProvider.hpp"
#include "TestUtilsUTF8.hpp"

namespace opencc {
namespace {

namespace fs = std::filesystem;

std::string PathString(const fs::path& path) { return path.u8string(); }

std::string NormalizePathString(std::string path) {
  for (char& ch : path) {
    if (ch == '\\') {
      ch = '/';
    }
  }
  return path;
}

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

void WriteLe16(std::ofstream& output, uint16_t value) {
  output.put(static_cast<char>(value & 0xff));
  output.put(static_cast<char>((value >> 8) & 0xff));
}

void WriteLe32(std::ofstream& output, uint32_t value) {
  output.put(static_cast<char>(value & 0xff));
  output.put(static_cast<char>((value >> 8) & 0xff));
  output.put(static_cast<char>((value >> 16) & 0xff));
  output.put(static_cast<char>((value >> 24) & 0xff));
}

struct ZipTestEntry {
  std::string name;
  std::string content;
  uint32_t localHeaderOffset;
};

void WriteStoredZip(const fs::path& path,
                    std::vector<std::pair<std::string, std::string>> entries) {
  std::ofstream output(path, std::ios::binary);
  std::vector<ZipTestEntry> writtenEntries;
  for (const auto& entry : entries) {
    const std::string& name = entry.first;
    const std::string& content = entry.second;
    const uint32_t localHeaderOffset =
        static_cast<uint32_t>(output.tellp());
    WriteLe32(output, 0x04034b50);
    WriteLe16(output, 20);
    WriteLe16(output, 0);
    WriteLe16(output, 0);
    WriteLe16(output, 0);
    WriteLe16(output, 0);
    WriteLe32(output, 0);
    WriteLe32(output, static_cast<uint32_t>(content.size()));
    WriteLe32(output, static_cast<uint32_t>(content.size()));
    WriteLe16(output, static_cast<uint16_t>(name.size()));
    WriteLe16(output, 0);
    output.write(name.data(), static_cast<std::streamsize>(name.size()));
    output.write(content.data(), static_cast<std::streamsize>(content.size()));
    writtenEntries.push_back(ZipTestEntry{name, content, localHeaderOffset});
  }

  const uint32_t centralDirectoryOffset =
      static_cast<uint32_t>(output.tellp());
  for (const ZipTestEntry& entry : writtenEntries) {
    WriteLe32(output, 0x02014b50);
    WriteLe16(output, 20);
    WriteLe16(output, 20);
    WriteLe16(output, 0);
    WriteLe16(output, 0);
    WriteLe16(output, 0);
    WriteLe16(output, 0);
    WriteLe32(output, 0);
    WriteLe32(output, static_cast<uint32_t>(entry.content.size()));
    WriteLe32(output, static_cast<uint32_t>(entry.content.size()));
    WriteLe16(output, static_cast<uint16_t>(entry.name.size()));
    WriteLe16(output, 0);
    WriteLe16(output, 0);
    WriteLe16(output, 0);
    WriteLe16(output, 0);
    WriteLe32(output, 0);
    WriteLe32(output, entry.localHeaderOffset);
    output.write(entry.name.data(),
                 static_cast<std::streamsize>(entry.name.size()));
  }
  const uint32_t centralDirectorySize =
      static_cast<uint32_t>(output.tellp()) - centralDirectoryOffset;

  WriteLe32(output, 0x06054b50);
  WriteLe16(output, 0);
  WriteLe16(output, 0);
  WriteLe16(output, static_cast<uint16_t>(writtenEntries.size()));
  WriteLe16(output, static_cast<uint16_t>(writtenEntries.size()));
  WriteLe32(output, centralDirectorySize);
  WriteLe32(output, centralDirectoryOffset);
  WriteLe16(output, 0);
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

std::string InlineSingleStepConfig(const std::string& segmentationEntries,
                                   const std::string& conversionDict) {
  return std::string("{\n"
                     "  \"name\": \"Inline Dict Test\",\n"
                     "  \"segmentation\": {\n"
                     "    \"type\": \"mmseg\",\n"
                     "    \"dict\": {\n"
                     "      \"type\": \"inline\",\n"
                     "      \"entries\": ") +
         segmentationEntries +
         "\n"
         "    }\n"
         "  },\n"
         "  \"conversion_chain\": [{\n"
         "    \"dict\": " +
         conversionDict +
         "\n"
         "  }]\n"
         "}\n";
}

std::string FindOcd2DictionaryDir(const std::string& configTestDirPath) {
  std::vector<fs::path> candidates;
  if (!PACKAGE_DATA_DIRECTORY.empty()) {
    candidates.push_back(fs::u8path(PACKAGE_DATA_DIRECTORY));
  }

  const fs::path configDir = fs::u8path(configTestDirPath);
  candidates.push_back(configDir.parent_path().parent_path() / "data" /
                       "dictionary");
  candidates.push_back(fs::current_path() / "data" / "dictionary");
  candidates.push_back(fs::current_path().parent_path() / "data" / "dictionary");

  for (const fs::path& candidate : candidates) {
    if (candidate.empty()) {
      continue;
    }
    const fs::path phrases = candidate / "STPhrases.ocd2";
    const fs::path characters = candidate / "STCharacters.ocd2";
    if (fs::is_regular_file(phrases) && fs::is_regular_file(characters)) {
      return PathString(candidate) + "/";
    }
  }
  return "";
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
  const std::string converted = converter->Convert(std::string_view(input));
  EXPECT_EQ(expected, converted);
}

TEST_F(ConfigTest, ConvertLength) {
  const std::string result = converter->Convert(std::string_view(input));
  EXPECT_EQ(expected.length(), result.length());
  EXPECT_EQ(expected, result);
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
    EXPECT_EQ(expected, tempConverter->Convert(std::string_view(input)));
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
    EXPECT_EQ(utf8("滑鼠"),
              tempConverter->Convert(std::string_view(utf8("鼠标"))));
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
    EXPECT_EQ(utf8("滑鼠"),
              tempConverter->Convert(std::string_view(utf8("鼠标"))));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, ZipProviderFindsConfigNameAndResources) {
  const fs::path tempDir = MakeTempDir("opencc-zip-provider-test");
  const fs::path zipPath = tempDir / "resources.zip";
  WriteStoredZip(zipPath, {
                              {"config.json", SingleDictConfig("dict.txt")},
                              {"dict.txt", utf8("鼠标\t滑鼠\n")},
                          });

  try {
    std::shared_ptr<ResourceProvider> provider(
        new ZipResourceProvider(PathString(zipPath)));
    const ConverterPtr tempConverter =
        config.NewFromFile("config.json", provider);
    EXPECT_EQ(utf8("滑鼠"),
              tempConverter->Convert(std::string_view(utf8("鼠标"))));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, ZipProviderDoesNotOverrideAbsoluteConfigPath) {
  const fs::path tempDir = MakeTempDir("opencc-zip-absolute-config-test");
  const fs::path zipPath = tempDir / "resources.zip";
  const fs::path configPath = tempDir / "config.json";
  WriteStoredZip(zipPath, {
                              {"config.json",
                               InlineSingleStepConfig(
                                   "{\n"
                                   "        \"鼠标\": \"鼠标\"\n"
                                   "      }",
                                   "{\n"
                                   "      \"type\": \"inline\",\n"
                                   "      \"entries\": {\n"
                                   "        \"鼠标\": \"乙\"\n"
                                   "      }\n"
                                   "    }")},
                          });
  WriteFile(configPath,
            InlineSingleStepConfig(
                "{\n"
                "        \"鼠标\": \"鼠标\"\n"
                "      }",
                "{\n"
                "      \"type\": \"inline\",\n"
                "      \"entries\": {\n"
                "        \"鼠标\": \"甲\"\n"
                "      }\n"
                "    }"));

  try {
    std::shared_ptr<ResourceProvider> provider(
        new ZipResourceProvider(PathString(zipPath)));
    const ConverterPtr tempConverter =
        config.NewFromFile(PathString(configPath), provider);
    EXPECT_EQ(utf8("甲"),
              tempConverter->Convert(std::string_view(utf8("鼠标"))));
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
    EXPECT_EQ(utf8("乙"),
              tempConverter->Convert(std::string_view(utf8("鼠标"))));
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
    EXPECT_EQ(utf8("滑鼠"),
              tempConverter->Convert(std::string_view(utf8("鼠标"))));
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
    EXPECT_EQ(utf8("第一"),
              tempConverter->Convert(std::string_view(utf8("鼠标"))));
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

TEST_F(ConfigTest, FilesystemResourceCacheKeyIncludesFreshness) {
  const fs::path tempDir = MakeTempDir("opencc-resource-cache-key-test");
  const fs::path resourceDir = tempDir / "resources";
  fs::create_directories(resourceDir);
  const fs::path dictPath = resourceDir / "dict.txt";
  WriteFile(dictPath, utf8("鼠标\t滑鼠\n"));

  try {
    FilesystemResourceProvider provider({PathString(resourceDir)});
    const std::shared_ptr<const ResourceProvider::Resource> resource =
        provider.GetResource("dict.txt");
    EXPECT_EQ(NormalizePathString(PathString(dictPath)),
              NormalizePathString(resource->Name()));
    const std::string oldKey =
        resource->Name() + "\n" + std::to_string(resource->Size());
    EXPECT_NE(oldKey, resource->CacheKey());
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
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
    EXPECT_EQ(utf8("伺服器"),
              tempConverter->Convert(std::string_view(utf8("服务器"))));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }
  fs::remove_all(tempDir);
}

TEST_F(ConfigTest, InlineDictBasicConversion) {
  const std::string json = InlineSingleStepConfig(
      "{\n"
      "        \"麦旋风\": \"麦旋风\"\n"
      "      }",
      "{\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"麦旋风\": \"冰炫風\"\n"
      "      }\n"
      "    }");

  const ConverterPtr inlineConverter = config.NewFromString(json, "");
  EXPECT_EQ(utf8("我想吃冰炫風"),
            inlineConverter->Convert(std::string_view(utf8("我想吃麦旋风"))));
}

TEST_F(ConfigTest, InlineDictInGroupTakesPriorityOverFollowingFileDict) {
  const std::string json =
      std::string("{\n"
                  "  \"name\": \"Inline Group Priority Test\",\n"
                  "  \"segmentation\": {\n"
                  "    \"type\": \"mmseg\",\n"
                  "    \"dict\": {\"type\": \"text\", \"file\": \"config_test_phrases.txt\"}\n"
                  "  },\n"
                  "  \"conversion_chain\": [{\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"group\",\n"
                  "      \"match_policy\": \"short_circuit\",\n"
                  "      \"dicts\": [\n"
                  "        {\n"
                  "          \"type\": \"inline\",\n"
                  "          \"entries\": {\n"
                  "            \"燕燕于飞\": \"自訂覆寫\"\n"
                  "          }\n"
                  "        },\n"
                  "        {\"type\": \"text\", \"file\": \"config_test_phrases.txt\"}\n"
                  "      ]\n"
                  "    }\n"
                  "  }]\n"
                  "}\n");

  const ConverterPtr inlineConverter =
      config.NewFromString(json, {CONFIG_TEST_DIR_PATH + "/"});
  EXPECT_EQ(utf8("自訂覆寫"),
            inlineConverter->Convert(std::string_view(utf8("燕燕于飞"))));
}

TEST_F(ConfigTest, GroupDictWithoutMatchPolicyDefaultsToShortCircuit) {
  const std::string json =
      std::string("{\n"
                  "  \"name\": \"Legacy Group Policy Test\",\n"
                  "  \"segmentation\": {\n"
                  "    \"type\": \"mmseg\",\n"
                  "    \"dict\": {\"type\": \"text\", \"file\": \"config_test_phrases.txt\"}\n"
                  "  },\n"
                  "  \"conversion_chain\": [{\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"group\",\n"
                  "      \"dicts\": [\n"
                  "        {\n"
                  "          \"type\": \"inline\",\n"
                  "          \"entries\": {\n"
                  "            \"燕燕于飞\": \"自訂覆寫\"\n"
                  "          }\n"
                  "        },\n"
                  "        {\"type\": \"text\", \"file\": \"config_test_phrases.txt\"}\n"
                  "      ]\n"
                  "    }\n"
                  "  }]\n"
                  "}\n");

  testing::internal::CaptureStderr();
  const ConverterPtr legacyConverter =
      config.NewFromString(json, {CONFIG_TEST_DIR_PATH + "/"});
  const std::string warning = testing::internal::GetCapturedStderr();
  EXPECT_NE(std::string::npos,
            warning.find("warning: config does not conform to schema"));
  EXPECT_EQ(utf8("自訂覆寫"),
            legacyConverter->Convert(std::string_view(utf8("燕燕于飞"))));
}

TEST_F(ConfigTest, UnionDictGroupPrefersLaterLongerMatch) {
  const std::string json =
      std::string("{\n"
                  "  \"name\": \"Union Group Test\",\n"
                  "  \"segmentation\": {\n"
                  "    \"type\": \"mmseg\",\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"inline\",\n"
                  "      \"entries\": {\n"
                  "        \"意大利面\": \"意大利面\"\n"
                  "      }\n"
                  "    }\n"
                  "  },\n"
                  "  \"conversion_chain\": [{\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"group\",\n"
                  "      \"match_policy\": \"union\",\n"
                  "      \"dicts\": [\n"
                  "        {\n"
                  "          \"type\": \"inline\",\n"
                  "          \"entries\": {\n"
                  "            \"意大利\": \"義大利\"\n"
                  "          }\n"
                  "        },\n"
                  "        {\n"
                  "          \"type\": \"inline\",\n"
                  "          \"entries\": {\n"
                  "            \"意大利面\": \"義大利麵\"\n"
                  "          }\n"
                  "        }\n"
                  "      ]\n"
                  "    }\n"
                  "  }]\n"
                  "}\n");

  const ConverterPtr inlineConverter = config.NewFromString(json, "");
  EXPECT_EQ(utf8("義大利麵"),
            inlineConverter->Convert(std::string_view(utf8("意大利面"))));
}

TEST_F(ConfigTest, InlineDictOutputStillProcessedByLaterChainStep) {
  const std::string json =
      std::string("{\n"
                  "  \"name\": \"Inline Chain Test\",\n"
                  "  \"segmentation\": {\n"
                  "    \"type\": \"mmseg\",\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"inline\",\n"
                  "      \"entries\": {\n"
                  "        \"A\": \"A\",\n"
                  "        \"B\": \"B\"\n"
                  "      }\n"
                  "    }\n"
                  "  },\n"
                  "  \"conversion_chain\": [\n"
                  "    {\n"
                  "      \"dict\": {\n"
                  "        \"type\": \"inline\",\n"
                  "        \"entries\": {\n"
                  "          \"A\": \"B\"\n"
                  "        }\n"
                  "      }\n"
                  "    },\n"
                  "    {\n"
                  "      \"dict\": {\n"
                  "        \"type\": \"inline\",\n"
                  "        \"entries\": {\n"
                  "          \"B\": \"C\"\n"
                  "        }\n"
                  "      }\n"
                  "    }\n"
                  "  ]\n"
                  "}\n");

  const ConverterPtr inlineConverter = config.NewFromString(json, "");
  EXPECT_EQ("C", inlineConverter->Convert(std::string_view("A")));
}

TEST_F(ConfigTest, InlineSegmentationDictUsesLongestMatch) {
  const std::string json =
      std::string("{\n"
                  "  \"name\": \"Inline Segmentation Test\",\n"
                  "  \"segmentation\": {\n"
                  "    \"type\": \"mmseg\",\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"inline\",\n"
                  "      \"entries\": {\n"
                  "        \"ABC\": \"ABC\",\n"
                  "        \"AB\": \"AB\",\n"
                  "        \"D\": \"D\"\n"
                  "      }\n"
                  "    }\n"
                  "  },\n"
                  "  \"conversion_chain\": [{\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"inline\",\n"
                  "      \"entries\": {\n"
                  "        \"ABC\": \"X\",\n"
                  "        \"AB\": \"Y\",\n"
                  "        \"C\": \"Z\",\n"
                  "        \"D\": \"D\"\n"
                  "      }\n"
                  "    }\n"
                  "  }]\n"
                  "}\n");

  const ConverterPtr inlineConverter = config.NewFromString(json, "");
  EXPECT_EQ("XD", inlineConverter->Convert(std::string_view("ABCD")));
}

TEST_F(ConfigTest, InlineDictPreservesExactStringSemantics) {
  const std::string json = InlineSingleStepConfig(
      "{\n"
      "        \"A\": \"A\",\n"
      "        \" A \": \" A \"\n"
      "      }",
      "{\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \" A \": \"B\"\n"
      "      }\n"
      "    }");

  const ConverterPtr inlineConverter = config.NewFromString(json, "");
  EXPECT_EQ("A", inlineConverter->Convert(std::string_view("A")));
  EXPECT_EQ("B", inlineConverter->Convert(std::string_view(" A ")));
}

TEST_F(ConfigTest, InlineDictValidationErrors) {
  const auto ExpectInvalidFormat = [this](const std::string& json,
                                          const std::string& expectedMessage) {
    testing::internal::CaptureStderr();
    try {
      const ConverterPtr _ = config.NewFromString(json, "");
      (void)testing::internal::GetCapturedStderr();
      FAIL() << "Expected InvalidFormat";
    } catch (const InvalidFormat& e) {
      const std::string warning = testing::internal::GetCapturedStderr();
      EXPECT_NE(std::string::npos,
                warning.find("warning: config does not conform to schema"));
      EXPECT_NE(std::string::npos,
                std::string(e.what()).find(expectedMessage));
    }
  };

  ExpectInvalidFormat(
      "{\n"
      "  \"segmentation\": {\n"
      "    \"type\": \"mmseg\",\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"A\": \"A\"\n"
      "      }\n"
      "    }\n"
      "  },\n"
      "  \"conversion_chain\": [{\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\"\n"
      "    }\n"
      "  }]\n"
      "}\n",
      "Required property not found: entries");

  ExpectInvalidFormat(
      "{\n"
      "  \"segmentation\": {\n"
      "    \"type\": \"mmseg\",\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"A\": \"A\"\n"
      "      }\n"
      "    }\n"
      "  },\n"
      "  \"conversion_chain\": [{\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": []\n"
      "    }\n"
      "  }]\n"
      "}\n",
      "Property must be an object: entries");

  ExpectInvalidFormat(
      "{\n"
      "  \"segmentation\": {\n"
      "    \"type\": \"mmseg\",\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"A\": \"A\"\n"
      "      }\n"
      "    }\n"
      "  },\n"
      "  \"conversion_chain\": [{\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"\": \"B\"\n"
      "      }\n"
      "    }\n"
      "  }]\n"
      "}\n",
      "Inline dictionary key must be a non-empty string");

  ExpectInvalidFormat(
      "{\n"
      "  \"segmentation\": {\n"
      "    \"type\": \"mmseg\",\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"A\": \"A\"\n"
      "      }\n"
      "    }\n"
      "  },\n"
      "  \"conversion_chain\": [{\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"A\": \"\"\n"
      "      }\n"
      "    }\n"
      "  }]\n"
      "}\n",
      "Inline dictionary value must be a non-empty string: A");

  ExpectInvalidFormat(
      "{\n"
      "  \"segmentation\": {\n"
      "    \"type\": \"mmseg\",\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"A\": \"A\"\n"
      "      }\n"
      "    }\n"
      "  },\n"
      "  \"conversion_chain\": [{\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"A\": 1\n"
      "      }\n"
      "    }\n"
      "  }]\n"
      "}\n",
      "Inline dictionary value must be a non-empty string: A");

  ExpectInvalidFormat(
      "{\n"
      "  \"segmentation\": {\n"
      "    \"type\": \"mmseg\",\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"entries\": {\n"
      "        \"A\": \"A\"\n"
      "      }\n"
      "    }\n"
      "  },\n"
      "  \"conversion_chain\": [{\n"
      "    \"dict\": {\n"
      "      \"type\": \"inline\",\n"
      "      \"may_output_tofu\": true,\n"
      "      \"entries\": {\n"
      "        \"A\": \"B\"\n"
      "      }\n"
      "    }\n"
      "  }]\n"
      "}\n",
      "Inline dictionary does not support may_output_tofu");
}

TEST_F(ConfigTest, InlineDictSupportsJsoncCommentsAndTrailingComma) {
  const std::string json =
      std::string("{\n"
                  "  \"name\": \"Inline JSONC Test\",\n"
                  "  \"segmentation\": {\n"
                  "    \"type\": \"mmseg\",\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"inline\",\n"
                  "      \"entries\": {\n"
                  "        \"麦旋风\": \"麦旋风\"\n"
                  "      }\n"
                  "    }\n"
                  "  },\n"
                  "  \"conversion_chain\": [{\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"inline\",\n"
                  "      \"entries\": {\n"
                  "        // Custom entry.\n"
                  "        \"麦旋风\": \"冰炫風\",\n"
                  "      }\n"
                  "    }\n"
                  "  }]\n"
                  "}\n");

  const ConverterPtr inlineConverter = config.NewFromString(json, "");
  EXPECT_EQ(utf8("冰炫風"),
            inlineConverter->Convert(std::string_view(utf8("麦旋风"))));
}

TEST_F(ConfigTest, InlineSegmentationAndConversionWorksWithOcd2GroupDicts) {
  const std::string ocd2Dir = FindOcd2DictionaryDir(CONFIG_TEST_DIR_PATH);
  if (ocd2Dir.empty()) {
    GTEST_SKIP() << "STPhrases.ocd2/STCharacters.ocd2 not found";
  }

  const std::string json =
      std::string("{\n"
                  "  \"name\": \"Inline Segmentation+Conversion ocd2 Test\",\n"
                  "  \"segmentation\": {\n"
                  "    \"type\": \"mmseg\",\n"
                  "    \"dict\": {\n"
                  "      \"type\": \"group\",\n"
                  "      \"match_policy\": \"short_circuit\",\n"
                  "      \"dicts\": [\n"
                  "        {\n"
                  "          \"type\": \"inline\",\n"
                  "          \"entries\": {\n"
                  "            \"台湾\": \"台灣\"\n"
                  "          }\n"
                  "        },\n"
                  "        {\"type\": \"ocd2\", \"file\": \"STPhrases.ocd2\"}\n"
                  "      ]\n"
                  "    }\n"
                  "  },\n"
                  "  \"conversion_chain\": [\n"
                  "    {\n"
                  "      \"dict\": {\n"
                  "        \"type\": \"group\",\n"
                  "        \"match_policy\": \"short_circuit\",\n"
                  "        \"dicts\": [\n"
                  "          {\n"
                  "            \"type\": \"inline\",\n"
                  "            \"entries\": {\n"
                  "              \"台灣\": \"台灣\"\n"
                  "            }\n"
                  "          },\n"
                  "          {\"type\": \"ocd2\", \"file\": \"STPhrases.ocd2\"},\n"
                  "          {\"type\": \"ocd2\", \"file\": \"STCharacters.ocd2\"}\n"
                  "        ]\n"
                  "      }\n"
                  "    }\n"
                  "  ]\n"
                  "}\n");

  const ConverterPtr inlineConverter = config.NewFromString(json, {ocd2Dir});
  EXPECT_EQ(utf8("臺灣"),
            inlineConverter->Convert(std::string_view(utf8("台湾"))));
  EXPECT_EQ(utf8("台灣"),
            inlineConverter->Convert(std::string_view(utf8("台灣"))));
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
    EXPECT_EQ(expected, unicodeConverter->Convert(std::string_view(input)));
  } catch (...) {
    fs::remove_all(tempDir);
    throw;
  }

  fs::remove_all(tempDir);
}
#endif

// ── Normalization tests ──────────────────────────────────────────────────────

TEST_F(ConfigTest, NormalizationRunsBeforeSegmentationAndConversion) {
  // normalization: 甲 → 乙   main conversion: 乙 → 丙
  // Without normalization the main dict would not match 甲.
  const std::string config = R"({
    "name": "Normalization Test",
    "normalization": [{"dict": {"type": "inline", "entries": {"甲": "乙"}}}],
    "conversion_chain": [{"dict": {"type": "inline", "entries": {"乙": "丙"}}}]
  })";
  Config c;
  const ConverterPtr conv = c.NewFromString(config, CONFIG_TEST_DIR_PATH);
  EXPECT_EQ(utf8("丙"), conv->Convert(utf8("甲")));
}

TEST_F(ConfigTest, AbsentNormalizationPreservesOriginalBehavior) {
  const std::string config = R"({
    "name": "No Normalization Test",
    "conversion_chain": [{"dict": {"type": "inline", "entries": {"甲": "丙"}}}]
  })";
  Config c;
  const ConverterPtr conv = c.NewFromString(config, CONFIG_TEST_DIR_PATH);
  EXPECT_EQ(utf8("丙"), conv->Convert(utf8("甲")));
}

TEST_F(ConfigTest, NormalizationMissingFileDictThrows) {
  const std::string config = R"({
    "name": "Bad Normalization",
    "normalization": [{"dict": {"type": "ocd2", "file": "nonexistent.ocd2"}}],
    "conversion_chain": [{"dict": {"type": "inline", "entries": {"甲": "丙"}}}]
  })";
  Config c;
  EXPECT_THROW(c.NewFromString(config, CONFIG_TEST_DIR_PATH), Exception);
}

TEST_F(ConfigTest, NormalizationGetConversionChainIsNonNull) {
  // Regression: converters built from configs with a normalization field must
  // expose a non-null GetConversionChain() so that downstream consumers such
  // as librime do not crash on null-pointer dereference.
  // The returned chain must represent the main conversion_chain (乙→丙), not
  // the normalization chain (甲→乙).
  const std::string config = R"({
    "name": "Normalization Chain Test",
    "normalization": [{"dict": {"type": "inline", "entries": {"甲": "乙"}}}],
    "conversion_chain": [{"dict": {"type": "inline", "entries": {"乙": "丙"}}}]
  })";
  Config c;
  const ConverterPtr conv = c.NewFromString(config, CONFIG_TEST_DIR_PATH);
  const ConversionChainPtr chain = conv->GetConversionChain();
  ASSERT_NE(nullptr, chain);
  // Verify it is the main chain: 乙 → 丙 (not the normalization chain 甲 → 乙).
  const SegmentsPtr result = chain->Convert(SegmentsPtr(new Segments{utf8("乙")}));
  ASSERT_EQ(1u, result->Length());
  EXPECT_EQ(utf8("丙"), result->At(0));
}

} // namespace opencc
