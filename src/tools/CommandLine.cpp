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

#include "CmdLineOutput.hpp"
#include "Config.hpp"
#include "Converter.hpp"
#include "UTF8Util.hpp"

using namespace opencc;

Optional<string> inputFileName = Optional<string>::Null();
Optional<string> outputFileName = Optional<string>::Null();
string configFileName;
bool noFlush;
Config config;
ConverterPtr converter;

FILE* GetOutputStream() {
  if (outputFileName.IsNull()) {
    return stdout;
  } else {
    FILE* fp = fopen(outputFileName.Get().c_str(), "w");
    if (!fp) {
      throw FileNotWritable(outputFileName.Get());
    }
    return fp;
  }
}

void ConvertLineByLine() {
  std::istream& inputStream = std::cin;
  FILE* fout = GetOutputStream();
  bool isFirstLine = true;
  while (!inputStream.eof()) {
    if (!isFirstLine) {
      fputs("\n", fout);
    } else {
      isFirstLine = false;
    }
    string line;
    std::getline(inputStream, line);
    const string& converted = converter->Convert(line);
    fputs(converted.c_str(), fout);
    if (!noFlush) {
      // Flush every line if the output stream is stdout.
      fflush(fout);
    }
  }
  fclose(fout);
}

void Convert(string inputFileName) {
  const int BUFFER_SIZE = 1024 * 1024;
  static bool bufferInitialized = false;
  static string buffer;
  static char* bufferBegin;
  static const char* bufferEnd;
  static char* bufferPtr;
  static size_t bufferSizeAvailble;
  if (!bufferInitialized) {
    bufferInitialized = true;
    buffer.resize(BUFFER_SIZE + 1);
    bufferBegin = const_cast<char*>(buffer.c_str());
    bufferEnd = buffer.c_str() + BUFFER_SIZE;
    bufferPtr = bufferBegin;
    bufferSizeAvailble = BUFFER_SIZE;
  }

  bool needToRemove = false;
  if (!outputFileName.IsNull() && inputFileName == outputFileName.Get()) {
    // Special case: input == output
    const string tempFileName = std::tmpnam(nullptr);
    std::ifstream src(inputFileName, std::ios::binary);
    std::ofstream dst(tempFileName, std::ios::binary);
    dst << src.rdbuf();
    dst.close();
    inputFileName = tempFileName;
    needToRemove = true;
  }

  FILE* fin = fopen(inputFileName.c_str(), "r");
  if (!fin) {
    throw FileNotFound(inputFileName);
  }
  FILE* fout = GetOutputStream();
  while (!feof(fin)) {
    size_t length = fread(bufferPtr, sizeof(char), bufferSizeAvailble, fin);
    bufferPtr[length] = '\0';
    size_t remainingLength = 0;
    string remainingTemp;
    if (length == bufferSizeAvailble) {
      // fread may breaks UTF8 character
      // Find the end of last character
      char* lastChPtr = bufferBegin;
      while (lastChPtr < bufferEnd) {
        size_t nextCharLen = UTF8Util::NextCharLength(lastChPtr);
        if (lastChPtr + nextCharLen > bufferEnd) {
          break;
        }
        lastChPtr += nextCharLen;
      }
      remainingLength = bufferEnd - lastChPtr;
      if (remainingLength > 0) {
        remainingTemp = UTF8Util::FromSubstr(lastChPtr, remainingLength);
        *lastChPtr = '\0';
      }
    }
    // Perform conversion
    const string& converted = converter->Convert(buffer);
    fputs(converted.c_str(), fout);
    if (!noFlush) {
      // Flush every line if the output stream is stdout.
      fflush(fout);
    }
    // Reset pointer
    bufferPtr = bufferBegin + remainingLength;
    bufferSizeAvailble = BUFFER_SIZE - remainingLength;
    if (remainingLength > 0) {
      strncpy(bufferBegin, remainingTemp.c_str(), remainingLength);
    }
  }
  fclose(fout);
  if (needToRemove) {
    // Remove temporary file.
    std::remove(inputFileName.c_str());
  }
}

int main(int argc, const char* argv[]) {
  try {
    TCLAP::CmdLine cmd("Open Chinese Convert (OpenCC) Command Line Tool", ' ',
                       VERSION);
    CmdLineOutput cmdLineOutput;
    cmd.setOutput(&cmdLineOutput);

    TCLAP::ValueArg<string> configArg(
        "c", "config", "Configuration file", false /* required */,
        "s2t.json" /* default */, "file" /* type */, cmd);
    TCLAP::ValueArg<string> outputArg(
        "o", "output", "Write converted text to <file>.", false /* required */,
        "" /* default */, "file" /* type */, cmd);
    TCLAP::ValueArg<string> inputArg(
        "i", "input", "Read original text from <file>.", false /* required */,
        "" /* default */, "file" /* type */, cmd);
    TCLAP::ValueArg<bool> noFlushArg(
        "", "noflush", "Disable flush for every line", false /* required */,
        false /* default */, "bool" /* type */, cmd);
    cmd.parse(argc, argv);
    configFileName = configArg.getValue();
    noFlush = noFlushArg.getValue();
    if (inputArg.isSet()) {
      inputFileName = Optional<string>(inputArg.getValue());
    }
    if (outputArg.isSet()) {
      outputFileName = Optional<string>(outputArg.getValue());
      noFlush = true;
    }
    converter = config.NewFromFile(configFileName);
    bool lineByLine = inputFileName.IsNull();
    if (lineByLine) {
      ConvertLineByLine();
    } else {
      Convert(inputFileName.Get());
    }
  } catch (TCLAP::ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId()
              << std::endl;
  } catch (Exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
