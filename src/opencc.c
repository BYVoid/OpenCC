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

#include "opencc.h"
#include "opencc_converter.h"
#include "opencc_encoding.h"
#include "opencc_utils.h"

typedef struct
{
	opencc_dictionary_t dicts;
	opencc_convert_direction_t convert_direction;
	opencc_convert_errno_t errno;
	opencc_converter_t converter;
} opencc_description;

size_t opencc_convert(opencc_t odt, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left)
{
	opencc_description * od = (opencc_description *) odt;

	if (od->dicts == NULL)
	{
		/* TODO:沒有加載辭典 */
		return OPENCC_CONVERT_ERROR;
	}

	return converter_convert(od->converter, inbuf, inbuf_left, outbuf, outbuf_left);
}

char * opencc_convert_utf8(opencc_t odt, const char * inbuf, size_t length)
{
	if (length == (size_t) -1 || length > strlen(inbuf))
		length = strlen(inbuf);

	/* 將輸入數據轉換爲wchar_t字符串 */
	wchar_t * winbuf = utf8_to_wcs(inbuf, length);
	if (winbuf == NULL)
	{
		/* 輸入數據轉換失敗 */
		return NULL;
	}

	/* 設置輸出UTF8文本緩衝區空間 */
	size_t outbuf_len = length;
	size_t outsize = outbuf_len;
	char * original_outbuf = (char *) malloc(sizeof(char) * (outbuf_len + 1));
	char * outbuf = original_outbuf;

	/* 設置轉換緩衝區空間 */
	size_t wbufsize = length;
	wchar_t * woutbuf = (wchar_t *) malloc(sizeof(wchar_t) * (wbufsize + 1));

	wchar_t * pinbuf = winbuf;
	wchar_t * poutbuf = woutbuf;
	size_t inbuf_left, outbuf_left;

	inbuf_left = wcslen(winbuf);
	outbuf_left = wbufsize;

	while (inbuf_left > 0)
	{
		size_t retval = opencc_convert(odt, &pinbuf, &inbuf_left, &poutbuf, &outbuf_left);
		if (retval == OPENCC_CONVERT_ERROR)
		{
			free(outbuf);
			free(winbuf);
			free(woutbuf);
			return NULL;
		}

		*poutbuf = L'\0';

		char * ubuff = wcs_to_utf8(woutbuf, (size_t) -1);

		if (ubuff == NULL)
		{
			free(outbuf);
			free(winbuf);
			free(woutbuf);
			return NULL;
		}

		size_t ubuff_len = strlen(ubuff);

		while (ubuff_len > outsize)
		{
			size_t outbuf_offset = outbuf - original_outbuf;
			outsize += outbuf_len;
			outbuf_len += outbuf_len;
			original_outbuf = (char *) realloc(original_outbuf, sizeof(char) * outbuf_len);
			outbuf = original_outbuf + outbuf_offset;
		}

		strncpy(outbuf, ubuff, ubuff_len);
		free(ubuff);

		outbuf += ubuff_len;
		*outbuf = '\0';

		outbuf_left = wbufsize;
		poutbuf = woutbuf;
	}

	free(winbuf);
	free(woutbuf);

	original_outbuf = (char *) realloc(original_outbuf,
			sizeof(char) * (strlen(original_outbuf) + 1));

	return original_outbuf;
}

opencc_t opencc_open(opencc_convert_direction_t convert_direction)
{
	opencc_description * od;
	od = (opencc_description *) malloc(sizeof(opencc_description));

	od->convert_direction = convert_direction;
	od->errno = OPENCC_CONVERT_ERROR_VOID;
	od->dicts = NULL;
	od->converter = converter_open();

	/* 加載默認辭典 */
	int retval;
	if (convert_direction == OPENCC_CONVERT_SIMP_TO_TRAD)
		retval = opencc_dict_load((opencc_t) od, "simp_to_trad.ocd", OPENCC_DICTIONARY_TYPE_DATRIE);
	else if (convert_direction == OPENCC_CONVERT_TRAD_TO_SIMP)
		retval = opencc_dict_load((opencc_t) od, "trad_to_simp.ocd", OPENCC_DICTIONARY_TYPE_DATRIE);
	else if (convert_direction == OPENCC_CONVERT_CUSTOM)
		;
	else
		debug_should_not_be_here();

	if (retval == -1)
	{
		opencc_close((opencc_t) od);
		return (opencc_t) -1;
	}

	return (opencc_t) od;
}

int opencc_close(opencc_t odt)
{
	opencc_description * od = (opencc_description *) odt;

	converter_close(od->converter);
	if (od->dicts)
		dict_close(od->dicts);
	free(od);

	return 0;
}

opencc_convert_errno_t opencc_errno(opencc_t odt)
{
	opencc_description * od;
	od = (opencc_description *) odt;
	return od->errno;
}

void opencc_perror(opencc_t odt)
{
	switch (opencc_errno(odt))
	{
	case OPENCC_CONVERT_ERROR_VOID:
		break;
	case OPENCC_CONVERT_ERROR_OUTBUF_NOT_ENOUGH:
		fprintf(stderr, "Output buffer is not enough for one segment.\n");
		break;
	}
}

size_t opencc_dict_get_lexicon(opencc_t odt, opencc_entry * lexicon)
{
	opencc_description * od = (opencc_description *) odt;
	if (od->dicts == NULL)
	{
		return (size_t) -1;
	}
	else
	{
		return dict_get_lexicon(od->dicts, lexicon);
	}
}

int opencc_dict_load(opencc_t odt, const char * dict_filename,
		opencc_dictionary_type dict_type)
{
	opencc_description * od = (opencc_description *) odt;

	if (od->dicts == NULL)
	{
		od->dicts = dict_open(dict_filename, dict_type);
		if (od->dicts == (opencc_dictionary_t) -1)
		{
			od->dicts = NULL;
			return -1;
		}
	}
	else
	{
		int retval = dict_load(od->dicts, dict_filename, dict_type);
		converter_assign_dicts(od->converter, od->dicts);
		return retval;
	}
}
