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

#include "../dictionary/datrie.h"
#include "../dictionary/text.h"
#include "../dict_group.h"
#include "../encoding.h"
#include "../utils.h"
#include <locale.h>
#include <unistd.h>

#ifndef VERSION
#define VERSION ""
#endif

#define DATRIE_SIZE 1000000
#define DATRIE_WORD_MAX_COUNT 500000
#define DATRIE_WORD_MAX_LENGTH 32
#define BUFFER_SIZE 1024

typedef struct {
  uint32_t cursor;
  ucs4_t* pointer;
} Value;

typedef struct {
  ucs4_t* key;
  Value* value;
  size_t length;
  size_t value_count;
} Entry;

Entry lexicon[DATRIE_WORD_MAX_COUNT];
uint32_t lexicon_count, words_set_count;
int words_set[DATRIE_WORD_MAX_COUNT];
ucs4_t words_set_char[DATRIE_WORD_MAX_COUNT];
DatrieItem dat[DATRIE_SIZE];
uint32_t lexicon_index_length, lexicon_cursor_end;

void match_word(const DatrieItem* dat,
                const ucs4_t* word,
                int* match_pos,
                int* id,
                int limit) {
  int i, j, p;
  for (i = 0, p = 0;
       word[p] && (limit == 0 || p < limit) && dat[i].base != DATRIE_UNUSED;
       p++) {
    int k = encode_char(word[p]);
    j = dat[i].base + k;
    if ((j < 0) || (j > DATRIE_SIZE) || (dat[j].parent != i)) {
      break;
    }
    i = j;
  }
  if (match_pos) {
    *match_pos = p;
  }
  if (id) {
    *id = i;
  }
}

int unused(int i) {
  if ((i >= 0) && (i < DATRIE_SIZE)) {
    return dat[i].parent == DATRIE_UNUSED;
  }
  return 0;
}

int is_prefix(const ucs4_t* a, const ucs4_t* b) {
  const ucs4_t* p = a, * q = b;
  while (*p != 0) {
    if (*q == 0) {
      return 0;
    }
    if (*p != *q) {
      return 0;
    }
    p++;
    q++;
  }
  return 1;
}

int binary_search(const ucs4_t* str) {
  int a = 0, b = lexicon_count - 1, c;
  while (a + 1 < b) {
    c = (a + b) / 2;

    if (ucs4cmp(str, lexicon[c].key) <= 0) {
      b = c;
    } else {
      a = c + 1;
    }
  }
  if (is_prefix(str,
                lexicon[a].key) &&
      ((a == 0) || !is_prefix(str, lexicon[a - 1].key))) {
    return a;
  }
  if (is_prefix(str, lexicon[b].key) && !is_prefix(str, lexicon[b - 1].key)) {
    return b;
  }
  return -1;
}

int wcmp(const void* a, const void* b) {
  return *(const ucs4_t*)a < *(const ucs4_t*)b ? -1 : 1;
}

void get_words_with_prefix(ucs4_t* word, int p) {
  int i;
  static ucs4_t buff[DATRIE_WORD_MAX_LENGTH];
  static ucs4_t words_set_char_buff[DATRIE_WORD_MAX_COUNT];

  for (i = 0; i < p; i++) {
    buff[i] = word[i];
  }
  buff[p] = 0;
  words_set_count = 0;
  for (i = binary_search(buff);
       (uint32_t)i < lexicon_count && is_prefix(buff, lexicon[i].key); i++) {
    if (ucs4cmp(buff, lexicon[i].key) == 0) {
      continue;
    }
    words_set_char_buff[words_set_count] = lexicon[i].key[p];
    words_set[words_set_count++] = i;
  }
  words_set_char_buff[words_set_count] = 0;
  qsort(words_set_char_buff, words_set_count, sizeof(words_set_char_buff[0]),
        wcmp);
  ucs4_t* wfp, * wp, last;
  for (last = 0, wfp = words_set_char_buff, wp = words_set_char; *wfp; wfp++) {
    if (*wfp != last) {
      last = *wfp;
      *wp = *wfp;
      wp++;
    }
  }
  *wp = 0;
}

