/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 Carbo Kuo <byvoid@byvoid.com>
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

#ifndef __OPENCC_H_
#define __OPENCC_H_

#ifdef __cplusplus

#include "Export.hpp"
#include "SimpleConverter.hpp"
#include <string>

extern "C" {
#else
#include <stddef.h>
#endif

#ifndef OPENCC_EXPORT
#define OPENCC_EXPORT
#endif

/**
 * @defgroup opencc_c_api OpenCC C API
 *
 * API in C language
 */

/**
 * Filename of default Simplified to Traditional configuration
 *
 * @ingroup opencc_c_api
 */
#define OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD "s2t.json"

/**
 * Filename of default Traditional to Simplified configuration
 *
 * @ingroup opencc_c_api
 */
#define OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP "t2s.json"

/**
 * Type of opencc descriptor
 *
 * @ingroup opencc_c_api
 */
typedef void* opencc_t;

/**
 * Makes an instance of opencc
 *
 * @param configFileName Location of configuration file. If this is set to NULL,
 *                       OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD will be loaded.
 * @return            A description pointer of the newly allocated instance of
 *                    opencc. On error the return value will be (opencc_t) -1.
 * @ingroup opencc_c_api
 */
OPENCC_EXPORT opencc_t opencc_open(const char* configFileName);
#ifdef _MSC_VER
/**
 * Makes an instance of opencc (wide char / Unicode)
 *
 * @param configFileName Location of configuration file. If this is set to NULL,
 *                       OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD will be loaded.
 * @return            A description pointer of the newly allocated instance of
 *                    opencc. On error the return value will be (opencc_t) -1.
 * @ingroup opencc_c_api
 */
OPENCC_EXPORT opencc_t opencc_open_w(const wchar_t* configFileName);
#endif /* _MSC_VER */

/**
 * Destroys an instance of opencc
 *
 * @param opencc The description pointer.
 * @return 0 on success or non-zero number on failure.
 * @ingroup opencc_c_api
 */
OPENCC_EXPORT int opencc_close(opencc_t opencc);

/**
 * Converts UTF-8 std::string
 *
 * @param opencc The opencc description pointer.
 * @param input  The UTF-8 encoded std::string.
 * @param length The maximum length in byte to convert. If length is (size_t)-1,
 *               the whole std::string (terminated by '\0') will be converted.
 * @param output The buffer to store converted text. You MUST make sure this
 *               buffer has sufficient space.
 *
 * @return       The length of converted std::string or (size_t)-1 on error.
 *
 * @ingroup opencc_c_api
 */
OPENCC_EXPORT size_t opencc_convert_utf8_to_buffer(opencc_t opencc,
                                                   const char* input,
                                                   size_t length, char* output);

/**
 * Converts UTF-8 std::string
 * This function returns an allocated C-Style std::string, which stores
 * the converted std::string.
 * You MUST call opencc_convert_utf8_free() to release allocated memory.
 *
 * @param opencc The opencc description pointer.
 * @param input  The UTF-8 encoded std::string.
 * @param length The maximum length in byte to convert. If length is (size_t)-1,
 *               the whole std::string (terminated by '\0') will be converted.
 *
 * @return       The newly allocated UTF-8 std::string that stores text
 * converted, or NULL on error.
 * @ingroup opencc_c_api
 */
OPENCC_EXPORT char* opencc_convert_utf8(opencc_t opencc, const char* input,
                                        size_t length);

/**
 * Releases allocated buffer by opencc_convert_utf8
 *
 * @param str    Pointer to the allocated std::string buffer by
 * opencc_convert_utf8.
 *
 * @ingroup opencc_c_api
 */
OPENCC_EXPORT void opencc_convert_utf8_free(char* str);

/**
 * Returns the last error message
 *
 * Note that this function is the only one which is NOT thread-safe.
 *
 * @ingroup opencc_c_api
 */
OPENCC_EXPORT const char* opencc_error(void);

#ifdef __cplusplus
} // extern "C"
#endif

/**
 * @defgroup opencc_cpp_api OpenCC C++ Comprehensive API
 *
 * Comprehensive API in C++ language
 */

#endif
