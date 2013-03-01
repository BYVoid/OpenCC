/*
* Open Chinese Convert
*
* Copyright 2013 BYVoid <byvoid.kcp@gmail.com>
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "opencc_types.h"

#define FALSE (0)
#define TRUE (!(0))
#define INFINITY_INT ((~0U)>>1)

#ifndef BIG_ENDIAN
#	define BIG_ENDIAN (0)
#endif

#ifndef LITTLE_ENDIAN
#	define LITTLE_ENDIAN (1)
#endif

#ifdef ENABLE_GETTEXT
#	include <libintl.h>
#	include <locale.h>
#	define _(STRING) dgettext(PACKAGE_NAME, STRING)
#else
#	define _(STRING) STRING
#endif

typedef void * converter_t;
typedef void * config_t;
typedef void * dictionary_group_t;
typedef void * dictionary_set_t;

#endif