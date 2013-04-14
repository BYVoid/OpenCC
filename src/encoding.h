/**
 * @file
 * UCS4-UTF8 Encoding module.
 *
 * @license
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

#ifndef __OPENCC_ENCODING_H_
#define __OPENCC_ENCODING_H_

#include "common.h"

/**
 * Converts a UTF-8 string into UCS-4.
 *
 * @param utf8   UTF-8 string
 * @param length Length of UTF-8 string or 0 to consider as \0 ended string
 * @return       The converted UCS-4 string. Must be free when not in use.
 */
ucs4_t* utf8_to_ucs4(const char* utf8, size_t length);

/**
 * Converts a UCS-4 string into UTF-8.
 *
 * @param ucs4   UCS-4 string
 * @param length Length of UCS-4 string or 0 to consider as \0 ended string
 * @return       The converted UTF-8 string. Must be free when not in use.
 */
char* ucs4_to_utf8(const ucs4_t* ucs4, size_t length);

size_t ucs4len(const ucs4_t* str);

int ucs4cmp(const ucs4_t* str1, const ucs4_t* str2);

void ucs4cpy(ucs4_t* dest, const ucs4_t* src);

void ucs4ncpy(ucs4_t* dest, const ucs4_t* src, size_t len);

#endif /* __OPENCC_ENCODING_H_ */
