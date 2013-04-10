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

#ifndef __DICTIONARY_GROUP_H_
#define __DICTIONARY_GROUP_H_

#include "common.h"
#include "dict.h"

typedef enum {
  DICTIONARY_ERROR_VOID,
  DICTIONARY_ERROR_NODICT,
  DICTIONARY_ERROR_CANNOT_ACCESS_DICTFILE,
  DICTIONARY_ERROR_INVALID_DICT,
  DICTIONARY_ERROR_INVALID_INDEX,
} dictionary_error;

DictGroup* dict_group_new(DictChain* t_DictChain);

void dict_group_delete(DictGroup* dict_group);

int dict_group_load(DictGroup* dict_group,
                          const char* filename,
                          opencc_dictionary_type type);

const ucs4_t* const* dict_group_match_longest(
    DictGroup* dict_group,
    const ucs4_t* word,
    size_t maxlen,
    size_t* match_length);

size_t dict_group_get_all_match_lengths(DictGroup* dict_group,
                                              const ucs4_t* word,
                                              size_t* match_length);

Dict* dict_group_get_dict(DictGroup* dict_group, size_t index);

dictionary_error dictionary_errno(void);

void dictionary_perror(const char* spec);

#endif /* __DICTIONARY_GROUP_H_ */
