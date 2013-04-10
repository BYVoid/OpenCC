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

#include "dict_group.h"
#include "dict_chain.h"

DictChain* DictChain_open(config_t config) {
  DictChain* dict_chain = (DictChain*)malloc(sizeof(DictChain));
  dict_chain->count = 0;
  dict_chain->config = config;
  return dict_chain;
}

void DictChain_close(DictChain* dict_chain) {
  size_t i;
  for (i = 0; i < dict_chain->count; i++) {
    DictGroup_close(dict_chain->groups[i]);
  }
  free(dict_chain);
}

DictGroup_t DictChain_new_group(DictChain* dict_chain) {
  if (dict_chain->count + 1 == DICTIONARY_GROUP_MAX_COUNT) {
    return (DictGroup_t)-1;
  }
  DictGroup_t group = DictGroup_open(dict_chain);
  dict_chain->groups[dict_chain->count++] = group;
  return group;
}

DictGroup_t DictChain_get_group(DictChain* dict_chain, size_t index) {
  if (index >= dict_chain->count) {
    return (DictGroup_t)-1;
  }
  return dict_chain->groups[index];
}
