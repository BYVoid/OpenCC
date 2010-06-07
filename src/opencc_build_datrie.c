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
#include <stdlib.h>
#include <locale.h>

typedef struct
{
	wchar_t simp[DATRIE_WORD_MAX_LENGTH];
	wchar_t trad[DATRIE_WORD_MAX_LENGTH];
	int length;
} Entry;

Entry lexicon[DATRIE_WORD_MAX_COUNT];
int lexicon_count;
int words_set[DATRIE_WORD_MAX_COUNT],words_set_count;
wchar_t words_set_char[DATRIE_WORD_MAX_COUNT];
DoubleArrayTrie newdat;

int unused(int i)
{
	if (i >= 0 && i < DATRIE_SIZE)
		return newdat.items[i].parent == DATRIE_UNUSED;
	return FALSE;
}

int is_prefix(const wchar_t *a,const wchar_t *b)
{
	const wchar_t *p = a,*q = b;
	while (*p != 0)
	{
		if (*q == 0)
			return FALSE;
		if (*p != *q)
			return FALSE;
		p++;
		q++;
	}
	return TRUE;
}

int binary_search(const wchar_t *str)
{
	int a = 0,b = lexicon_count - 1,c;
	while (a + 1 < b)
	{
		c = (a + b) / 2;
		if (wcscmp(str,lexicon[c].simp) <= 0)
			b = c;
		else
			a = c+1;
	}
	if (is_prefix(str,lexicon[a].simp) && (a == 0 || !is_prefix(str,lexicon[a-1].simp)))
		return a;
	if (is_prefix(str,lexicon[b].simp) && !is_prefix(str,lexicon[b-1].simp))
		return b;
	return -1;
}

int wcmp(const void *a, const void *b)
{
	return *(const wchar_t *)a < *(const wchar_t *)b ? -1 : 1;
}

void get_words_with_prefix(wchar_t * word, int p)
{
	int i;
	static wchar_t buff[DATRIE_WORD_MAX_LENGTH];
	static wchar_t words_set_char_buff[DATRIE_WORD_MAX_COUNT];
	
	for (i = 0; i < p; i ++)
		buff[i] = word[i];
	buff[p] = 0;
	
	words_set_count = 0;
	for (i = binary_search(buff); i < lexicon_count && is_prefix(buff,lexicon[i].simp); i ++)
	{
		if (wcscmp(buff,lexicon[i].simp) == 0)
			continue;
		words_set_char_buff[words_set_count] = lexicon[i].simp[p];
		words_set[words_set_count ++] = i;
	}
	words_set_char_buff[words_set_count] = 0;
	
	qsort(words_set_char_buff, words_set_count, sizeof(words_set_char_buff[0]), wcmp);
	
	wchar_t * wfp, * wp, last;
	for (last = 0, wfp = words_set_char_buff, wp = words_set_char; *wfp; wfp ++)
	{
		if (*wfp != last)
		{
			last = *wfp;
			*wp = *wfp;
			wp ++;
		}
	}
	*wp = 0;
}

int words_space_available(int delta)
{
	wchar_t * wp;
	for (wp = words_set_char; *wp; wp ++)
		if (!unused(encode_char(*wp) + delta))
			return FALSE;
	return TRUE;
}

void insert_first_char(int id)
{
	Entry * word = &lexicon[id];
	int k = encode_char(word->simp[0]);
	newdat.items[k].base = DATRIE_UNUSED;
	newdat.items[k].parent = 0;
	if (word->length == 1)
		newdat.items[k].word = id;
}

int insert_words(int delta, int parent,int word_len)
{
	int i;
	for (i = 0; i < words_set_count; i ++)
	{
		int j = words_set[i];
		int k = encode_char(lexicon[j].simp[word_len]) + delta;
		newdat.items[k].parent = parent;
		if (lexicon[j].length == word_len + 1)
		{
			newdat.items[k].word = j;
		}
	}
}

void insert(int id)
{
	static int space_min = 0;
	Entry * word = &lexicon[id];
	for (;;)
	{
		int p,i;
		
		match_word(&newdat, word->simp, &p, &i, 0);
		if (p == word->length)
			return;
		
		get_words_with_prefix(word->simp, p);
		
		int delta;
		delta = space_min - words_set_char[0];
		for (; delta < DATRIE_SIZE; delta ++)
			if (words_space_available(delta))
				break;
		
		if (delta == DATRIE_SIZE)
		{
			//error
		}
		
		insert_words(delta, i, p);
		
		newdat.items[i].base = delta;
		while (!unused(space_min))
			space_min++;
	}
}

int cmp(const void *a, const void *b)
{
	return wcscmp((const wchar_t *)a, (const wchar_t *)b);
}

void make(void)
{
	int i;
	for (i = 1; i < DATRIE_SIZE; i ++)
	{
		newdat.items[i].parent = newdat.items[i].base = DATRIE_UNUSED;
		newdat.items[i].word = -1;
	}
	newdat.items[0].parent = newdat.items[0].base = 0;
	
	qsort(lexicon, lexicon_count, sizeof(lexicon[0]), cmp);
	for (i = 0; i < lexicon_count; i ++)
		insert_first_char(i);
	for (i = 0; i < lexicon_count; i ++)
		insert(i);
	
}

void init(void)
{
	#define BUFFSIZE 1024
	int i, tlen;
	FILE * fp;
	wchar_t buff[BUFFSIZE];
	setlocale(LC_ALL, "zh_CN.UTF-8");
	fp = fopen("datrie.in","r");
	fgetws(buff, BUFFSIZE,fp);
	swscanf(buff,L"%d", &lexicon_count);
	
	for (i = 0; i < lexicon_count; i ++)
	{
		fgetws(buff, BUFFSIZE,fp);
		swscanf(buff,L"%ls%ls", lexicon[i].simp, lexicon[i].trad);
		lexicon[i].length = wcslen(lexicon[i].simp);
#if 0
		wcscpy(lexicon[i].trad,buff + lexicon[i].length + 1);
		tlen = wcslen(lexicon[i].trad);
		if (lexicon[i].trad[tlen-1] == L'\n' || lexicon[i].trad[tlen-1] == WEOF)
			lexicon[i].trad[tlen-1] = 0;
#endif
	}
	fclose(fp);
}

void writefile()
{
	FILE * fp;
	int i;
	fp = fopen("datrie.txt","w");
	fprintf(fp, "%d\n", lexicon_count);
	
	for (i = 0; i < lexicon_count; i ++)
	{
		fprintf(fp, "%ls\n", lexicon[i].trad);
	}
	
	for (i = 0; i < DATRIE_SIZE; i ++)
	{
		if (newdat.items[i].parent != DATRIE_UNUSED)
		{
			fprintf(fp,"%d %d %d %d\n", i, newdat.items[i].base, newdat.items[i].parent, newdat.items[i].word);
		}
	}
	
	fclose(fp);
}

void debug()
{
	int i;
	FILE * fp;
	
	fp = fopen("datrie.txt","w");
	
	for (i = 0; i < DATRIE_SIZE; i ++)
	{
		if (newdat.items[i].parent != DATRIE_UNUSED)
		{
			fprintf(fp,"[%6d] Base:%6d Parent:%6d",i,newdat.items[i].base,newdat.items[i].parent);
			if (newdat.items[i].word != -1)
				fprintf(fp," [%ls] [%ls]",lexicon[newdat.items[i].word].simp,lexicon[newdat.items[i].word].trad);
			fprintf(fp,"\n");
		}
	}
	
	fclose(fp);
}

int main(void)
{
	init();
	make();
	writefile();
	//debug();
}
