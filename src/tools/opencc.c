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

#include "../opencc.h"
#include "../utils.h"
#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef VERSION
#define VERSION ""
#endif

#define BUFFER_SIZE 65536

void convert(const char* input_file,
             const char* output_file,
             const char* config_file) {
  opencc_t od = opencc_open(config_file);
  if (od == (opencc_t)-1) {
    opencc_perror(_("OpenCC initialization error"));
    exit(1);
  }
  FILE* fp = stdin;
  FILE* fpo = stdout;
  if (input_file) {
    fp = fopen(input_file, "r");
    if (!fp) {
      fprintf(stderr, _("Can not read file: %s\n"), input_file);
      exit(1);
    }
    skip_utf8_bom(fp);
  }
  if (output_file) {
    fpo = fopen(output_file, "w");
    if (!fpo) {
      fprintf(stderr, _("Can not write file: %s\n"), output_file);
      exit(1);
    }
  }
  size_t size = BUFFER_SIZE;
  char* buffer_in = NULL, * buffer_out = NULL;
  buffer_in = (char*)malloc(size * sizeof(char));
  char* lookahead = (char*)malloc(size * sizeof(char));
  size_t lookahead_size = 0;
  while (!feof(fp)) {
    size_t read;
    if (lookahead_size > 0) {
      memcpy(buffer_in, lookahead, lookahead_size);
      read =
        fread(buffer_in + lookahead_size, 1, size - lookahead_size,
              fp) + lookahead_size;
      lookahead_size = 0;
    } else {
      read = fread(buffer_in, 1, size, fp);
    }
    // If we haven't finished reading after filling the entire buffer,
    // then it could be that we broke within an UTF-8 character, in
    // that case we must backtrack and find the boundary
    if (read == size) {
      // Find the boundary of last UTF-8 character
      int i;
      for (i = read - 1; i >= 0; i--) {
        char c = buffer_in[i];
        if (!(c & 0x80) || ((c & 0xC0) == 0xC0)) {
          break;
        }
      }
      assert(i >= 0);
      memcpy(lookahead, buffer_in + i, read - i);
      lookahead_size = read - i;
      buffer_in[i] = '\0';
    } else {
      buffer_in[read] = '\0';
    }
    buffer_out = opencc_convert_utf8(od, buffer_in, (size_t)-1);
    if (buffer_out != (char*)-1) {
      fprintf(fpo, "%s", buffer_out);
      opencc_convert_utf8_free(buffer_out);
    } else {
      opencc_perror(_("OpenCC error"));
      break;
    }
  }

  if (lookahead_size > 0) {
    assert(lookahead_size < size);
    lookahead[lookahead_size] = '\0';
    buffer_out = opencc_convert_utf8(od, lookahead, (size_t)-1);
    if (buffer_out != (char*)-1) {
      fprintf(fpo, "%s", buffer_out);
      opencc_convert_utf8_free(buffer_out);
    } else {
      opencc_perror(_("OpenCC error"));
    }
  }
  opencc_close(od);
  free(lookahead);
  free(buffer_in);
  fclose(fp);
  fclose(fpo);
}

void show_version() {
  printf(_("\n"));
  printf(_("Open Chinese Convert (OpenCC) Command Line Tool\n"));
  printf(_("Version %s\n"), VERSION);
  printf(_("\n"));
  printf(_("Author: %s\n"), "BYVoid <byvoid@byvoid.com>");
  printf(_("Bug Report: %s\n"), "http://github.com/BYVoid/OpenCC/issues");
  printf(_("\n"));
}

void show_usage() {
  show_version();
  printf(_("Usage:\n"));
  printf(_(" opencc [Options]\n"));
  printf(_("\n"));
  printf(_("Options:\n"));
  printf(_(" -i [file], --input=[file]   Read original text from [file].\n"));
  printf(_(" -o [file], --output=[file]  Write converted text to [file].\n"));
  printf(_(
           " -c [file], --config=[file]  Load configuration of conversion from [file].\n"));
  printf(_(" -v, --version               Print version and build information.\n"));
  printf(_(" -h, --help                  Print this help.\n"));
  printf(_("\n"));
  printf(_(
           "With no input file, reads standard input and writes converted stream to standard output.\n"));
  printf(_(
           "Default configuration(%s) will be loaded if not set.\n"),
         OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
  printf(_("\n"));
}

int main(int argc, char** argv) {
#ifdef ENABLE_GETTEXT
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE_NAME, LOCALEDIR);
#endif /* ifdef ENABLE_GETTEXT */
  static struct option longopts[] =
  {
    { "version", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { "input", required_argument, NULL, 'i' },
    { "output", required_argument, NULL, 'o' },
    { "config", required_argument, NULL, 'c' },
    { 0, 0, 0, 0 },
  };
  static int oc;
  static char* input_file, * output_file, * config_file;
  while ((oc = getopt_long(argc, argv, "vh?i:o:c:", longopts, NULL)) != -1) {
    switch (oc) {
    case 'v':
      show_version();
      return 0;
    case 'h':
    case '?':
      show_usage();
      return 0;
    case 'i':
      input_file = mstrcpy(optarg);
      break;
    case 'o':
      output_file = mstrcpy(optarg);
      break;
    case 'c':
      config_file = mstrcpy(optarg);
      break;
    }
  }
  if (config_file == NULL) {
    config_file = mstrcpy(OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD);
  }
  convert(input_file, output_file, config_file);
  free(input_file);
  free(output_file);
  free(config_file);
  return 0;
}
