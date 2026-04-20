/*
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
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

#include <cstring>
#include <chrono>
#include <fstream>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "src/CmdLineOutput.hpp"
#include "src/Config.hpp"
#include "src/Converter.hpp"
#include "src/UTF8Util.hpp"

using namespace opencc;

class OpenCCOutput : public CmdLineOutput {
public:
  void usage(TCLAP::CmdLineInterface& cmd) override {
    CmdLineOutput::usage(cmd);
    std::cout
        << "Built-in Configurations:" << std::endl
        << std::endl
        << "   s2t.json    Simplified Chinese to Traditional Chinese"
        << std::endl
        << "   t2s.json    Traditional Chinese to Simplified Chinese"
        << std::endl
        << "   s2tw.json   Simplified Chinese to Traditional Chinese"
           " (Taiwan Standard)"
        << std::endl
        << "   tw2s.json   Traditional Chinese (Taiwan Standard) to"
           " Simplified Chinese"
        << std::endl
        << "   s2hk.json   Simplified Chinese to Traditional Chinese"
           " (Hong Kong variant)"
        << std::endl
        << "   hk2s.json   Traditional Chinese (Hong Kong variant) to"
           " Simplified Chinese"
        << std::endl
        << "   s2twp.json  Simplified Chinese to Traditional Chinese"
           " (Taiwan Standard) with Taiwanese idiom"
        << std::endl
        << "   tw2sp.json  Traditional Chinese (Taiwan Standard) to"
           " Simplified Chinese with Mainland Chinese idiom"
        << std::endl
        << "   tw2t.json   Traditional Chinese (Taiwan Standard) to"
           " Traditional Chinese (OpenCC Standard)"
        << std::endl
        << "   t2tw.json   Traditional Chinese (OpenCC Standard) to"
           " Taiwan Standard"
        << std::endl
        << "   hk2t.json   Traditional Chinese (Hong Kong variant) to"
           " Traditional Chinese (OpenCC Standard)"
        << std::endl
        << "   t2hk.json   Traditional Chinese (OpenCC Standard) to"
           " Hong Kong variant"
        << std::endl
        << "   t2jp.json   Traditional Chinese Characters (Kyujitai) to"
           " New Japanese Kanji (Shinjitai)"
        << std::endl
        << "   jp2t.json   New Japanese Kanji (Shinjitai) to Traditional"
           " Chinese Characters (Kyujitai) (OpenCC Standard)"
        << std::endl
        << std::endl;
  }
};

Optional<std::string> inputFileName = Optional<std::string>::Null();
Optional<std::string> outputFileName = Optional<std::string>::Null();
Optional<std::string> measuredResultFileName = Optional<std::string>::Null();
std::string configFileName;
bool noFlush;
Config config;
ConverterPtr converter;

struct MeasurementResult {
  double loadMs = 0.0;
  double convertMs = 0.0;
  double writeMs = 0.0;
  double totalMs = 0.0;
  size_t inputBytes = 0;
  size_t outputBytes = 0;
  bool lineByLine = false;
};

MeasurementResult measurement;

double DurationToMilliseconds(
    const std::chrono::steady_clock::duration& duration) {
  return std::chrono::duration<double, std::milli>(duration).count();
}

void WriteMeasuredResult() {
  if (measuredResultFileName.IsNull()) {
    return;
  }

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
  writer.StartObject();
  writer.Key("config");
  writer.String(configFileName.c_str());
  writer.Key("input");
  if (inputFileName.IsNull()) {
    writer.Null();
  } else {
    writer.String(inputFileName.Get().c_str());
  }
  writer.Key("output");
  if (outputFileName.IsNull()) {
    writer.Null();
  } else {
    writer.String(outputFileName.Get().c_str());
  }
  writer.Key("mode");
  writer.String(measurement.lineByLine ? "line_by_line" : "file");
  writer.Key("load_ms");
  writer.Double(measurement.loadMs);
  writer.Key("convert_ms");
  writer.Double(measurement.convertMs);
  writer.Key("write_ms");
  writer.Double(measurement.writeMs);
  writer.Key("total_ms");
  writer.Double(measurement.totalMs);
  writer.Key("input_bytes");
  writer.Uint64(measurement.inputBytes);
  writer.Key("output_bytes");
  writer.Uint64(measurement.outputBytes);
  writer.EndObject();

  std::ofstream ofs(measuredResultFileName.Get().c_str(), std::ios::binary);
  if (!ofs.is_open()) {
    throw FileNotWritable(measuredResultFileName.Get());
  }
  ofs << buffer.GetString() << std::endl;
}

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
#ifdef _WIN32
  if (_isatty(_fileno(stdin))) {
    fprintf(stderr,
            "Reading from standard input. Press Ctrl+Z then Enter to "
            "finish.\n");
  }
#else
  if (isatty(fileno(stdin))) {
    fprintf(stderr,
            "Reading from standard input. Press Ctrl+D to finish.\n");
  }
#endif
  std::istream& inputStream = std::cin;
  FILE* fout = GetOutputStream();
  bool isFirstLine = true;
  while (!inputStream.eof()) {
    if (!isFirstLine) {
      fputs("\n", fout);
    } else {
      isFirstLine = false;
    }
    std::string line;
    std::getline(inputStream, line);
    measurement.inputBytes += line.size();
    const auto convertStart = std::chrono::steady_clock::now();
    const std::string& converted = converter->Convert(line);
    measurement.convertMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - convertStart);
    measurement.outputBytes += converted.size();
    const auto writeStart = std::chrono::steady_clock::now();
    fputs(converted.c_str(), fout);
    if (!noFlush) {
      // Flush every line if the output stream is stdout.
      fflush(fout);
    }
    measurement.writeMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - writeStart);
  }
  fclose(fout);
}

void Convert(std::string fileName) {
  const int BUFFER_SIZE = 1024 * 1024;
  static bool bufferInitialized = false;
  static std::string buffer;
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
  if (!outputFileName.IsNull() && fileName == outputFileName.Get()) {
    // Special case: input == output
    std::ifstream src(fileName, std::ios::binary);
#ifdef _WIN32
    const std::string tempFileName = std::tmpnam(nullptr);
#else
    // std::tmpnam is deprecated
    std::string tempFileName;
    const char* tmpDirEnv = std::getenv("TMPDIR");
    if (tmpDirEnv != nullptr) {
      tempFileName = tmpDirEnv;
    }
#ifdef P_tmpdir
    if (tempFileName.empty()) {
      tempFileName = P_tmpdir;
    }
#endif
    if (tempFileName.empty()) {
      tempFileName = "/tmp";
    }
    tempFileName += "/openccXXXXXX";
    int fd = mkstemp(const_cast<char*>(tempFileName.c_str()));
    if (fd == 0) {
      throw FileNotWritable(tempFileName);
    }
#endif
    std::ofstream dst(tempFileName, std::ios::binary);
    dst << src.rdbuf();
    dst.close();
    fileName = tempFileName;
    needToRemove = true;
  }

  FILE* fin = fopen(fileName.c_str(), "r");
  if (!fin) {
    throw FileNotFound(fileName);
  }
  FILE* fout = GetOutputStream();
  while (!feof(fin)) {
    size_t length = fread(bufferPtr, sizeof(char), bufferSizeAvailble, fin);
    measurement.inputBytes += length;
    bufferPtr[length] = '\0';
    size_t remainingLength = 0;
    std::string remainingTemp;
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
    const auto convertStart = std::chrono::steady_clock::now();
    const std::string& converted = converter->Convert(buffer);
    measurement.convertMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - convertStart);
    measurement.outputBytes += converted.size();
    const auto writeStart = std::chrono::steady_clock::now();
    fputs(converted.c_str(), fout);
    if (!noFlush) {
      // Flush every line if the output stream is stdout.
      fflush(fout);
    }
    measurement.writeMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - writeStart);
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
    std::remove(fileName.c_str());
  }
}

int main(int argc, const char* argv[]) {
  try {
    const auto totalStart = std::chrono::steady_clock::now();
    TCLAP::CmdLine cmd("Open Chinese Convert (OpenCC) Command Line Tool", ' ',
                       VERSION);
    OpenCCOutput cmdLineOutput;
    cmd.setOutput(&cmdLineOutput);

    TCLAP::ValueArg<std::string> configArg(
        "c", "config", "Configuration file", false /* required */,
        "s2t.json" /* default */, "file" /* type */, cmd);
    TCLAP::ValueArg<std::string> outputArg(
        "o", "output", "Write converted text to <file>.", false /* required */,
        "" /* default */, "file" /* type */, cmd);
    TCLAP::ValueArg<std::string> inputArg(
        "i", "input", "Read original text from <file>.", false /* required */,
        "" /* default */, "file" /* type */, cmd);
    TCLAP::ValueArg<bool> noFlushArg(
        "", "noflush", "Disable flush for every line", false /* required */,
        false /* default */, "bool" /* type */, cmd);
    TCLAP::MultiArg<std::string> pathArg(
        "", "path", "Additional paths to locate config and dictionary files.",
        false /* required */, "file" /* type */, cmd);
    TCLAP::ValueArg<std::string> measuredResultArg(
        "", "measured_result",
        "Write measured timing results as JSON to <file>.", false /* required */,
        "" /* default */, "file" /* type */, cmd);
    cmd.parse(argc, argv);
    configFileName = configArg.getValue();
    noFlush = noFlushArg.getValue();
    if (measuredResultArg.isSet()) {
      measuredResultFileName =
          Optional<std::string>(measuredResultArg.getValue());
    }
    if (inputArg.isSet()) {
      inputFileName = Optional<std::string>(inputArg.getValue());
    }
    if (outputArg.isSet()) {
      outputFileName = Optional<std::string>(outputArg.getValue());
      noFlush = true;
    }
    const auto loadStart = std::chrono::steady_clock::now();
    converter = config.NewFromFile(configFileName, pathArg.getValue(), argv[0]);
    measurement.loadMs +=
        DurationToMilliseconds(std::chrono::steady_clock::now() - loadStart);
    bool lineByLine = inputFileName.IsNull();
    measurement.lineByLine = lineByLine;
    if (lineByLine) {
      ConvertLineByLine();
    } else {
      Convert(inputFileName.Get());
    }
    measurement.totalMs +=
        DurationToMilliseconds(std::chrono::steady_clock::now() - totalStart);
    WriteMeasuredResult();
  } catch (TCLAP::ArgException& e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId()
              << std::endl;
    return 1;
  } catch (Exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
