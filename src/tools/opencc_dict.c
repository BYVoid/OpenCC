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

#include "opencc_dictionary.h"
#include "opencc_utils.h"
#include "dictionary/opencc_dictionary_datrie.h"
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define VERSION "Unknown"
#endif

#define DATRIE_SIZE 1000000
#define DATRIE_WORD_MAX_COUNT 500000
#define DATRIE_WORD_MAX_LENGTH 32
#define BUFFER_SIZE 1024

typedef struct
{
	wchar_t * key;
	wchar_t * value;
	int length;
	int pos;
} Entry;

Entry lexicon[DATRIE_WORD_MAX_COUNT];
size_t lexicon_count, words_set_count;
int words_set[DATRIE_WORD_MAX_COUNT];
wchar_t words_set_char[DATRIE_WORD_MAX_COUNT];
DoubleArrayTrieItem dat[DATRIE_SIZE];

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
		if (wcscmp(str,lexicon[c].key) <= 0)
			b = c;
		else
			a = c+1;
	}
	if (is_prefix(str,lexicon[a].key) && (a == 0 || !is_prefix(str,lexicon[a-1].key)))
		return a;
	if (is_prefix(str,lexicon[b].key) && !is_prefix(str,lexicon[b-1].key))
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
	for (i = binary_search(buff); i < lexicon_count && is_prefix(buff,lexicon[i].key); i ++)
	{
		if (wcscmp(buff,lexicon[i].key) == 0)
			continue;
		words_set_char_buff[words_set_count] = lexicon[i].key[p];
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
	int k = encode_char(word->key[0]);
	dat[k].base = DATRIE_UNUSED;
	dat[k].parent = 0;
	if (word->length == 1)
		dat[k].word = lexicon[id].pos;
}

void insert_words(int delta, int parent,int word_len)
{
	int i;
	for (i = 0; i < words_set_count; i ++)
	{
		int j = words_set[i];
		int k = encode_char(lexicon[j].key[word_len]) + delta;
		dat[k].parent = parent;
		if (lexicon[j].length == word_len + 1)
		{
			dat[k].word = lexicon[j].pos;
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
		
		match_word(dat, word->key, &p, &i, 0);
		if (p == word->length)
			return;
		
		get_words_with_prefix(word->key, p);
		
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

void make(void)
{
	int i;
	for (i = 1; i < DATRIE_SIZE; i ++)
	{
		dat[i].parent = dat[i].base = DATRIE_UNUSED;
		dat[i].word = -1;
	}
	dat[0].parent = dat[0].base = 0;
	
	for (i = 0; i < lexicon_count; i ++)
		insert_first_char(i);
	for (i = 0; i < lexicon_count; i ++)
		insert(i);
}

int cmp(const void *a, const void *b)
{
	return wcscmp(((const opencc_entry *)a)->key, ((const opencc_entry *)b)->key);
}

void init(const char * file_name)
{
	opencc_dictionary_t dt = dict_open(file_name, OPENCC_DICTIONARY_TYPE_TEXT);

	if (dt == (opencc_dictionary_t) -1)
	{
		fprintf(stderr, "Can not read data from %s\n", file_name);
		exit(1);
	}

	static opencc_entry tlexicon[DATRIE_WORD_MAX_COUNT];

	lexicon_count = dict_get_lexicon(dt, tlexicon);
	qsort(tlexicon, lexicon_count, sizeof(tlexicon[0]), cmp);

	size_t i;
	lexicon[0].pos = 0;
	for (i = 0; i < lexicon_count; i ++)
	{
		lexicon[i].key = tlexicon[i].key;
		lexicon[i].value = tlexicon[i].value;
		lexicon[i].length = wcslen(lexicon[i].key);
		if (i > 0)
		{
			lexicon[i].pos = lexicon[i-1].pos + wcslen(lexicon[i-1].value) + 1;
		}
	}
}

void output(const char * file_name)
{
	FILE * fp = fopen(file_name, "wb");

	if (!fp)
	{
		fprintf(stderr, "Can not write file: %s\n", file_name);
		exit(1);
	}

	size_t i, item_count;
	
	for (i = DATRIE_SIZE - 1; i > 0; i --)
		if (dat[i].parent != DATRIE_UNUSED)
			break;
	item_count = i + 1;

	size_t lexicon_length = lexicon[lexicon_count - 1].pos +
			wcslen(lexicon[lexicon_count - 1].value) + 1;

	fwrite(OPENCC_DICHEADER, sizeof(char), strlen(OPENCC_DICHEADER), fp);
	fwrite(&lexicon_length, sizeof(size_t), 1, fp);
	fwrite(&item_count, sizeof(size_t), 1, fp);

	for (i = 0; i < lexicon_count; i ++)
	{
		fwrite(lexicon[i].value, sizeof(wchar_t), wcslen(lexicon[i].value) + 1, fp);
	}
	
	fwrite(dat, sizeof(dat[0]), item_count, fp);
	
	fclose(fp);
}

#if 0
void write_text_file()
{
	FILE * fp;
	int i;
	fp = fopen("datrie.txt","w");
	fprintf(fp, "%d\n", lexicon_count);

	for (i = 0; i < lexicon_count; i ++)
	{
		fprintf(fp, "%ls\n", lexicon[i].value);
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
#endif

void show_version()
{
	printf("\nOpen Chinese Convert (OpenCC) Dictionary Tool\nVersion %s\n\n",VERSION);
}

void show_usage()
{
	show_version();
	printf("Usage:\n");
	printf("  opencc_dict -i input_file -o output_file\n\n");
	printf("    -i\n");
	printf("      Read data from input_file.\n");
	printf("    -o\n");
	printf("      Write converted data to output_file.\n");
	printf("\n");
	printf("\n");
}

int main(int argc, char ** argv)
{
	static int oc;
	static char input_file[BUFFER_SIZE], output_file[BUFFER_SIZE];
	int input_file_specified = FALSE, output_file_specified = FALSE;

	while((oc = getopt(argc, argv, "vh-:i:o:")) != -1)
	{
		switch (oc)
		{
		case 'v':
			show_version();
			return;
		case 'h':
		case '?':
			show_usage();
			return;
		case '-':
			if (strcmp(optarg, "version") == 0)
				show_version();
			else if (strcmp(optarg, "help") == 0)
				show_usage();
			else
				show_usage();
			return;
		case 'i':
			strcpy(input_file, optarg);
			input_file_specified = TRUE;
			break;
		case 'o':
			strcpy(output_file, optarg);
			output_file_specified = TRUE;
			break;
		}
	}

	if (!input_file_specified)
	{
		fprintf(stderr, "Please specify input file using -i.\n");
		show_usage();
		return 1;
	}

	if (!output_file_specified)
	{
		fprintf(stderr, "Please specify output file using -o.\n");
		show_usage();
		return 1;
	}

	init(input_file);
	make();
	output(output_file);

	return 0;
}
