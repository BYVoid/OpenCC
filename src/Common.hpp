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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>

#include <cstdio>
#include <cassert>
#include <cstddef>

#include "Optional.hpp"
#include "Exception.hpp"

using std::string;

// Forward decalarations and alias
namespace Opencc {
  class Config;
  class DictEntry;
  class Dict;
  class TextDict;
  class DartsDict;
  class DictGroup;
  class Segmentation;
  class Conversion;
  class ConversionChain;
  using DictEntryPtr = std::shared_ptr<DictEntry>;
  using DictEntryPtrVector = std::vector<DictEntryPtr>;
  using DictEntryPtrVectorPtr = std::shared_ptr<DictEntryPtrVector>;
  using DictPtr = std::shared_ptr<Dict>;
  using TextDictPtr = std::shared_ptr<TextDict>;
  using DartsDictPtr = std::shared_ptr<DartsDict>;
  using DictGroupPtr = std::shared_ptr<DictGroup>;
  using SegmentationPtr = std::shared_ptr<Segmentation>;
  using ConversionPtr = std::shared_ptr<Conversion>;
  using ConversionChainPtr = std::shared_ptr<ConversionChain>;
}
using StringVector = std::vector<string>;
using StringVectorPtr = std::shared_ptr<StringVector>;

#ifdef ENABLE_GETTEXT
# include <libintl.h>
# include <locale.h>
# define _(STRING) dgettext(PACKAGE_NAME, STRING)
#else // ENABLE_GETTEXT
# define _(STRING) STRING
#endif // ENABLE_GETTEXT

#ifndef PKGDATADIR
#define PKGDATADIR ""
#endif
