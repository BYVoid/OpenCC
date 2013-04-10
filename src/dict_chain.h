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

#ifndef __DICTIONARY_SET_H_
#define __DICTIONARY_SET_H_

#include "common.h"

DictChain_t DictChain_open(config_t config);

void DictChain_close(DictChain_t t_dictionary);

DictGroup_t DictChain_new_group(DictChain_t t_dictionary);

DictGroup_t DictChain_get_group(DictChain_t t_dictionary,
                                            size_t index);

size_t DictChain_count_group(DictChain_t t_dictionary);

config_t DictChain_get_config(DictChain_t t_dictionary);

#endif /* __DICTIONARY_SET_H_ */
