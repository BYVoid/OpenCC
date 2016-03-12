/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
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

#pragma once

// Microsoft Visual C++ specific
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable : 4251 4266 4350 4503 4512 4514 4710 4820)
#endif

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "Exception.hpp"
#include "Export.hpp"
#include "Optional.hpp"

using std::list;
using std::string;
using std::vector;

// Forward decalarations and alias
namespace opencc {
class BinaryDict;
class Config;
class Conversion;
class ConversionChain;
class Converter;
class DartsDict;
class Dict;
class DictEntry;
class DictGroup;
class Lexicon;
class MultiValueDictEntry;
class NoValueDictEntry;
class Segmentation;
class Segments;
class SerializableDict;
class SingleValueDictEntry;
class TextDict;
typedef std::shared_ptr<BinaryDict> BinaryDictPtr;
typedef std::shared_ptr<Conversion> ConversionPtr;
typedef std::shared_ptr<ConversionChain> ConversionChainPtr;
typedef std::shared_ptr<Converter> ConverterPtr;
typedef std::shared_ptr<DartsDict> DartsDictPtr;
typedef std::shared_ptr<Dict> DictPtr;
typedef std::shared_ptr<DictGroup> DictGroupPtr;
typedef std::shared_ptr<Lexicon> LexiconPtr;
typedef std::shared_ptr<Segmentation> SegmentationPtr;
typedef std::shared_ptr<Segments> SegmentsPtr;
typedef std::shared_ptr<SerializableDict> SerializableDictPtr;
typedef std::shared_ptr<TextDict> TextDictPtr;
}

#ifndef PKGDATADIR
const string PACKAGE_DATA_DIRECTORY = "";
#else // ifndef PKGDATADIR
const string PACKAGE_DATA_DIRECTORY = PKGDATADIR "/";
#endif // ifndef PKGDATADIR

#ifndef VERSION
#define VERSION "1.0.*"
#endif // ifndef VERSION
