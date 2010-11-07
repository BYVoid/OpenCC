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
#include "encoding.h"
#include <iconv.h>
#include <errno.h>

#define OUTER_ENCODIND "UTF-8"

#if BYTEORDER == LITTLE_ENDIAN
#	define INNER_ENCODIND "UCS-4LE"
#else
#	define INNER_ENCODIND "UCS-4BE"
#endif

#define INITIAL_BUFF_SIZE 1024
#define GET_BIT(byte,pos) (((byte)>>(pos))&1)
#define BITMASK(length) ((1 << length) - 1)

ucs4_t * utf8_to_ucs4(const char * utf8, size_t length)
{
	if (length == 0)
		length = (size_t) -1;
	size_t i;
	for (i = 0; i < length && utf8[i] != '\0'; i ++);
	length = i;

	size_t freesize = INITIAL_BUFF_SIZE;
	ucs4_t * ucs4 = (ucs4_t *) malloc(sizeof(ucs4_t) * freesize);
	ucs4_t * pucs4 = ucs4;

	for (i = 0; i < length; i ++)
	{
		ucs4_t byte[4] = {'\0'};
		if (GET_BIT(utf8[i], 7) == 0)
		{
			/* U-00000000 - U-0000007F */
			/* 0xxxxxxx */
			byte[0] = utf8[i] & BITMASK(7);
		}
		else if (GET_BIT(utf8[i], 5) == 0)
		{
			/* U-00000080 - U-000007FF */
			/* 110xxxxx 10xxxxxx */
			if (i + 1 >= length)
				return (ucs4_t *) -1;

			byte[0] = (utf8[i + 1] & BITMASK(6)) +
					((utf8[i] & BITMASK(2)) << 6);
			byte[1] = (utf8[i] >> 2) & BITMASK(3);

			i += 1;
		}
		else if (GET_BIT(utf8[i], 4) == 0)
		{
			/* U-00000800 - U-0000FFFF */
			/* 1110xxxx 10xxxxxx 10xxxxxx */
			if (i + 2 >= length)
				return (ucs4_t *) -1;

			byte[0] = (utf8[i + 2] & BITMASK(6)) +
					((utf8[i + 1] & BITMASK(2)) << 6);
			byte[1] = ((utf8[i + 1] >> 2) & BITMASK(4))
					+ ((utf8[i] & BITMASK(4)) << 4);

			i += 2;
		}
		else if (GET_BIT(utf8[i], 3) == 0)
		{
			/* U-00010000 - U-001FFFFF */
			/* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
			if (i + 3 >= length)
				return (ucs4_t *) -1;

			byte[0] = (utf8[i + 3] & BITMASK(6)) +
					((utf8[i + 2] & BITMASK(2)) << 6);
			byte[1] = ((utf8[i + 2] >> 2) & BITMASK(4)) +
					((utf8[i + 1] & BITMASK(4)) << 4);
			byte[2] = ((utf8[i + 1] >> 4) & BITMASK(2)) +
					((utf8[i] & BITMASK(3)) << 2);

			i += 3;
		}
		else if (GET_BIT(utf8[i], 2) == 0)
		{
			/* U-00200000 - U-03FFFFFF */
			/* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
			if (i + 4 >= length)
				return (ucs4_t *) -1;

			byte[0] = (utf8[i + 4] & BITMASK(6)) +
					((utf8[i + 3] & BITMASK(2)) << 6);
			byte[1] = ((utf8[i + 3] >> 2) & BITMASK(4)) +
					((utf8[i + 2] & BITMASK(4)) << 4);
			byte[2] = ((utf8[i + 2] >> 4) & BITMASK(2)) +
					((utf8[i + 1] & BITMASK(6)) << 2);
			byte[3] = utf8[i] & BITMASK(2);
			i += 4;
		}
		else if (GET_BIT(utf8[i], 2) == 0)
		{
			/* U-04000000 - U-7FFFFFFF */
			/* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
			if (i + 5 >= length)
				return (ucs4_t *) -1;

			byte[0] = (utf8[i + 5] & BITMASK(6)) +
					((utf8[i + 4] & BITMASK(2)) << 6);
			byte[1] = ((utf8[i + 4] >> 2) & BITMASK(4)) +
					((utf8[i + 3] & BITMASK(4)) << 4);
			byte[2] = ((utf8[i + 3] >> 4) & BITMASK(2)) +
					((utf8[i + 2] & BITMASK(6)) << 2);
			byte[3] = (utf8[i + 1] & BITMASK(6)) +
					((utf8[i] & BITMASK(1)) << 6);
			i += 5;
		}
		else
		{
			return (ucs4_t *) -1;
		}

		if (freesize == 0)
		{
			freesize = pucs4 - ucs4;
			ucs4 = (ucs4_t *) realloc(ucs4, sizeof(ucs4_t) * (freesize + freesize));
		}

		#if BYTEORDER == LITTLE_ENDIAN
			*pucs4 = (byte[3] << 24) + (byte[2] << 16) + (byte[1] << 8) + byte[0];
		#else
			*pucs4 = (byte[0] << 24) + (byte[1] << 16) + (byte[2] << 8) + byte[3];
		#endif

		pucs4 ++;
		freesize --;
	}

	*pucs4 = '\0';
	ucs4 = (ucs4_t *) realloc(ucs4, sizeof(ucs4_t) * (pucs4 - ucs4 + 1));
	return ucs4;
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
