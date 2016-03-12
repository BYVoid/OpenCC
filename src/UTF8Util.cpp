/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
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

#include "UTF8Util.hpp"

using namespace opencc;

void UTF8Util::SkipUtf8Bom(FILE* fp) {
  /* UTF-8 BOM is EF BB BF */
  if (fp == NULL) {
    return;
  }
  /* If we are not at beginning of file, return */
  if (ftell(fp) != 0) {
    return;
  }

  /* Try to read first 3 bytes */
  int bom[3];
  int n;
  for (n = 0; n <= 2 && (bom[n] = getc(fp)) != EOF; n++) {
  }
  /* If we can only read <3 bytes, push them back */
  /* Or if first 3 bytes is not BOM, push them back */
  if ((n < 3) || (bom[0] != 0xEF) || (bom[1] != 0xBB) || (bom[2] != 0xBF)) {
    for (n--; n >= 0; n--) {
      ungetc(bom[n], fp);
    }
  }

  /* Otherwise, BOM is already skipped */
}
