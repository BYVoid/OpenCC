/*
* Open Chinese Convert
*
* Copyright 2010 BYVoid <byvoid1@gmail.com>
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

#ifndef __OPENCC_DATRIE_H_
#define __OPENCC_DATRIE_H_

#include "opencc_utils.h"

#define DATRIE_UNUSED -1

typedef struct
{
	int base;
	int parent;
	int word;
} DoubleArrayTrieItem;

const wchar_t * get_trad_in_datrie(const wchar_t *, int *, int);
void get_match_lengths(const wchar_t *, int *);

#endif /* __OPENCC_DATRIE_H_ */
