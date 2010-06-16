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

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define VERSION "Unknown"
#endif

#define BUFFER_SIZE 32768

void convert(const char * input_file, const char * output_file)
{
	static char inbuf[BUFFER_SIZE + 1];
	
	opencc_t od = opencc_open(OPENCC_CONVERT_SIMP_TO_TRAD);
	if (od == (opencc_t) -1)
	{
		fprintf(stderr, "Can not create OpenCC.\n");
		exit(1);
	}

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
	
	while (fgets(inbuf, BUFFER_SIZE, fp) != NULL)
	{
		char * outbuf;
		outbuf = opencc_convert_utf8(od, inbuf, (size_t) -1);
		if (outbuf == NULL)
		{
			opencc_perror(od);
			break;
		}
		fprintf(fpo,"%s",outbuf);
	}
	
	opencc_close(od);
	
	fclose(fp);
	fclose(fpo);
}

void show_version()
{
	printf("\nOpen Chinese Convert (OpenCC) Command Line Tool\nVersion %s\n\n",VERSION);
}

void show_usage()
{
	show_version();
	printf("Usage:\n");
	printf("  opencc [-i input_file] [-o output_file]\n\n");
	printf("    -i\n");
	printf("      Read original text from input_file.\n");
	printf("    -o\n");
	printf("      Write converted text to output_file.\n");
	printf("\n");
	printf("  Note:\n");
	printf("    Text from standard input will be read if input_file is not set\n");
	printf("    and will be written to standard output if output_file is not set.\n");
	printf("\n");
	printf("\n");
}

int main(int argc, char ** argv)
{
	static int oc;
	static char input_file[BUFFER_SIZE], output_file[BUFFER_SIZE];
	
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
