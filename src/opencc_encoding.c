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
#include "opencc_encoding.h"
#include <iconv.h>
#include <errno.h>

#define INITIAL_BUFF_SIZE 1024

wchar_t * utf8_to_wcs(const char * inbuf, size_t inbuf_len)
{
	iconv_t cd = iconv_open("WCHAR_T", "UTF8");
	
	if (cd == (iconv_t) -1)
	{
		return (wchar_t *) -1;
	}
	
	char * pinbuf = (char *) inbuf;
	size_t insize = strlen(inbuf);
	if (inbuf_len < insize)
		insize = inbuf_len;

	size_t outbuf_len = INITIAL_BUFF_SIZE;
	wchar_t * outbuf = (wchar_t *) malloc(sizeof(wchar_t) * outbuf_len);
	char * poutbuf = (char *) outbuf;
	size_t outsize = outbuf_len * sizeof(wchar_t);

	while (insize > 0)
	{
		size_t retval = iconv(cd, &pinbuf, &insize, &poutbuf, &outsize);
		if (retval == (size_t) -1 && errno != E2BIG)
		{
			free(outbuf);
			return (wchar_t *) -1;
		}

		if (insize == 0)
			break;

		outbuf = (wchar_t *) realloc(outbuf, sizeof(wchar_t) * (outbuf_len + outbuf_len));
		poutbuf = (char *) outbuf + (outbuf_len * sizeof(wchar_t) - outsize);
		outbuf_len += outbuf_len;
		outsize = (outbuf + outbuf_len - (wchar_t *) poutbuf) * sizeof(wchar_t);
	}
	
	*((wchar_t *) poutbuf) = L'\0';

	outbuf = (wchar_t *) realloc(outbuf, sizeof(wchar_t) * ((wchar_t *) poutbuf - outbuf + 1));
	
	iconv_close(cd);
	
	return outbuf;
}

char * wcs_to_utf8(const wchar_t * inbuf, size_t inbuf_len)
{
	iconv_t cd = iconv_open("UTF8", "WCHAR_T");

	if (cd == (iconv_t) -1)
	{
		return (char *) -1;
	}
	
	char * pinbuf = (char *) inbuf;
	size_t insize = wcslen(inbuf);
	if (inbuf_len < insize)
		insize = inbuf_len;
	insize *= sizeof(wchar_t);

	size_t outbuf_len = INITIAL_BUFF_SIZE;
	char * outbuf = (char *) malloc(sizeof(char) * outbuf_len);
	char * poutbuf = outbuf;
	size_t outsize = outbuf_len;

	while (insize > 0)
	{
		size_t retval = iconv(cd, &pinbuf, &insize, &poutbuf, &outsize);
		if (retval == (size_t) -1 && errno != E2BIG)
		{
			free(outbuf);
			return (char *) -1;
		}

		if (insize == 0)
			break;

		outbuf = (char *) realloc(outbuf, sizeof(char) * (outbuf_len + outbuf_len));
		poutbuf = outbuf + (outbuf_len - outsize);
		outbuf_len += outbuf_len;
		outsize = outbuf + outbuf_len - poutbuf;
	}

	*poutbuf = '\0';
	
	iconv_close(cd);
	
	outbuf = (char *) realloc(outbuf, sizeof(char) * (poutbuf - outbuf + 1));
	
	return outbuf;
}
