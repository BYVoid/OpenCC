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

#include "../encoding.h"
#include "text.h"

#define INITIAL_DICTIONARY_SIZE 1024
#define ENTRY_BUFF_SIZE 128
#define ENTRY_WBUFF_SIZE ENTRY_BUFF_SIZE / sizeof(size_t)

int qsort_entry_cmp(const void* a, const void* b) {
  return ucs4cmp(((TextEntry*)a)->key, ((TextEntry*)b)->key);
}

int parse_entry(const char* buff, TextEntry* entry_i) {
  size_t length;
  const char* pbuff;

  /* 解析鍵 */
  for (pbuff = buff; *pbuff != '\t' && *pbuff != '\0'; ++pbuff) {}

  if (*pbuff == '\0') {
    return -1;
  }
  length = pbuff - buff;

  ucs4_t* ucs4_buff;
  ucs4_buff = utf8_to_ucs4(buff, length);

  if (ucs4_buff == (ucs4_t*)-1) {
    return -1;
  }
  entry_i->key = (ucs4_t*)malloc((length + 1) * sizeof(ucs4_t));
  ucs4cpy(entry_i->key, ucs4_buff);
  free(ucs4_buff);

  /* 解析值 */
  size_t value_i, value_count = INITIAL_DICTIONARY_SIZE;
  entry_i->value = (ucs4_t**)malloc(value_count * sizeof(ucs4_t*));

  for (value_i = 0; *pbuff != '\0' && *pbuff != '\n'; ++value_i) {
    if (value_i >= value_count) {
      value_count += value_count;
      entry_i->value = (ucs4_t**)realloc(
        entry_i->value,
        value_count * sizeof(ucs4_t*)
        );
    }

    for (buff = ++pbuff;
         *pbuff != ' ' && *pbuff != '\0' && *pbuff != '\n' && *pbuff != '\r';
         ++pbuff) {}
    length = pbuff - buff;
    ucs4_buff = utf8_to_ucs4(buff, length);

    if (ucs4_buff == (ucs4_t*)-1) {
      /* 發生錯誤 回退內存申請 */
      ssize_t i;

      for (i = value_i - 1; i >= 0; --i) {
        free(entry_i->value[i]);
      }
      free(entry_i->value);
      free(entry_i->key);
      return -1;
    }

    entry_i->value[value_i] = (ucs4_t*)malloc((length + 1) * sizeof(ucs4_t));
    ucs4cpy(entry_i->value[value_i], ucs4_buff);
    free(ucs4_buff);
  }

  entry_i->value = (ucs4_t**)realloc(
    entry_i->value,
    value_count * sizeof(ucs4_t*)
    );
  entry_i->value[value_i] = NULL;

  return 0;
}

Dict* dict_text_new(const char* filename) {
  TextDict* text_dictionary;

  text_dictionary = (TextDict*)malloc(sizeof(TextDict));
  text_dictionary->entry_count = INITIAL_DICTIONARY_SIZE;
  text_dictionary->max_length = 0;
  text_dictionary->lexicon = (TextEntry*)malloc(
    sizeof(TextEntry) * text_dictionary->entry_count);
  text_dictionary->word_buff = NULL;

  static char buff[ENTRY_BUFF_SIZE];

  FILE* fp = fopen(filename, "r");

  if (fp == NULL) {
    dict_text_delete((Dict*)text_dictionary);
    return (Dict*)-1;
  }
  skip_utf8_bom(fp);

  size_t i = 0;

  while (fgets(buff, ENTRY_BUFF_SIZE, fp)) {
    if (i >= text_dictionary->entry_count) {
      text_dictionary->entry_count += text_dictionary->entry_count;
      text_dictionary->lexicon = (TextEntry*)realloc(
        text_dictionary->lexicon,
        sizeof(TextEntry) * text_dictionary->entry_count
        );
    }

    if (parse_entry(buff, text_dictionary->lexicon + i) == -1) {
      text_dictionary->entry_count = i;
      dict_text_delete((Dict*)text_dictionary);
      return (Dict*)-1;
    }

    size_t length = ucs4len(text_dictionary->lexicon[i].key);

    if (length > text_dictionary->max_length) {
      text_dictionary->max_length = length;
    }

    i++;
  }

  fclose(fp);

  text_dictionary->entry_count = i;
  text_dictionary->lexicon = (TextEntry*)realloc(
    text_dictionary->lexicon,
    sizeof(TextEntry) * text_dictionary->entry_count
    );
  text_dictionary->word_buff = (ucs4_t*)
                               malloc(sizeof(ucs4_t) *
                                      (text_dictionary->max_length + 1));

  qsort(text_dictionary->lexicon,
        text_dictionary->entry_count,
        sizeof(text_dictionary->lexicon[0]),
        qsort_entry_cmp
        );

  return (Dict*)text_dictionary;
}

