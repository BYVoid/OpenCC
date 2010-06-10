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

#include "opencc_datrie.h"
#include "opencc_datrie_table.h"

int encode_char(wchar_t ch)
{
	return (int)ch;
}

void match_word(const DoubleArrayTrieItem *dat, const wchar_t * word, size_t *match_pos, size_t *id, size_t limit)
{
	size_t i, p;
	for (i = 0,p = 0; word[p] && (limit == 0 || p < limit) && dat[i].base != DATRIE_UNUSED; p ++)
	{
		int k = encode_char(word[p]);
		int j = dat[i].base + k;
		if (j < 0 || j > DATRIE_SIZE || dat[j].parent != i)
			break;
		i = j;
	}
	if (match_pos)
		*match_pos = p;
	if (id)
		*id = i;
}

void datrie_get_match_lengths(const wchar_t * word, size_t * match_length)
{
	match_length[0] = 0;
	
	size_t i, p;
	for (i = 0,p = 0; word[p] && dat[i].base != DATRIE_UNUSED; p ++)
	{
		int k = encode_char(word[p]);
		int j = dat[i].base + k;
		if (j < 0 || j > DATRIE_SIZE || dat[j].parent != i)
			break;
		i = j;
		
		if (dat[i].word != -1)
			match_length[++ match_length[0]] = p + 1;
	}
}

const wchar_t * datrie_match(const wchar_t * word, size_t * match_length, size_t length_limit)
{
	size_t pos, item;
	match_word(dat, word, &pos, &item, length_limit);
	
	while (dat[item].word == -1 && pos > 1)
		match_word(dat, word, &pos, &item, pos - 1);
	
	if (match_length)
		*match_length = pos;
	
	if (pos == 0 || dat[item].word == -1)
		return NULL;
	
	return words[dat[item].word];
}
