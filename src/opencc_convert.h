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

#ifndef __OPENCC_CONVERT_H_
#define __OPENCC_CONVERT_H_

#include "dictionary/opencc_dictionary.h"

typedef struct
{
	int initialized;
	size_t buffer_size;
	size_t * match_length;
	size_t * min_len;
	size_t * parent;
	size_t * path;
} opencc_sp_seg_buffer;

typedef struct
{
	opencc_dictionary_t dicts;
	opencc_convert_direction_t convert_direction;
	opencc_convert_errno_t errno;
	opencc_sp_seg_buffer sp_seg_buffer;
} opencc_description;

#endif /* __OPENCC_CONVERT_H_ */
