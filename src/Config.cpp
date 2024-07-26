/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 Carbo Kuo <byvoid@byvoid.com>
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
#include <list>
#include <unordered_map>

#include <rapidjson/document.h>

#include "Config.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "DictGroup.hpp"
#include "Exception.hpp"
#include "MarisaDict.hpp"
#include "MaxMatchSegmentation.hpp"
#include "TextDict.hpp"

#ifdef ENABLE_DARTS
#include "DartsDict.hpp"
#endif

typedef rapidjson::GenericValue<rapidjson::UTF8<char>> JSONValue;

namespace opencc {

namespace {

class ConfigInternal {
public:
  std::vector<std::string> paths;

  const JSONValue& GetProperty(const JSONValue& doc, const char* name) {
    if (!doc.HasMember(name)) {
      throw InvalidFormat("Required property not found: " + std::string(name));
    }
    return doc[name];
  }

  const JSONValue& GetObjectProperty(const JSONValue& doc, const char* name) {
    const JSONValue& obj = GetProperty(doc, name);
    if (!obj.IsObject()) {
      throw InvalidFormat("Property must be an object: " + std::string(name));
    }
    return obj;
  }

  const JSONValue& GetArrayProperty(const JSONValue& doc, const char* name) {
    const JSONValue& obj = GetProperty(doc, name);
    if (!obj.IsArray()) {
      throw InvalidFormat("Property must be an array: " + std::string(name));
    }
    return obj;
  }

  const char* GetStringProperty(const JSONValue& doc, const char* name) {
    const JSONValue& obj = GetProperty(doc, name);
    if (!obj.IsString()) {
      throw InvalidFormat("Property must be a std::string: " +
                          std::string(name));
    }
    return obj.GetString();
  }

  template <typename DICT>
  DictPtr LoadDictWithPaths(const std::string& fileName) {
    // Working directory
    std::shared_ptr<DICT> dict;
    if (SerializableDict::TryLoadFromFile<DICT>(fileName, &dict)) {
      return dict;
    }
    for (const std::string& dirPath : paths) {
      std::string path = dirPath + '/' + fileName;
      if (SerializableDict::TryLoadFromFile<DICT>(path, &dict)) {
        return dict;
      }
    }
    throw FileNotFound(fileName);
  }

  DictPtr LoadDictFromFile(const std::string& type,
                           const std::string& fileName) {
    if (type == "text") {
      DictPtr dict = LoadDictWithPaths<TextDict>(fileName);
      return MarisaDict::NewFromDict(*dict.get());
    }
#ifdef ENABLE_DARTS
    if (type == "ocd") {
      return LoadDictWithPaths<DartsDict>(fileName);
    }
#endif
    if (type == "ocd2") {
      return LoadDictWithPaths<MarisaDict>(fileName);
    }
    throw InvalidFormat("Unknown dictionary type: " + type);
    return nullptr;
  }

  DictPtr ParseDict(const JSONValue& doc) {
    // Required: type
    std::string type = GetStringProperty(doc, "type");

    if (type == "group") {
      std::list<DictPtr> dicts;
      const JSONValue& docs = GetArrayProperty(doc, "dicts");
      for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
        if (docs[i].IsObject()) {
          DictPtr dict = ParseDict(docs[i]);
          dicts.push_back(dict);
        } else {
          throw InvalidFormat("Element of the array must be an object");
        }
      }
      return DictGroupPtr(new DictGroup(dicts));
    } else {
      std::string fileName = GetStringProperty(doc, "file");
      DictPtr dict = LoadDictFromFile(type, fileName);
      return dict;
    }
  }

  SegmentationPtr ParseSegmentation(const JSONValue& doc) {
    SegmentationPtr segmentation;

    // Required: type
    std::string type = GetStringProperty(doc, "type");
    if (type == "mmseg") {
      // Required: dict
      DictPtr dict = ParseDict(GetObjectProperty(doc, "dict"));
      segmentation = SegmentationPtr(new MaxMatchSegmentation(dict));
    } else {
      throw InvalidFormat("Unknown segmentation type: " + type);
    }
    return segmentation;
  }

  ConversionPtr ParseConversion(const JSONValue& doc) {
    // Required: dict
    DictPtr dict = ParseDict(GetObjectProperty(doc, "dict"));
    ConversionPtr conversion(new Conversion(dict));

    return conversion;
  }

