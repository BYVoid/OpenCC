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

#include "dict.h"
#include "dictionary/datrie.h"
#include "dictionary/text.h"

Dict* dict_new(const char* filename, opencc_dictionary_type type) {
  Dict* dictionary = (Dict*)malloc(sizeof(Dict));
  dictionary->type = type;
  switch (type) {
  case OPENCC_DICTIONARY_TYPE_TEXT:
    dictionary->dict = dict_text_new(filename);
    break;
  case OPENCC_DICTIONARY_TYPE_DATRIE:
    dictionary->dict = dict_datrie_new(filename);
    break;
  default:
    free(dictionary);
    dictionary = (Dict*)-1;              /* TODO:辭典格式不支持 */
  }
  return dictionary;
}

void dict_delete(Dict* dict) {
  switch (dict->type) {
  case OPENCC_DICTIONARY_TYPE_TEXT:
    dict_text_delete(dict->dict);
    break;
  case OPENCC_DICTIONARY_TYPE_DATRIE:
    dict_datrie_delete(dict->dict);
    break;
  default:
    debug_should_not_be_here();
  }
  free(dict);
}

const ucs4_t* const* dict_match_longest(Dict* dict,
                                              const ucs4_t* word,
                                              size_t maxlen,
                                              size_t* match_length) {
  switch (dict->type) {
  case OPENCC_DICTIONARY_TYPE_TEXT:
    return dict_text_match_longest(dict->dict,
                                         word,
                                         maxlen,
                                         match_length);
    break;
  case OPENCC_DICTIONARY_TYPE_DATRIE:
    return dict_datrie_match_longest(dict->dict,
                                           word,
                                           maxlen,
                                           match_length);
    break;
  default:
    debug_should_not_be_here();
  }
  return (const ucs4_t* const*)-1;
}

size_t dict_get_all_match_lengths(Dict* dict,
                                        const ucs4_t* word,
                                        size_t* match_length) {
  switch (dict->type) {
  case OPENCC_DICTIONARY_TYPE_TEXT:
    return dict_text_get_all_match_lengths(dict->dict,
                                                 word,
                                                 match_length);
    break;
  case OPENCC_DICTIONARY_TYPE_DATRIE:
    return dict_datrie_get_all_match_lengths(dict->dict,
                                                   word,
                                                   match_length);
    break;
  default:
    debug_should_not_be_here();
  }
  return (size_t)-1;
}
