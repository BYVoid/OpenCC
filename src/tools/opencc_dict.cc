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
#include "../DictStorage.h"
#include <locale.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>

using std::vector;
using std::string;

#ifndef VERSION
#define VERSION ""
#endif

#define DATRIE_SIZE 1000000
#define DATRIE_WORD_MAX_COUNT 500000
#define BUFFER_SIZE 1024

const char* OCDHEADER = "OPENCCDICTIONARY1";

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
  /* TODO add datrie support */
  Dict* dictionary = dict_abs->dict;
  Opencc::DictStorage dictStorage;
  dictStorage.serialize(dictionary, filename);
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
#ifdef DEBUG_WRITE_TEXT
  write_text_file();
#endif /* ifdef DEBUG_WRITE_TEXT */
  return 0;
}
