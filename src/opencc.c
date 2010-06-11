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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iconv.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define VERSION "Unknown"
#endif

#define INPUT_BUFFER_SIZE 32768
#define OUTPUT_BUFFER_SIZE 256

int utf82wcs(const char * inbuf, wchar_t * outbuf, size_t outbuf_size)
{
	iconv_t cd = iconv_open("WCHAR_T", "UTF8");
	
	if (cd == (iconv_t) -1)
	{
		*outbuf = L'\0';
		return -1;
	}
	
	char * pinbuf = (char *) inbuf;
	char * poutbuf = (char *) outbuf;
	size_t insize = strlen(inbuf);
	size_t outsize = outbuf_size;

	size_t retval = iconv(cd, &pinbuf, &insize, &poutbuf, &outsize);
	
	if (retval == (size_t) -1)
		return -1;
	
	if (outsize >= sizeof (wchar_t))
		*((wchar_t *) poutbuf) = L'\0';
	else
		return -1;
	
	iconv_close(cd);
	
	return 0;
}

int wcs2utf8(const wchar_t * inbuf, char * outbuf, size_t outbuf_size)
{
	iconv_t cd = iconv_open("UTF8", "WCHAR_T");
	
	if (cd == (iconv_t) -1)
	{
		*outbuf = '\0';
		return -1;
	}
	
	char * pinbuf = (char *) inbuf;
	char * poutbuf = outbuf;
	size_t insize = wcslen(inbuf) * sizeof(wchar_t);
	size_t outsize = outbuf_size;

	size_t retval = iconv(cd, &pinbuf, &insize, &poutbuf, &outsize);
	
	if (retval == (size_t) -1)
		return -1;
	
	if (outsize >= sizeof (char))
		*poutbuf = '\0';
	else
		return -1;
	
	iconv_close(cd);
	
	return 0;
}

void convert(const char * input_file, const char * output_file)
{
	static char inbuf[INPUT_BUFFER_SIZE + 1];
	static char outbuf[OUTPUT_BUFFER_SIZE + 1];
	static wchar_t winbuf[INPUT_BUFFER_SIZE + 1];
	static wchar_t woutbuf[OUTPUT_BUFFER_SIZE + 1];
	
	FILE * fp = stdin;
	FILE * fpo = stdout;
	
	if (*input_file)
	{
		fp = fopen(input_file, "r");
		if (!fp)
		{
			fprintf(stderr, "Can not read file: %s\n", input_file);
			exit(1);
		}
	}
	
	if (*output_file)
	{
		fpo = fopen(output_file, "w");
		if (!fpo)
		{
			fprintf(stderr, "Can not write file: %s\n", output_file);
			exit(1);
		}
	}
	
	opencc_t od = opencc_open(OPENCC_CONVERT_SIMP_TO_TRAD);
	
	while (fgets(inbuf, INPUT_BUFFER_SIZE, fp) != NULL)
	{
		utf82wcs(inbuf, winbuf, INPUT_BUFFER_SIZE);
	
		wchar_t * pinbuf = winbuf, * poutbuf = woutbuf;
		size_t inbuf_left, outbuf_left;
		size_t ccnt;

		inbuf_left = wcslen(winbuf);
		outbuf_left = OUTPUT_BUFFER_SIZE;

		while ((ccnt = opencc_convert(od, &pinbuf, &inbuf_left, &poutbuf, &outbuf_left)) > 0)
		{
			if (ccnt == OPENCC_CONVERT_ERROR)
			{
				opencc_perror(od);
				break;
			}

			*poutbuf = L'\0';

			wcs2utf8(woutbuf, outbuf, OUTPUT_BUFFER_SIZE);
			fprintf(fpo,"%s",outbuf);

			outbuf_left = OUTPUT_BUFFER_SIZE;
			poutbuf = woutbuf;
		}
		
	}
	
	opencc_close(od);
	
	fclose(fp);
	fclose(fpo);
}

void show_version()
{
	fprintf(stderr, "\nOpen Chinese Convert (OpenCC) Command Line Tool\nVersion %s\n\n",VERSION);
}

void show_usage()
{
	show_version();
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  opencc [-i input_file] [-o output_file]\n\n");
	fprintf(stderr, "    -i\n");
	fprintf(stderr, "      Read original text from input_file.\n");
	fprintf(stderr, "    -o\n");
	fprintf(stderr, "      Write converted text to output_file.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  Note:\n");
	fprintf(stderr, "    Text from standard input will be read if input_file is not set\n");
	fprintf(stderr, "    and will be written to standard output if output_file is not set.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\n");
}

int main(int argc, char ** argv)
{
	static int oc;
	static char input_file[OUTPUT_BUFFER_SIZE], output_file[OUTPUT_BUFFER_SIZE];
	
	while((oc = getopt(argc, argv, "vh-:i:o:")) != -1)
	{
		switch (oc)
		{
		case 'v':
			show_version();
			return;
		case 'h':
		case '?':
			show_usage();
			return;
		case '-':
			if (strcmp(optarg, "version") == 0)
				show_version();
			else if (strcmp(optarg, "help") == 0)
				show_usage();
			else
				show_usage();
			return;
		case 'i':
			strcpy(input_file, optarg);
			break;
		case 'o':
			strcpy(output_file, optarg);
			break;
		}
	}
	
	convert(input_file, output_file);
	
	return 0;
}
