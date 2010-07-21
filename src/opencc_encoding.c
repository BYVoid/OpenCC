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

#define OUTER_ENCODIND "UTF-8"
#define INNER_ENCODIND "UCS-4LE"
/* FIXME le*/
#define INITIAL_BUFF_SIZE 1024

ucs4_t * utf8_to_ucs4(const char * inbuf, size_t inbuf_len)
{
	iconv_t cd = iconv_open(INNER_ENCODIND, OUTER_ENCODIND);
	
	if (cd == (iconv_t) -1)
	{
		return (ucs4_t *) -1;
	}
	
	char * pinbuf = (char *) inbuf;
	size_t insize = strlen(inbuf);
	if (inbuf_len < insize)
		insize = inbuf_len;

	size_t outbuf_len = INITIAL_BUFF_SIZE;
	ucs4_t * outbuf = (ucs4_t *) malloc(sizeof(ucs4_t) * outbuf_len);
	char * poutbuf = (char *) outbuf;
	size_t outsize = outbuf_len * sizeof(ucs4_t);

	while (insize > 0)
	{
		size_t retval = iconv(cd, &pinbuf, &insize, &poutbuf, &outsize);
		if (retval == (size_t) -1 && errno != E2BIG)
		{
			free(outbuf);
			return (ucs4_t *) -1;
		}

		if (insize == 0)
			break;

		outbuf = (ucs4_t *) realloc(outbuf, sizeof(ucs4_t) * (outbuf_len + outbuf_len));
		poutbuf = (char *) outbuf + (outbuf_len * sizeof(ucs4_t) - outsize);
		outbuf_len += outbuf_len;
		outsize = (outbuf + outbuf_len - (ucs4_t *) poutbuf) * sizeof(ucs4_t);
	}
	
	*((ucs4_t *) poutbuf) = L'\0';

	outbuf = (ucs4_t *) realloc(outbuf, sizeof(ucs4_t) * ((ucs4_t *) poutbuf - outbuf + 1));
	
	iconv_close(cd);
	
	return outbuf;
}

char * ucs4_to_utf8(const ucs4_t * inbuf, size_t inbuf_len)
{
	iconv_t cd = iconv_open(OUTER_ENCODIND, INNER_ENCODIND);

	if (cd == (iconv_t) -1)
	{
		return (char *) -1;
	}
	
	char * pinbuf = (char *) inbuf;
	size_t insize = ucs4len(inbuf);
	if (inbuf_len < insize)
		insize = inbuf_len;
	insize *= sizeof(ucs4_t);

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

size_t ucs4len(const ucs4_t * str)
{
	const register ucs4_t * pstr = str;
	while (*pstr)
		++ pstr;
	return pstr - str;
}

int ucs4cmp(const ucs4_t * src, const ucs4_t * dst)
{   
	register int ret = 0;
	while(!(ret = *src - *dst) && *dst)
		++src, ++dst;
	return ret;
}

void ucs4cpy(ucs4_t * dest, const ucs4_t * src)
{
	while (*src)
		*dest ++ = *src ++;
	*dest = 0;
}

void ucs4ncpy(ucs4_t * dest, const ucs4_t * src, size_t len)
{
	while (*src && len -- > 0)
		*dest ++ = *src ++;
}
