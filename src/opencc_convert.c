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

wchar_t * mmsp_seg(wchar_t * dest, const wchar_t * text)
{
	/* 正向最大分詞 */
	static wchar_t buff_single[2] = {0, 0};
	int i, match_len, start, bound;
	wchar_t * pdest = dest;
	
	bound = -1;
	
	for (i = start = 0; text[i]; i ++)
	{
		if (i == bound)
		{
			/* 對歧義部分進行最短路徑分詞 */
			pdest = sp_seg(pdest, text, start, bound);
			start = i;
		}
	
		const wchar_t * rs = get_trad_in_datrie(text + i, &match_len, 0);
		
		if (rs == NULL)
			match_len = 1;
		
		if (i + match_len > bound)
			bound = i + match_len;
	}
	
	pdest = sp_seg(pdest, text, start, i);
	*pdest = 0;
	
	return dest;
}

wchar_t * mm_seg(wchar_t * dest, const wchar_t * text)
{
	const wchar_t * ptext;
	wchar_t * pdest;
	int match_pos;
	
	for (ptext = text, pdest = dest; *ptext;)
	{
		const wchar_t * match_rs = get_trad_in_datrie(ptext, &match_pos, 0);
		if (match_rs == NULL)
			*pdest ++ = *ptext ++;
		else
		{
			const wchar_t * pmrs;
			for (pmrs = match_rs; *pmrs; pmrs ++)
				*pdest ++ = *pmrs;
			ptext += match_pos;
		}
	}
	
	*pdest = 0;
	
	return dest;
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

wchar_t * opencc_simp_to_trad(wchar_t * dest, const wchar_t * text)
{
	return mmsp_seg(dest, text);
}
