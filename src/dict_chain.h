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

#ifndef __DICTIONARY_SET_H_
#define __DICTIONARY_SET_H_

#include "common.h"

#define DICTIONARY_GROUP_MAX_COUNT 128
typedef struct {
  config_t config;
  size_t count;
  DictGroup_t groups[DICTIONARY_GROUP_MAX_COUNT];
} DictChain;

DictChain* DictChain_open(config_t config);

void DictChain_close(DictChain* dict_chain);

DictGroup_t DictChain_new_group(DictChain* dict_chain);

DictGroup_t DictChain_get_group(DictChain* dict_chain, size_t index);

#endif /* __DICTIONARY_SET_H_ */
