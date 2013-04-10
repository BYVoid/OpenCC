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

#include "dictionary_group.h"
#include "dict_chain.h"

#define DICTIONARY_GROUP_MAX_COUNT 128

struct _DictChain {
  config_t config;
  size_t count;
  dictionary_group_t groups[DICTIONARY_GROUP_MAX_COUNT];
};
typedef struct _DictChain DictChain_desc;

DictChain_t DictChain_open(config_t config) {
  DictChain_desc* DictChain =
    (DictChain_desc*)malloc(sizeof(DictChain_desc));
  DictChain->count = 0;
  DictChain->config = config;
  return DictChain;
}

void DictChain_close(DictChain_t t_dictionary) {
  DictChain_desc* DictChain = (DictChain_desc*)t_dictionary;
  size_t i;
  for (i = 0; i < DictChain->count; i++) {
    dictionary_group_close(DictChain->groups[i]);
  }
  free(DictChain);
}

dictionary_group_t DictChain_new_group(DictChain_t t_dictionary) {
  DictChain_desc* DictChain = (DictChain_desc*)t_dictionary;
  if (DictChain->count + 1 == DICTIONARY_GROUP_MAX_COUNT) {
    return (dictionary_group_t)-1;
  }
  dictionary_group_t group = dictionary_group_open(t_dictionary);
  DictChain->groups[DictChain->count++] = group;
  return group;
}

dictionary_group_t DictChain_get_group(DictChain_t t_dictionary,
                                            size_t index) {
  DictChain_desc* DictChain = (DictChain_desc*)t_dictionary;
  if (index >= DictChain->count) {
    return (dictionary_group_t)-1;
  }
  return DictChain->groups[index];
}

size_t DictChain_count_group(DictChain_t t_dictionary) {
  DictChain_desc* DictChain = (DictChain_desc*)t_dictionary;
  return DictChain->count;
}

config_t DictChain_get_config(DictChain_t t_dictionary) {
  DictChain_desc* DictChain = (DictChain_desc*)t_dictionary;
  return DictChain->config;
}
