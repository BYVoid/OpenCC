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
#include <string_view>

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
#include "src/ConversionAmbiguities.hpp"
#include "src/ConversionInspection.hpp"
#include "src/Converter.hpp"
#include "src/Exception.hpp"
#include "src/ResourceProvider.hpp"
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
  Ambiguities,
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
        << "   s2hkp.json  Simplified Chinese to Traditional Chinese (Hong Kong variant, with Hong Kong Phrases)"
        << std::endl
        << "   hk2sp.json  Traditional Chinese (Hong Kong variant) to Simplified Chinese (Mainland China Phrases)"
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
        << "   t2jp.json   Old Japanese Kanji (Kyūjitai) to New Japanese Kanji (Shinjitai)"
        << std::endl
        << "   jp2t.json   New Japanese Kanji (Shinjitai) to Old Japanese Kanji (Kyūjitai)"
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
bool variationSelectorWarningShown = false;
std::string variationSelectorScanTail;

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
  case OutputMode::Ambiguities:
    writer.String("ambiguities");
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

void PrintInteractiveStdinHint() {
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
}

void PrintVariationSelectorWarningOnce() {
  if (variationSelectorWarningShown) {
    return;
  }
  variationSelectorWarningShown = true;
  fprintf(stderr,
          "warning: input contains Unicode variation selectors (possible IVS); "
          "conversion results may be inaccurate.\n");
}

void WarnIfTextContainsVariationSelector(const char* input, size_t length) {
  if (variationSelectorWarningShown) {
    return;
  }
  if (UTF8Util::ContainsVariationSelector(input, length)) {
    PrintVariationSelectorWarningOnce();
  }
}

void WarnIfStreamChunkContainsVariationSelector(const char* input,
                                                size_t length) {
  if (variationSelectorWarningShown) {
    return;
  }

  std::string scan = variationSelectorScanTail;
  scan.append(input, length);
  if (UTF8Util::ContainsVariationSelector(scan.data(), scan.size())) {
    PrintVariationSelectorWarningOnce();
    variationSelectorScanTail.clear();
    return;
  }

  const size_t maxVariationSelectorUtf8Bytes = 4;
  const size_t keepBytes = scan.size() < maxVariationSelectorUtf8Bytes
                               ? scan.size()
                               : maxVariationSelectorUtf8Bytes;
  variationSelectorScanTail.assign(scan.data() + scan.size() - keepBytes,
                                   keepBytes);
}

// Serializes the segmentation-only view of an inspection result as a JSON
// object with "input" and "segments" fields. Used with --segmentation mode.
std::string SerializeSegmentationResultJson(
    const ConversionInspectionResult& result) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  writer.StartObject();
  writer.Key("input");
  writer.String(result.input.c_str(), result.input.size());
  writer.Key("segments");
  writer.StartArray();
  for (const auto& seg : result.segments) {
    writer.String(seg.c_str(), seg.size());
  }
  writer.EndArray();
  writer.EndObject();
  return buffer.GetString();
}

// Writes a ConversionInspectionResult as a JSON object into writer.
// Handles both SingleStageConverter results (segments + stages) and
// PipelineConverter results (pipelineStages), and is called recursively
// for nested pipeline stages.
template <typename Writer>
void WriteInspectionResultJson(Writer& writer,
                               const ConversionInspectionResult& result) {
  writer.StartObject();
  writer.Key("input");
  writer.String(result.input.c_str(), result.input.size());
  writer.Key("segments");
  writer.StartArray();
  for (const auto& seg : result.segments) {
    writer.String(seg.c_str(), seg.size());
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
      writer.String(seg.c_str(), seg.size());
    }
    writer.EndArray();
    writer.EndObject();
  }
  writer.EndArray();
  writer.Key("pipelineStages");
  writer.StartArray();
  for (const auto& ps : result.pipelineStages) {
    WriteInspectionResultJson(writer, ps);
  }
  writer.EndArray();
  writer.Key("output");
  writer.String(result.output.c_str(), result.output.size());
  writer.EndObject();
}

