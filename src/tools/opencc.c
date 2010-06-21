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
#include "opencc_utils.h"
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

void convert(const char * input_file, const char * output_file, const char * config_file)
{
	static char inbuf[BUFFER_SIZE + 1];
	
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
	
	while (fgets(inbuf, BUFFER_SIZE, fp) != NULL)
	{
		char * outbuf;
		outbuf = opencc_convert_utf8(od, inbuf, (size_t) -1);
		if (outbuf == (char *) -1)
		{
			opencc_perror(_("OpenCC error"));
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
	printf(_("\nOpen Chinese Convert (OpenCC) Command Line Tool\nVersion %s\n\n"),VERSION);
}

void show_usage()
{
	show_version();
	printf(_("Usage:\n"));
	printf(_("  opencc [-i input_file] [-o output_file] [-c config_file]\n\n"));
	printf(_("    -i input_file\n"));
	printf(_("      Read original text from input_file.\n"));
	printf(_("    -o output_file\n"));
	printf(_("      Write converted text to output_file.\n"));
	printf(_("    -c config_file\n"));
	printf(_("      Load dictionary configuration from config_file.\n"));
	printf(_("\n"));
	printf(_("  Note:\n"));
	printf(_("    Text from standard input will be read if input_file is not set\n"
			"    and will be written to standard output if output_file is not set.\n"));
	printf(_("    Default configuration(%s) will be load if config_file is not set.\n"), OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
	printf(_("\n"));
	printf(_("\n"));
}

int main(int argc, char ** argv)
{
	static int oc;
	static char *input_file, *output_file, *config_file;
	
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	textdomain(GETTEXT_PACKAGE);

	while((oc = getopt(argc, argv, "vh-:i:o:c:")) != -1)
	{
		switch (oc)
		{
		case 'v':
			show_version();
			return 0;
		case 'h':
		case '?':
			show_usage();
			return 0;
		case '-':
			if (strcmp(optarg, "version") == 0)
				show_version();
			else if (strcmp(optarg, "help") == 0)
				show_usage();
			else
				show_usage();
			return 0;
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
		config_file = mstrcpy(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);

	convert(input_file, output_file, config_file);

	free(input_file);
	free(output_file);
	free(config_file);

	return 0;
}
