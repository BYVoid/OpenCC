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

#ifndef __OPENCC_DICTIONARY_TEXT_H_
#define __OPENCC_DICTIONARY_TEXT_H_

#include "opencc_dictionary_abstract.h"

dict_ptr dict_text_open(const char * filename);

void dict_text_close(dict_ptr dp);

const wchar_t * dict_text_match_longest(dict_ptr dp, const wchar_t * word,
		size_t length);

size_t dict_text_get_all_match_lengths(dict_ptr dp, const wchar_t * word,
		size_t * match_length);

size_t dict_text_get_lexicon(dict_ptr dp, opencc_entry * lexicon);

#endif /* __OPENCC_DICTIONARY_TEXT_H_ */
