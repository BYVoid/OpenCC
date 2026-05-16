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

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "src/CmdLineOutput.hpp"
#include "src/Config.hpp"
#include "src/ConversionInspection.hpp"
#include "src/Converter.hpp"
#include "src/Segments.hpp"
#include "src/UTF8Util.hpp"
#include "src/tools/CommandLineMain.hpp"
#include "src/tools/PlatformIO.hpp"

using namespace opencc;
using namespace opencc::tools;

enum class OutputMode {
  Convert,
  Segmentation,
  Inspect,
};

class OpenCCOutput : public CmdLineOutput {
public:
  void usage(TCLAP::CmdLineInterface& cmd) override {
    CmdLineOutput::usage(cmd);
    std::cout
        << "Built-in Configurations:" << std::endl
        << std::endl
        << "   s2t.json    Simplified Chinese to Traditional Chinese (OpenCC Standard)"
        << std::endl
        << "   t2s.json    Traditional Chinese (OpenCC Standard) to Simplified Chinese"
        << std::endl
        << "   s2tw.json   Simplified Chinese to Traditional Chinese (Taiwan Standard)"
        << std::endl
        << "   tw2s.json   Traditional Chinese (Taiwan Standard) to Simplified Chinese"
        << std::endl
        << "   s2hk.json   Simplified Chinese to Traditional Chinese (Hong Kong variant)"
        << std::endl
        << "   hk2s.json   Traditional Chinese (Hong Kong variant) to Simplified Chinese"
        << std::endl
        << "   s2twp.json  Simplified Chinese to Traditional Chinese (Taiwan Standard, with Taiwan Phrases)"
        << std::endl
        << "   tw2sp.json  Traditional Chinese (Taiwan Standard) to Simplified Chinese (Mainland China Phrases)"
        << std::endl
        << "   tw2t.json   Traditional Chinese (Taiwan Standard) to Traditional Chinese (OpenCC Standard)"
        << std::endl
        << "   t2tw.json   Traditional Chinese (OpenCC Standard) to Traditional Chinese (Taiwan Standard)"
        << std::endl
        << "   hk2t.json   Traditional Chinese (Hong Kong variant) to Traditional Chinese (OpenCC Standard)"
        << std::endl
        << "   t2hk.json   Traditional Chinese (OpenCC Standard) to Traditional Chinese (Hong Kong variant)"
        << std::endl
        << "   t2jp.json   Traditional Chinese Characters (Kyūjitai) to New Japanese Kanji (Shinjitai)"
        << std::endl
        << "   jp2t.json   New Japanese Kanji (Shinjitai) to Traditional Chinese Characters (Kyūjitai)"
        << std::endl
        << std::endl;
  }
};

Optional<std::string> inputFileName = Optional<std::string>::Null();
Optional<std::string> outputFileName = Optional<std::string>::Null();
Optional<std::string> measuredResultFileName = Optional<std::string>::Null();
std::string configFileName;
bool noFlush;
bool inPlace = false;
OutputMode outputMode = OutputMode::Convert;
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
  OutputMode outputMode = OutputMode::Convert;
};

MeasurementResult measurement;

bool ReadLine(FILE* fp, std::string* line) {
  line->clear();
  int ch;
  while ((ch = fgetc(fp)) != EOF) {
    if (ch == '\n') {
      return true;
    }
    line->push_back(static_cast<char>(ch));
  }
  return !line->empty();
}

void CopyFileContents(FILE* src, FILE* dst, const std::string& srcFileName,
                      const std::string& dstFileName) {
  const size_t BUFFER_SIZE = 1024 * 1024;
  std::string buffer(BUFFER_SIZE, '\0');
  while (true) {
    const size_t length = fread(&buffer[0], sizeof(char), buffer.size(), src);
    if (length > 0) {
      const size_t written = fwrite(buffer.data(), sizeof(char), length, dst);
      if (written != length) {
        throw FileNotWritable(dstFileName);
      }
    }
    if (length < buffer.size()) {
      if (ferror(src)) {
        throw FileNotFound(srcFileName);
      }
      break;
    }
  }
}

