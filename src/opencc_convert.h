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

#ifdef __cplusplus
extern "C" {
#endif

#include <wchar.h>

#define OPENCC_CONVERT_ERROR ((size_t) -1)

typedef void * opencc_t;

typedef enum
{
	OPENCC_CONVERT_SIMP_TO_TRAD,
} opencc_convert_direction_t;

typedef enum
{
	OPENCC_CONVERT_ERROR_VOID,
	OPENCC_CONVERT_ERROR_OUTBUF_NOT_ENOUGH,
} opencc_convert_errno_t;

opencc_t opencc_open(opencc_convert_direction_t);
void opencc_close(opencc_t);
size_t opencc_convert(opencc_t, wchar_t **, size_t *, wchar_t **, size_t *);
opencc_convert_errno_t opencc_errno(opencc_t);
void opencc_perror(opencc_t);

#ifdef __cplusplus
};
#endif

#endif /* __OPENCC_CONVERT_H_ */