  ConversionChainPtr ParseConversionChain(const JSONValue& docs) {
    std::list<ConversionPtr> conversions;
    for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
      const JSONValue& doc = docs[i];
      if (doc.IsObject()) {
        ConversionPtr conversion = ParseConversion(doc);
        conversions.push_back(conversion);
      } else {
      }
    }
    ConversionChainPtr chain(new ConversionChain(conversions));
    return chain;
  }

  std::string FindConfigFile(std::string fileName,
                             const std::vector<std::string>& configPaths) {
    std::ifstream ifs;

    // Working directory
    ifs.open(UTF8Util::GetPlatformString(fileName).c_str());
    if (ifs.is_open()) {
      return fileName;
    }
    // Package data directory
    if (PACKAGE_DATA_DIRECTORY != "") {
      std::string prefixedFileName = PACKAGE_DATA_DIRECTORY + fileName;
      ifs.open(UTF8Util::GetPlatformString(prefixedFileName).c_str());
      if (ifs.is_open()) {
        return prefixedFileName;
      }
      prefixedFileName += ".json";
      ifs.open(UTF8Util::GetPlatformString(prefixedFileName).c_str());
      if (ifs.is_open()) {
        return prefixedFileName;
      }
    }

    for (const std::string& dirPath : configPaths) {
      std::string path = dirPath + '/' + fileName;
      ifs.open(UTF8Util::GetPlatformString(path).c_str());
      if (ifs.is_open()) {
        return path;
      }
    }

    throw FileNotFound(fileName);
  }
};
} // namespace

Config::Config() : internal(new ConfigInternal()) {}

Config::~Config() { delete (ConfigInternal*)internal; }

ConverterPtr Config::NewFromFile(const std::string& fileName,
                                 const std::vector<std::string>& paths) {
  ConfigInternal* impl = (ConfigInternal*)internal;
  std::string prefixedFileName = impl->FindConfigFile(fileName, paths);
  std::ifstream ifs(UTF8Util::GetPlatformString(prefixedFileName));
  std::string content(std::istreambuf_iterator<char>(ifs),
                      (std::istreambuf_iterator<char>()));

#if defined(_WIN32) || defined(_WIN64)
  UTF8Util::ReplaceAll(prefixedFileName, "\\", "/");
#endif // if defined(_WIN32) || defined(_WIN64)
  size_t slashPos = prefixedFileName.rfind("/");
  std::string configDirectory = "";
  if (slashPos != std::string::npos) {
    configDirectory = prefixedFileName.substr(0, slashPos) + "/";
  }
  std::vector<std::string> dictPaths = paths;
  if (!configDirectory.empty()) {
    dictPaths.push_back(configDirectory);
  }
  return NewFromString(content, dictPaths);
}

ConverterPtr Config::NewFromString(const std::string& json,
                                   const std::string& configDirectory) {
  std::vector<std::string> paths;
  if (!configDirectory.empty()) {
    if (configDirectory.back() == '/' || configDirectory.back() == '\\')
      paths.push_back(configDirectory);
    else
      paths.push_back(configDirectory + '/');
  }
  return NewFromString(json, paths);
}

ConverterPtr Config::NewFromString(const std::string& json,
                                   const std::vector<std::string>& paths) {
  rapidjson::Document doc;

  doc.ParseInsitu<0>(const_cast<char*>(json.c_str()));
  if (doc.HasParseError()) {
    throw InvalidFormat("Error parsing JSON"); // doc.GetErrorOffset()
  }
  if (!doc.IsObject()) {
    throw InvalidFormat("Root of configuration must be an object");
  }

  // Optional: name
  std::string name;
  if (doc.HasMember("name") && doc["name"].IsString()) {
    name = doc["name"].GetString();
  }

  ConfigInternal* impl = (ConfigInternal*)internal;
  impl->paths = paths;
  if (PACKAGE_DATA_DIRECTORY != "") {
    impl->paths.push_back(PACKAGE_DATA_DIRECTORY);
  }

  // Required: segmentation
  SegmentationPtr segmentation =
      impl->ParseSegmentation(impl->GetObjectProperty(doc, "segmentation"));

  // Required: conversion_chain
  ConversionChainPtr chain = impl->ParseConversionChain(
      impl->GetArrayProperty(doc, "conversion_chain"));
  return ConverterPtr(new Converter(name, segmentation, chain));
}

}; // namespace opencc
