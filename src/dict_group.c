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

#include "config_reader.h"
#include "dict_group.h"
#include "dict_chain.h"

#define DICTIONARY_MAX_COUNT 128

struct _DictGroup {
  DictChain_t DictChain;
  size_t count;
  dictionary_t dicts[DICTIONARY_MAX_COUNT];
};
typedef struct _DictGroup DictGroup_desc;

static dictionary_error errnum = DICTIONARY_ERROR_VOID;

DictGroup_t DictGroup_open(DictChain_t t_DictChain) {
  DictGroup_desc* DictGroup =
    (DictGroup_desc*)malloc(sizeof(DictGroup_desc));
  DictGroup->count = 0;
  DictGroup->DictChain = t_DictChain;
  return DictGroup;
}

void DictGroup_close(DictGroup_t t_dictionary) {
  DictGroup_desc* DictGroup = (DictGroup_desc*)t_dictionary;
  size_t i;
  for (i = 0; i < DictGroup->count; i++) {
    dictionary_close(DictGroup->dicts[i]);
  }
  free(DictGroup);
}

static char* try_find_dictionary_with_config(
  DictGroup_desc* DictGroup,
  const char* filename) {
  if (is_absolute_path(filename)) {
    return NULL;
  }
  /* Get config path */
  if (DictGroup->DictChain == NULL) {
    return NULL;
  }
  config_t config = DictChain_get_config(DictGroup->DictChain);
  if (config == NULL) {
    return NULL;
  }
  const char* config_path = config_get_file_path(config);
  if (config_path == NULL) {
    return NULL;
  }
  char* config_path_filename = (char*)malloc(strlen(config_path) + strlen(
                                               filename) + 3);
  sprintf(config_path_filename, "%s/%s%c", config_path, filename, '\0');
  FILE* fp = fopen(config_path_filename, "r");
  if (fp) {
    fclose(fp);
    return config_path_filename;
  }
  return NULL;
}

int DictGroup_load(DictGroup_t t_dictionary,
                          const char* filename,
                          opencc_dictionary_type type) {
  DictGroup_desc* DictGroup = (DictGroup_desc*)t_dictionary;
  dictionary_t dictionary;
  char* path = try_open_file(filename);
  if (path == NULL) {
    path = try_find_dictionary_with_config(DictGroup, filename);
    if (path == NULL) {
      errnum = DICTIONARY_ERROR_CANNOT_ACCESS_DICTFILE;
      return -1;
    }
  }
  dictionary = dictionary_open(path, type);
  free(path);
  if (dictionary == (dictionary_t)-1) {
    errnum = DICTIONARY_ERROR_INVALID_DICT;
    return -1;
  }
  DictGroup->dicts[DictGroup->count++] = dictionary;
  return 0;
}

dictionary_t DictGroup_get_dictionary(DictGroup_t t_dictionary,
                                             size_t index) {
  DictGroup_desc* DictGroup = (DictGroup_desc*)t_dictionary;
  if (index >= DictGroup->count) {
    errnum = DICTIONARY_ERROR_INVALID_INDEX;
    return (dictionary_t)-1;
  }
  return DictGroup->dicts[index];
}

size_t DictGroup_count(DictGroup_t t_dictionary) {
  DictGroup_desc* DictGroup = (DictGroup_desc*)t_dictionary;
  return DictGroup->count;
}

const ucs4_t* const* DictGroup_match_longest(
  DictGroup_t t_dictionary,
  const ucs4_t* word,
  size_t maxlen,
  size_t* match_length) {
  DictGroup_desc* DictGroup = (DictGroup_desc*)t_dictionary;
  if (DictGroup->count == 0) {
    errnum = DICTIONARY_ERROR_NODICT;
    return (const ucs4_t* const*)-1;
  }
  const ucs4_t* const* retval = NULL;
  size_t t_match_length, max_length = 0;
  size_t i;
  for (i = 0; i < DictGroup->count; i++) {
    /* 依次查找每個辭典，取得最長匹配長度 */
    const ucs4_t* const* t_retval = dictionary_match_longest(
      DictGroup->dicts[i],
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

size_t DictGroup_get_all_match_lengths(DictGroup_t t_dictionary,
                                              const ucs4_t* word,
                                              size_t* match_length) {
  DictGroup_desc* DictGroup = (DictGroup_desc*)t_dictionary;
  if (DictGroup->count == 0) {
    errnum = DICTIONARY_ERROR_NODICT;
    return (size_t)-1;
  }
  size_t rscnt = 0;
  size_t i;
  for (i = 0; i < DictGroup->count; i++) {
    size_t retval;
    retval = dictionary_get_all_match_lengths(
      DictGroup->dicts[i],
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

DictChain_t DictGroup_get_DictChain(
  DictGroup_t t_dictionary) {
  DictGroup_desc* DictGroup = (DictGroup_desc*)t_dictionary;
  return DictGroup->DictChain;
}
