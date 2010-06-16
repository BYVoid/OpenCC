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

#ifndef __OPENCC_DICTIONARY_ABSTRACT_H_
#define __OPENCC_DICTIONARY_ABSTRACT_H_

#include "../opencc_utils.h"
#include "../opencc.h"

#define DICTIONARY_MAX_COUNT 128

typedef void * dict_ptr;

typedef struct
{
	opencc_dictionary_type type;
	char * filename;
	dict_ptr dict;
} opencc_dictionary;

typedef struct
{
	size_t dict_count;
	opencc_dictionary dict[DICTIONARY_MAX_COUNT];
} opencc_dictionary_description;

dict_ptr dict_abstract_open(opencc_dictionary * dict);

void dict_abstract_close(opencc_dictionary * dict);

const wchar_t * dict_abstract_match_longest(opencc_dictionary * dict, const wchar_t * word,
		size_t length);

size_t dict_abstract_get_all_match_lengths(opencc_dictionary * dict, const wchar_t * word,
		size_t * match_length);

size_t dict_abstract_get_lexicon(opencc_dictionary * dict, opencc_entry * lexicon);

#endif /* __OPENCC_DICTIONARY_ABSTRACT_H_ */
