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

#define DATRIE_SIZE 300000
#define DATRIE_WORD_MAX_COUNT 100000
#define DATRIE_WORD_MAX_LENGTH 12

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
DoubleArrayTrieItem dat[DATRIE_SIZE];

int encode_char(wchar_t ch)
{
	return (int)ch;
}

void match_word(const DoubleArrayTrieItem *dat, const wchar_t * word, int *match_pos, int *id, int limit)
{
	int i, j, p;
	for (i = 0,p = 0; word[p] && (limit == 0 || p < limit) && dat[i].base != DATRIE_UNUSED; p ++)
	{
		int k = encode_char(word[p]);
		j = dat[i].base + k;
		if (j < 0 || j > DATRIE_SIZE || dat[j].parent != i)
			break;
		i = j;
	}
	if (match_pos)
		*match_pos = p;
	if (id)
		*id = i;
}

int unused(int i)
{
	if (i >= 0 && i < DATRIE_SIZE)
		return dat[i].parent == DATRIE_UNUSED;
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
	dat[k].base = DATRIE_UNUSED;
	dat[k].parent = 0;
	if (word->length == 1)
		dat[k].word = id;
}

void insert_words(int delta, int parent,int word_len)
{
	int i;
	for (i = 0; i < words_set_count; i ++)
	{
		int j = words_set[i];
		int k = encode_char(lexicon[j].simp[word_len]) + delta;
		dat[k].parent = parent;
		if (lexicon[j].length == word_len + 1)
		{
			dat[k].word = j;
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
		
		match_word(dat, word->simp, &p, &i, 0);
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
			fprintf(stderr,"DATRIE_SIZE Not Enough!\n");
			exit(1);
		}
		
		insert_words(delta, i, p);
		
		dat[i].base = delta;
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
		dat[i].parent = dat[i].base = DATRIE_UNUSED;
		dat[i].word = -1;
	}
	dat[0].parent = dat[0].base = 0;
	
	qsort(lexicon, lexicon_count, sizeof(lexicon[0]), cmp);
	for (i = 0; i < lexicon_count; i ++)
		insert_first_char(i);
	for (i = 0; i < lexicon_count; i ++)
		insert(i);
	
}

void init(void)
{
	#define BUFFSIZE 1024
	int i;
	FILE * fp = stdin;
	wchar_t buff[BUFFSIZE];
	
	setlocale(LC_ALL, "zh_CN.UTF-8");
	
	for (i = 0; fgetws(buff, BUFFSIZE,fp); i ++)
	{
		swscanf(buff,L"%ls%ls", lexicon[i].simp, lexicon[i].trad);
		lexicon[i].length = wcslen(lexicon[i].simp);
#if 0
		wcscpy(lexicon[i].trad,buff + lexicon[i].length + 1);
		int tlen = wcslen(lexicon[i].trad);
		if (lexicon[i].trad[tlen-1] == L'\n' || lexicon[i].trad[tlen-1] == WEOF)
			lexicon[i].trad[tlen-1] = 0;
#endif
	}
	
	lexicon_count = i;
	
	fclose(fp);
}

void output()
{
	FILE * fp = stdout;
	int i, item_max, word_max_length = 0;
	
	fprintf(fp, "static const wchar_t * const words[] = {\n");

	for (i = 0; i < lexicon_count; i ++)
	{
		fprintf(fp, "\tL\"%ls\", /* %6d */\n", lexicon[i].trad, i);
		if (lexicon[i].length > word_max_length)
			word_max_length = lexicon[i].length;
	}
	
	fprintf(fp, "};\n\n");
	fprintf(fp, "static const DoubleArrayTrieItem dat[] = {\n");
	fprintf(fp, "\t/*  Base  Parent    Word          ID */\n");
	
	for (i = DATRIE_SIZE - 1; i > 0; i --)
		if (dat[i].parent != DATRIE_UNUSED)
			break;
	item_max = i;
	
	int unused = 0;
	for (i = 0; i <= item_max; i ++)
	{
		fprintf(fp, "\t{ %6d, %6d, %6d}, /* %6d */\n", dat[i].base, dat[i].parent, dat[i].word, i);
		if (dat[i].parent == DATRIE_UNUSED)
			unused ++;
	}
	
	fprintf(fp, "};\n\n");
	
	//fprintf(fp, "/*\n* Total: %d\n* Unused: %d\n* Storage Efficiency: %.2lf percent\n*/\n\n", item_max + 1, unused, (item_max + 1 - unused) / (double)(item_max + 1) * 100);
	
	fprintf(fp, "#define DATRIE_SIZE (%d)\n", item_max + 1);
	fprintf(fp, "#define DATRIE_WORD_MAX_LENGTH (%d)\n", word_max_length);
	
	fclose(fp);
}

void write_text_file()
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
		if (dat[i].parent != DATRIE_UNUSED)
		{
			fprintf(fp,"%d %d %d %d\n", i, dat[i].base, dat[i].parent, dat[i].word);
		}
	}
	
	fclose(fp);
}

void write_debug()
{
	int i;
	FILE * fp;
	
	fp = fopen("datrie.txt","w");
	
	for (i = 0; i < DATRIE_SIZE; i ++)
	{
		if (dat[i].parent != DATRIE_UNUSED)
		{
			fprintf(fp,"[%6d] Base:%6d Parent:%6d",i,dat[i].base,dat[i].parent);
			if (dat[i].word != -1)
				fprintf(fp," [%ls] [%ls]",lexicon[dat[i].word].simp,lexicon[dat[i].word].trad);
			fprintf(fp,"\n");
		}
	}
	
	fclose(fp);
}

int main(int argc, char **argv)
{
	init();
	make();
	output();
	//write_debug();
	return 0;
}
