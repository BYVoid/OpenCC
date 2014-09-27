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

#include "CmdLineOutput.hpp"
#include "Config.hpp"
#include "Converter.hpp"

using opencc::Config;
using opencc::FileNotFound;
using opencc::FileNotWritable;
using opencc::Optional;

void ShowVersion() {
  printf(_("\n"));
  printf(_("Open Chinese Convert (OpenCC) Command Line Tool\n"));
  printf(_("Version %s\n"), VERSION);
  printf(_("\n"));
  printf(_("Author: %s\n"), "Carbo Kuo <byvoid@byvoid.com>");
  printf(_("Bug Report: %s\n"), "http://github.com/BYVoid/OpenCC/issues");
  printf(_("\n"));
}

void ShowUsage() {
  ShowVersion();
  printf(_("Usage:\n"));
  printf(_(" opencc [Options]\n"));
  printf(_("\n"));
  printf(_("Options:\n"));
  printf(_(" -i [file], --input=[file]   Read original text from [file].\n"));
  printf(_(" -o [file], --output=[file]  Write converted text to [file].\n"));
  printf(_(" -c [file], --config=[file]  Load configuration from [file].\n"));
  printf(_(" -v, --version               Print version and build information.\n"));
  printf(_(" -h, --help                  Print this help.\n"));
  printf(_("\n"));
  printf(_(
           "With no input file, reads standard input and writes converted stream to standard output.\n"));
  printf(_(
           "Default configuration (simplified to traditional) will be loaded if not set.\n"));
  printf(_("\n"));
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

  config.LoadFile(configFileName);
  auto converter = config.GetConverter();
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
    Optional<string> inputFileName;
    Optional<string> outputFileName;
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
  }
  return 0;
}
