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

#include <getopt.h>
#include <locale.h>

#include "TextDict.hpp"
#include "DartsDict.hpp"

using namespace Opencc;

void ShowVersion() {
  printf(_("\n"));
  printf(_("Open Chinese Convert (OpenCC) Dictionary Tool\n"));
  printf(_("Version %s\n"), VERSION);
  printf(_("\n"));
  printf(_("Author: %s\n"), "Carbo Kuo <byvoid@byvoid.com>");
  printf(_("Bug Report: %s\n"), "http://github.com/BYVoid/OpenCC/issues");
  printf(_("\n"));
}

void ShowUsage() {
  ShowVersion();
  printf(_("Usage:\n"));
  printf(_("  opencc_dict -i [file] -o [file] -f [format] -t [format]\n"));
  printf(_("\n"));
  printf(_("Options:\n"));
  printf(_(" -i [file], --input=[file]     Read dictionary from [file].\n"));
  printf(_(" -o [file], --output=[file]    Write converted dictionary to [file].\n"));
  printf(_(" -f [format], --from=[format]  Format of input dictionary.\n"));
  printf(_(" -t [format], --to=[format]    Format of output dictionary.\n"));
  printf(_("   Available formats are:\n"));
  printf(_("     text - Plain text format\n"));
  printf(_("     ocd  - Opencc optimized format for fast look up\n"));
  printf(_(" -v, --version                 Print version and build information.\n"));
  printf(_(" -h, --help                    Print this help.\n"));
  printf(_("\n"));
}

SerializableDictPtr CreateDictionary(const string& format) {
  if (format == "text") {
    return SerializableDictPtr(new TextDict);
  } else if (format == "ocd") {
    return SerializableDictPtr(new DartsDict);
  } else {
    fprintf(stderr, _("Unknown dictionary format: %s\n"), format.c_str());
    exit(2);
  }
  return nullptr;
}

void ConvertDictionary(const string& inputFileName, const string& outputFileName, const string& formatFrom, const string& formatTo) {
  SerializableDictPtr dictFrom = CreateDictionary(formatFrom);
  SerializableDictPtr dictTo = CreateDictionary(formatTo);
  dictFrom->LoadFromFile(inputFileName);
  dictTo->LoadFromDict(dictFrom.get());
  dictTo->SerializeToFile(outputFileName);
}

int main(int argc, const char * argv[]) {
#ifdef ENABLE_GETTEXT
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE_NAME, LOCALEDIR);
#endif /* ifdef ENABLE_GETTEXT */
  struct option longopts[] =
  {
    { "version", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { "input", required_argument, NULL, 'i' },
    { "output", required_argument, NULL, 'o' },
    { "from", required_argument, NULL, 'f' },
    { "to", required_argument, NULL, 't' },
    { 0, 0, 0, 0 },
  };
  int oc;
  Optional<string> inputFileName;
  Optional<string> outputFileName;
  Optional<string> formatFrom;
  Optional<string> formatTo;
  while ((oc = getopt_long(argc, (char*const*)argv, "vh?i:o:f:t:", longopts, NULL)) != -1) {
    switch (oc) {
      case 'v':
        ShowVersion();
        return 0;
      case 'h':
      case '?':
        ShowUsage();
        return 0;
      case 'i':
        inputFileName = Optional<string>(optarg);
        break;
      case 'o':
        outputFileName = Optional<string>(optarg);
        break;
      case 'f':
        formatFrom = Optional<string>(optarg);
        break;
      case 't':
        formatTo = Optional<string>(optarg);
        break;
    }
  }
  if (inputFileName.IsNull() || outputFileName.IsNull()) {
    fprintf(stderr, _("Please specify input and output file using -i and -o.\n"));
    ShowUsage();
    return 1;
  }
  if (formatFrom.IsNull() || formatTo.IsNull()) {
    fprintf(stderr, _("Please specify format of input and output using -f and -t.\n"));
    ShowUsage();
    return 1;
  }
  ConvertDictionary(inputFileName.Get(), outputFileName.Get(), formatFrom.Get(), formatTo.Get());
  return 0;
}
