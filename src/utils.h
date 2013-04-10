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

#ifndef __OPENCC_UTILS_H_
#define __OPENCC_UTILS_H_

#include "common.h"

#define debug_should_not_be_here()                                      \
  do {                                                                  \
    fprintf(stderr, "Should not be here %s: %d\n", __FILE__, __LINE__); \
    assert(0);                                                          \
  } while (0)                                                           \

void perr(const char* str);

int qsort_int_cmp(const void* a, const void* b);

char* mstrcpy(const char* str);

char* mstrncpy(const char* str, size_t n);

void skip_utf8_bom(FILE* fp);

const char* executable_path(void);

char* try_open_file(const char* path);

char* get_file_path(const char* filename);

int is_absolute_path(const char* path);

#endif /* __OPENCC_UTILS_H_ */
