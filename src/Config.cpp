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
#include "MaxMatchSegmentation.hpp"
#include "ConversionChain.hpp"
#include "TextDict.hpp"
#include "DartsDict.hpp"
#include "DictGroup.hpp"
#include "document.h"

using namespace Opencc;
using JSONValue = rapidjson::GenericValue<rapidjson::UTF8<>>;

Config::Config() {
}

Config::Config(const string fileName) {
  LoadFile(fileName);
}

void Config::LoadFile(const string fileName) {
  std::ifstream ifs(fileName);
  string content(std::istreambuf_iterator<char>(ifs),
                 (std::istreambuf_iterator<char>()));
  LoadString(content);
}

JSONValue& GetProperty(JSONValue& doc, const char* name) {
  if (!doc.HasMember(name)) {
    throw runtime_error("Required property not found: " + string(name));
  }
  return doc[name];
}

JSONValue& GetObjectProperty(JSONValue& doc, const char* name) {
  JSONValue& obj = GetProperty(doc, name);
  if (!obj.IsObject()) {
    throw runtime_error("Property must be an object: " + string(name));
  }
  return obj;
}

JSONValue& GetArrayProperty(JSONValue& doc, const char* name) {
  JSONValue& obj = GetProperty(doc, name);
  if (!obj.IsArray()) {
    throw runtime_error("Property must be an array: " + string(name));
  }
  return obj;
}

const char* GetStringProperty(JSONValue& doc, const char* name) {
  JSONValue& obj = GetProperty(doc, name);
  if (!obj.IsString()) {
    throw runtime_error("Property must be a string: " + string(name));
  }
  return obj.GetString();
}

DictPtr ParseDict(JSONValue& doc) {
  // Required: type
  string type = GetStringProperty(doc, "type");
  DictPtr dict;
  if (type == "group") {
    DictGroup* dictGroup = new DictGroup;
    JSONValue& docs = GetArrayProperty(doc, "dicts");
    for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
      if (docs[i].IsObject()) {
        DictPtr dict = ParseDict(docs[i]);
        dictGroup->AddDict(dict);
      } else {
        throw runtime_error("Element of the array must be an object");
      }
    }
    dict.reset(dictGroup);
  } else if (type == "text") {
    string fileName = GetStringProperty(doc, "file");
    TextDict* textDict = new TextDict();
    // TODO try multiple paths
    textDict->LoadFromFile(fileName);
    dict.reset(textDict);
  } else if (type == "darts") {
    string fileName = GetStringProperty(doc, "file");
    DartsDict* dartsDict = new DartsDict();
    dartsDict->LoadFromFile(fileName);
    dict.reset(dartsDict);
  } else {
    throw runtime_error("Unknown type: " + type);
  }
  return dict;
}

ConversionPtr ParseConversion(JSONValue& doc) {
  // Required: type
  string type = GetStringProperty(doc, "type");
  if (type == "mmseg") {
    // Required: type
    DictPtr dict = ParseDict(GetObjectProperty(doc, "dict"));
    SegmentationPtr segmentation(new MaxMatchSegmentation(dict));
    ConversionPtr conversion(new Conversion(segmentation));
    return conversion;
  } else {
    throw runtime_error("Unknown type: " + type);
  }
}

ConversionChainPtr ParseConversionChain(JSONValue& docs) {
  ConversionChainPtr chain(new ConversionChain);
  for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
    JSONValue& doc = docs[i];
    if (doc.IsObject()) {
      ConversionPtr conversion = ParseConversion(doc);
      chain->AddConversion(conversion);
    } else {
    }
  }
  return chain;
}

void Config::LoadString(const string json) {
  rapidjson::Document doc;
  doc.ParseInsitu<0>((char*)json.c_str());
  if (doc.HasParseError()) {
    throw runtime_error("Error parsing JSON"); //doc.GetErrorOffset()
  }
  if (!doc.IsObject()) {
    throw runtime_error("Root of configuration must be an object");
  }
  // Optional: name
  if (doc.HasMember("name") && doc["name"].IsString()) {
    name = doc["name"].GetString();
  }
  // Required: conversion_chain
  if (doc.HasMember("conversion_chain") && doc["conversion_chain"].IsArray()) {
    chain = ParseConversionChain(doc["conversion_chain"]);
  } else {
    throw runtime_error("Required property 'conversion_chain' not found.");
  }
}
