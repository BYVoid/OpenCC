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

#define INPUT_BUFFER_SIZE 1048576
#define OUTPUT_BUFFER_SIZE 64

wchar_t inbuf[INPUT_BUFFER_SIZE + 1],outbuf[OUTPUT_BUFFER_SIZE + 1];

int main()
{
	setlocale(LC_ALL, "zh_CN.utf8");
	
	FILE * fp = stdin;
	//fp = fopen("testsimp.txt","r");
	
	opencc_t od = opencc_open(OPENCC_CONVERT_SIMP_TO_TRAD);
	
	while (fgetws(inbuf, INPUT_BUFFER_SIZE, fp) != NULL)
	{
		wchar_t * pinbuf = inbuf, * poutbuf = outbuf;
		size_t inbuf_left, outbuf_left;
		size_t ccnt;

		inbuf_left = wcslen(inbuf);
		outbuf_left = OUTPUT_BUFFER_SIZE;

		while ((ccnt = opencc_convert(od, &pinbuf, &inbuf_left, &poutbuf, &outbuf_left)) > 0)
		{
			if (ccnt == OPENCC_CONVERT_ERROR)
			{
				opencc_perror(od);
				break;
			}

			*poutbuf = 0;

			//printf("%ls %d %d\n", outbuf, inbuf_left, outbuf_left);
			printf("%ls",outbuf);

			outbuf_left = OUTPUT_BUFFER_SIZE;
			poutbuf = outbuf;
		}
		
	}
	
	opencc_close(od);

	return 0;
}
