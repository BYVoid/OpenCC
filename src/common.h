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

#include "opencc_types.h"

#define FALSE (0)
#define TRUE (!(0))
#define INFINITY_INT ((~0U) >> 1)

#ifndef BIG_ENDIAN
# define BIG_ENDIAN (0)
#endif // ifndef BIG_ENDIAN

#ifndef LITTLE_ENDIAN
# define LITTLE_ENDIAN (1)
#endif // ifndef LITTLE_ENDIAN

#ifdef ENABLE_GETTEXT
# include <libintl.h>
# include <locale.h>
# define _(STRING) dgettext(PACKAGE_NAME, STRING)
#else // ifdef ENABLE_GETTEXT
# define _(STRING) STRING
#endif // ifdef ENABLE_GETTEXT

typedef void* converter_t;
typedef void* config_t;
typedef void* dictionary_t;

struct SDictGroup;
struct SDictChain;

typedef struct SDictGroup DictGroup;
typedef struct SDictChain DictChain;

#define DICTIONARY_MAX_COUNT 128
struct SDictGroup {
  DictChain* DictChain;
  size_t count;
  dictionary_t dicts[DICTIONARY_MAX_COUNT];
};

#define DICTIONARY_GROUP_MAX_COUNT 128
struct SDictChain {
  config_t config;
  size_t count;
  DictGroup* groups[DICTIONARY_GROUP_MAX_COUNT];
};

#endif // ifndef __COMMON_H_
