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
#include "opencc_datrie.h"
#include "opencc_utils.h"

#define OPENCC_SP_SEG_DEFAULT_BUFFER_SIZE 1024

typedef struct
{
	int initialized;
	size_t buffer_size;
	size_t * match_length;
	size_t * min_len;
	size_t * parent;
	size_t * path;
} opencc_sp_seg_buffer;

typedef struct
{
	opencc_convert_direction_t convert_direction;
	opencc_convert_errno_t errno;
	opencc_sp_seg_buffer sp_seg_buffer;
} opencc_description;

void sp_seg_buffer_free(opencc_sp_seg_buffer * ossb)
{
	free(ossb->match_length);
	free(ossb->min_len);
	free(ossb->parent);
	free(ossb->path);
}

void sp_seg_set_buffer_size(opencc_sp_seg_buffer * ossb, size_t buffer_size)
{
	if (ossb->initialized == TRUE)
		sp_seg_buffer_free(ossb);
	
	ossb->buffer_size = buffer_size;
	ossb->match_length = (size_t *) malloc((buffer_size + 1) * sizeof(size_t));
	ossb->min_len = (size_t *) malloc(buffer_size * sizeof(size_t));
	ossb->parent = (size_t *) malloc(buffer_size * sizeof(size_t));
	ossb->path = (size_t *) malloc(buffer_size * sizeof(size_t));
	
	ossb->initialized = TRUE;
}

size_t sp_seg(opencc_description * od, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left, size_t length)
{
	/* 最短路徑分詞 */
	
	/* 對長度爲1時特殊優化 */
	if (length == 1)
	{
		const wchar_t * match_rs = datrie_match(*inbuf, NULL, 1);
		
		if (match_rs == NULL)
			**outbuf = **inbuf;
		else
			**outbuf = *match_rs;
		
		(*outbuf) ++,(*outbuf_left) --;
		(*inbuf) ++,(*inbuf_left) --;

		/* 必須保證有一個字符空間 */
		return 1;
	}
	
	/* 設置緩衝區空間 */
	opencc_sp_seg_buffer * ossb = &(od->sp_seg_buffer);
	size_t buffer_size_need = length + 1;
	if (ossb->initialized == FALSE || ossb->buffer_size < buffer_size_need)
		sp_seg_set_buffer_size(&(od->sp_seg_buffer), buffer_size_need);
	
	size_t i, j;

	for (i = 0; i <= length; i ++)
		ossb->min_len[i] = INFINITY_INT;
	
	ossb->min_len[0] = ossb->parent[0] = 0;
	
	for (i = 0; i < length; i ++)
	{
		/* 獲取所有匹配長度 */
		datrie_get_match_lengths((*inbuf) + i, ossb->match_length);
		
		if (ossb->match_length[1] != 1)
			ossb->match_length[++ ossb->match_length[0]] = 1;
		
		/* 動態規劃求最短分割路徑 */
		for (j = 1; j <= ossb->match_length[0]; j ++)
		{
			size_t k = ossb->match_length[j];
			ossb->match_length[j] = 0;
			
			if (ossb->min_len[i] + 1 <= ossb->min_len[i + k])
			{
				ossb->min_len[i + k] = ossb->min_len[i] + 1;
				ossb->parent[i + k] = i;
			}
		}
	}
	
	/* 取得最短分割路徑 */
	for (i = length, j = ossb->min_len[length]; i != 0; i = ossb->parent[i])
		ossb->path[--j] = i;
	
	size_t inbuf_left_start = *inbuf_left;
	size_t begin, end;

	/* 根據最短分割路徑轉換 */
	for (i = begin = 0; i < ossb->min_len[length]; i ++)
	{
		end = ossb->path[i];
		
		size_t match_len;
		const wchar_t * match_rs = datrie_match(*inbuf, &match_len, end - begin);
		
		/* 輸出緩衝區剩餘空間小於分詞長度 */
		if (match_len > *outbuf_left)
			break;

		if (match_rs == NULL)
		{
			**outbuf = **inbuf;
			(*outbuf) ++, (*outbuf_left) --;
			(*inbuf) ++, (*inbuf_left) --;
		}
		else
		{
			for (; *match_rs; match_rs ++)
			{
				**outbuf = *match_rs;
				(*outbuf) ++,(*outbuf_left) --;
				(*inbuf) ++,(*inbuf_left) --;
			}
		}
		
		begin = end;
	}
	
	return inbuf_left_start - *inbuf_left;
}

