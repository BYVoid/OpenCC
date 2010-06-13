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

#include "opencc_dictionary_text.h"
#include "../opencc_encoding.h"

#define INITIAL_DICTIONARY_SIZE 1024
#define ENTRY_BUFF_SIZE 128

typedef struct
{
	wchar_t * key;
	wchar_t * value;
} entry;

typedef struct
{
	size_t entry_count;
	size_t max_length;
	entry * lexicon;
	wchar_t * word_buff;
} text_dictionary;

int qsort_entry_cmp(const void *a, const void *b)
{
	return wcscmp(((entry *)a)->key, ((entry *)b)->key);
}

dict_ptr dict_text_open(const char * filename)
{
	text_dictionary * td;
	td = (text_dictionary *) malloc(sizeof(text_dictionary));
	td->entry_count = INITIAL_DICTIONARY_SIZE;
	td->max_length = 0;
	td->lexicon = (entry *) malloc(sizeof(entry) * td->entry_count);

	static char buff[ENTRY_BUFF_SIZE];
	static char key_buff[ENTRY_BUFF_SIZE];
	static char value_buff[ENTRY_BUFF_SIZE];
	static wchar_t wbuff[ENTRY_BUFF_SIZE];

	FILE * fp = fopen(filename,"r");
	if (fp == NULL)
		return (dict_ptr) -1;

	size_t i = 0;
	while (fgets(buff, ENTRY_BUFF_SIZE, fp))
	{
		if (i >= td->entry_count)
		{
			td->entry_count += td->entry_count;
			td->lexicon = (entry *) realloc(td->lexicon, sizeof(entry) * td->entry_count);
		}

		sscanf(buff, "%s %s", key_buff, value_buff);

		utf8_to_wcs(key_buff, wbuff, ENTRY_BUFF_SIZE);

		size_t length = wcslen(wbuff);
		if (length > td->max_length)
			td->max_length = length;

		td->lexicon[i].key = (wchar_t *) malloc((length + 1) * sizeof(wchar_t));
		wcscpy(td->lexicon[i].key, wbuff);

		utf8_to_wcs(value_buff, wbuff, ENTRY_BUFF_SIZE);
		td->lexicon[i].value = (wchar_t *) malloc((wcslen(wbuff) + 1) * sizeof(wchar_t));
		wcscpy(td->lexicon[i].value, wbuff);

		i ++;
	}

	fclose(fp);

	td->entry_count = i;
	td->lexicon = (entry *) realloc(td->lexicon, sizeof(entry) * td->entry_count);
	td->word_buff = (wchar_t *) malloc(sizeof(wchar_t) * (td->max_length + 1));

	qsort(td->lexicon, td->entry_count, sizeof(td->lexicon[0]), qsort_entry_cmp);

	return (dict_ptr) td;
}

void dict_text_close(dict_ptr dp)
{
	text_dictionary * td = (text_dictionary *) dp;

	size_t i;
	for (i = 0; i < td->entry_count; i ++)
	{
		free(td->lexicon[i].key);
		free(td->lexicon[i].value);
	}

	free(td->lexicon);
	free(td->word_buff);
	free(td);
}

const wchar_t * dict_text_match_longest(dict_ptr dp, const wchar_t * word,
		size_t length)
{
	text_dictionary * td = (text_dictionary *) dp;

	if (td->entry_count == 0)
		return NULL;

	if (length == 0)
		length = wcslen(word);
	size_t len = td->max_length;
	if (length < len)
		len = length;

	wcsncpy(td->word_buff, word, len);
	td->word_buff[len] = L'\0';

	entry buff;
	buff.key = td->word_buff;

	for (; len > 0; len --)
	{
		td->word_buff[len] = L'\0';
		entry * brs = (entry *) bsearch(&buff, td->lexicon, td->entry_count,
				sizeof(td->lexicon[0]), qsort_entry_cmp);

		if (brs != NULL)
			return brs->value;
	}

	return NULL;
}

void dict_text_get_all_match_lengths(dict_ptr dp, const wchar_t * word,
		size_t * match_length)
{

}
