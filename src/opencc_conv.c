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
#include <locale.h>
#include <stdio.h>

wchar_t buff[SEGMENT_BUFF_SIZE],rs[SEGMENT_BUFF_SIZE];

int main()
{
	setlocale(LC_ALL, "zh_CN.UTF-8");
	
	FILE * fp = stdin;
	//fp = fopen("testsimp.txt","r");
	
	while (fgetws(buff, SEGMENT_BUFF_SIZE, fp) != NULL)
	{
		//simp_to_trad(rs,buff);
		words_segmention(rs,buff);
		printf("%ls",rs);
	}
	
	return 0;
}
