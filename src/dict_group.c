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

static dictionary_error errnum = DICTIONARY_ERROR_VOID;

DictGroup* dict_group_new(DictChain* dict_chain) {
  DictGroup* dict_group =
    (DictGroup*)malloc(sizeof(DictGroup));
  dict_group->count = 0;
  dict_group->dict_chain = dict_chain;
  return dict_group;
}

void dict_group_delete(DictGroup* dict_group) {
  size_t i;
  for (i = 0; i < dict_group->count; i++) {
    dict_delete(dict_group->dicts[i]);
  }
  free(dict_group);
}

static char* try_find_dictionary_with_config(
  DictGroup* dict_group,
  const char* filename) {
  if (is_absolute_path(filename)) {
    return NULL;
  }
  /* Get config path */
  if (dict_group->dict_chain == NULL) {
    return NULL;
  }
  Config* config = dict_group->dict_chain->config;
  if (config == NULL) {
    return NULL;
  }
  const char* config_path = config->file_path;
  if (config_path == NULL) {
    return NULL;
  }
  char* config_path_filename = (char*)malloc(strlen(config_path) + strlen(
                                               filename) + 2);
  sprintf(config_path_filename, "%s/%s", config_path, filename);
  FILE* fp = fopen(config_path_filename, "r");
  if (fp) {
    fclose(fp);
    return config_path_filename;
  }
  free(config_path_filename);
  return NULL;
}

int dict_group_load(DictGroup* dict_group,
                          const char* filename,
                          opencc_dictionary_type type) {
  Dict* dictionary;
  char* path = try_open_file(filename);
  if (path == NULL) {
    path = try_find_dictionary_with_config(dict_group, filename);
    if (path == NULL) {
      errnum = DICTIONARY_ERROR_CANNOT_ACCESS_DICTFILE;
      return -1;
    }
  }
  dictionary = dict_new(path, type);
  free(path);
  if (dictionary == (Dict*)-1) {
    errnum = DICTIONARY_ERROR_INVALID_DICT;
    return -1;
  }
  dict_group->dicts[dict_group->count++] = dictionary;
  return 0;
}

Dict* dict_group_get_dict(DictGroup* dict_group, size_t index) {
  if (index >= dict_group->count) {
    errnum = DICTIONARY_ERROR_INVALID_INDEX;
    return (Dict*)-1;
  }
  return dict_group->dicts[index];
}

const ucs4_t* const* dict_group_match_longest(
    DictGroup* dict_group,
    const ucs4_t* word,
    size_t maxlen,
    size_t* match_length) {
  if (dict_group->count == 0) {
    errnum = DICTIONARY_ERROR_NODICT;
    return (const ucs4_t* const*)-1;
  }
  const ucs4_t* const* retval = NULL;
  size_t t_match_length, max_length = 0;
  size_t i;
  for (i = 0; i < dict_group->count; i++) {
    /* 依次查找每個辭典，取得最長匹配長度 */
    const ucs4_t* const* t_retval = dict_match_longest(
      dict_group->dicts[i],
      word,
      maxlen,
      &t_match_length);
    if (t_retval != NULL) {
      if (t_match_length > max_length) {
        max_length = t_match_length;
        retval = t_retval;
      }
    }
  }
  if (match_length != NULL) {
    *match_length = max_length;
  }
  return retval;
}

size_t dict_group_get_all_match_lengths(DictGroup* dict_group,
                                              const ucs4_t* word,
                                              size_t* match_length) {
  if (dict_group->count == 0) {
    errnum = DICTIONARY_ERROR_NODICT;
    return (size_t)-1;
  }
  size_t rscnt = 0;
  size_t i;
  for (i = 0; i < dict_group->count; i++) {
    size_t retval;
    retval = dict_get_all_match_lengths(
      dict_group->dicts[i],
      word,
      match_length + rscnt
      );
    rscnt += retval;
    /* 去除重複長度 */
    if ((i > 0) && (rscnt > 1)) {
      qsort(match_length, rscnt, sizeof(match_length[0]), qsort_int_cmp);
      size_t j, k;
      for (j = 0, k = 1; k < rscnt; k++) {
        if (match_length[k] != match_length[j]) {
          match_length[++j] = match_length[k];
        }
      }
      rscnt = j + 1;
    }
  }
  return rscnt;
}

dictionary_error dictionary_errno(void) {
  return errnum;
}

void dictionary_perror(const char* spec) {
  perr(spec);
  perr("\n");
  switch (errnum) {
  case DICTIONARY_ERROR_VOID:
    break;
  case DICTIONARY_ERROR_NODICT:
    perr(_("No dictionary loaded"));
    break;
  case DICTIONARY_ERROR_CANNOT_ACCESS_DICTFILE:
    perror(_("Can not open dictionary file"));
    break;
  case DICTIONARY_ERROR_INVALID_DICT:
    perror(_("Invalid dictionary file"));
    break;
  case DICTIONARY_ERROR_INVALID_INDEX:
    perror(_("Invalid dictionary index"));
    break;
  default:
    perr(_("Unknown"));
  }
}
