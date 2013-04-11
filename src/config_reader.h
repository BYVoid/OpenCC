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

#ifndef __OPENCC_CONFIG_H_
#define __OPENCC_CONFIG_H_

#include "common.h"
#include "dict_chain.h"

typedef enum {
  CONFIG_ERROR_VOID,
  CONFIG_ERROR_CANNOT_ACCESS_CONFIG_FILE,
  CONFIG_ERROR_PARSE,
  CONFIG_ERROR_NO_PROPERTY,
  CONFIG_ERROR_INVALID_DICT_TYPE,
} config_error;

Config* config_open(const char* filename);

void config_close(Config* config);

DictChain* config_get_dict_chain(Config* config);

config_error config_errno(void);

void config_perror(const char* spec);

#endif /* __OPENCC_CONFIG_H_ */