// Serializes the full inspection result as a JSON object. Used with
// --inspect mode. Handles both single-stage and pipeline converters.
std::string SerializeInspectionResultJson(
    const ConversionInspectionResult& result) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  WriteInspectionResultJson(writer, result);
  return buffer.GetString();
}

std::string ConvertLineByMode(const std::string& line) {
  WarnIfTextContainsVariationSelector(line.c_str(), line.size());
  const auto convertStart = std::chrono::steady_clock::now();
  std::string output;
  if (outputMode == OutputMode::Segmentation) {
    // True segmentation-only path: call the segmenter directly without
    // running any conversion stage.
    const SegmentsPtr& segments =
        converter->GetSegmentation()->Segment(std::string_view(line));
    ConversionInspectionResult result;
    result.input = line;
    result.segments = segments->ToVector();
    output = SerializeSegmentationResultJson(result);
  } else if (outputMode == OutputMode::Inspect) {
    const ConversionInspectionResult& result = converter->Inspect(line);
    output = SerializeInspectionResultJson(result);
  } else {
    output = converter->Convert(std::string_view(line));
  }
  measurement.convertMs += DurationToMilliseconds(
      std::chrono::steady_clock::now() - convertStart);
  return output;
}

// Writes one define-on-first-use JSONL record. Record kinds:
//   {"def":"<source>"}            defines the next global source index
//   {"lit":"<text>"}              literal (unambiguous) output run
//   {"amb":{"t":"<text>","s":N}}  ambiguous span; N is a global source index
//   {"end":{...}}                 stream summary
// Returns the number of bytes written to fout, so callers can account for
// the actual emitted size (record framing and JSON escaping included).
size_t WriteAmbiguityChunkRecords(const AmbiguityStream::Chunk& chunk,
                                  FILE* fout) {
  rapidjson::StringBuffer buffer;
  size_t bytesWritten = 0;
  auto flushRecord = [&buffer, fout, &bytesWritten]() {
    fputs(buffer.GetString(), fout);
    fputc('\n', fout);
    bytesWritten += buffer.GetSize() + 1;
    buffer.Clear();
  };
  for (const std::string& source : chunk.newSources) {
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key("def");
    writer.String(source.c_str(), source.size());
    writer.EndObject();
    flushRecord();
  }
  size_t consumed = 0;
  auto writeLiteral = [&](size_t until) {
    if (until > consumed) {
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      writer.StartObject();
      writer.Key("lit");
      writer.String(chunk.output.c_str() + consumed, until - consumed);
      writer.EndObject();
      flushRecord();
    }
  };
  for (const auto& span : chunk.ambiguities) {
    writeLiteral(span.outputOffset);
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key("amb");
    writer.StartObject();
    writer.Key("t");
    writer.String(chunk.output.c_str() + span.outputOffset,
                  span.outputLength);
    writer.Key("s");
    writer.Uint64(span.sourceIndex);
    writer.EndObject();
    writer.EndObject();
    flushRecord();
    consumed = span.outputOffset + span.outputLength;
  }
  writeLiteral(chunk.output.size());
  return bytesWritten;
}

