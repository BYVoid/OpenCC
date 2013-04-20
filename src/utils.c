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

#include "utils.h"
#include <unistd.h>

#ifdef __APPLE__
  #include "TargetConditionals.h"
  #ifdef TARGET_OS_MAC
    #include <mach-o/dyld.h>
  #elif TARGET_OS_IPHONE
  #elif TARGET_IPHONE_SIMULATOR
  #else /* ifdef TARGET_OS_MAC */
  #endif /* ifdef TARGET_OS_MAC */
#elif defined _WIN32 || defined _WIN64
  #include "Windows.h"
#endif /* ifdef __APPLE__ */

#if defined _WIN32 || defined _WIN64
  #define PATH_SEPARATOR '\\'
#else
  #define PATH_SEPARATOR '/'
#endif

#define PATH_BUFFER_SIZE 4096

void perr(const char* str) {
  fputs(str, stderr);
}

int qsort_int_cmp(const void* a, const void* b) {
  return *((int*)a) - *((int*)b);
}

char* mstrcpy(const char* str) {
  char* strbuf = (char*)malloc(sizeof(char) * (strlen(str) + 1));

  strcpy(strbuf, str);
  return strbuf;
}

char* mstrncpy(const char* str, size_t n) {
  char* strbuf = (char*)malloc(sizeof(char) * (n + 1));

  strncpy(strbuf, str, n);
  strbuf[n] = '\0';
  return strbuf;
}

void skip_utf8_bom(FILE* fp) {
  int bom[3];
  int n;

  /* UTF-8 BOM is EF BB BF */
  if (fp == NULL) {
    return;
  }

  /* If we are not at beginning of file, return */
  if (ftell(fp) != 0) {
    return;
  }

  /* Try to read first 3 bytes */
  for (n = 0; n <= 2 && (bom[n] = getc(fp)) != EOF; n++) {}

  /* If we can only read <3 bytes, push them back */
  /* Or if first 3 bytes is not BOM, push them back */
  if ((n < 3) || (bom[0] != 0xEF) || (bom[1] != 0xBB) || (bom[2] != 0xBF)) {
    for (n--; n >= 0; n--) {
      ungetc(bom[n], fp);
    }
  }

  /* Otherwise, BOM is already skipped */
}

const char* executable_path(void) {
  static char path_buffer[PATH_BUFFER_SIZE];
  static int calculated = 0;

  if (!calculated) {
#ifdef __linux
    ssize_t res = readlink("/proc/self/exe", path_buffer, sizeof(path_buffer));
    assert(res != -1);
#elif __APPLE__
    uint32_t size = sizeof(path_buffer);
    int res = _NSGetExecutablePath(path_buffer, &size);
    assert(res == 0);
#elif _WIN32 || _WIN64
    // NOTE: for "C:\\opencc.exe" on Windows, the returned path "C:" is
    // incorrect until a '/' is appended to it later in try_open_file()
    DWORD res = GetModuleFileNameA(NULL, path_buffer, PATH_BUFFER_SIZE);
    assert(res != 0);
#else
    /* Other unsupported os */
    assert(0);
#endif /* ifdef __linux */
    char* last_sep = strrchr(path_buffer, PATH_SEPARATOR);
    assert(last_sep != NULL);
    *last_sep = '\0';
    calculated = 1;
  }
  return path_buffer;
}

char* try_open_file(const char* path) {
  /* Try to find file in current working directory */
  FILE* fp = fopen(path, "r");

  if (fp) {
    fclose(fp);
    return mstrcpy(path);
  }

  /* If path is absolute, return NULL */
  if (is_absolute_path(path)) {
    return NULL;
  }

  /* Try to find file in executable directory */
  const char* exe_dir = executable_path();
  char* filename =
    (char*)malloc(sizeof(char) * (strlen(path) + strlen(exe_dir) + 2));
  sprintf(filename, "%s/%s", exe_dir, path);
  fp = fopen(filename, "r");

  if (fp) {
    fclose(fp);
    return filename;
  }
  free(filename);

  /* Try to use PKGDATADIR */
  filename =
    (char*)malloc(sizeof(char) * (strlen(path) + strlen(PKGDATADIR) + 2));
  sprintf(filename, "%s/%s", PKGDATADIR, path);
  fp = fopen(filename, "r");

  if (fp) {
    fclose(fp);
    return filename;
  }
  free(filename);
  return NULL;
}

char* get_file_path(const char* filename) {
  const char* last_sep = strrchr(filename, '/');

  if (last_sep == NULL) {
    last_sep = filename;
  }
  char* path = mstrncpy(filename, last_sep - filename);
  return path;
}

int is_absolute_path(const char* path) {
  if (path[0] == '/') {
    return 1;
  }

  if (path[1] == ':') {
    return 1;
  }
  return 0;
}