void dict_text_delete(Dict* dict) {
  TextDict* text_dictionary = (TextDict*)dict;

  size_t i;

  for (i = 0; i < text_dictionary->entry_count; ++i) {
    free(text_dictionary->lexicon[i].key);

    ucs4_t** j;

    for (j = text_dictionary->lexicon[i].value; *j; ++j) {
      free(*j);
    }
    free(text_dictionary->lexicon[i].value);
  }

  free(text_dictionary->lexicon);
  free(text_dictionary->word_buff);
  free(text_dictionary);
}

const ucs4_t* const* dict_text_match_longest(Dict* dict,
                                             const ucs4_t* word,
                                             size_t maxlen,
                                             size_t* match_length) {
  TextDict* text_dictionary = (TextDict*)dict;

  if (text_dictionary->entry_count == 0) {
    return NULL;
  }

  if (maxlen == 0) {
    maxlen = ucs4len(word);
  }
  size_t len = text_dictionary->max_length;

  if (maxlen < len) {
    len = maxlen;
  }

  ucs4ncpy(text_dictionary->word_buff, word, len);
  text_dictionary->word_buff[len] = L'\0';

  TextEntry buff;
  buff.key = text_dictionary->word_buff;

  for (; len > 0; len--) {
    text_dictionary->word_buff[len] = L'\0';
    TextEntry* brs = (TextEntry*)bsearch(
      &buff,
      text_dictionary->lexicon,
      text_dictionary->entry_count,
      sizeof(text_dictionary->lexicon[0]),
      qsort_entry_cmp
      );

    if (brs != NULL) {
      if (match_length != NULL) {
        *match_length = len;
      }
      return (const ucs4_t* const*)brs->value;
    }
  }

  if (match_length != NULL) {
    *match_length = 0;
  }
  return NULL;
}

size_t dict_text_get_all_match_lengths(Dict* dict,
                                       const ucs4_t* word,
                                       size_t* match_length) {
  TextDict* text_dictionary = (TextDict*)dict;

  size_t rscnt = 0;

  if (text_dictionary->entry_count == 0) {
    return rscnt;
  }

  size_t length = ucs4len(word);
  size_t len = text_dictionary->max_length;

  if (length < len) {
    len = length;
  }

  ucs4ncpy(text_dictionary->word_buff, word, len);
  text_dictionary->word_buff[len] = L'\0';

  TextEntry buff;
  buff.key = text_dictionary->word_buff;

  for (; len > 0; len--) {
    text_dictionary->word_buff[len] = L'\0';
    TextEntry* brs = (TextEntry*)bsearch(
      &buff,
      text_dictionary->lexicon,
      text_dictionary->entry_count,
      sizeof(text_dictionary->lexicon[0]),
      qsort_entry_cmp
      );

    if (brs != NULL) {
      match_length[rscnt++] = len;
    }
  }

  return rscnt;
}

size_t dict_text_get_lexicon(Dict* dict, TextEntry* lexicon) {
  TextDict* text_dictionary = (TextDict*)dict;

  size_t i;

  for (i = 0; i < text_dictionary->entry_count; i++) {
    lexicon[i].key = text_dictionary->lexicon[i].key;
    lexicon[i].value = text_dictionary->lexicon[i].value;
  }

  return text_dictionary->entry_count;
}
