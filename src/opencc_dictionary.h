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

#ifndef __OPENCC_DICTIONARY_H_
#define __OPENCC_DICTIONARY_H_

#include "opencc.h"

typedef void * opencc_dictionary_t;

opencc_dictionary_t dict_open(const char * dict_filename, opencc_dictionary_type dict_type);

int dict_close(opencc_dictionary_t ddt);

int dict_load(opencc_dictionary_t ddt, const char * dict_filename,
		opencc_dictionary_type dict_type);

const wchar_t * dict_match_longest(opencc_dictionary_t ddt, const wchar_t * word,
		size_t length);

void dict_get_all_match_lengths(opencc_dictionary_t ddt, const wchar_t * word,
		size_t * match_length);

#endif /* __OPENCC_DICTIONARY_H_ */
