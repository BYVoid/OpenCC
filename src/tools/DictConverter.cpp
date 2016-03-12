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
#include "DartsDict.hpp"
#include "TextDict.hpp"

using namespace opencc;

DictPtr LoadDictionary(const string& format, const string& inputFileName) {
  if (format == "text") {
    return SerializableDict::NewFromFile<TextDict>(inputFileName);
  } else if (format == "ocd") {
    return SerializableDict::NewFromFile<DartsDict>(inputFileName);
  } else {
    fprintf(stderr, "Unknown dictionary format: %s\n", format.c_str());
    exit(2);
  }
  return nullptr;
}

SerializableDictPtr ConvertDictionary(const string& format,
                                      const DictPtr dict) {
  if (format == "text") {
    return TextDict::NewFromDict(*dict.get());
  } else if (format == "ocd") {
    return DartsDict::NewFromDict(*dict.get());
  } else {
    fprintf(stderr, "Unknown dictionary format: %s\n", format.c_str());
    exit(2);
  }
  return nullptr;
}

void ConvertDictionary(const string inputFileName, const string outputFileName,
                       const string formatFrom, const string formatTo) {
  DictPtr dictFrom = LoadDictionary(formatFrom, inputFileName);
  SerializableDictPtr dictTo = ConvertDictionary(formatTo, dictFrom);
  dictTo->SerializeToFile(outputFileName);
}

int main(int argc, const char* argv[]) {
  try {
    TCLAP::CmdLine cmd("Open Chinese Convert (OpenCC) Dictionary Tool", ' ',
                       VERSION);
    CmdLineOutput cmdLineOutput;
    cmd.setOutput(&cmdLineOutput);

    vector<string> dictFormats{"text", "ocd"};
    TCLAP::ValuesConstraint<string> allowedVals(dictFormats);

    TCLAP::ValueArg<string> toArg("t", "to", "Output format",
                                  true /* required */, "" /* default */,
                                  &allowedVals /* type */, cmd);
    TCLAP::ValueArg<string> fromArg("f", "from", "Input format",
                                    true /* required */, "" /* default */,
                                    &allowedVals /* type */, cmd);
    TCLAP::ValueArg<string> outputArg(
        "o", "output", "Path to output dictionary", true /* required */,
        "" /* default */, "file" /* type */, cmd);
    TCLAP::ValueArg<string> inputArg("i", "input", "Path to input dictionary",
                                     true /* required */, "" /* default */,
                                     "file" /* type */, cmd);
    cmd.parse(argc, argv);
    ConvertDictionary(inputArg.getValue(), outputArg.getValue(),
                      fromArg.getValue(), toArg.getValue());
  } catch (TCLAP::ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId()
              << std::endl;
  } catch (Exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