int words_space_available(int delta) {
  ucs4_t* wp;
  for (wp = words_set_char; *wp; wp++) {
    if (!unused(encode_char(*wp) + delta)) {
      return 0;
    }
  }
  return 1;
}

void insert_first_char(int id) {
  Entry* word = lexicon + id;
  int key = encode_char(word->key[0]);
  dat[key].base = DATRIE_UNUSED;
  dat[key].parent = 0;
  if (word->length == 1) {
    dat[key].word = (id);
  }
}

void insert_words(int delta, int parent, size_t word_len) {
  int i;
  for (i = 0; (uint32_t)i < words_set_count; i++) {
    int j = words_set[i];
    int k = encode_char(lexicon[j].key[word_len]) + delta;
    dat[k].parent = parent;
    if (lexicon[j].length == word_len + 1) {
      dat[k].word = (j);
    }
  }
}

void insert(int id) {
  static int space_min = 0;
  Entry* word = &lexicon[id];
  for (;;) {
    int p, i;
    match_word(dat, word->key, &p, &i, 0);
    if ((size_t)p == word->length) {
      return;
    }
    get_words_with_prefix(word->key, p);
    int delta;
    delta = space_min - words_set_char[0];
    for (; delta < DATRIE_SIZE; delta++) {
      if (words_space_available(delta)) {
        break;
      }
    }
    if (delta == DATRIE_SIZE) {
      fprintf(stderr, "DATRIE_SIZE Not Enough!\n");
      exit(1);
    }
    insert_words(delta, i, p);
    dat[i].base = delta;
    while (!unused(space_min)) {
      space_min++;
    }
  }
}

void make(void) {
  size_t i;
  for (i = 1; i < DATRIE_SIZE; i++) {
    dat[i].parent = dat[i].base = DATRIE_UNUSED;
    dat[i].word = -1;
  }
  dat[0].parent = dat[0].base = 0;
  for (i = 0; i < lexicon_count; i++) {
    insert_first_char(i);
  }
  for (i = 0; i < lexicon_count; i++) {
    insert(i);
  }
}

int cmp(const void* a, const void* b) {
  return ucs4cmp(((const TextEntry*)a)->key, ((const TextEntry*)b)->key);
}

void init(const char* filename) {
  DictGroup* DictGroup = dict_group_new(NULL);
  if (dict_group_load(DictGroup, filename,
                            OPENCC_DICTIONARY_TYPE_TEXT) == -1) {
    dictionary_perror("Dictionary loading error");
    fprintf(stderr, _("\n"));
    exit(1);
  }
  Dict* dict_abs = dict_group_get_dict(DictGroup, 0);
  if (dict_abs == (Dict*)-1) {
    dictionary_perror("Dictionary loading error");
    fprintf(stderr, _("\n"));
    exit(1);
  }
  static TextEntry tlexicon[DATRIE_WORD_MAX_COUNT];
  /* TODO add datrie support */
  Dict* dictionary = dict_abs->dict;
  lexicon_count = dict_text_get_lexicon(dictionary, tlexicon);
  qsort(tlexicon, lexicon_count, sizeof(tlexicon[0]), cmp);
  size_t i;
  size_t lexicon_cursor = 0;
  for (i = 0; i < lexicon_count; i++) {
    lexicon[i].key = tlexicon[i].key;
    lexicon[i].length = ucs4len(lexicon[i].key);
    size_t j;
    for (j = 0; tlexicon[i].value[j] != NULL; j++) {}
    lexicon[i].value_count = j;
    lexicon_index_length += lexicon[i].value_count + 1;
    lexicon[i].value = (Value*)malloc(lexicon[i].value_count * sizeof(Value));
    for (j = 0; j < lexicon[i].value_count; j++) {
      lexicon[i].value[j].cursor = lexicon_cursor;
      lexicon[i].value[j].pointer = tlexicon[i].value[j];
      lexicon_cursor += ucs4len(tlexicon[i].value[j]) + 1;
    }
  }
  lexicon_cursor_end = lexicon_cursor;
}

