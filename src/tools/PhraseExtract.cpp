/*
 * Open Chinese Convert
 *
 * Copyright 2015 Carbo Kuo <byvoid@byvoid.com>
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

#include <fstream>

#include "CmdLineOutput.hpp"
#include "PhraseExtract.hpp"

using opencc::Exception;
using opencc::PhraseExtract;
using opencc::UTF8StringSlice;

void Extract(const std::vector<std::string>& inputFiles,
             const std::string& outputFile) {
  std::ostringstream buffer;
  for (const auto& inputFile : inputFiles) {
    std::ifstream ifs(inputFile);
    const std::string contents((std::istreambuf_iterator<char>(ifs)),
                               (std::istreambuf_iterator<char>()));
    buffer << contents;
  }
  const std::string& text = buffer.str();
  PhraseExtract extractor;
  extractor.SetWordMaxLength(2);
  extractor.SetPrefixSetLength(1);
  extractor.SetSuffixSetLength(1);
  extractor.Extract(text);
  std::ofstream ofs(outputFile);
  for (const auto& word : extractor.Words()) {
    const PhraseExtract::Signals& signals = extractor.Signal(word);
    const double entropy = signals.prefixEntropy + signals.suffixEntropy;
    const double logProbablity = extractor.LogProbability(word);
    ofs << word << " " << signals.frequency << " " << logProbablity << " "
        << signals.cohesion << " " << entropy << " " << signals.prefixEntropy
        << " " << signals.suffixEntropy << std::endl;
  }
  ofs.close();
}

int main(int argc, const char* argv[]) {
  try {
    TCLAP::CmdLine cmd("Open Chinese Convert (OpenCC) Phrase Extractor", ' ',
                       VERSION);
    CmdLineOutput cmdLineOutput;
    cmd.setOutput(&cmdLineOutput);
    TCLAP::UnlabeledMultiArg<std::string> fileNames(
        "fileName", "Input files", true /* required */, "files");
    cmd.add(fileNames);
    TCLAP::ValueArg<std::string> outputArg(
        "o", "output", "Output file", true /* required */, "" /* default */,
        "file" /* type */, cmd);
    cmd.parse(argc, argv);
    Extract(fileNames.getValue(), outputArg.getValue());
  } catch (TCLAP::ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId()
              << std::endl;
  } catch (Exception& e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
