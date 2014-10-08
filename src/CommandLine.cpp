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

using opencc::Config;
using opencc::Exception;
using opencc::FileNotFound;
using opencc::FileNotWritable;
using opencc::Optional;

std::istream& GetInputStream(const Optional<string>& inputFileName) {
  if (inputFileName.IsNull()) {
    return std::cin;
  } else {
    std::ifstream* stream = new std::ifstream(inputFileName.Get());
    if (!stream->is_open()) {
      throw FileNotFound(inputFileName.Get());
    }
    return *stream;
  }
}

std::ostream& GetOutputStream(const Optional<string>& outputFileName) {
  if (outputFileName.IsNull()) {
    return std::cout;
  } else {
    std::ofstream* stream = new std::ofstream(outputFileName.Get());
    if (!stream->is_open()) {
      throw FileNotWritable(outputFileName.Get());
    }
    return *stream;
  }
}

string Read(std::istream& inputStream, bool lineByLine) {
  // TODO block read
  string line;
  std::getline(inputStream, line);
  return line;
}

void Convert(const Optional<string>& inputFileName,
             const Optional<string>& outputFileName,
             const string& configFileName,
             const bool noFlush) {
  Config config;
  auto converter = config.NewFromFile(configFileName);
  std::istream& inputStream = GetInputStream(inputFileName);
  std::ostream& outputStream = GetOutputStream(outputFileName);
  bool lineByLine = inputFileName.IsNull();
  while (!inputStream.eof()) {
    const string& text = Read(inputStream, lineByLine);
    const string& converted = converter->Convert(text);
    outputStream << converted << '\n';
    if (!noFlush) {
      // Flush every line if the output stream is stdout.
      outputStream.flush();
    }
  }
  outputStream.flush();
}

int main(int argc, const char* argv[]) {
  try {
    TCLAP::CmdLine cmd("Open Chinese Convert (OpenCC) Command Line Tool",
                       ' ',
                       VERSION);
    CmdLineOutput cmdLineOutput;
    cmd.setOutput(&cmdLineOutput);

    TCLAP::ValueArg<string> configArg("c", "config",
                                      "Configuration file",
                                      false /* required */,
                                      "s2t.json" /* default */,
                                      "file" /* type */,
                                      cmd);
    TCLAP::ValueArg<string> outputArg("o", "output",
                                      "Write converted text to",
                                      false /* required */,
                                      "" /* default */,
                                      "file" /* type */,
                                      cmd);
    TCLAP::ValueArg<string> inputArg("i", "input",
                                     "Read original text from",
                                     false /* required */,
                                     "" /* default */,
                                     "file" /* type */,
                                     cmd);
    TCLAP::ValueArg<bool> noFlushArg("", "noflush",
                                     "Disable flush for every line",
                                     false /* required */,
                                     false /* default */,
                                     "bool" /* type */,
                                     cmd);
    cmd.parse(argc, argv);
    Optional<string> inputFileName = Optional<string>::Null();
    Optional<string> outputFileName = Optional<string>::Null();
    string configFileName = configArg.getValue();
    bool noFlush = noFlushArg.getValue();
    if (inputArg.isSet()) {
      inputFileName = Optional<string>(inputArg.getValue());
    }
    if (outputArg.isSet()) {
      outputFileName = Optional<string>(outputArg.getValue());
      noFlush = true;
    }
    Convert(inputFileName, outputFileName, configFileName, noFlush);
  } catch (TCLAP::ArgException& e) {
    std::cerr << "error: " << e.error()
        << " for arg " << e.argId() << std::endl;
  } catch (Exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
