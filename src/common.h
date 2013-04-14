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

#ifndef __COMMON_H_
#define __COMMON_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "opencc_types.h"

#define INFINITY_INT ((~0U) >> 1)

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

struct SConfig;
struct SConverter;
struct SDict;
struct SDictGroup;
struct SDictChain;
struct SDictMeta;

typedef struct SConfig Config;
typedef struct SConverter Converter;
typedef struct SDict Dict;
typedef struct SDictGroup DictGroup;
typedef struct SDictChain DictChain;
typedef struct SDictMeta DictMeta;

struct SDict {
  opencc_dictionary_type type;
  Dict* dict;
};

#define DICTIONARY_MAX_COUNT 128
struct SDictGroup {
  DictChain* dict_chain;
  size_t count;
  Dict* dicts[DICTIONARY_MAX_COUNT];
};

#define DICTIONARY_GROUP_MAX_COUNT 128
struct SDictChain {
  Config* config;
  size_t count;
  DictGroup* groups[DICTIONARY_GROUP_MAX_COUNT];
};

struct SDictMeta {
  opencc_dictionary_type dict_type;
  char* file_name;
  size_t index;
  size_t stamp;
};

struct SConfig {
  char* title;
  char* description;
  DictChain* dict_chain;
  char* file_path;
  DictMeta dicts[DICTIONARY_MAX_COUNT];
  size_t dicts_count;
  size_t stamp;
};

struct SConverter {
  opencc_conversion_mode conversion_mode;
  DictChain* dict_chain;
  DictGroup* current_dict_group;
  void* data;
};

#endif // __COMMON_H_
