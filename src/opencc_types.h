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

#ifndef __OPENCC_TYPES_H_
#define __OPENCC_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

#include <stddef.h>
#include <stdint.h>

typedef void* opencc_t;

typedef uint32_t ucs4_t;

enum _opencc_error {
  OPENCC_ERROR_VOID,
  OPENCC_ERROR_DICTLOAD,
  OPENCC_ERROR_CONFIG,
  OPENCC_ERROR_ENCODING,
  OPENCC_ERROR_ENCODIND = OPENCC_ERROR_ENCODING,
  OPENCC_ERROR_CONVERTER
};
typedef enum _opencc_error opencc_error;

enum _opencc_dictionary_type {
  OPENCC_DICTIONARY_TYPE_TEXT,
  OPENCC_DICTIONARY_TYPE_DATRIE
};
typedef enum _opencc_dictionary_type opencc_dictionary_type;

enum _opencc_conversion_mode {
  OPENCC_CONVERSION_FAST = 0,
  OPENCC_CONVERSION_SEGMENT_ONLY = 1,
  OPENCC_CONVERSION_LIST_CANDIDATES = 2
};
typedef enum _opencc_conversion_mode opencc_conversion_mode;

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

#endif /* __OPENCC_TYPES_H_ */
