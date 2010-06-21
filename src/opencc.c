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
#include "opencc_config.h"
#include "opencc_converter.h"
#include "opencc_encoding.h"
#include "opencc_utils.h"

static opencc_error errno = OPENCC_ERROR_VOID;

typedef struct
{
	opencc_dictionary_t dicts;
	opencc_converter_t converter;
} opencc_description;

size_t opencc_convert(opencc_t odt, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left)
{
	opencc_description * od = (opencc_description *) odt;

	size_t retval = converter_convert
			(od->converter, inbuf, inbuf_left, outbuf, outbuf_left);

	if (retval == (size_t) -1)
		errno = OPENCC_ERROR_CONVERTER;

	return retval;
}

char * opencc_convert_utf8(opencc_t odt, const char * inbuf, size_t length)
{
	if (length == (size_t) -1 || length > strlen(inbuf))
		length = strlen(inbuf);

	/* 將輸入數據轉換爲wchar_t字符串 */
	wchar_t * winbuf = utf8_to_wcs(inbuf, length);
	if (winbuf == (wchar_t *) -1)
	{
		/* 輸入數據轉換失敗 */
		errno = OPENCC_ERROR_ENCODIND;
		return (char *) -1;
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
		if (retval == (size_t) -1)
		{
			free(outbuf);
			free(winbuf);
			free(woutbuf);
			return (char *) -1;
		}

		*poutbuf = L'\0';

		char * ubuff = wcs_to_utf8(woutbuf, (size_t) -1);

		if (ubuff == (char *) -1)
		{
			free(outbuf);
			free(winbuf);
			free(woutbuf);
			errno = OPENCC_ERROR_ENCODIND;
			return (char *) -1;
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

opencc_t opencc_open(const char * config_file)
{
	opencc_description * od;
	od = (opencc_description *) malloc(sizeof(opencc_description));

	od->dicts = NULL;
	od->converter = converter_open();

	/* 加載默認辭典 */
	int retval;
	if (config_file == NULL)
		retval = 0;
	else
	{
		size_t dict_count;
		config_t ct = config_open(config_file);

		if (ct == (config_t) -1)
		{
			errno = OPENCC_ERROR_CONFIG;
			return (opencc_t) -1;
		}

		opencc_dictionary * dicts = config_get_dictionary(ct, &dict_count);

		int i, ret;
		for (i = dict_count - 1; i >= 0 ; i --)
		{
			ret = opencc_dict_load((opencc_t) od, dicts[i].file_name, dicts[i].dict_type);

			if (ret == -1)
			{
				opencc_close((opencc_t) od);
				config_close(ct);
				errno = OPENCC_ERROR_DICTLOAD;
				return (opencc_t) -1;
			}
		}

		config_close(ct);
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

int opencc_dict_load(opencc_t odt, const char * dict_filename,
		opencc_dictionary_type dict_type)
{
	opencc_description * od = (opencc_description *) odt;

	int retval;
	if (od->dicts == NULL)
	{
		od->dicts = dict_open(dict_filename, dict_type);
		if (od->dicts == (opencc_dictionary_t) -1)
		{
			od->dicts = NULL;
			return -1;
		}
		retval = 0;
	}
	else
	{
		retval = dict_load(od->dicts, dict_filename, dict_type);
	}

	converter_assign_dicts(od->converter, od->dicts);
	return retval;
}

opencc_error opencc_errno(void)
{
	return errno;
}

void opencc_perror(const char * spec)
{
	perr(spec);
	perr("\n");
	switch (errno)
	{
	case OPENCC_ERROR_VOID:
		break;
	case OPENCC_ERROR_DICTLOAD:
		dict_perror(_("Dictionary loading error"));
		break;
	case OPENCC_ERROR_CONFIG:
		config_perror(_("Configuration error"));
		break;
	case OPENCC_ERROR_CONVERTER:
		converter_perror(_("Converter error"));
		break;
	default:
		perr(_("Unknown"));
	}
	perr("\n");
}
