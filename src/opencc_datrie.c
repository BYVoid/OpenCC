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
#include <stdio.h>

int words_count;
wchar_t words[DATRIE_WORD_MAX_COUNT][DATRIE_WORD_MAX_LENGTH];
DoubleArrayTrie dat;
int initialized = FALSE;

void initialize()
{
	#define BUFFSIZE 128
	int i, tlen;
	FILE * fp;
	wchar_t buff[BUFFSIZE];
	
	for (i = 1; i < DATRIE_SIZE; i ++)
	{
		dat.items[i].parent = dat.items[i].base = DATRIE_UNUSED;
		dat.items[i].word = -1;
	}
	
	fp = fopen("datrie.txt","r");
	fgetws(buff, BUFFSIZE,fp);
	swscanf(buff, L"%d", &words_count);
	
	for (i = 0; i < words_count; i ++)
	{
		fgetws(buff, BUFFSIZE,fp);
		swscanf(buff, L"%ls", words[i]);
		tlen = wcslen(words[i]);
		if (words[i][tlen-1] == L'\n' || words[i][tlen-1] == WEOF)
			words[i][tlen-1] = 0;
	}
	
	int base, parent, word;
	while (fgetws(buff, BUFFSIZE,fp) != NULL)
	{
		swscanf(buff, L"%d %d %d %d", &i, &base, &parent, &word);
	
		dat.items[i].base = base;
		dat.items[i].parent = parent;
		dat.items[i].word = word;
	}
	
	fclose(fp);
	initialized = TRUE;
}

int encode_char(wchar_t ch)
{
	return (int)ch;
}

void get_match_lengths(const wchar_t * word, int * match_length)
{
	match_length[0] = 0;
	
	int i, j, p;
	for (i = 0,p = 0; word[p] && dat.items[i].base != DATRIE_UNUSED; p ++)
	{
		int k = encode_char(word[p]);
		j = dat.items[i].base + k;
		if (j < 0 || j > DATRIE_SIZE || dat.items[j].parent != i)
			break;
		i = j;
		
		if (dat.items[i].word != -1)
			match_length[++ match_length[0]] = p + 1;
	}
}

void match_word(const DoubleArrayTrie *dat, const wchar_t * word, int *match_pos, int *id, int limit)
{
	int i, j, p;
	for (i = 0,p = 0; word[p] && (limit == 0 || p < limit) && dat->items[i].base != DATRIE_UNUSED; p ++)
	{
		int k = encode_char(word[p]);
		j = dat->items[i].base + k;
		if (j < 0 || j > DATRIE_SIZE || dat->items[j].parent != i)
			break;
		i = j;
	}
	if (match_pos)
		*match_pos = p;
	if (id)
		*id = i;
}

const wchar_t * get_trad_in_datrie(const wchar_t * word, int * match_pos,int limit)
{
	if (!initialized)
		initialize();
	int pos, item;
	match_word(&dat, word, &pos, &item, limit);
	
	if (match_pos)
		*match_pos = pos;
	
	if (pos == 0 || dat.items[item].word == -1)
		return NULL;
	
	return words[dat.items[item].word];
}
