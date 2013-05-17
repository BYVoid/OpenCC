/**
 * @file
 * OpenCC API.
 *
 * @license
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

#ifndef __OPENCC_H_
#define __OPENCC_H_

/**
 * @defgroup opencc_api OpenCC API
 * 
 * API in C language
 */

#include "opencc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Filename of default Simplified to Traditional configuration.
 *
 * @ingroup opencc_api
 */
#define OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD "zhs2zht.ini"

/**
 * Filename of default Traditional to Simplified configuration.
 *
 * @ingroup opencc_api
 */
#define OPENCC_DEFAULT_CONFIG_TRAD_TO_SIMP "zht2zhs.ini"

/**
 * Makes an instance of opencc.
 * Leave config_file to NULL if you do not want to load any configuration file.
 *
 * @param config_file Location of configuration file.
 * @return            A description pointer of the newly allocated instance of
 *                    opencc. On error the return value will be (opencc_t) -1.
 * @ingroup opencc_api
 */
opencc_t opencc_open(const char* config_file);

/**
 * Destroys an instance of opencc.
 *
 * @param od The description pointer.
 * @return 0 on success or non-zero number on failure.
 */
int opencc_close(opencc_t od);

/**
 * Converts a UCS-4 string from *inbuf to *outbuf.
 * Do not forget to assign **outbuf to L'\0' after called if you want to use it
 * as a C-Style string.
 *
 * @param od         The opencc description pointer.
 * @param inbuf      The pointer to the UCS-4 string.
 * @param inbufleft  The maximum number of characters in *inbuf to be converted.
 * @param outbuf     The pointer to the output buffer.
 * @param outbufleft The size of output buffer.
 *
 * @return           The number of characters in the input buffer that has been
 *                   converted.
 * @ingroup opencc_api
 */
size_t opencc_convert(opencc_t od,
                      ucs4_t** inbuf,
                      size_t* inbufleft,
                      ucs4_t** outbuf,
                      size_t* outbufleft);

/**
 * Converts UTF-8 string from inbuf.
 * This function returns an allocated C-Style string via malloc(), which stores
 * the converted string.
 * You should call opencc_convert_utf8_free() to release allocated memory.
 *
 * @param od     The opencc description pointer.
 * @param inbuf  The UTF-8 encoded string.
 * @param length The maximum length of inbuf to convert. If length is set to -1,
 *               the whole c-style string in inbuf will be converted.
 *
 * @return       The newly allocated UTF-8 string that stores text converted
 *               from inbuf.
 * @ingroup opencc_api
 */
char* opencc_convert_utf8(opencc_t od, const char* inbuf, size_t length);

/**
 * Releases allocated buffer by opencc_convert_utf8.
 *
 * @param buf    Pointer to the allocated string buffer by opencc_convert_utf8.
 *
 * @ingroup opencc_api
 */
void opencc_convert_utf8_free(char* buf);

/**
 * Loads a dictionary to default dictionary chain.
 *
 * @param od             The opencc description pointer.
 * @param dict_filename  The name (or location) of the dictionary file.
 * @param dict_type      The type of the dictionary.
 *
 * @return               0 on success or non-zero number on failure.
 *
 * @ingroup opencc_api
 * @deprecated This function is not recommended to use and will be removed.
 */
int opencc_dict_load(opencc_t od,
                     const char* dict_filename,
                     opencc_dictionary_type dict_type);

/**
 * Changes the mode of conversion.
 *
 * @param od               The opencc description pointer.
 * @param conversion_mode  Conversion mode. Options are
 *                         - OPENCC_CONVERSION_FAST
 *                         - OPENCC_CONVERSION_SEGMENT_ONLY
 *                         - OPENCC_CONVERSION_LIST_CANDIDATES
 * @ingroup opencc_api
 */
void opencc_set_conversion_mode(opencc_t od,
                                opencc_conversion_mode conversion_mode);

/**
 * Returns an opencc_convert_errno_t which describes the last error.
 *
 * @return The error type.
 */
opencc_error opencc_errno(void);

/**
 * Prints the error message to stderr.
 *
 * @param spec Prefix message.
 * @ingroup opencc_api
 */
void opencc_perror(const char* spec);

#ifdef __cplusplus
}
#endif

#endif /* __OPENCC_H_ */
