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

using namespace opencc;

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

  std::ofstream ofs(measuredResultFileName.Get().c_str(), std::ios::binary);
  if (!ofs.is_open()) {
    throw FileNotWritable(measuredResultFileName.Get());
  }
  ofs << buffer.GetString() << std::endl;
}

FILE* GetOutputStream() {
  if (outputFileName.IsNull()) {
    SetBinaryMode(stdout);
    return stdout;
  } else {
    FILE* fp = fopen(outputFileName.Get().c_str(), "wb");
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

void ConvertFile(std::string fileName) {
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

  FILE* fin = fopen(fileName.c_str(), "rb");
  if (!fin) {
    throw FileNotFound(fileName);
  }
  FILE* fout = GetOutputStream();

  if (outputMode == OutputMode::Segmentation ||
      outputMode == OutputMode::Inspect) {
    // Inspect/segmentation modes process line by line using std::getline to
    // handle arbitrarily long lines.
    std::ifstream inputStream(fileName, std::ios::binary);
    fclose(fin);
    fin = nullptr;
    bool isFirstLine = true;
    std::string line;
    while (std::getline(inputStream, line)) {
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
    fclose(fout);
    if (needToRemove) {
      std::remove(fileName.c_str());
    }
    return;
  }

  ConvertStream(fin, fout);
  fclose(fin);
  fclose(fout);
  if (needToRemove) {
    // Remove temporary file.
    std::remove(fileName.c_str());
  }
}

void ConvertStdin() {
  SetBinaryMode(stdin);
  FILE* fout = GetOutputStream();
  ConvertStream(stdin, fout);
  fclose(fout);
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
    TCLAP::SwitchArg segmentationArg(
        "", "segmentation",
        "Output segmentation result as JSON instead of converted text.", cmd,
        false);
    TCLAP::SwitchArg inspectArg(
        "", "inspect",
        "Output full inspection result (segmentation + per-stage conversion + "
        "final output) as JSON.",
        cmd, false);
    cmd.parse(argc, argv);

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
