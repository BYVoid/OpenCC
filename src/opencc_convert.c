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

wchar_t * segment_sp(wchar_t * pdest, const wchar_t * text, int start, int end)
{
	/* 最短路徑分詞 */
	static int match_length[SEGMENT_BUFF_SIZE + 1];
	static int min_len[SEGMENT_BUFF_SIZE], parent[SEGMENT_BUFF_SIZE], path[SEGMENT_BUFF_SIZE];
	
	int i, j, k;
	
	for (i = start; i <= end; i ++)
		min_len[i - start] = parent[i - start] = INFINITY_INT;
	
	min_len[0] = parent[0] = 0;
	
	for (i = start; i < end; i ++)
	{
		get_match_lengths(text + i, match_length); /* 獲取所有匹配長度 */
		
		if (match_length[1] != 1)
			match_length[++ match_length[0]] = 1;
		
		for (j = 1; j <= match_length[0]; j ++) /* 動態規劃求最短分割路徑 */
		{
			k = match_length[j];
			match_length[j] = 0;
			
			if (min_len[i - start] + 1 <= min_len[i - start + k])
			{
				min_len[i - start + k] = min_len[i - start] + 1;
				parent[i - start + k] = i - start;
			}
		}
	}
	
	i = end - start; /* 取得最短分割路徑 */
	j = min_len[i];
	while (i != 0)
	{
		path[--j] = i;
		i = parent[i];
	}
	
	j = 0;
	for (i = 0; i < min_len[end - start]; i ++) /* 根據最短分割路徑轉換 */
	{
		k = path[i];
		
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

wchar_t * words_segmention(wchar_t * dest, const wchar_t * text)
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
			pdest = segment_sp(pdest, text, start, bound);
			start = i;
		}
	
		const wchar_t * rs = get_trad_in_datrie(text + i, &match_len, 0);
		
		if (rs == NULL)
			match_len = 1;
		
		if (i + match_len > bound)
			bound = i + match_len;
	}
	
	pdest = segment_sp(pdest, text, start, i);
	*pdest = 0;
	
	return dest;
}

wchar_t * simp_to_trad(wchar_t * dest, const wchar_t * text)
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
		//*pdest ++ = ' ';
	}
	
	*pdest = 0;
	
	return dest;
}
