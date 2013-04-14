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

#ifndef __OPENCC_DICTIONARY_TEXT_H_
#define __OPENCC_DICTIONARY_TEXT_H_

#include "../dict.h"

typedef struct {
  ucs4_t* key;
  ucs4_t** value;
} TextEntry;

typedef struct {
  size_t entry_count;
  size_t max_length;
  TextEntry* lexicon;
  ucs4_t* word_buff;
} TextDict;

Dict* dict_text_new(const char* filename);

void dict_text_delete(Dict* dict);

const ucs4_t* const* dict_text_match_longest(Dict* dict,
                                             const ucs4_t* word,
                                             size_t maxlen,
                                             size_t* match_length);

size_t dict_text_get_all_match_lengths(Dict* dict,
                                       const ucs4_t* word,
                                       size_t* match_length);

size_t dict_text_get_lexicon(Dict* dict, TextEntry* lexicon);

#endif /* __OPENCC_DICTIONARY_TEXT_H_ */
