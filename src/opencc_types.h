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

#ifndef __OPENCC_TYPES_H_
#define __OPENCC_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef void * opencc_t;

typedef uint32_t ucs4_t;

typedef enum
{
	OPENCC_ERROR_VOID,
	OPENCC_ERROR_DICTLOAD,
	OPENCC_ERROR_CONFIG,
	OPENCC_ERROR_ENCODIND,
	OPENCC_ERROR_CONVERTER,
} opencc_error;

typedef enum
{
	OPENCC_DICTIONARY_TYPE_TEXT,
	OPENCC_DICTIONARY_TYPE_DATRIE,
} opencc_dictionary_type;

typedef struct
{
	opencc_dictionary_type dict_type;
	char * file_name;
} opencc_dictionary;

#ifdef __cplusplus
};
#endif

#endif /* __OPENCC_TYPES_H_ */
