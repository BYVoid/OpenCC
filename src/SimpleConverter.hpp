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

#include "Export.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#ifndef __OPENCC_SIMPLECONVERTER_HPP_
#define __OPENCC_SIMPLECONVERTER_HPP_

#include "ConversionInspection.hpp"
#include "ResourceProvider.hpp"

/**
 * @defgroup opencc_simple_api OpenCC C++ Simple API
 *
 * Simple API in C++ language
 */

namespace opencc {

struct ConfigLoadOptions;

/**
 * A high-level converter.
 * @ingroup opencc_simple_api
 */
class OPENCC_EXPORT SimpleConverter {
public:
  /**
   * Constructor of SimpleConverter
   * @param configFileName File name of configuration.
   */
  explicit SimpleConverter(const std::string& configFileName);

  /**
   * Constructor of SimpleConverter
   * @param configFileName File name of configuration.
   * @param options Configuration loading options.
   */
  SimpleConverter(const std::string& configFileName,
                  const ConfigLoadOptions& options);

  /**
   * Constructor of SimpleConverter
   * @param configFileName File name of configuration.
   * @param paths Additional paths to locate configuration and dictionary files.
   */
  SimpleConverter(const std::string& configFileName,
                  const std::vector<std::string>& paths);

  /**
   * Constructor of SimpleConverter
   * @param configFileName File name of configuration.
   * @param paths Additional paths to locate configuration and dictionary files.
   * @param options Configuration loading options.
   */
  SimpleConverter(const std::string& configFileName,
                  const std::vector<std::string>& paths,
                  const ConfigLoadOptions& options);

  /**
   * Constructor of SimpleConverter
   * @param configFileName File name of configuration.
   * @param paths Additional paths to locate configuration and dictionary files.
   * @param argv0 Path of the executable (argv[0]), in addition to additional
   * paths.
   */
  SimpleConverter(const std::string& configFileName,
                  const std::vector<std::string>& paths, const char* argv0);

  /**
   * Constructor of SimpleConverter
   * @param configFileName File name of configuration.
   * @param paths Additional paths to locate configuration and dictionary files.
   * @param argv0 Path of the executable (argv[0]), in addition to additional
   * paths.
   * @param options Configuration loading options.
   */
  SimpleConverter(const std::string& configFileName,
                  const std::vector<std::string>& paths, const char* argv0,
                  const ConfigLoadOptions& options);

  /**
   * Constructor of SimpleConverter
   * @param configFileName File name of configuration.
   * @param provider Provider used to locate dictionary/resource files
   * referenced by the configuration.
   */
  SimpleConverter(const std::string& configFileName,
                  std::shared_ptr<ResourceProvider> provider);

  /**
   * Constructor of SimpleConverter
   * @param configFileName File name of configuration.
   * @param provider Provider used to locate dictionary/resource files
   * referenced by the configuration.
   * @param options Configuration loading options.
   */
  SimpleConverter(const std::string& configFileName,
                  std::shared_ptr<ResourceProvider> provider,
                  const ConfigLoadOptions& options);

  ~SimpleConverter();

  /**
   * Converts a text
   * @param input Text to be converted.
   */
  std::string Convert(const std::string& input) const;

  /**
   * Converts a text without requiring a null-terminated buffer.
   * New code should prefer this overload when a string_view is already
   * available.
   * @param input Text to be converted.
   */
  std::string Convert(std::string_view input) const;

  /**
   * Converts a text
   * @param input A C-Style std::string (terminated by '\0') to be converted.
   */
  std::string Convert(const char* input) const;

  /**
   * Converts a text
   * @param input  A C-Style std::string limited by a given length to be
   * converted.
   * @param length Maximal length in byte of the input std::string.
   */
  std::string Convert(const char* input, size_t length) const;

  /**
   * Converts a text and writes to an allocated buffer
   * Please make sure the buffer has sufficient space.
   * @param input  A C-Style std::string (terminated by '\0') to be converted.
   * @param output Buffer to write the converted text.
   * @return       Length of converted text.
   */
  size_t Convert(const char* input, char* output) const;

  /**
   * Converts a text and writes to an allocated buffer
   * Please make sure the buffer has sufficient space.
   * @param input  A C-Style std::string limited by a given length to be
   * converted.
   * @param length Maximal length in byte of the input std::string.
   * @param output Buffer to write the converted text.
   * @return       Length of converted text.
   */
  size_t Convert(const char* input, size_t length, char* output) const;

  /**
   * Inspects the conversion process for a given text.
   * Returns the segmentation result, per-stage conversion outputs, and final
   * output string.
   * @param input Text to be inspected.
   */
  ConversionInspectionResult Inspect(std::string_view input) const;

private:
  const void* internalData;
};

} // namespace opencc

#endif
