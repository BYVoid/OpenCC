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

#ifndef __OPENCC_DICTIONARY_ABSTRACT_H_
#define __OPENCC_DICTIONARY_ABSTRACT_H_

#include "common.h"
#include "utils.h"

Dict* dict_new(const char* filename, opencc_dictionary_type type);

void dict_delete(Dict* dict);

const ucs4_t* const* dict_match_longest(Dict* dict,
                                        const ucs4_t* word,
                                        size_t maxlen,
                                        size_t* match_length);

size_t dict_get_all_match_lengths(Dict* dict,
                                  const ucs4_t* word,
                                  size_t* match_length);

#endif /* __OPENCC_DICTIONARY_ABSTRACT_H_ */
