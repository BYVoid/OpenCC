/*
 * Open Chinese Convert
 *
 * Copyright 2010-2013 BYVoid <byvoid@byvoid.com>
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

#include "config_reader.h"
#include "dict_group.h"
#include "dict_chain.h"

#define LINE_BUFFER_SIZE 8192
#define CONFIG_DICT_TYPE_OCD "OCD"
#define CONFIG_DICT_TYPE_TEXT "TEXT"

static config_error errnum = CONFIG_ERROR_VOID;

static int qsort_dictionary_buffer_cmp(const void* a, const void* b) {
  if (((DictMeta*)a)->index < ((DictMeta*)b)->index) {
    return -1;
  }
  if (((DictMeta*)a)->index > ((DictMeta*)b)->index) {
    return 1;
  }
  return ((DictMeta*)a)->stamp < ((DictMeta*)b)->stamp ? -1 : 1;
}

static int load_dictionary(Config* config) {
  if (config->dicts_count == 0) {
    return 0;
  }
  // Sort dictionaries
  qsort(config->dicts,
		    config->dicts_count,
		    sizeof(config->dicts[0]),
		    qsort_dictionary_buffer_cmp);
  DictGroup* group = dict_chain_add_group(config->dict_chain);
  size_t last_index = 0;
  size_t i;
  for (i = 0; i < config->dicts_count; i++) {
    if (config->dicts[i].index > last_index) {
      last_index = config->dicts[i].index;
      group = dict_chain_add_group(config->dict_chain);
    }
    if (dict_group_load(group,
                        config->dicts[i].file_name,
                        config->dicts[i].dict_type) == -1)
      return -1;
  }
  return 0;
}

static int parse_add_dict(Config* config, size_t index, const char* dictstr) {
  const char* pstr = dictstr;
  while (*pstr != '\0' && *pstr != ' ') {
    pstr++;
  }
  opencc_dictionary_type dict_type;
  if (strncmp(dictstr, CONFIG_DICT_TYPE_OCD,
              sizeof(CONFIG_DICT_TYPE_OCD) - 1) == 0) {
    dict_type = OPENCC_DICTIONARY_TYPE_DATRIE;
  } else if (strncmp(dictstr, CONFIG_DICT_TYPE_TEXT,
                     sizeof(CONFIG_DICT_TYPE_OCD) - 1) == 0) {
    dict_type = OPENCC_DICTIONARY_TYPE_TEXT;
  } else {
    errnum = CONFIG_ERROR_INVALID_DICT_TYPE;
    return -1;
  }
  while (*pstr != '\0' && (*pstr == ' ' || *pstr == '\t')) {
    pstr++;
  }
  size_t i = config->dicts_count++;
  config->dicts[i].dict_type = dict_type;
  config->dicts[i].file_name = mstrcpy(pstr);
  config->dicts[i].index = index;
  config->dicts[i].stamp = config->stamp++;
  return 0;
}

static int parse_property(Config* config, const char* key, const char* value) {
  if (strncmp(key, "dict", 4) == 0) {
    int index = 0;
    sscanf(key + 4, "%d", &index);
    return parse_add_dict(config, index, value);
  } else if (strcmp(key, "title") == 0) {
    free(config->title);
    config->title = mstrcpy(value);
    return 0;
  } else if (strcmp(key, "description") == 0) {
    free(config->description);
    config->description = mstrcpy(value);
    return 0;
  }
  errnum = CONFIG_ERROR_NO_PROPERTY;
  return -1;
}

static int parse_line(const char* line, char** key, char** value) {
  const char* line_begin = line;
  while (*line != '\0' && (*line != ' ' && *line != '\t' && *line != '=')) {
    line++;
  }
  size_t key_len = line - line_begin;
  while (*line != '\0' && *line != '=') {
    line++;
  }
  if (*line == '\0') {
    return -1;
  }
  assert(*line == '=');
  *key = mstrncpy(line_begin, key_len);
  line++;
  while (*line != '\0' && (*line == ' ' || *line == '\t')) {
    line++;
  }
  if (*line == '\0') {
    free(*key);
    return -1;
  }
  *value = mstrcpy(line);
  return 0;
}

static char* parse_trim(char* str) {
  for (; *str != '\0' && (*str == ' ' || *str == '\t'); str++) {}
  register char* prs = str;
  for (; *prs != '\0' && *prs != '\n' && *prs != '\r'; prs++) {}
  for (prs--; prs > str && (*prs == ' ' || *prs == '\t'); prs--) {}
  *(++prs) = '\0';
  return str;
}

static int parse(Config* config, const char* filename) {
  char* path = try_open_file(filename);
  if (path == NULL) {
    errnum = CONFIG_ERROR_CANNOT_ACCESS_CONFIG_FILE;
    return -1;
  }
  config->file_path = get_file_path(path);
  FILE* fp = fopen(path, "r");
  assert(fp != NULL);
  free(path);
  skip_utf8_bom(fp);
  static char buff[LINE_BUFFER_SIZE];
  while (fgets(buff, LINE_BUFFER_SIZE, fp) != NULL) {
    char* trimed_buff = parse_trim(buff);
    if ((*trimed_buff == ';') || (*trimed_buff == '#') ||
        (*trimed_buff == '\0')) {
      /* Comment Line or empty line */
      continue;
    }
    char* key = NULL, * value = NULL;
    if (parse_line(trimed_buff, &key, &value) == -1) {
      free(key);
      free(value);
      fclose(fp);
      errnum = CONFIG_ERROR_PARSE;
      return -1;
    }
    if (parse_property(config, key, value) == -1) {
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

DictChain* config_get_dict_chain(Config* config) {
  if (config->dict_chain != NULL) {
    dict_chain_delete(config->dict_chain);
  }
  config->dict_chain = dict_chain_new(config);
  if (load_dictionary(config) == -1) {
    dict_chain_delete(config->dict_chain);
    config->dict_chain = NULL;
  }
  return config->dict_chain;
}

config_error config_errno(void) {
  return errnum;
}

void config_perror(const char* spec) {
  perr(spec);
  perr("\n");
  switch (errnum) {
  case CONFIG_ERROR_VOID:
    break;
  case CONFIG_ERROR_CANNOT_ACCESS_CONFIG_FILE:
    perror(_("Can not access configuration file"));
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

Config* config_open(const char* filename) {
  Config* config = (Config*)malloc(sizeof(Config));
  config->title = NULL;
  config->description = NULL;
  config->dicts_count = 0;
  config->stamp = 0;
  config->dict_chain = NULL;
  config->file_path = NULL;
  if (parse(config, filename) == -1) {
    config_close((Config*)config);
    return (Config*)-1;
  }
  return (Config*)config;
}

void config_close(Config* config) {
  size_t i;
  for (i = 0; i < config->dicts_count; i++) {
    free(config->dicts[i].file_name);
  }
  free(config->title);
  free(config->description);
  free(config->file_path);
  free(config);
}