void SetBinaryMode(FILE* fp) {
#ifdef _WIN32
  _setmode(_fileno(fp), _O_BINARY);
#else
  (void)fp;
#endif
}

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
  writer.Key("output_mode");
  switch (measurement.outputMode) {
  case OutputMode::Segmentation:
    writer.String("segmentation");
    break;
  case OutputMode::Inspect:
    writer.String("inspect");
    break;
  default:
    writer.String("convert");
    break;
  }
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

  FILE* fp = OpenFileUtf8(measuredResultFileName.Get(), "wb");
  if (!fp) {
    throw FileNotWritable(measuredResultFileName.Get());
  }
  fputs(buffer.GetString(), fp);
  fputc('\n', fp);
  fclose(fp);
}

FILE* GetOutputStream() {
  if (outputFileName.IsNull()) {
    SetBinaryMode(stdout);
    return stdout;
  } else {
    FILE* fp = OpenFileUtf8(outputFileName.Get(), "wb");
    if (!fp) {
      throw FileNotWritable(outputFileName.Get());
    }
    return fp;
  }
}

// Serializes the segmentation-only view of an inspection result as a JSON
// object with "input" and "segments" fields. Used with --segmentation mode.
std::string SerializeSegmentationResultJson(
    const ConversionInspectionResult& result) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  writer.StartObject();
  writer.Key("input");
  writer.String(result.input.c_str());
  writer.Key("segments");
  writer.StartArray();
  for (const auto& seg : result.segments) {
    writer.String(seg.c_str());
  }
  writer.EndArray();
  writer.EndObject();
  return buffer.GetString();
}

// Serializes the full inspection result as a JSON object with "input",
// "segments", "stages", and "output" fields. Used with --inspect mode.
std::string SerializeInspectionResultJson(
    const ConversionInspectionResult& result) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  writer.StartObject();
  writer.Key("input");
  writer.String(result.input.c_str());
  writer.Key("segments");
  writer.StartArray();
  for (const auto& seg : result.segments) {
    writer.String(seg.c_str());
  }
  writer.EndArray();
  writer.Key("stages");
  writer.StartArray();
  for (const auto& stage : result.stages) {
    writer.StartObject();
    writer.Key("index");
    writer.Uint64(stage.index);
    writer.Key("segments");
    writer.StartArray();
    for (const auto& seg : stage.segments) {
      writer.String(seg.c_str());
    }
    writer.EndArray();
    writer.EndObject();
  }
  writer.EndArray();
  writer.Key("output");
  writer.String(result.output.c_str());
  writer.EndObject();
  return buffer.GetString();
}

std::string ConvertLineByMode(const std::string& line) {
  const auto convertStart = std::chrono::steady_clock::now();
  std::string output;
  if (outputMode == OutputMode::Segmentation) {
    // True segmentation-only path: call the segmenter directly without
    // running any conversion stage.
    const SegmentsPtr& segments =
        converter->GetSegmentation()->Segment(line);
    ConversionInspectionResult result;
    result.input = line;
    result.segments = segments->ToVector();
    output = SerializeSegmentationResultJson(result);
  } else if (outputMode == OutputMode::Inspect) {
    const ConversionInspectionResult& result = converter->Inspect(line);
    output = SerializeInspectionResultJson(result);
  } else {
    output = converter->Convert(line);
  }
  measurement.convertMs += DurationToMilliseconds(
      std::chrono::steady_clock::now() - convertStart);
  return output;
}

