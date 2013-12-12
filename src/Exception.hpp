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

#pragma once
#include <string>
#include <stdexcept>

#ifdef _MSC_VER
// Until Visual Studio 2013 (12.0), C++ 11 "noexcept" qualifier is not supported
#define noexcept
#endif

namespace Opencc {
  class FileNotFound : public std::exception {
  public:
    FileNotFound(const std::string fileName) {
      message = fileName + " not found or not accessible.";
    }
    virtual const char* what() const noexcept {
      return message.c_str();
    }
  private:
    std::string message;
  };
  
  class FileNotWritable : public std::exception {
  public:
    FileNotWritable(const std::string fileName) {
      message = fileName + " not writable";
    }
    virtual const char* what() const noexcept {
      return message.c_str();
    }
  private:
    std::string message;
  };
  
  class InvalidFormat : public std::logic_error {
  public:
    InvalidFormat(const std::string message) : std::logic_error(message) {
    }
  };
}
