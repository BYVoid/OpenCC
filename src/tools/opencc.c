/*
* Open Chinese Convert
*
* Copyright 2010 BYVoid <byvoid.kcp@gmail.com>
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

#include "../opencc.h"
#include "../utils.h"
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#define BUFFER_SIZE 32768

void convert(const char * input_file, const char * output_file, const char * config_file)
{
	opencc_t od = opencc_open(config_file);
	if (od == (opencc_t) -1)
	{
		opencc_perror(_("OpenCC initialization error"));
		exit(1);
	}

	FILE * fp = stdin;
	FILE * fpo = stdout;
	
	if (input_file)
	{
		fp = fopen(input_file, "r");
		if (!fp)
		{
			fprintf(stderr, _("Can not read file: %s\n"), input_file);
			exit(1);
		}
	}
	
	if (output_file)
	{
		fpo = fopen(output_file, "w");
		if (!fpo)
		{
			fprintf(stderr, _("Can not write file: %s\n"), output_file);
			exit(1);
		}
	}
	
	size_t size = BUFFER_SIZE;
	char * buffer_in = NULL, * buffer_out = NULL;
	buffer_in = (char *) malloc(size * sizeof(char));
	
	while (fgets(buffer_in, size, fp) != NULL)
	{
		size_t freesize = size;
		
		char * buffer_in_p = buffer_in;
		size_t line_length = strlen(buffer_in_p);
		while (line_length + 1 == freesize && buffer_in_p[line_length - 2] != '\n')
		{
			//如果一行沒讀完，則最後一個字符不是換行，且讀滿緩衝區
			buffer_in_p += size - 1;
			freesize = size + 1;
			size += size;
			size_t offset = buffer_in_p - buffer_in;
			buffer_in = (char *) realloc(buffer_in, size * sizeof(char));
			buffer_in_p = buffer_in + offset;
			
			if (fgets(buffer_in_p, freesize, fp) == NULL)
				break;

			line_length = strlen(buffer_in_p);
		}
		
		buffer_out = opencc_convert_utf8(od, buffer_in, (size_t) -1);
		if (buffer_out != (char *) -1)
		{
			fprintf(fpo, "%s", buffer_out);
		}
		else
		{
			opencc_perror(_("OpenCC error"));
			break;
		}
	}
	
	opencc_close(od);
	
	free(buffer_in);
	free(buffer_out);
	
	fclose(fp);
	fclose(fpo);
}

void show_version()
{
  printf(_("\n"));
	printf(_("Open Chinese Convert (OpenCC) Command Line Tool\n"));
	printf(_("Version %s\n"), VERSION);
	printf(_("\n"));
	printf(_("Author: %s\n"), "BYVoid <byvoid.kcp@gmail.com>");
	printf(_("Bug Report: %s\n"), "http://code.google.com/p/open-chinese-convert/issues/entry");
	printf(_("\n"));
}

void show_usage()
{
	show_version();
	printf(_("Usage:\n"));
	printf(_(" opencc [Options]\n"));
	printf(_("\n"));
	printf(_("Options:\n"));
	printf(_(" -i [file], --input=[file]   Read original text from [file].\n"));
	printf(_(" -o [file], --output=[file]  Write converted text to [file].\n"));
	printf(_(" -c [file], --config=[file]  Load configuration of conversion from [file].\n"));
	printf(_(" -v, --version               Print version and build information.\n"));
	printf(_(" -h, --help                  Print this help.\n"));
	printf(_("\n"));
	printf(_("With no input file, reads standard input and writes converted stream to standard output.\n"));
	printf(_("Default configuration(%s) will be loaded if not set.\n"), OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
	printf(_("\n"));
}

int main(int argc, char ** argv)
{
#ifdef ENABLE_GETTEXT
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE_NAME, LOCALEDIR);
#endif

	static struct option longopts[] =
	{
		{ "version", no_argument, NULL, 'v' },
		{ "help", no_argument, NULL, 'h' },
		{ "input", required_argument, NULL, 'i' },
		{ "output", required_argument, NULL, 'o' },
		{ "config", required_argument, NULL, 'c' },
		{ 0, 0, 0, 0 },
	};

	static int oc;
	static char *input_file, *output_file, *config_file;

	while((oc = getopt_long(argc, argv, "vh:i:o:c:", longopts, NULL)) != -1)
	{
		switch (oc)
		{
		case 'v':
			show_version();
			return 0;
		case 'h':
			show_usage();
			return 0;
		case '?':
			printf(_("Please use %s --help.\n"), argv[0]);
			return 1;
		case 'i':
			input_file = mstrcpy(optarg);
			break;
		case 'o':
			output_file = mstrcpy(optarg);
			break;
		case 'c':
			config_file = mstrcpy(optarg);
			break;
		}
	}

	if (config_file == NULL)
	{
		config_file = mstrcpy(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
	}

	convert(input_file, output_file, config_file);

	free(input_file);
	free(output_file);
	free(config_file);

	return 0;
}
