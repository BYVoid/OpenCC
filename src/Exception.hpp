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

#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

#include "Export.hpp"

#if defined(_MSC_VER) && _MSC_VER < 1900
// Before Visual Studio 2015 (14.0), C++ 11 "noexcept" qualifier is not supported
#define noexcept
#endif // ifdef _MSC_VER

namespace opencc {

class OPENCC_EXPORT Exception {
public:
  Exception() {}

  virtual ~Exception() throw() {}

  Exception(const std::string& _message) : message(_message) {}

  virtual const char* what() const noexcept { return message.c_str(); }

protected:
  std::string message;
};

class OPENCC_EXPORT FileNotFound : public Exception {
public:
  FileNotFound(const std::string& fileName)
      : Exception(fileName + " not found or not accessible.") {}
};

class OPENCC_EXPORT FileNotWritable : public Exception {
public:
  FileNotWritable(const std::string& fileName)
      : Exception(fileName + " not writable.") {}
};

class OPENCC_EXPORT InvalidFormat : public Exception {
public:
  InvalidFormat(const std::string& message)
      : Exception("Invalid format: " + message) {}
};

class OPENCC_EXPORT InvalidTextDictionary : public InvalidFormat {
public:
  InvalidTextDictionary(const std::string& _message, size_t lineNum)
      : InvalidFormat("") {
    std::ostringstream buffer;
    buffer << "Invalid text dictionary at line " << lineNum << ": " << _message;
    message = buffer.str();
  }
};

class OPENCC_EXPORT InvalidUTF8 : public Exception {
public:
  InvalidUTF8(const std::string& _message)
      : Exception("Invalid UTF8: " + _message) {}
};

class OPENCC_EXPORT ShouldNotBeHere : public Exception {
public:
  ShouldNotBeHere() : Exception("ShouldNotBeHere! This must be a bug.") {}
};

} // namespace opencc
