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

#include "opencc_dictionary_abstract.h"
#include "opencc_dictionary_text.h"
#include "opencc_dictionary_datrie.h"

dict_ptr dict_abstract_open(dictionary * dict)
{
	switch (dict->type)
	{
	case OPENCC_DICTIONARY_TYPE_TEXT:
		return dict_text_open(dict->filename);
		break;
	case OPENCC_DICTIONARY_TYPE_DATRIE:
		return dict_datrie_open(dict->filename);
		break;
	default:
		return (dict_ptr) -1; /* 辭典格式不支持 */
	}
}

void dict_abstract_close(dictionary * dict)
{
	switch (dict->type)
	{
	case OPENCC_DICTIONARY_TYPE_TEXT:
		dict_text_close(dict->dict);
		break;
	case OPENCC_DICTIONARY_TYPE_DATRIE:
		dict_datrie_close(dict->dict);
		break;
	default:
		debug_should_not_be_here();
	}
}

const wchar_t * dict_abstract_match_longest(dictionary * dict, const wchar_t * word,
		size_t length)
{
	switch (dict->type)
	{
	case OPENCC_DICTIONARY_TYPE_TEXT:
		return dict_text_match_longest(dict->dict, word, length);
		break;
	case OPENCC_DICTIONARY_TYPE_DATRIE:
		return dict_datrie_match_longest(dict->dict, word, length);
		break;
	default:
		debug_should_not_be_here();
	}
}

size_t dict_abstract_get_all_match_lengths(dictionary * dict, const wchar_t * word,
		size_t * match_length)
{
	switch (dict->type)
	{
	case OPENCC_DICTIONARY_TYPE_TEXT:
		return dict_text_get_all_match_lengths(dict->dict, word, match_length);
		break;
	case OPENCC_DICTIONARY_TYPE_DATRIE:
		return dict_datrie_get_all_match_lengths(dict->dict, word, match_length);
		break;
	default:
		debug_should_not_be_here();
	}
}

size_t dict_abstract_get_lexicon(dictionary * dict, opencc_entry * lexicon)
{
	switch (dict->type)
	{
	case OPENCC_DICTIONARY_TYPE_TEXT:
		return dict_text_get_lexicon(dict->dict, lexicon);
		break;
	case OPENCC_DICTIONARY_TYPE_DATRIE:
		return dict_datrie_get_lexicon(dict->dict, lexicon);
		break;
	default:
		debug_should_not_be_here();
	}
}
