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
#include "opencc_dictionary_abstract.h"

const wchar_t * dict_match_longest(opencc_dictionary_t ddt, const wchar_t * word,
		size_t length)
{
	opencc_dictionary_description * dd = (opencc_dictionary_description *) ddt;

	const wchar_t * retvel = NULL;
	size_t match_length, max_length = 0;

	int i;
	/* 依次查找每個辭典，取得最長匹配長度 */
	for (i = dd->dict_count - 1; i >= 0; i --)
	{
		const wchar_t * t_retvel =
				dict_abstract_match_longest(dd->dict + i, word, length);

		if (t_retvel != NULL)
		{
			match_length = wcslen(t_retvel);
			if (match_length > max_length)
			{
				max_length = match_length;
				retvel = t_retvel;
			}
		}
	}

	return retvel;
}

void dict_get_all_match_lengths(opencc_dictionary_t ddt, const wchar_t * word,
		size_t * match_length)
{
	opencc_dictionary_description * dd = (opencc_dictionary_description *) ddt;

	int i;
	/* 依次查找每個辭典，取得最長匹配長度 */
	for (i = 0; i < dd->dict_count; i --)
	{
		dict_abstract_get_all_match_lengths(dd->dict + i, word, match_length);
		/* 去除重複長度 */
		if (i > 0 && match_length[0] > 1)
		{
			qsort(match_length + 1, match_length[0], sizeof(match_length[0]), qsort_int_cmp);
			int j, k;
			for (j = 1, k = 2; k < match_length[0]; k ++)
			{
				if (match_length[k] != match_length[j])
					match_length[++ j] = match_length[k];
			}
			match_length[0] = j;
		}
	}
}

int dict_load(opencc_dictionary_t ddt, const char * dict_filename,
		opencc_dictionary_type dict_type)
{
	opencc_dictionary_description * dd = (opencc_dictionary_description *) ddt;
	opencc_dictionary dict;
	dict.filename = (char *) dict_filename;
	dict.type = dict_type;

	FILE * fp = fopen(dict.filename,"rb");
	if (!fp)
	{
		fclose(fp);
		return -1; /* 辭典文件無法訪問 */
	}
	fclose(fp);

	dict_ptr dp = dict_abstract_open(&dict);

	if (dp == (dict_ptr) -1)
		return -1; /* 辭典讀取錯誤 */

	size_t i = dd->dict_count ++;
	dd->dict[i].type = dict.type;
	dd->dict[i].filename = (char *) malloc(sizeof(char) * (strlen(dict.filename) + 1));
	strcpy(dd->dict[i].filename, dict.filename);
	dd->dict[i].dict = dp;

	return 0;
}

int dict_close(opencc_dictionary_t ddt)
{
	opencc_dictionary_description * dd = (opencc_dictionary_description *) ddt;
	int i;
	for (i = 0;i < dd->dict_count; i ++)
	{
		dict_abstract_close(dd->dict + i);
		free(dd->dict[i].filename);
	}
	free(dd);
	return 0;
}

opencc_dictionary_t dict_open(const char * dict_filename, opencc_dictionary_type dict_type)
{
	opencc_dictionary_description * dd;
	dd = (opencc_dictionary_description *) malloc(sizeof(opencc_dictionary_description));

	dd->dict_count = 0;
	if (dict_load((opencc_dictionary_t) dd, dict_filename, dict_type) == -1)
	{
		dict_close((opencc_dictionary_t) dd);
		return (opencc_dictionary_t) -1;
	}

	return (opencc_dictionary_t) dd;
}
