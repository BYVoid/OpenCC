/**
 * Open Chinese Convert
 *
 * Copyright 2010-2013 BYVoid <byvoid@byvoid.com>
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

#include "Config.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "DartsDict.hpp"
#include "DictGroup.hpp"
#include "MaxMatchSegmentation.hpp"
#include "TextDict.hpp"
#include "UTF8Util.hpp"

#include "document.h"

#include <unordered_map>
#include <mutex>

using namespace opencc;

std::unordered_map<string, // type
  std::unordered_map<string, // configDirectory
    std::unordered_map<string, // file
      DictPtr
    >
  >
> dictCache;
std::mutex dictCacheLock;

typedef rapidjson::GenericValue < rapidjson::UTF8 < char >> JSONValue;

const JSONValue& GetProperty(const JSONValue& doc, const char* name) {
  if (!doc.HasMember(name)) {
    throw InvalidFormat("Required property not found: " + string(name));
  }
  return doc[name];
}

const JSONValue& GetObjectProperty(const JSONValue& doc, const char* name) {
  const JSONValue& obj = GetProperty(doc, name);
  if (!obj.IsObject()) {
    throw InvalidFormat("Property must be an object: " + string(name));
  }
  return obj;
}

const JSONValue& GetArrayProperty(const JSONValue& doc, const char* name) {
  const JSONValue& obj = GetProperty(doc, name);
  if (!obj.IsArray()) {
    throw InvalidFormat("Property must be an array: " + string(name));
  }
  return obj;
}

const char* GetStringProperty(const JSONValue& doc, const char* name) {
  const JSONValue& obj = GetProperty(doc, name);
  if (!obj.IsString()) {
    throw InvalidFormat("Property must be a string: " + string(name));
  }
  return obj.GetString();
}

template<typename DICT>
SerializableDictPtr LoadDictWithPaths(const string& fileName,
                                      const string& configDirectory) {
  // Working directory
  std::shared_ptr<DICT> dict;
  if (SerializableDict::TryLoadFromFile<DICT>(fileName, &dict)) {
    return dict;
  }
  // Configuration directory
  if ((configDirectory != "") &&
      SerializableDict::TryLoadFromFile<DICT>(configDirectory + fileName,
                                              &dict)) {
    return dict;
  }
  // Package data directory
  if ((PACKAGE_DATA_DIRECTORY != "") &&
      SerializableDict::TryLoadFromFile<DICT>(PACKAGE_DATA_DIRECTORY + fileName,
                                              &dict)) {
    return dict;
  }
  throw FileNotFound(fileName);
}

DictPtr ParseDict(const JSONValue& doc, const string& configDirectory) {
  // Required: type
  string type = GetStringProperty(doc, "type");
  DictPtr dict;
  if (type == "group") {
    list<DictPtr> dicts;
    const JSONValue& docs = GetArrayProperty(doc, "dicts");
    for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
      if (docs[i].IsObject()) {
        DictPtr dict = ParseDict(docs[i], configDirectory);
        dicts.push_back(dict);
      } else {
        throw InvalidFormat("Element of the array must be an object");
      }
    }
    return DictGroupPtr(new DictGroup(dicts));
  } else {
    string fileName = GetStringProperty(doc, "file");
    // Read from cache
    DictPtr& cache = dictCache[type][configDirectory][fileName];
    if (cache != nullptr) {
      return cache;
    }
    if (type == "text") {
      dict = LoadDictWithPaths<TextDict>(fileName, configDirectory);
    } else if (type == "ocd") {
      dict = LoadDictWithPaths<DartsDict>(fileName, configDirectory);
    } else {
      throw InvalidFormat("Unknown dictionary type: " + type);
    }
    // Update Cache
    dictCacheLock.lock();
    cache = dict;
    dictCacheLock.unlock();
    return dict;
  }
}

SegmentationPtr ParseSegmentation(const JSONValue& doc,
                                  const string& configDirectory) {
  SegmentationPtr segmentation;

  // Required: type
  string type = GetStringProperty(doc, "type");
  if (type == "mmseg") {
    // Required: dict
    DictPtr dict = ParseDict(GetObjectProperty(doc, "dict"), configDirectory);
    segmentation = SegmentationPtr(new MaxMatchSegmentation(dict));
  } else {
    throw InvalidFormat("Unknown segmentation type: " + type);
  }
  return segmentation;
}

ConversionPtr ParseConversion(const JSONValue& doc,
                              const string& configDirectory) {
  // Required: dict
  DictPtr dict = ParseDict(GetObjectProperty(doc, "dict"), configDirectory);
  ConversionPtr conversion(new Conversion(dict));

  return conversion;
}

ConversionChainPtr ParseConversionChain(const JSONValue& docs,
                                        const string& configDirectory) {
  list<ConversionPtr> conversions;
  for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
    const JSONValue& doc = docs[i];
    if (doc.IsObject()) {
      ConversionPtr conversion = ParseConversion(doc, configDirectory);
      conversions.push_back(conversion);
    } else {}
  }
  ConversionChainPtr chain(new ConversionChain(conversions));
  return chain;
}

string FindConfigFile(string fileName) {
  std::ifstream ifs;

  // Working directory
  ifs.open(fileName.c_str());
  if (ifs.is_open()) {
    return fileName;
  }
  // Package data directory
  if (PACKAGE_DATA_DIRECTORY != "") {
    string prefixedFileName = PACKAGE_DATA_DIRECTORY + fileName;
    ifs.open(prefixedFileName.c_str());
    if (ifs.is_open()) {
      return prefixedFileName;
    }
  }
  throw FileNotFound(fileName);
}

Config::Config(const ConverterPtr _converter) : converter(_converter) {}

Config Config::NewFromFile(const string& fileName) {
  string prefixedFileName = FindConfigFile(fileName);
  std::ifstream ifs(prefixedFileName);
  string content(std::istreambuf_iterator<char>(ifs),
                 (std::istreambuf_iterator<char>()));

#if defined(_WIN32) || defined(_WIN64)
  UTF8Util::ReplaceAll(prefixedFileName, "\\", "/");
#endif // if defined(_WIN32) || defined(_WIN64)
  size_t slashPos = prefixedFileName.rfind("/");
  string configDirectory = "";
  if (slashPos != string::npos) {
    configDirectory = prefixedFileName.substr(0, slashPos) + "/";
  }
  return NewFromString(content, configDirectory);
}

Config Config::NewFromString(const string& json, const string& configDirectory) {
  rapidjson::Document doc;

  doc.ParseInsitu<0>((char*)json.c_str());
  if (doc.HasParseError()) {
    throw InvalidFormat("Error parsing JSON"); // doc.GetErrorOffset()
  }
  if (!doc.IsObject()) {
    throw InvalidFormat("Root of configuration must be an object");
  }

  // Optional: name
  string name;
  if (doc.HasMember("name") && doc["name"].IsString()) {
    name = doc["name"].GetString();
  }

  // Required: segmentation
  SegmentationPtr segmentation =
    ParseSegmentation(GetObjectProperty(doc, "segmentation"), configDirectory);

  // Required: conversion_chain
  ConversionChainPtr chain =
    ParseConversionChain(GetArrayProperty(doc,
                                          "conversion_chain"), configDirectory);
  return Config(ConverterPtr(new Converter(name, segmentation, chain)));
}

ConverterPtr Config::GetConverter() const {
  return converter;
}