void ConvertStream(FILE* fin, FILE* fout) {
  const int BUFFER_SIZE = 1024 * 1024;
  const size_t MAX_KEEP_CHARS = 16;
  std::string buffer(BUFFER_SIZE + 1, '\0');
  char* bufferBegin = &buffer[0];
  const char* bufferEnd = bufferBegin + BUFFER_SIZE;
  char* bufferPtr = bufferBegin;
  size_t bufferSizeAvailble = BUFFER_SIZE;

  while (!feof(fin)) {
    size_t length = fread(bufferPtr, sizeof(char), bufferSizeAvailble, fin);
    if (length == 0 && bufferPtr == bufferBegin) {
      break;
    }
    measurement.inputBytes += length;
    bufferPtr[length] = '\0';
    size_t convertLength = (bufferPtr - bufferBegin) + length;
    size_t remainingLength = 0;
    std::string remainingTemp;
    if (length == bufferSizeAvailble) {
      // fread may break a UTF-8 character. Keep the partial character for the
      // next chunk and convert only complete characters.
      char* lastChPtr = bufferBegin;
      while (lastChPtr < bufferEnd) {
        size_t nextCharLen = UTF8Util::NextCharLength(lastChPtr);
        if (lastChPtr + nextCharLen > bufferEnd) {
          break;
        }
        lastChPtr += nextCharLen;
      }

      // Also keep up to MAX_KEEP_CHARS complete characters before the boundary
      // to preserve multi-character phrases that may span across buffer
      // boundaries for correct longest-prefix segmentation.
      char* keepStart = lastChPtr;
      size_t charsKept = 0;
      while (keepStart > bufferBegin && charsKept < MAX_KEEP_CHARS) {
        size_t prevCharLen = UTF8Util::PrevCharLength(keepStart);
        keepStart -= prevCharLen;
        charsKept++;
      }

      convertLength = keepStart - bufferBegin;
      remainingLength = bufferEnd - keepStart;
      if (remainingLength > 0) {
        remainingTemp = UTF8Util::FromSubstr(keepStart, remainingLength);
      }
    }

    // Null-terminate the portion to convert
    bufferBegin[convertLength] = '\0';

    const auto convertStart = std::chrono::steady_clock::now();
    const std::string& converted = converter->Convert(bufferBegin);
    measurement.convertMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - convertStart);
    measurement.outputBytes += converted.size();
    const auto writeStart = std::chrono::steady_clock::now();
    fputs(converted.c_str(), fout);
    if (!noFlush) {
      fflush(fout);
    }
    measurement.writeMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - writeStart);

    bufferPtr = bufferBegin + remainingLength;
    bufferSizeAvailble = BUFFER_SIZE - remainingLength;
    if (remainingLength > 0) {
      memcpy(bufferBegin, remainingTemp.c_str(), remainingLength);
    }
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
  std::string line;
  while (std::getline(inputStream, line)) {
    if (!isFirstLine) {
      fputs("\n", fout);
    } else {
      isFirstLine = false;
    }
    measurement.inputBytes += line.size();
    const std::string& output = ConvertLineByMode(line);
    measurement.outputBytes += output.size();
    const auto writeStart = std::chrono::steady_clock::now();
    fputs(output.c_str(), fout);
    if (!noFlush) {
      // Flush every line if the output stream is stdout.
      fflush(fout);
    }
    measurement.writeMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - writeStart);
  }
  fclose(fout);
}

void ConvertFileStreams(FILE* fin, FILE* fout) {
  try {
    if (outputMode == OutputMode::Segmentation ||
        outputMode == OutputMode::Inspect) {
      // Inspect/segmentation modes process line by line using std::getline to
      // handle arbitrarily long lines.
      bool isFirstLine = true;
      std::string line;
      while (ReadLine(fin, &line)) {
        if (!line.empty() && line.back() == '\r') {
          line.pop_back();
        }
        measurement.inputBytes += line.size();
        if (!isFirstLine) {
          fputs("\n", fout);
        } else {
          isFirstLine = false;
        }
        const std::string& output = ConvertLineByMode(line);
        measurement.outputBytes += output.size();
        const auto writeStart = std::chrono::steady_clock::now();
        fputs(output.c_str(), fout);
        if (!noFlush) {
          fflush(fout);
        }
        measurement.writeMs += DurationToMilliseconds(
            std::chrono::steady_clock::now() - writeStart);
      }
    } else {
      ConvertStream(fin, fout);
    }
  } catch (...) {
    fclose(fin);
    fclose(fout);
    throw;
  }

  fclose(fin);
  fclose(fout);
}