size_t mmsp_seg(opencc_description * od, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left)
{
	/* 歧義分割最短路徑分詞 */
	size_t i, start, bound;
	const wchar_t * inbuf_start = *inbuf;
	size_t inbuf_left_start = *inbuf_left;
	size_t sp_seg_length;
	
	bound = 0;
	
	for (i = start = 0; inbuf_start[i] && *inbuf_left > 0 && *outbuf_left > 0; i ++)
	{
		if (i != 0 && i == bound)
		{
			/* 對歧義部分進行最短路徑分詞 */
			sp_seg_length = sp_seg(od, inbuf, inbuf_left, outbuf, outbuf_left, bound - start);
			if (sp_seg_length ==  OPENCC_CONVERT_ERROR)
				return OPENCC_CONVERT_ERROR;
			if (sp_seg_length == 0)
			{
				if (inbuf_left_start - *inbuf_left > 0)
					return inbuf_left_start - *inbuf_left;
				od->errno = OPENCC_CONVERT_ERROR_OUTBUF_NOT_ENOUGH;
				return OPENCC_CONVERT_ERROR;
			}
			start = i;
		}
	
		size_t match_len;
		const wchar_t * match_rs = datrie_match(inbuf_start + i, &match_len, 0);
		
		if (match_rs == NULL)
			match_len = 1;
		
		if (i + match_len > bound)
			bound = i + match_len;
	}
	
	if (*inbuf_left > 0 && *outbuf_left > 0)
	{
		sp_seg_length = sp_seg(od, inbuf, inbuf_left, outbuf, outbuf_left, bound - start);
		if (sp_seg_length ==  OPENCC_CONVERT_ERROR)
			return OPENCC_CONVERT_ERROR;
		if (sp_seg_length == 0)
		{
			if (inbuf_left_start - *inbuf_left > 0)
				return inbuf_left_start - *inbuf_left;
			od->errno = OPENCC_CONVERT_ERROR_OUTBUF_NOT_ENOUGH;
			return OPENCC_CONVERT_ERROR;
		}
	}

	return inbuf_left_start - *inbuf_left;
}

size_t mm_seg(opencc_description * od, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left)
{
	/* 正向最大分詞 */
	size_t inbuf_left_start = *inbuf_left;

	for (; **inbuf && *inbuf_left > 0 && *outbuf_left > 0;)
	{
		size_t match_len;
		const wchar_t * match_rs = datrie_match(*inbuf, &match_len, *inbuf_left);

		/* 輸出緩衝區剩餘空間小於分詞長度 */
		if (match_len > *outbuf_left)
		{
			if (inbuf_left_start - *inbuf_left > 0)
				break;
			od->errno = OPENCC_CONVERT_ERROR_OUTBUF_NOT_ENOUGH;
			return OPENCC_CONVERT_ERROR;
		}

		if (match_rs == NULL)
		{
			**outbuf = **inbuf;
			(*outbuf) ++, (*outbuf_left) --;
			(*inbuf) ++, (*inbuf_left) --;
		}
		else
		{
			for (; *match_rs; match_rs ++)
			{
				**outbuf = *match_rs;
				(*outbuf) ++,(*outbuf_left) --;
				(*inbuf) ++,(*inbuf_left) --;
			}
		}
	}

	return inbuf_left_start - *inbuf_left;
}

size_t opencc_convert(opencc_t odt, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left)
{
	opencc_description * od = (opencc_description *) odt;

	if (od->convert_direction == OPENCC_CONVERT_SIMP_TO_TRAD)
	{
		return mm_seg(od, inbuf, inbuf_left, outbuf, outbuf_left);
	}
}

opencc_t opencc_open(opencc_convert_direction_t convert_direction)
{
	opencc_description * od;
	od = (opencc_description *) malloc(sizeof(opencc_description));

	od->convert_direction = convert_direction;
	od->errno = OPENCC_CONVERT_ERROR_VOID;
	od->sp_seg_buffer.initialized = FALSE;
	od->sp_seg_buffer.buffer_size = OPENCC_SP_SEG_DEFAULT_BUFFER_SIZE;
	od->sp_seg_buffer.match_length = od->sp_seg_buffer.min_len
			= od->sp_seg_buffer.parent = od->sp_seg_buffer.path = NULL;

	return (opencc_t) od;
}

void opencc_close(opencc_t odt)
{
	opencc_description * od;
	od = (opencc_description *) odt;
	sp_seg_buffer_free(&(od->sp_seg_buffer));
	free(od);
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
