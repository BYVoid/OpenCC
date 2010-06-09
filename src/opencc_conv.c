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

#define BUFF_SIZE 1048576

wchar_t buff[BUFF_SIZE],rs[BUFF_SIZE];

int main()
{
	setlocale(LC_ALL, "zh_CN.utf8");
	
	FILE * fp = stdin;
	
	opencc_set_segment_buff_size(BUFF_SIZE);
	
	while (fgetws(buff, BUFF_SIZE, fp) != NULL)
	{
		opencc_simp_to_trad(rs,buff);
		
		printf("%ls",rs);
	}
	
	return 0;
}
