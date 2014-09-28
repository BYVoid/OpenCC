/*
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

#pragma once

#include <algorithm>
#include <fstream>
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

using std::string;
using std::vector;
using std::list;

// Forward decalarations and alias
namespace opencc {
  class Config;
  class DictEntry;
  class Dict;
  class SerializableDict;
  class TextDict;
  class DartsDict;
  class DictGroup;
  class Segmentation;
  class Conversion;
  class ConversionChain;
  class Converter;
  typedef std::shared_ptr<Dict> DictPtr;
  typedef std::shared_ptr<SerializableDict> SerializableDictPtr;
  typedef std::shared_ptr<TextDict> TextDictPtr;
  typedef std::shared_ptr<DartsDict> DartsDictPtr;
  typedef std::shared_ptr<DictGroup> DictGroupPtr;
  typedef std::shared_ptr<Segmentation> SegmentationPtr;
  typedef std::shared_ptr<Conversion> ConversionPtr;
  typedef std::shared_ptr<ConversionChain> ConversionChainPtr;
  typedef std::shared_ptr<Converter> ConverterPtr;
}

#ifdef ENABLE_GETTEXT
# include <libintl.h>
# include <locale.h>
# define _(STRING) dgettext(PACKAGE_NAME, STRING)
#else // ENABLE_GETTEXT
# define _(STRING) STRING
#endif // ENABLE_GETTEXT

#ifndef PKGDATADIR
const string PACKAGE_DATA_DIRECTORY = "";
#else // ifndef PKGDATADIR
const string PACKAGE_DATA_DIRECTORY = PKGDATADIR "/";
#endif // ifndef PKGDATADIR

#ifndef VERSION
# define VERSION "1.0.*"
#endif // ifndef VERSION
