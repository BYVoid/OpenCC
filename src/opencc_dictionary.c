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
#include "opencc_encoding.h"
#include "dictionary/opencc_dictionary_abstract.h"

static dictionary_error errnum = DICTIONARY_ERROR_VOID;

const ucs4_t * dict_match_longest(opencc_dictionary_t ddt, const ucs4_t * word,
		size_t length)
{
	opencc_dictionary_description * dd = (opencc_dictionary_description *) ddt;

	const ucs4_t * retvel = NULL;
	size_t match_length, max_length = 0;

	if (dd->dict_count == 0)
	{
		errnum = DICTIONARY_ERROR_NODICT;
		return (const ucs4_t *) -1;
	}

	int i;
	/* 依次查找每個辭典，取得最長匹配長度 */
	for (i = dd->dict_count - 1; i >= 0; i --)
	{
		const ucs4_t * t_retvel =
				dict_abstract_match_longest(dd->dict + i, word, length);

		if (t_retvel != NULL)
		{
			match_length = ucs4len(t_retvel);
			if (match_length > max_length)
			{
				max_length = match_length;
				retvel = t_retvel;
			}
		}
	}

	return retvel;
}

size_t dict_get_all_match_lengths(opencc_dictionary_t ddt, const ucs4_t * word,
		size_t * match_length)
{
	opencc_dictionary_description * dd = (opencc_dictionary_description *) ddt;
	size_t rscnt = 0;

	if (dd->dict_count == 0)
	{
		errnum = DICTIONARY_ERROR_NODICT;
		return (size_t) -1;
	}

	int i;
	for (i = 0; i < dd->dict_count; i --)
	{
		size_t retval;
		retval = dict_abstract_get_all_match_lengths(dd->dict + i, word, match_length + rscnt);
		rscnt += retval;
		/* 去除重複長度 */
		if (i > 0 && rscnt > 1)
		{
			qsort(match_length, rscnt, sizeof(match_length[0]), qsort_int_cmp);
			int j, k;
			for (j = 0, k = 1; k < rscnt; k ++)
			{
				if (match_length[k] != match_length[j])
					match_length[++ j] = match_length[k];
			}
			rscnt = j;
		}
	}

	return rscnt;
}

size_t dict_get_lexicon(opencc_dictionary_t ddt, opencc_entry * lexicon)
{
	opencc_dictionary_description * dd = (opencc_dictionary_description *) ddt;

	if (dd->dict_count == 0)
	{
		errnum = DICTIONARY_ERROR_NODICT;
		return (size_t) -1;
	}

	size_t count = 0;
	int i;
	for (i = dd->dict_count - 1; i >= 0; i --)
	{
		count += dict_abstract_get_lexicon(dd->dict + i, lexicon + count);
	}

	return count;
}

int dict_load(opencc_dictionary_t ddt, const char * dict_filename,
		opencc_dictionary_type dict_type)
{
	opencc_dictionary_description * dd = (opencc_dictionary_description *) ddt;
	dictionary dict;

	dict.filename = (char *) malloc(sizeof(char) * (strlen(dict_filename) + 1));
	strcpy(dict.filename, dict_filename);
	dict.type = dict_type;

	FILE * fp = fopen(dict.filename, "rb");
	if (!fp)
	{
		/* 使用 PKGDATADIR 路徑 */
		dict.filename = (char *) realloc(dict.filename,
				sizeof(char) * (strlen(dict_filename) + strlen(PKGDATADIR) + 2));
		sprintf(dict.filename, "%s/%s", PKGDATADIR, dict_filename);

		fp = fopen(dict.filename, "rb");
		if (!fp)
		{
			free(dict.filename);
			errnum = DICTIONARY_ERROR_CANNOT_ACCESS_DICTFILE;
			return -1; /* 辭典文件無法訪問 */
		}
	}
	fclose(fp);

	dict_ptr dp = dict_abstract_open(&dict);

	free(dict.filename);

	if (dp == (dict_ptr) -1)
	{
		errnum = DICTIONARY_ERROR_INVALID_DICT;
		return -1; /* 辭典讀取錯誤 */
	}

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

dictionary_error dict_errnum(void)
{
	return errnum;
}

void dict_perror(const char * spec)
{
	perr(spec);
	perr("\n");
	switch(errnum)
	{
	case DICTIONARY_ERROR_VOID:
		break;
	case DICTIONARY_ERROR_NODICT:
		perr(_("No dictionary loaded"));
		break;
	case DICTIONARY_ERROR_CANNOT_ACCESS_DICTFILE:
		perror(_("Can not open dictionary file"));
		break;
	case DICTIONARY_ERROR_INVALID_DICT:
		perror(_("Invalid dictionary file"));
		break;
	default:
		perr(_("Unknown"));
	}
}
