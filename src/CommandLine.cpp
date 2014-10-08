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

void ShowVersion() {
  printf("\n");
  printf("Open Chinese Convert (OpenCC) Command Line Tool\n");
  printf("Version %s\n", VERSION);
  printf("\n");
  printf("Author: %s\n", "Carbo Kuo <byvoid@byvoid.com>");
  printf("Bug Report: %s\n", "http://github.com/BYVoid/OpenCC/issues");
  printf("\n");
}

void ShowUsage() {
  ShowVersion();
  printf("Usage:\n");
  printf(" opencc [Options]\n");
  printf("\n");
  printf("Options:\n");
  printf(" -i [file], --input=[file]   Read original text from [file].\n");
  printf(" -o [file], --output=[file]  Write converted text to [file].\n");
  printf(" -c [file], --config=[file]  Load configuration from [file].\n");
  printf(" -v, --version               Print version and build information.\n");
  printf(" -h, --help                  Print this help.\n");
  printf("\n");
  printf("With no input file, reads stdin and writes to stdout.\n");
  printf("By default configuration is simplified to traditional.\n");
  printf("\n");
}

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

void Convert(const Optional<string>& inputFileName,
             const Optional<string>& outputFileName,
             const string& configFileName) {
  Config config;
  auto converter = config.NewFromFile(configFileName);
  std::istream& inputStream = GetInputStream(inputFileName);
  std::ostream& outputStream = GetOutputStream(outputFileName);
  while (!inputStream.eof()) {
    string line;
    std::getline(inputStream, line);
    string converted = converter->Convert(line);
    outputStream << converted << std::endl;
    outputStream.flush();
  }
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
    cmd.parse(argc, argv);
    Optional<string> inputFileName = Optional<string>::Null();
    Optional<string> outputFileName = Optional<string>::Null();
    string configFileName = configArg.getValue();
    if (inputArg.isSet()) {
      inputFileName = Optional<string>(inputArg.getValue());
    }
    if (outputArg.isSet()) {
      outputFileName = Optional<string>(outputArg.getValue());
    }
    Convert(inputFileName, outputFileName, configFileName);
  } catch (TCLAP::ArgException& e) {
    std::cerr << "error: " << e.error()
        << " for arg " << e.argId() << std::endl;
  } catch (Exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