// Streaming --ambiguities over any input stream (file or stdin): bounded
// memory regardless of line length or segmentation, emitting
// define-on-first-use records. This is the only --ambiguities output
// format, so consumers see one schema no matter how the tool is invoked.
void ConvertAmbiguitiesStream(FILE* fin, FILE* fout) {
  const int BUFFER_SIZE = 1024 * 1024;
  std::string buffer(BUFFER_SIZE, '\0');
  AmbiguityStream stream(converter);
  // convertedBytes counts the underlying converted text (reported in the
  // end record); measurement.outputBytes gets the bytes actually written,
  // consistent with every other output mode.
  size_t convertedBytes = 0;
  size_t ambiguityCount = 0;

  auto emitChunk = [&](const AmbiguityStream::Chunk& chunk) {
    convertedBytes += chunk.output.size();
    ambiguityCount += chunk.ambiguities.size();
    const auto writeStart = std::chrono::steady_clock::now();
    measurement.outputBytes += WriteAmbiguityChunkRecords(chunk, fout);
    if (!noFlush) {
      fflush(fout);
    }
    measurement.writeMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - writeStart);
  };

  bool finished = false;
  bool readError = false;
  while (!feof(fin)) {
    size_t length = fread(&buffer[0], sizeof(char), buffer.size(), fin);
    if (length == 0) {
      readError = ferror(fin) != 0;
      break;
    }
    measurement.inputBytes += length;
    WarnIfStreamChunkContainsVariationSelector(buffer.data(), length);

    const auto convertStart = std::chrono::steady_clock::now();
    const bool isFinalChunk = length < buffer.size() && feof(fin);
    const AmbiguityStream::Chunk chunk =
        isFinalChunk ? stream.Finish({buffer.data(), length})
                     : stream.ConvertChunk({buffer.data(), length});
    measurement.convertMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - convertStart);
    emitChunk(chunk);
    if (isFinalChunk) {
      finished = true;
      break;
    }
  }
  if (readError) {
    // Do NOT emit the end record: it is the stream's integrity signal, and
    // emitting it after a failed read would falsely mark a truncated stream
    // as complete.  The missing end record plus the error exit tell the
    // consumer the stream is incomplete.
    throw Exception("Error reading input stream in --ambiguities mode.");
  }
  if (!finished) {
    const auto convertStart = std::chrono::steady_clock::now();
    const AmbiguityStream::Chunk chunk = stream.Finish();
    measurement.convertMs += DurationToMilliseconds(
        std::chrono::steady_clock::now() - convertStart);
    emitChunk(chunk);
  }

  rapidjson::StringBuffer endBuffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(endBuffer);
  writer.StartObject();
  writer.Key("end");
  writer.StartObject();
  writer.Key("output_bytes");
  writer.Uint64(convertedBytes);
  writer.Key("ambiguities");
  writer.Uint64(ambiguityCount);
  writer.Key("sources");
  writer.Uint64(stream.SourceCount());
  writer.EndObject();
  writer.EndObject();
  const auto writeStart = std::chrono::steady_clock::now();
  fputs(endBuffer.GetString(), fout);
  fputc('\n', fout);
  measurement.outputBytes += endBuffer.GetSize() + 1;
  if (!noFlush) {
    fflush(fout);
  }
  measurement.writeMs += DurationToMilliseconds(
      std::chrono::steady_clock::now() - writeStart);
}

void ConvertStream(FILE* fin, FILE* fout) {
  const int BUFFER_SIZE = 1024 * 1024;
  std::string buffer(BUFFER_SIZE, '\0');
  ConverterStream stream(converter);

  while (!feof(fin)) {
    size_t length = fread(&buffer[0], sizeof(char), buffer.size(), fin);
    if (length == 0) {
      break;
    }
    measurement.inputBytes += length;
    WarnIfStreamChunkContainsVariationSelector(buffer.data(), length);

    const auto convertStart = std::chrono::steady_clock::now();
    const bool isFinalChunk = length < buffer.size() && feof(fin);
    const std::string converted =
        isFinalChunk ? stream.Finish({buffer.data(), length})
                     : stream.ConvertChunk({buffer.data(), length});
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
    if (isFinalChunk) {
      return;
    }
  }

  const auto convertStart = std::chrono::steady_clock::now();
  const std::string& converted = stream.Finish();
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
}

