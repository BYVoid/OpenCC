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

#ifndef __OPENCC_SIMPLECONVERTER_HPP_
#define __OPENCC_SIMPLECONVERTER_HPP_

/**
* @defgroup opencc_simple_api OpenCC C++ Simple API
*
* Simple API in C++ language
*/

namespace opencc {
/**
* A high level converter
* This interface does not require C++11 to compile.
* @ingroup opencc_simple_api
*/
class OPENCC_EXPORT SimpleConverter {
public:
  /**
  * Constructor of SimpleConverter
  * @param configFileName File name of configuration.
  */
  SimpleConverter(const std::string& configFileName);

  ~SimpleConverter();

  /**
  * Converts a text
  * @param input Text to be converted.
  */
  std::string Convert(const std::string& input) const;

  /**
  * Converts a text
  * @param input A C-Style string (terminated by '\0') to be converted.
  */
  std::string Convert(const char* input) const;

  /**
  * Converts a text
  * @param input  A C-Style string limited by a given length to be converted.
  * @param length Maximal length in byte of the input string.
  */
  std::string Convert(const char* input, size_t length) const;

  /**
  * Converts a text and writes to an allocated buffer
  * Please make sure the buffer has sufficent space.
  * @param input  A C-Style string (terminated by '\0') to be converted.
  * @param output Buffer to write the converted text.
  * @return       Length of converted text.
  */
  size_t Convert(const char* input, char* output) const;

  /**
  * Converts a text and writes to an allocated buffer
  * Please make sure the buffer has sufficent space.
  * @param input  A C-Style string limited by a given length to be converted.
  * @param length Maximal length in byte of the input string.
  * @param output Buffer to write the converted text.
  * @return       Length of converted text.
  */
  size_t Convert(const char* input, size_t length, char* output) const;

private:
  const void* internalData;
};

} // namespace opencc

#endif
