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

#include "opencc_convert.h"
#include "opencc_datrie.h"
#include "opencc_utils.h"

#define DEFAULT_SEGMENT_BUFF_SIZE 10240

typedef struct
{
	opencc_convert_direction_t convert_direction;
	opencc_convert_errno_t errno;
} opencc_description;

int segment_buff_size = DEFAULT_SEGMENT_BUFF_SIZE;
int * sp_seg_match_length = NULL;
int * sp_seg_min_len = NULL;
int * sp_seg_parent = NULL;
int * sp_seg_path = NULL;
int sp_seg_initialized = FALSE;

void sp_seg_set_buff_size()
{
	if (sp_seg_initialized == TRUE)
	{
		free(sp_seg_match_length);
		free(sp_seg_min_len);
		free(sp_seg_parent);
		free(sp_seg_path);
	}
	
	sp_seg_match_length = (int *) malloc((segment_buff_size + 1) * sizeof(int));
	sp_seg_min_len = (int *) malloc(segment_buff_size * sizeof(int));
	sp_seg_parent = (int *) malloc(segment_buff_size * sizeof(int));
	sp_seg_path = (int *) malloc(segment_buff_size * sizeof(int));
	
	sp_seg_initialized = TRUE;
}

wchar_t * sp_seg(wchar_t * pdest, const wchar_t * text, int start, int end)
{
	/* 最短路徑分詞 */
	int i, j, k;
	
	if (end - start == 1) /* 對長度爲1時特殊優化 */
	{
		const wchar_t * match_rs = get_trad_in_datrie(text + start, NULL, 1);
		
		if (match_rs == NULL)
			*pdest ++ = *(text + start);
		else
			*pdest ++ = *match_rs;
		
		return pdest;
	}
	
	if (sp_seg_initialized == FALSE)
		sp_seg_set_buff_size();
	
	for (i = start; i <= end; i ++)
		sp_seg_min_len[i - start] = sp_seg_parent[i - start] = INFINITY_INT;
	
	sp_seg_min_len[0] = sp_seg_parent[0] = 0;
	
	for (i = start; i < end; i ++)
	{
		get_match_lengths(text + i, sp_seg_match_length); /* 獲取所有匹配長度 */
		
		if (sp_seg_match_length[1] != 1)
			sp_seg_match_length[++ sp_seg_match_length[0]] = 1;
		
		for (j = 1; j <= sp_seg_match_length[0]; j ++) /* 動態規劃求最短分割路徑 */
		{
			k = sp_seg_match_length[j];
			sp_seg_match_length[j] = 0;
			
			if (sp_seg_min_len[i - start] + 1 <= sp_seg_min_len[i - start + k])
			{
				sp_seg_min_len[i - start + k] = sp_seg_min_len[i - start] + 1;
				sp_seg_parent[i - start + k] = i - start;
			}
		}
	}
	
	i = end - start; /* 取得最短分割路徑 */
	j = sp_seg_min_len[i];
	while (i != 0)
	{
		sp_seg_path[--j] = i;
		i = sp_seg_parent[i];
	}
	
	j = 0;
	for (i = 0; i < sp_seg_min_len[end - start]; i ++) /* 根據最短分割路徑轉換 */
	{
		k = sp_seg_path[i];
		
		const wchar_t * match_rs = get_trad_in_datrie(text + start + j, NULL, k - j);
		
		if (match_rs == NULL)
			*pdest ++ = *(text + start + j);
		else
		{
			const wchar_t * pmrs;
			for (pmrs = match_rs; *pmrs; pmrs ++)
				*pdest ++ = *pmrs;
		}
		
		j = k;
	}
	
	return pdest;
}

void mmsp_seg(opencc_description * od, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left)
{
	/* 正向最大分詞 */
	int i, start, bound;
	int length_limit = *inbuf_left;
	
	bound = -1;
	
	for (i = start = 0; i < length_limit && (*inbuf)[i]; i ++)
	{
		if (i == bound)
		{
			/* 對歧義部分進行最短路徑分詞 */
			//sp_seg(od, inbuf, inbuf_left, outbuf, outbuf_left, start, bound);
			start = i;
		}
	
		int match_len;
		get_trad_in_datrie((*inbuf) + i, &match_len, 0);
		
		//if (rs == NULL)
			match_len = 1;
		
		if (i + match_len > bound)
			bound = i + match_len;
	}
	
	//pdest = sp_seg(pdest, text, start, i);
}

size_t mm_seg(opencc_description * od, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left)
{
	size_t inbuf_left_start = *inbuf_left;

	for (; **inbuf && *inbuf_left > 0 && *outbuf_left > 0;)
	{
		int match_len;
		const wchar_t * match_rs = get_trad_in_datrie(*inbuf, &match_len, *inbuf_left);

		if (match_len > *outbuf_left) /* 輸出緩衝區剩餘空間小於分詞長度 */
		{
			if (inbuf_left_start - *inbuf_left > 0)
				break;
			od->errno = OPENCC_CONVERT_ERROR_OUTBUF_NOT_ENOUGH;
			return (size_t) -1;
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

int opencc_set_segment_buff_size(int size)
{
	segment_buff_size = size;
	sp_seg_set_buff_size();
}

int opencc_get_segment_buff_size(void)
{
	return segment_buff_size;
}

size_t opencc_convert(opencc_t odt, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left)
{
	opencc_description * od = (opencc_description *) odt;

	if (od->convert_direction == OPENCC_CONVERT_SIMP_TO_TRAD)
	{
		return mm_seg(od, inbuf, inbuf_left, outbuf, outbuf_left);
		//return mmsp_seg(od, inbuf, inbuf_left, outbuf, outbuf_left);
	}
}

opencc_t opencc_open(opencc_convert_direction_t convert_direction)
{
	opencc_description * od;
	od = (opencc_description *) malloc(sizeof(opencc_description));

	od->convert_direction = convert_direction;
	od->errno = OPENCC_CONVERT_ERROR_VOID;

	return (opencc_t) od;
}

void opencc_close(opencc_t odt)
{
	opencc_description * od;
	od = (opencc_description *) odt;
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
