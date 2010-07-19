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

#include "opencc_config.h"

#define BUFFER_SIZE 8192
#define CONFIG_DICT_TYPE_OCD "OCD"
#define CONFIG_DICT_TYPE_TEXT "TEXT"

typedef struct
{
	char * title;
	char * description;
	opencc_dictionary * dicts;
	size_t dicts_count;
} config_desc;

static config_error errnum = CONFIG_ERROR_VOID;

static int parse_add_dict(config_desc * cd, const char * dictstr)
{
	const char * pstr = dictstr;

	while (*pstr != '\0' && *pstr !=' ')
		pstr ++;

	opencc_dictionary_type dict_type;

	if (strncmp(dictstr, CONFIG_DICT_TYPE_OCD, sizeof(CONFIG_DICT_TYPE_OCD) - 1) == 0)
		dict_type = OPENCC_DICTIONARY_TYPE_DATRIE;
	else if (strncmp(dictstr, CONFIG_DICT_TYPE_TEXT, sizeof(CONFIG_DICT_TYPE_OCD) - 1) == 0)
		dict_type = OPENCC_DICTIONARY_TYPE_TEXT;
	else
	{
		errnum = CONFIG_ERROR_INVALID_DICT_TYPE;
		return -1;
	}

	while (*pstr != '\0' && (*pstr == ' ' || *pstr == '\t'))
		pstr ++;

	size_t i = cd->dicts_count ++;
	if (cd->dicts_count == 1)
	{
		cd->dicts = (opencc_dictionary *) malloc(sizeof(opencc_dictionary));
	}
	else
	{
		cd->dicts = (opencc_dictionary *)
				realloc(cd->dicts, cd->dicts_count * sizeof(opencc_dictionary));
	}

	cd->dicts[i].dict_type = dict_type;
	cd->dicts[i].file_name = mstrcpy(pstr);

	return 0;
}

static int parse_property(config_desc * cd, const char * key, const char * value)
{
	if (strcmp(key, "dict") == 0)
	{
		return parse_add_dict(cd, value);
	}
	else if (strcmp(key, "title") == 0)
	{
		free(cd->title);
		cd->title = mstrcpy(value);
		return 0;
	}
	else if (strcmp(key, "description") == 0)
	{
		free(cd->description);
		cd->description = mstrcpy(value);
		return 0;
	}

	errnum = CONFIG_ERROR_NO_PROPERTY;
	return -1;
}

static int parse_line(const char * line, char ** key, char ** value)
{
	const char * line_begin = line;

	while (*line != '\0' && (*line != ' ' && *line != '\t' && *line != '='))
		line ++;

	size_t key_len = line - line_begin;

	while (*line != '\0' && *line != '=')
		line ++;

	if (*line == '\0')
		return -1;

	assert(*line == '=');

	*key = mstrncpy(line_begin, key_len);

	line ++;
	while (*line != '\0' && (*line == ' ' || *line =='\t'))
		line ++;

	if (*line == '\0')
	{
		free(*key);
		return -1;
	}

	*value = mstrcpy(line);

	return 0;
}

static char * parse_trim(char * str)
{
	for (; *str != '\0' && (*str == ' ' || *str =='\t'); str ++ );
	register char * prs = str;
	for (; *prs != '\0' && *prs != '\n' && *prs != '\r'; prs ++);
	for (prs --; prs > str && (*prs == ' ' || *prs == '\t'); prs --);
	*(++prs) = '\0';
	return str;
}

static int parse(config_desc * cd, const char * filename)
{
	FILE * fp = fopen(filename, "rb");
	if (!fp)
	{
		/* 使用 PKGDATADIR 路徑 */
		char * pkg_filename =
				(char *) malloc(sizeof(char) * (strlen(filename) + strlen(PKGDATADIR) + 1));
		sprintf(pkg_filename, "%s/%s", PKGDATADIR, filename);

		fp = fopen(pkg_filename, "rb");
		if (!fp)
		{
			free(pkg_filename);
			errnum = CONFIG_ERROR_CANNOT_ACCESS_CONFIG_FILE;
			return -1;
		}
		free(pkg_filename);
	}

	static char buff[BUFFER_SIZE];

	while (fgets(buff, BUFFER_SIZE, fp) != NULL)
	{
		char * trimed_buff = parse_trim(buff);
		if (*trimed_buff == ';' || *trimed_buff == '#' || *trimed_buff == '\0')
		{
			/* Comment Line or empty line */
			continue;
		}

		char * key, * value;

		if (parse_line(trimed_buff, &key, &value) == -1)
		{
			free(key);
			free(value);
			fclose(fp);
			errnum = CONFIG_ERROR_PARSE;
			return -1;
		}

		if (parse_property(cd, key, value) == -1)
		{
			free(key);
			free(value);
			fclose(fp);
			return -1;
		}

		free(key);
		free(value);
	}

	fclose(fp);
	return 0;
}

opencc_dictionary * config_get_dictionary(config_t ct, size_t * dict_count)
{
	config_desc * cd = (config_desc *) ct;

	*dict_count = cd->dicts_count;

	return cd->dicts;
}

config_error config_errnum(void)
{
	return errnum;
}

void config_perror(const char * spec)
{
	perr(spec);
	perr("\n");
	switch (errnum)
	{
	case CONFIG_ERROR_VOID:
		break;
	case CONFIG_ERROR_CANNOT_ACCESS_CONFIG_FILE:
		perror(_("Can not access configureation file"));
		break;
	case CONFIG_ERROR_PARSE:
		perr(_("Configuration file parse error"));
		break;
	case CONFIG_ERROR_NO_PROPERTY:
		perr(_("Invalid property"));
		break;
	case CONFIG_ERROR_INVALID_DICT_TYPE:
		perr(_("Invalid dictionary type"));
		break;
	default:
		perr(_("Unknown"));
	}
}

config_t config_open(const char * filename)
{
	config_desc * cd = (config_desc *) malloc(sizeof(config_desc));

	cd->title = NULL;
	cd->description = NULL;
	cd->dicts = NULL;
	cd->dicts_count = 0;

	if (parse(cd, filename) == -1)
	{
		config_close((config_t) cd);
		return (config_t) -1;
	}

	return (config_t) cd;
}

int config_close(config_t ct)
{
	config_desc * cd = (config_desc *) ct;

	free(cd->title);
	free(cd->description);

	size_t i;
	for (i = 0; i < cd->dicts_count; i ++)
		free(cd->dicts[i].file_name);

	free(cd->dicts);

	free(cd);

	return 0;
}
