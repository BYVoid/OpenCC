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

#include "opencc_dictionary.h"

typedef void * opencc_converter_t;

void converter_assign_dicts(opencc_converter_t cdt, opencc_dictionary_t dicts);

opencc_converter_t converter_open();

void converter_close(opencc_converter_t cdt);

size_t converter_convert(opencc_converter_t cdt, wchar_t ** inbuf, size_t * inbuf_left,
		wchar_t ** outbuf, size_t * outbuf_left);

#endif /* __OPENCC_CONVERT_H_ */