void output(const char* file_name) {
  FILE* fp = fopen(file_name, "wb");
  if (!fp) {
    fprintf(stderr, _("Can not write file: %s\n"), file_name);
    exit(1);
  }
  uint32_t i, item_count;
  for (i = DATRIE_SIZE - 1; i > 0; i--) {
    if (dat[i].parent != DATRIE_UNUSED) {
      break;
    }
  }
  item_count = i + 1;
  fwrite("OPENCCDATRIE", sizeof(char), strlen("OPENCCDATRIE"), fp);
  /* 詞彙表長度 */
  fwrite(&lexicon_cursor_end, sizeof(uint32_t), 1, fp);
  for (i = 0; i < lexicon_count; i++) {
    size_t j;
    for (j = 0; j < lexicon[i].value_count; j++) {
      fwrite(lexicon[i].value[j].pointer, sizeof(ucs4_t),
             ucs4len(lexicon[i].value[j].pointer) + 1, fp);
    }
  }
  /* 詞彙索引表長度 */
  fwrite(&lexicon_index_length, sizeof(uint32_t), 1, fp);
  for (i = 0; i < lexicon_count; i++) {
    size_t j;
    for (j = 0; j < lexicon[i].value_count; j++) {
      fwrite(&lexicon[i].value[j].cursor, sizeof(uint32_t), 1, fp);
    }
    uint32_t dem = (uint32_t)-1;
    fwrite(&dem, sizeof(uint32_t), 1, fp);             /* 分隔符 */
  }
  fwrite(&lexicon_count, sizeof(uint32_t), 1, fp);
  fwrite(&item_count, sizeof(uint32_t), 1, fp);
  fwrite(dat, sizeof(dat[0]), item_count, fp);
  fclose(fp);
}

#ifdef DEBUG_WRITE_TEXT
void write_text_file() {
  FILE* fp;
  int i;
  fp = fopen("datrie.txt", "w");
  fprintf(fp, "%d\n", lexicon_count);
  for (i = 0; i < lexicon_count; i++) {
    char* buff = ucs4_to_utf8(lexicon[i].value, (size_t)-1);
    fprintf(fp, "%s\n", buff);
    free(buff);
  }
  for (i = 0; i < DATRIE_SIZE; i++) {
    if (dat[i].parent != DATRIE_UNUSED) {
      fprintf(fp, "%d %d %d %d\n", i, dat[i].base, dat[i].parent, dat[i].word);
    }
  }
  fclose(fp);
}

#endif /* ifdef DEBUG_WRITE_TEXT */

void show_version() {
  printf(_("\nOpen Chinese Convert (OpenCC) Dictionary Tool\nVersion %s\n\n"),
         VERSION);
}

void show_usage() {
  show_version();
  printf(_("Usage:\n"));
  printf(_("  opencc_dict -i input_file -o output_file\n\n"));
  printf(_("    -i input_file\n"));
  printf(_("      Read data from input_file.\n"));
  printf(_("    -o output_file\n"));
  printf(_("      Write converted data to output_file.\n"));
  printf(_("\n"));
  printf(_("\n"));
}

int main(int argc, char** argv) {
  static int oc;
  static char input_file[BUFFER_SIZE], output_file[BUFFER_SIZE];
  int input_file_specified = 0, output_file_specified = 0;

#ifdef ENABLE_GETTEXT
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE_NAME, LOCALEDIR);
#endif /* ifdef ENABLE_GETTEXT */
  while ((oc = getopt(argc, argv, "vh-:i:o:")) != -1) {
    switch (oc) {
    case 'v':
      show_version();
      return 0;
    case 'h':
    case '?':
      show_usage();
      return 0;
    case '-':
      if (strcmp(optarg, "version") == 0) {
        show_version();
      } else {
        show_usage();
      }
      return 0;
    case 'i':
      strcpy(input_file, optarg);
      input_file_specified = 1;
      break;
    case 'o':
      strcpy(output_file, optarg);
      output_file_specified = 1;
      break;
    }
  }
  if (!input_file_specified) {
    fprintf(stderr, _("Please specify input file using -i.\n"));
    show_usage();
    return 1;
  }
  if (!output_file_specified) {
    fprintf(stderr, _("Please specify output file using -o.\n"));
    show_usage();
    return 1;
  }
  init(input_file);
  make();
  output(output_file);
#ifdef DEBUG_WRITE_TEXT
  write_text_file();
#endif /* ifdef DEBUG_WRITE_TEXT */
  return 0;
}