void ConvertLineByLine() {
  PrintInteractiveStdinHint();
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
    if (outputMode == OutputMode::Ambiguities) {
      // Falls through to the shared fclose epilogue below; an early return
      // here would leak both streams and break --in-place, which replaces
      // the output file after this function and requires it to be closed.
      ConvertAmbiguitiesStream(fin, fout);
    } else if (outputMode == OutputMode::Segmentation ||
               outputMode == OutputMode::Inspect) {
      // Inspect/segmentation modes process line by line using
      // std::getline to handle arbitrarily long lines.
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
  PrintInteractiveStdinHint();
  SetBinaryMode(stdin);
  FILE* fout = GetOutputStream();
  if (outputMode == OutputMode::Ambiguities) {
    ConvertAmbiguitiesStream(stdin, fout);
  } else {
    ConvertStream(stdin, fout);
  }
  fclose(fout);
}

namespace opencc {
namespace tools {

int CommandLineMain(std::vector<std::string> args) {
  try {
    const auto totalStart = std::chrono::steady_clock::now();
    TCLAP::CmdLine cmd("Open Chinese Convert (OpenCC) Command Line Tool", ' ',
                       OPENCC_VERSION);
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
    TCLAP::SwitchArg ambiguitiesArg(
        "", "ambiguities",
        "Convert while streaming JSONL records that mark one-to-many "
        "(ambiguous) conversion spans.",
        cmd, false);
    TCLAP::SwitchArg includeTofuRiskDictionariesArg(
        "", "include-tofu-risk-dictionaries",
        "Include dictionaries marked as possibly outputting tofu, i.e. "
        "Chinese characters that may render as missing-glyph boxes. By "
        "default, the command line tool skips these dictionaries.",
        cmd, false);
    const std::string argv0String = args.empty() ? std::string() : args[0];
    Optional<std::string> resourceZipFileName =
        Optional<std::string>::Null();
    std::vector<std::string> visibleArgs;
    if (!args.empty()) {
      visibleArgs.push_back(args[0]);
    }
    for (size_t i = 1; i < args.size(); i++) {
      const std::string& arg = args[i];
      std::string value;
      if (arg == "--resource-zip") {
        if (i + 1 >= args.size() || args[i + 1].empty() ||
            args[i + 1][0] == '-') {
          std::cerr << "error: Missing value for " << arg << std::endl;
          return 1;
        }
        value = args[++i];
      } else if (arg.rfind("--resource-zip=", 0) == 0) {
        value = arg.substr(std::string("--resource-zip=").size());
      } else {
        visibleArgs.push_back(arg);
        continue;
      }
      if (value.empty()) {
        std::cerr << "error: Missing value for " << arg << std::endl;
        return 1;
      }
      if (!resourceZipFileName.IsNull()) {
        std::cerr << "error: resource zip specified more than once."
                  << std::endl;
        return 1;
      }
      resourceZipFileName = Optional<std::string>(value);
    }
    cmd.parse(visibleArgs);

    // Validate mutual exclusion and dependencies
    if (segmentationArg.getValue() + inspectArg.getValue() +
            ambiguitiesArg.getValue() >
        1) {
      std::cerr << "error: --segmentation, --inspect and --ambiguities are "
                   "mutually exclusive."
                << std::endl;
      return 1;
    }

    if (segmentationArg.getValue()) {
      outputMode = OutputMode::Segmentation;
    } else if (inspectArg.getValue()) {
      outputMode = OutputMode::Inspect;
    } else if (ambiguitiesArg.getValue()) {
      outputMode = OutputMode::Ambiguities;
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
    ConfigLoadOptions configOptions;
    configOptions.includeTofuRiskDictionaries =
        includeTofuRiskDictionariesArg.getValue();
    if (!resourceZipFileName.IsNull()) {
      std::shared_ptr<ResourceProvider> provider(
          new ZipResourceProvider(resourceZipFileName.Get()));
      converter = config.NewFromFile(configFileName, provider, configOptions);
    } else {
      converter =
          config.NewFromFile(configFileName, pathArg.getValue(), argv0,
                             configOptions);
    }
    measurement.loadMs +=
        DurationToMilliseconds(std::chrono::steady_clock::now() - loadStart);
    if (outputMode == OutputMode::Segmentation &&
        converter->GetSegmentation() == nullptr) {
      std::cerr << "error: this configuration has no segmentation step; "
                   "--segmentation is not supported.\n";
      return 1;
    }
    if (outputMode == OutputMode::Ambiguities &&
        converter->GetConversionChain() == nullptr) {
      std::cerr << "error: this configuration has no single conversion "
                   "chain; --ambiguities is not supported.\n";
      return 1;
    }
    bool lineByLine = inputFileName.IsNull();
    measurement.lineByLine = lineByLine;
    measurement.outputMode = outputMode;
    if (lineByLine && (outputMode == OutputMode::Convert ||
                       outputMode == OutputMode::Ambiguities)) {
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
