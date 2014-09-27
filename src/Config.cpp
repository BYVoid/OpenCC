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

using namespace opencc;
typedef rapidjson::GenericValue<rapidjson::UTF8<>> JSONValue;

Config::Config() {}

Config::Config(const string fileName) {
  LoadFile(fileName);
}

JSONValue& GetProperty(JSONValue& doc, const char* name) {
  if (!doc.HasMember(name)) {
    throw InvalidFormat("Required property not found: " + string(name));
  }
  return doc[name];
}

JSONValue& GetObjectProperty(JSONValue& doc, const char* name) {
  JSONValue& obj = GetProperty(doc, name);
  if (!obj.IsObject()) {
    throw InvalidFormat("Property must be an object: " + string(name));
  }
  return obj;
}

JSONValue& GetArrayProperty(JSONValue& doc, const char* name) {
  JSONValue& obj = GetProperty(doc, name);
  if (!obj.IsArray()) {
    throw InvalidFormat("Property must be an array: " + string(name));
  }
  return obj;
}

const char* GetStringProperty(JSONValue& doc, const char* name) {
  JSONValue& obj = GetProperty(doc, name);
  if (!obj.IsString()) {
    throw InvalidFormat("Property must be a string: " + string(name));
  }
  return obj.GetString();
}

void LoadDictWithPaths(SerializableDictPtr dict,
                       string& fileName,
                       string& configDirectory) {
  // Working directory
  if (dict->TryLoadFromFile(fileName)) {
    return;
  }
  // Configuration directory
  if ((configDirectory != "") &&
      dict->TryLoadFromFile(configDirectory + fileName)) {
    return;
  }
  // Package data directory
  if ((PACKAGE_DATA_DIRECTORY != "") &&
      dict->TryLoadFromFile(PACKAGE_DATA_DIRECTORY + fileName)) {
    return;
  }
  throw FileNotFound(fileName);
}

DictPtr ParseDict(JSONValue& doc, string& configDirectory) {
  // Required: type
  string type = GetStringProperty(doc, "type");
  DictPtr dict;
  if (type == "group") {
    DictGroupPtr dictGroup(new DictGroup);
    JSONValue& docs = GetArrayProperty(doc, "dicts");
    for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
      if (docs[i].IsObject()) {
        DictPtr dict = ParseDict(docs[i], configDirectory);
        dictGroup->AddDict(dict);
      } else {
        throw InvalidFormat("Element of the array must be an object");
      }
    }
    dict = dictGroup;
  } else if (type == "text") {
    string fileName = GetStringProperty(doc, "file");
    SerializableDictPtr textDict(new TextDict());
    LoadDictWithPaths(textDict, fileName, configDirectory);
    dict = textDict;
  } else if (type == "ocd") {
    string fileName = GetStringProperty(doc, "file");
    SerializableDictPtr dartsDict(new DartsDict());
    LoadDictWithPaths(dartsDict, fileName, configDirectory);
    dict = dartsDict;
  } else {
    throw InvalidFormat("Unknown dictionary type: " + type);
  }
  return dict;
}

SegmentationPtr ParseSegmentation(JSONValue& doc, string& configDirectory) {
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

ConversionPtr ParseConversion(JSONValue& doc, string& configDirectory) {
  // Required: dict
  DictPtr dict = ParseDict(GetObjectProperty(doc, "dict"), configDirectory);
  ConversionPtr conversion(new Conversion(dict));

  return conversion;
}

ConversionChainPtr ParseConversionChain(JSONValue& docs,
                                        string& configDirectory) {
  ConversionChainPtr chain(new ConversionChain);
  for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
    JSONValue& doc = docs[i];
    if (doc.IsObject()) {
      ConversionPtr conversion = ParseConversion(doc, configDirectory);
      chain->AddConversion(conversion);
    } else {}
  }
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

void Config::LoadFile(const string fileName) {
  string prefixedFileName = FindConfigFile(fileName);
  std::ifstream ifs(prefixedFileName);
  string content(std::istreambuf_iterator<char>(ifs),
                 (std::istreambuf_iterator<char>()));

#if defined(_WIN32) || defined(_WIN64)
  UTF8Util::ReplaceAll(prefixedFileName, "\\", "/");
#endif // if defined(_WIN32) || defined(_WIN64)
  size_t slashPos = prefixedFileName.rfind("/");
  if (slashPos == string::npos) {
    configDirectory = "";
  } else {
    configDirectory = prefixedFileName.substr(0, slashPos) + "/";
  }
  LoadString(content);
}

void Config::LoadString(const string json) {
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
  converter = ConverterPtr(new Converter(name, segmentation, chain));
}

ConverterPtr Config::GetConverter() const {
  return converter;
}
