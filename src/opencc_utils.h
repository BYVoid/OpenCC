/*
* Open Chinese Convert
*
* Copyright 2010 BYVoid <byvoid1@gmail.com>
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

#ifndef __OPENCC_UTILS_H_
#define __OPENCC_UTILS_H_

#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define FALSE (0)
#define TRUE (!(0))
#define INFINITY_INT ((~0U)>>1)

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#ifdef HAVE_GETTEXT
#	include <libintl.h>
#	include <locale.h>
#	define _(STRING) dgettext(PACKAGE,STRING)
#else
#	define _(STRING) STRING
#endif

#define debug_should_not_be_here() \
	do { \
	fprintf(stderr, "Should not be here: %d %s\n", __LINE__, __FILE__); \
	exit(1); \
	} while(0)\

void perr(const char * str);

int qsort_int_cmp(const void * a, const void * b);

char * mstrcpy(const char * str);

char * mstrncpy(const char * str, size_t n);

#endif /* __OPENCC_UTILS_H_ */
