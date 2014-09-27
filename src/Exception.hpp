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
#include <stdexcept>
#include <string>

#include "Export.hpp"

#ifdef _MSC_VER

// Until Visual Studio 2013 (12.0), C++ 11 "noexcept" qualifier is not supported
# define noexcept
#endif // ifdef _MSC_VER

namespace opencc {
  class OPENCC_EXPORT Exception : public std::exception {
    public:
      Exception() {}

      virtual ~Exception() throw() {}

      Exception(std::string message_) : message(message_) {}

      virtual const char* what() const noexcept {
        return message.c_str();
      }

    private:
      std::string message;
  };

  class OPENCC_EXPORT FileNotFound : public Exception {
    public:
      FileNotFound(const std::string fileName) :
        Exception(fileName + " not found or not accessible") {}
  };

  class OPENCC_EXPORT FileNotWritable : public Exception {
    public:
      FileNotWritable(const std::string fileName) :
        Exception(fileName + " not writable") {}
  };

  class OPENCC_EXPORT InvalidFormat : public Exception {
    public:
      InvalidFormat(const std::string message) : Exception(message) {}
  };
}