void ConvertFile(std::string fileName) {
  FILE* fin = OpenFileUtf8(fileName, "rb");
  if (!fin) {
    throw FileNotFound(fileName);
  }

  if (!outputFileName.IsNull() &&
      IsSameFileUtf8(fileName, outputFileName.Get())) {
    if (!inPlace) {
      fclose(fin);
      throw Exception("Input and output refer to the same file; use "
                      "--in-place to overwrite it.");
    }

    std::string targetFileName = outputFileName.Get();
    GetRealPathUtf8(outputFileName.Get(), &targetFileName);
    if (HasMultipleLinksUtf8(targetFileName)) {
      fclose(fin);
      throw Exception("--in-place cannot safely update files with multiple "
                      "hard links.");
    }

    std::string tempFileName;
    FILE* fout = CreateTempFileNearUtf8(targetFileName, &tempFileName);
    if (!fout) {
      fclose(fin);
      if (tempFileName.empty()) {
        tempFileName = "temporary file";
      }
      throw FileNotWritable(tempFileName);
    }

    try {
      ConvertFileStreams(fin, fout);
      if (PreserveMetadataUtf8(targetFileName, tempFileName) != 0) {
        RemoveFileUtf8(tempFileName);
        throw FileNotWritable(tempFileName);
      }
      if (ReplaceFileUtf8(tempFileName, targetFileName) != 0) {
        RemoveFileUtf8(tempFileName);
        throw FileNotWritable(targetFileName);
      }
    } catch (...) {
      RemoveFileUtf8(tempFileName);
      throw;
    }
    return;
  }

  FILE* fout = GetOutputStream();
  ConvertFileStreams(fin, fout);
}

void ConvertStdin() {
  SetBinaryMode(stdin);
  FILE* fout = GetOutputStream();
  ConvertStream(stdin, fout);
  fclose(fout);
}

namespace opencc {
namespace tools {

int CommandLineMain(std::vector<std::string> args) {
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
    TCLAP::SwitchArg inPlaceArg(
        "", "in-place",
        "Allow overwriting the input file when input and output refer to the "
        "same file.",
        cmd, false);
    TCLAP::MultiArg<std::string> pathArg(
        "", "path", "Additional paths to locate config and dictionary files.",
        false /* required */, "file" /* type */, cmd);
    TCLAP::ValueArg<std::string> measuredResultArg(
        "", "measured_result",
        "Write measured timing results as JSON to <file>.", false /* required */,
        "" /* default */, "file" /* type */, cmd);
    TCLAP::SwitchArg segmentationArg(
        "", "segmentation",
        "Output segmentation result as JSON instead of converted text.", cmd,
        false);
    TCLAP::SwitchArg inspectArg(
        "", "inspect",
        "Output full inspection result (segmentation + per-stage conversion + "
        "final output) as JSON.",
        cmd, false);
    const std::string argv0String = args.empty() ? std::string() : args[0];
    cmd.parse(args);

    // Validate mutual exclusion and dependencies
    if (segmentationArg.getValue() && inspectArg.getValue()) {
      std::cerr << "error: --segmentation and --inspect are mutually exclusive."
                << std::endl;
      return 1;
    }

    if (segmentationArg.getValue()) {
      outputMode = OutputMode::Segmentation;
    } else if (inspectArg.getValue()) {
      outputMode = OutputMode::Inspect;
    } else {
      outputMode = OutputMode::Convert;
    }

    configFileName = configArg.getValue();
    noFlush = noFlushArg.getValue();
    inPlace = inPlaceArg.getValue();
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
    const char* argv0 = argv0String.empty() ? nullptr : argv0String.c_str();
    converter = config.NewFromFile(configFileName, pathArg.getValue(), argv0);
    measurement.loadMs +=
        DurationToMilliseconds(std::chrono::steady_clock::now() - loadStart);
    bool lineByLine = inputFileName.IsNull();
    measurement.lineByLine = lineByLine;
    measurement.outputMode = outputMode;
    if (lineByLine && outputMode == OutputMode::Convert) {
      ConvertStdin();
    } else if (lineByLine) {
      ConvertLineByLine();
    } else {
      ConvertFile(inputFileName.Get());
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

} // namespace tools
} // namespace opencc
