/*
 * Open Chinese Convert
 *
 * Copyright 2015 BYVoid <byvoid@byvoid.com>
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
#include "PhraseExtract.hpp"

using opencc::Exception;
using opencc::UTF8StringSlice;
using opencc::PhraseExtract;

void Extract(const vector<string>& inputFiles, const string& outputFile) {
  std::ostringstream buffer;
  for (const auto& inputFile : inputFiles) {
    std::ifstream ifs(inputFile);
    const string contents((std::istreambuf_iterator<char>(ifs)),
                          (std::istreambuf_iterator<char>()));
    buffer << contents;
  }
  const string& text = buffer.str();
  PhraseExtract phraseExtract;
  phraseExtract.SetWordMaxLength(2);
  phraseExtract.SetPrefixSetLength(1);
  phraseExtract.SetSuffixSetLength(1);
  phraseExtract.SetPreCalculationFilter(
      [](const PhraseExtract& phraseExtract, const UTF8StringSlice& word) {
        return word.UTF8Length() < 2;
      });
  phraseExtract.SetPostCalculationFilter(
      [](const PhraseExtract& phraseExtract, const UTF8StringSlice& word) {
        const double cohesion = phraseExtract.Cohesion(word);
        const double entropy = phraseExtract.Entropy(word);
        const double suffixEntropy = phraseExtract.SuffixEntropy(word);
        const double prefixEntropy = phraseExtract.PrefixEntropy(word);
        bool accept = cohesion >= 3.5 && entropy >= 3.3 &&
                      prefixEntropy >= 0.5 && suffixEntropy >= 0.5;
        return !accept;
      });

  phraseExtract.SetFullText(text);
  phraseExtract.SelectWords();
  std::ofstream ofs(outputFile);
  for (const auto& word : phraseExtract.Words()) {
    const size_t frequency = phraseExtract.Frequency(word);
    const double cohesion = phraseExtract.Cohesion(word);
    const double suffixEntropy = phraseExtract.SuffixEntropy(word);
    const double prefixEntropy = phraseExtract.PrefixEntropy(word);
    const double entropy = phraseExtract.Entropy(word);
    ofs << word.ToString() << " " << frequency << " " << cohesion << " "
        << entropy << " " << prefixEntropy << " " << suffixEntropy << std::endl;
  }
  ofs.close();
}

int main(int argc, const char* argv[]) {
  try {
    TCLAP::CmdLine cmd("Open Chinese Convert (OpenCC) Phrase Extractor", ' ',
                       VERSION);
    CmdLineOutput cmdLineOutput;
    cmd.setOutput(&cmdLineOutput);
    TCLAP::UnlabeledMultiArg<string> fileNames("fileName", "Input files",
                                               true /* required */, "files");
    cmd.add(fileNames);
    TCLAP::ValueArg<string> outputArg("o", "output", "Output file",
                                      true /* required */, "" /* default */,
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
