/*
 * Open Chinese Convert
 *
 * Copyright 2020-2026 Carbo Kuo and contributors
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

#include <benchmark/benchmark.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <memory>
#include <ostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <direct.h>
#include <process.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "Exception.hpp"
#include "SimpleConverter.hpp"
#include "TestUtilsUTF8.hpp"

namespace opencc {

namespace {

using JSONDocument = rapidjson::Document;
using JSONValue = rapidjson::Value;

struct BenchmarkConfig {
  std::string name;
  std::string path;
};

#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
void SetPluginSearchPath(const char* path) {
  if (path == nullptr || *path == '\0') {
    return;
  }
#ifdef _WIN32
  _putenv_s("OPENCC_SEGMENTATION_PLUGIN_PATH", path);
#else
  setenv("OPENCC_SEGMENTATION_PLUGIN_PATH", path, 1);
#endif
}
#endif

bool IsRegularFile(const std::string& path) {
  std::ifstream stream(path.c_str(), std::ios::binary);
  return stream.good();
}

bool EndsWith(const std::string& value, const std::string& suffix) {
  return value.size() >= suffix.size() &&
         value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string ReplaceSuffix(const std::string& value, const std::string& from,
                          const std::string& to) {
  if (!EndsWith(value, from)) {
    return value;
  }
  return value.substr(0, value.size() - from.size()) + to;
}

std::string GetProcessIdString() {
#ifdef _WIN32
  return std::to_string(_getpid());
#else
  return std::to_string(getpid());
#endif
}

std::string GetBaseNameWithoutExtension(const std::string& path) {
  const std::string::size_type slash_pos = path.find_last_of("/\\");
  const std::string::size_type start =
      slash_pos == std::string::npos ? 0 : slash_pos + 1;
  const std::string::size_type dot_pos = path.find_last_of('.');
  if (dot_pos == std::string::npos || dot_pos < start) {
    return path.substr(start);
  }
  return path.substr(start, dot_pos - start);
}

std::string BenchmarkTempDirectory() {
  return std::string(PROJECT_BINARY_DIR) + "/src/benchmark/";
}

bool IsAbsolutePath(const std::string& path) {
  if (path.empty()) {
    return false;
  }
#ifdef _WIN32
  return path.size() > 2 &&
         ((path[0] >= 'A' && path[0] <= 'Z') ||
          (path[0] >= 'a' && path[0] <= 'z')) &&
         path[1] == ':' && (path[2] == '/' || path[2] == '\\');
#else
  return path[0] == '/';
#endif
}

std::string JoinPath(const std::string& directory, const std::string& child) {
  if (directory.empty()) {
    return child;
  }
  const char last = directory[directory.size() - 1];
  if (last == '/' || last == '\\') {
    return directory + child;
  }
  return directory + "/" + child;
}

std::string GetParentDirectory(const std::string& path) {
  const std::string::size_type pos = path.find_last_of("/\\");
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(0, pos + 1);
}

std::string ResolveConfigPath(const std::string& config_name) {
#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
  if (config_name == "s2twp_jieba") {
    SetPluginSearchPath(OPENCC_BENCHMARK_JIEBA_PLUGIN_DIR);
    return std::string(OPENCC_BENCHMARK_JIEBA_CONFIG_DIR) + "/" + config_name +
           ".json";
  }
#endif
  return std::string(CMAKE_SOURCE_DIR) + "/data/config/" + config_name + ".json";
}

std::string ResolveTextDictionaryPath(const std::string& dict_file_name) {
  const std::string text_file_name =
      ReplaceSuffix(dict_file_name, ".ocd2", ".txt");
  const std::string source_path =
      std::string(CMAKE_SOURCE_DIR) + "/data/dictionary/" + text_file_name;
  if (IsRegularFile(source_path)) {
    return source_path;
  }

  const std::string generated_path =
      std::string(PROJECT_BINARY_DIR) + "/data/" + text_file_name;
  if (IsRegularFile(generated_path)) {
    return generated_path;
  }

  throw FileNotFound(text_file_name);
}

std::string ResolveSegmentationResourcePath(
    const std::string& source_config_directory, const std::string& resource_path) {
  if (IsAbsolutePath(resource_path)) {
    return resource_path;
  }

#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
  if (resource_path.find("jieba_dict/") == 0) {
    const std::string basename =
        resource_path.substr(std::string("jieba_dict/").size());
    const std::string plugin_build_path =
        JoinPath(std::string(OPENCC_BENCHMARK_JIEBA_PLUGIN_DIR), resource_path);
    if (IsRegularFile(plugin_build_path)) {
      return plugin_build_path;
    }
    const std::string plugin_parent_path =
        JoinPath(GetParentDirectory(OPENCC_BENCHMARK_JIEBA_PLUGIN_DIR), resource_path);
    if (IsRegularFile(plugin_parent_path)) {
      return plugin_parent_path;
    }
    const std::string source_jieba_path =
        std::string(CMAKE_SOURCE_DIR) + "/plugins/jieba/deps/cppjieba/dict/" +
        basename;
    if (IsRegularFile(source_jieba_path)) {
      return source_jieba_path;
    }
  }
#endif

  return JoinPath(source_config_directory, resource_path);
}

void RewriteDictNode(JSONValue& node, JSONDocument::AllocatorType& allocator,
                     const char* target_type) {
  if (!node.IsObject()) {
    return;
  }

  if (node.HasMember("type") && node["type"].IsString() &&
      std::string(node["type"].GetString()) == "group" && node.HasMember("dicts") &&
      node["dicts"].IsArray()) {
    for (auto& child : node["dicts"].GetArray()) {
      RewriteDictNode(child, allocator, target_type);
    }
    return;
  }

  if (!node.HasMember("type") || !node["type"].IsString() ||
      !node.HasMember("file") || !node["file"].IsString()) {
    return;
  }

  const std::string type = node["type"].GetString();
  const std::string file_name = node["file"].GetString();
  if (type != "ocd2" || !EndsWith(file_name, ".ocd2")) {
    return;
  }

  const std::string text_path = ResolveTextDictionaryPath(file_name);
  node["type"].SetString(target_type, allocator);
  node["file"].SetString(text_path.c_str(),
                         static_cast<rapidjson::SizeType>(text_path.size()),
                         allocator);
}

void RewriteConfigToText(JSONValue& node, JSONDocument::AllocatorType& allocator,
                         const char* target_type) {
  if (node.IsObject()) {
    auto type_member = node.FindMember("type");
    auto file_member = node.FindMember("file");
    if (type_member != node.MemberEnd() && file_member != node.MemberEnd()) {
      RewriteDictNode(node, allocator, target_type);
    }

    for (auto it = node.MemberBegin(); it != node.MemberEnd(); ++it) {
      RewriteConfigToText(it->value, allocator, target_type);
    }
    return;
  }

  if (node.IsArray()) {
    for (auto& child : node.GetArray()) {
      RewriteConfigToText(child, allocator, target_type);
    }
  }
}

void RewriteSegmentationResourcesToAbsolute(
    JSONValue& node, JSONDocument::AllocatorType& allocator,
    const std::string& source_config_directory) {
  if (node.IsObject()) {
    auto segmentation_member = node.FindMember("segmentation");
    if (segmentation_member != node.MemberEnd() &&
        segmentation_member->value.IsObject()) {
      auto resources_member =
          segmentation_member->value.FindMember("resources");
      if (resources_member != segmentation_member->value.MemberEnd() &&
          resources_member->value.IsObject()) {
        for (auto it = resources_member->value.MemberBegin();
             it != resources_member->value.MemberEnd(); ++it) {
          if (!it->value.IsString()) {
            continue;
          }
          const std::string resource_path = it->value.GetString();
          if (IsAbsolutePath(resource_path)) {
            continue;
          }
          const std::string absolute_path = ResolveSegmentationResourcePath(
              source_config_directory, resource_path);
          it->value.SetString(
              absolute_path.c_str(),
              static_cast<rapidjson::SizeType>(absolute_path.size()), allocator);
        }
      }
    }

    for (auto it = node.MemberBegin(); it != node.MemberEnd(); ++it) {
      RewriteSegmentationResourcesToAbsolute(it->value, allocator,
                                             source_config_directory);
    }
    return;
  }

  if (node.IsArray()) {
    for (auto& child : node.GetArray()) {
      RewriteSegmentationResourcesToAbsolute(child, allocator,
                                             source_config_directory);
    }
  }
}

std::string ReadFile(const std::string& path) {
  std::ifstream stream(path.c_str(), std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>());
}

class TemporaryTextConfigRegistry {
public:
  ~TemporaryTextConfigRegistry() {
    for (const std::string& path : paths_) {
      std::remove(path.c_str());
    }
  }

  std::string Create(const std::string& source_config_path,
                     const char* target_type,
                     const std::string& suffix) {
    const std::string content = ReadFile(source_config_path);
    JSONDocument document;
    document.Parse(content.c_str());
    if (document.HasParseError() || !document.IsObject()) {
      throw InvalidFormat("Error parsing benchmark config JSON");
    }

    RewriteConfigToText(document, document.GetAllocator(), target_type);
    RewriteSegmentationResourcesToAbsolute(document, document.GetAllocator(),
                                          GetParentDirectory(source_config_path));

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    const std::string temp_path = BuildTempPath(source_config_path, suffix);
    std::ofstream stream(temp_path.c_str(), std::ios::binary);
    stream << buffer.GetString();
    stream.close();
    if (!stream) {
      throw FileNotWritable(temp_path);
    }

    paths_.push_back(temp_path);
    return temp_path;
  }

private:
  std::string BuildTempPath(const std::string& source_config_path,
                            const std::string& suffix) {
    const std::string base_name =
        GetBaseNameWithoutExtension(source_config_path) + "." + suffix;
    const std::string pid = GetProcessIdString();
    std::string path;
    do {
      path = BenchmarkTempDirectory() + base_name + "." + pid + "." +
             std::to_string(counter_++) + ".json";
    } while (IsRegularFile(path));
    return path;
  }

  int counter_ = 0;
  std::vector<std::string> paths_;
};

TemporaryTextConfigRegistry& GetTemporaryTextConfigRegistry() {
  static TemporaryTextConfigRegistry registry;
  return registry;
}

class TemporaryFileRegistry {
public:
  ~TemporaryFileRegistry() {
    for (const std::string& path : paths_) {
      std::remove(path.c_str());
    }
  }

  std::string Create(const std::string& prefix, const std::string& extension) {
    const std::string pid = GetProcessIdString();
    std::string path;
    do {
      path = BenchmarkTempDirectory() + prefix + "." + pid + "." +
             std::to_string(counter_++) + extension;
    } while (IsRegularFile(path));
    paths_.push_back(path);
    return path;
  }

private:
  int counter_ = 0;
  std::vector<std::string> paths_;
};

TemporaryFileRegistry& GetTemporaryFileRegistry() {
  static TemporaryFileRegistry registry;
  return registry;
}

std::vector<std::string> InitializationConfigs() {
  std::vector<std::string> configs = {
      "hk2s", "hk2t", "jp2t", "s2hk", "s2t", "s2tw", "s2twp",
      "t2hk", "t2jp", "t2s",  "t2tw", "tw2s", "tw2sp", "tw2t",
  };
#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
  configs.push_back("s2twp_jieba");
#endif
  return configs;
}

std::vector<std::string> ConversionConfigs() {
  std::vector<std::string> configs = {"s2t", "s2twp"};
#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
  configs.push_back("s2twp_jieba");
#endif
  return configs;
}

std::vector<BenchmarkConfig> BuildBenchmarkConfigs(
    const std::vector<std::string>& config_names) {
  std::vector<BenchmarkConfig> configs;
  configs.reserve(config_names.size() * 2);
  for (const std::string& config_name : config_names) {
    const std::string source_path = ResolveConfigPath(config_name);
    configs.push_back(BenchmarkConfig{config_name + "/ocd2", source_path});
    try {
      configs.push_back(BenchmarkConfig{
          config_name + "/text_json",
          GetTemporaryTextConfigRegistry().Create(source_path, "text",
                                                 "benchmark-text"),
      });
    } catch (const FileNotFound&) {
      // Some legacy benchmark configs only ship precompiled ocd2 artifacts.
      // Skip the text-json variant when there is no source text dictionary.
    }
  }
  return configs;
}

std::string QuotePath(const std::string& path) {
  return "\"" + path + "\"";
}

std::string WrapSystemCommand(const std::string& command) {
#ifdef _WIN32
  return "\"" + command + "\"";
#else
  return command;
#endif
}

#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
bool IsJiebaConfig(const BenchmarkConfig& config) {
  return config.name.find("s2twp_jieba/") == 0;
}
#endif

std::string OpenccBinaryPath() {
  return OPENCC_BENCHMARK_OPENCC_PATH;
}

std::string SourceConfigDirectory() {
  return std::string(CMAKE_SOURCE_DIR) + "/data/config";
}

std::string BuildDataDirectory() {
  return std::string(PROJECT_BINARY_DIR) + "/data";
}

struct MeasuredResult {
  double loadMs = 0.0;
  double convertMs = 0.0;
  double writeMs = 0.0;
  double totalMs = 0.0;
  size_t inputBytes = 0;
  size_t outputBytes = 0;
};

MeasuredResult ParseMeasuredResult(const std::string& path) {
  const std::string content = ReadFile(path);
  JSONDocument document;
  document.Parse(content.c_str());
  if (document.HasParseError() || !document.IsObject()) {
    throw InvalidFormat("Error parsing measured result JSON");
  }

  MeasuredResult result;
  if (document.HasMember("load_ms") && document["load_ms"].IsNumber()) {
    result.loadMs = document["load_ms"].GetDouble();
  }
  if (document.HasMember("convert_ms") && document["convert_ms"].IsNumber()) {
    result.convertMs = document["convert_ms"].GetDouble();
  }
  if (document.HasMember("write_ms") && document["write_ms"].IsNumber()) {
    result.writeMs = document["write_ms"].GetDouble();
  }
  if (document.HasMember("total_ms") && document["total_ms"].IsNumber()) {
    result.totalMs = document["total_ms"].GetDouble();
  }
  if (document.HasMember("input_bytes") && document["input_bytes"].IsUint64()) {
    result.inputBytes = static_cast<size_t>(document["input_bytes"].GetUint64());
  }
  if (document.HasMember("output_bytes") &&
      document["output_bytes"].IsUint64()) {
    result.outputBytes =
        static_cast<size_t>(document["output_bytes"].GetUint64());
  }
  return result;
}

void RunCommandLineConversion(const BenchmarkConfig& config,
                              const std::string& input_path,
                              const std::string& output_path,
                              const std::string& measured_result_path) {
#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
  if (IsJiebaConfig(config)) {
    SetPluginSearchPath(OPENCC_BENCHMARK_JIEBA_PLUGIN_DIR);
  }
#endif

  std::string command = QuotePath(OpenccBinaryPath()) + " -i " +
                        QuotePath(input_path) + " -o " + QuotePath(output_path) +
                        " -c " + QuotePath(config.path) + " --path " +
                        QuotePath(BuildDataDirectory()) + " --path " +
                        QuotePath(SourceConfigDirectory()) +
                        " --measured_result " + QuotePath(measured_result_path);
  const int status = std::system(WrapSystemCommand(command).c_str());
  if (status != 0) {
    throw Exception("opencc command failed with status " +
                    std::to_string(status));
  }
}

SimpleConverter* Initialize(const BenchmarkConfig& config) {
  chdir(PROJECT_BINARY_DIR "/data");
  return new SimpleConverter(config.path);
}

void Convert(const SimpleConverter* converter, const std::string& text) {
  converter->Convert(text);
}

std::string FormatTimeValue(double time) {
  std::ostringstream os;
  if (time < 1.0) {
    os << std::fixed << std::setprecision(3) << time;
  } else if (time < 10.0) {
    os << std::fixed << std::setprecision(2) << time;
  } else if (time < 100.0) {
    os << std::fixed << std::setprecision(1) << time;
  } else if (time > 9999999999.0) {
    os << std::scientific << std::setprecision(4) << time;
  } else {
    os << std::fixed << std::setprecision(0) << time;
  }
  return os.str();
}

std::string FormatThroughputValue(const benchmark::BenchmarkReporter::Run& run) {
  const auto it = run.counters.find("Throughput");
  if (it == run.counters.end()) {
    return "";
  }

  std::ostringstream os;
  if (run.run_type == benchmark::BenchmarkReporter::Run::RT_Aggregate &&
      run.aggregate_unit == benchmark::StatisticUnit::kPercentage) {
    os << std::fixed << std::setprecision(2) << (100.0 * it->second.value)
       << "%";
  } else {
    os << std::fixed << std::setprecision(2) << it->second.value << " MB/s";
  }
  return os.str();
}

std::string FormatCounterValue(const benchmark::BenchmarkReporter::Run& run,
                               const char* key, const char* suffix = "") {
  const auto it = run.counters.find(key);
  if (it == run.counters.end()) {
    return "";
  }

  std::ostringstream os;
  if (run.run_type == benchmark::BenchmarkReporter::Run::RT_Aggregate &&
      run.aggregate_unit == benchmark::StatisticUnit::kPercentage) {
    os << std::fixed << std::setprecision(2) << (100.0 * it->second.value)
       << "%";
  } else {
    os << std::fixed << std::setprecision(2) << it->second.value << suffix;
  }
  return os.str();
}

std::string BenchmarkGroupName(const benchmark::BenchmarkReporter::Run& run) {
  const std::string name = run.benchmark_name();
  const std::string::size_type slash_pos = name.find('/');
  const std::string prefix =
      slash_pos == std::string::npos ? name : name.substr(0, slash_pos);
  if (prefix == "BM_Initialization") {
    return "Initialization";
  }
  if (prefix == "BM_ConvertLongText") {
    return "Convert Long Text";
  }
  if (prefix == "BM_Convert") {
    return "Convert";
  }
  if (prefix == "BM_CommandLineLongText") {
    return "Command Line Long Text";
  }
  if (prefix == "BM_CommandLine") {
    return "Command Line";
  }
  return prefix;
}

bool SameCounterNames(const benchmark::UserCounters& lhs,
                      const benchmark::UserCounters& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  auto lhs_it = lhs.begin();
  auto rhs_it = rhs.begin();
  while (lhs_it != lhs.end() && rhs_it != rhs.end()) {
    if (lhs_it->first != rhs_it->first) {
      return false;
    }
    ++lhs_it;
    ++rhs_it;
  }
  return true;
}

class ThroughputConsoleReporter : public benchmark::ConsoleReporter {
public:
  explicit ThroughputConsoleReporter(OutputOptions opts = OO_Defaults)
      : benchmark::ConsoleReporter(opts) {}

  void ReportRuns(const std::vector<Run>& reports) override {
    for (const auto& run : reports) {
      const std::string group_name = BenchmarkGroupName(run);
      const bool group_changed = !printed_header_ || group_name != current_group_;
      bool print_header = group_changed;
      print_header |=
          (output_options_ & OO_Tabular) &&
          (!SameCounterNames(run.counters, prev_counters_));
      if (print_header) {
        if (printed_header_) {
          GetOutputStream() << "\n";
        }
        current_group_ = group_name;
        GetOutputStream() << "[" << current_group_ << "]\n";
        printed_header_ = true;
        prev_counters_ = run.counters;
        PrintHeader(run);
      }
      PrintRunData(run);
    }
  }

protected:
  void PrintHeader(const Run& run) override {
    std::ostringstream header;
    header << std::left << std::setw(static_cast<int>(name_field_width_))
           << "Benchmark"
           << " " << std::right << std::setw(13) << "Time"
           << " " << std::setw(15) << "CPU"
           << " " << std::setw(12) << "Iterations";
    if (!run.counters.empty()) {
      header << " " << std::setw(16) << "Throughput";
      if (run.counters.count("LoadMs") != 0) {
        header << " " << std::setw(12) << "LoadMs"
               << " " << std::setw(12) << "ConvertMs"
               << " " << std::setw(12) << "WriteMs"
               << " " << std::setw(12) << "TotalMs";
      }
    }

    const std::string line(header.str().size(), '-');
    GetOutputStream() << line << "\n"
                      << header.str() << "\n"
                      << line << "\n";
  }

  void PrintRunData(const Run& run) override {
    std::ostream& out = GetOutputStream();
    out << std::left << std::setw(static_cast<int>(name_field_width_))
        << run.benchmark_name() << " ";

    if (run.run_type != Run::RT_Aggregate ||
        run.aggregate_unit == benchmark::StatisticUnit::kTime) {
      out << std::right << std::setw(10) << FormatTimeValue(run.GetAdjustedRealTime())
          << " " << std::setw(4) << benchmark::GetTimeUnitString(run.time_unit)
          << " " << std::setw(10) << FormatTimeValue(run.GetAdjustedCPUTime())
          << " " << std::setw(4) << benchmark::GetTimeUnitString(run.time_unit)
          << " ";
    } else {
      out << std::right << std::setw(10) << std::fixed << std::setprecision(2)
          << (100.0 * run.real_accumulated_time) << " " << std::setw(4) << "%"
          << " " << std::setw(10) << std::fixed << std::setprecision(2)
          << (100.0 * run.cpu_accumulated_time) << " " << std::setw(4) << "%"
          << " ";
    }

    out << std::right << std::setw(10) << run.iterations;

    if (!run.counters.empty()) {
      out << " " << std::setw(16) << FormatThroughputValue(run);
      if (run.counters.count("LoadMs") != 0) {
        out << " " << std::setw(12) << FormatCounterValue(run, "LoadMs", " ms")
            << " " << std::setw(12)
            << FormatCounterValue(run, "ConvertMs", " ms") << " "
            << std::setw(12) << FormatCounterValue(run, "WriteMs", " ms")
            << " " << std::setw(12)
            << FormatCounterValue(run, "TotalMs", " ms");
      }
    }

    if (!run.report_label.empty()) {
      out << " " << run.report_label;
    }

    out << "\n";
  }

private:
  std::string current_group_;
};

void SetThroughputCounter(benchmark::State& state, size_t bytes_per_iteration) {
  const double total_megabytes =
      static_cast<double>(state.iterations()) *
      static_cast<double>(bytes_per_iteration) / 1000000.0;
  state.counters["Throughput"] =
      benchmark::Counter(total_megabytes, benchmark::Counter::kIsRate);
}

void SetCommandLineCounters(benchmark::State& state, size_t total_input_bytes,
                            double total_load_ms, double total_convert_ms,
                            double total_write_ms, double total_total_ms) {
  const double throughput =
      total_total_ms > 0.0
          ? (static_cast<double>(total_input_bytes) / 1000000.0) /
                (total_total_ms / 1000.0)
          : 0.0;
  const double iterations = static_cast<double>(state.iterations());
  state.counters["Throughput"] = benchmark::Counter(throughput);
  state.counters["LoadMs"] = benchmark::Counter(total_load_ms / iterations);
  state.counters["ConvertMs"] =
      benchmark::Counter(total_convert_ms / iterations);
  state.counters["WriteMs"] = benchmark::Counter(total_write_ms / iterations);
  state.counters["TotalMs"] = benchmark::Counter(total_total_ms / iterations);
}

std::string ReadText(const std::string& filename) {
  const std::string benchmark_data_dir = CMAKE_SOURCE_DIR "/test/benchmark/";
  const std::string data_path = benchmark_data_dir + filename;
  std::ifstream stream(data_path.c_str());
  return std::string((std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>());
}

static void BM_Initialization(benchmark::State& state,
                              const BenchmarkConfig& config) {
  for (auto _ : state) {
    const SimpleConverter* converter = Initialize(config);
    state.PauseTiming();
    delete converter;
    state.ResumeTiming();
  }
}

static void BM_ConvertLongText(benchmark::State& state,
                               const BenchmarkConfig& config) {
  const std::string text = ReadText("zuozhuan.txt");
  const std::unique_ptr<SimpleConverter> converter(Initialize(config));
  for (auto _ : state) {
    Convert(converter.get(), text);
  }
  SetThroughputCounter(state, text.size());
}

static void BM_Convert(benchmark::State& state, const BenchmarkConfig& config,
                       int iteration) {
  std::ostringstream os;
  for (int i = 0; i < iteration; i++) {
    os << "Open Chinese Convert 開放中文轉換" << i << std::endl;
  }
  const std::string text = os.str();
  const std::unique_ptr<SimpleConverter> converter(Initialize(config));
  for (auto _ : state) {
    Convert(converter.get(), text);
  }
  SetThroughputCounter(state, text.size());
}

static void BM_CommandLineLongText(benchmark::State& state,
                                   const BenchmarkConfig& config) {
  const std::string input_path =
      GetTemporaryFileRegistry().Create("benchmark-cli-longtext-input", ".txt");
  const std::string output_path =
      GetTemporaryFileRegistry().Create("benchmark-cli-longtext-output", ".txt");
  const std::string measured_path =
      GetTemporaryFileRegistry().Create("benchmark-cli-longtext-measured",
                                        ".json");
  const std::string text = ReadText("zuozhuan.txt");
  {
    std::ofstream stream(input_path.c_str(), std::ios::binary);
    stream << text;
  }

  MeasuredResult measured;
  size_t total_input_bytes = 0;
  double total_load_ms = 0.0;
  double total_convert_ms = 0.0;
  double total_write_ms = 0.0;
  double total_total_ms = 0.0;
  for (auto _ : state) {
    RunCommandLineConversion(config, input_path, output_path, measured_path);
    measured = ParseMeasuredResult(measured_path);
    total_input_bytes += measured.inputBytes == 0 ? text.size() : measured.inputBytes;
    total_load_ms += measured.loadMs;
    total_convert_ms += measured.convertMs;
    total_write_ms += measured.writeMs;
    total_total_ms += measured.totalMs;
  }
  SetCommandLineCounters(state, total_input_bytes, total_load_ms,
                         total_convert_ms, total_write_ms, total_total_ms);
}

static void BM_CommandLine(benchmark::State& state, const BenchmarkConfig& config,
                           int iteration) {
  const std::string input_path =
      GetTemporaryFileRegistry().Create("benchmark-cli-input", ".txt");
  const std::string output_path =
      GetTemporaryFileRegistry().Create("benchmark-cli-output", ".txt");
  const std::string measured_path =
      GetTemporaryFileRegistry().Create("benchmark-cli-measured", ".json");

  std::ostringstream os;
  for (int i = 0; i < iteration; i++) {
    os << "Open Chinese Convert 開放中文轉換" << i << std::endl;
  }
  const std::string text = os.str();
  {
    std::ofstream stream(input_path.c_str(), std::ios::binary);
    stream << text;
  }

  MeasuredResult measured;
  size_t total_input_bytes = 0;
  double total_load_ms = 0.0;
  double total_convert_ms = 0.0;
  double total_write_ms = 0.0;
  double total_total_ms = 0.0;
  for (auto _ : state) {
    RunCommandLineConversion(config, input_path, output_path, measured_path);
    measured = ParseMeasuredResult(measured_path);
    total_input_bytes += measured.inputBytes == 0 ? text.size() : measured.inputBytes;
    total_load_ms += measured.loadMs;
    total_convert_ms += measured.convertMs;
    total_write_ms += measured.writeMs;
    total_total_ms += measured.totalMs;
  }
  SetCommandLineCounters(state, total_input_bytes, total_load_ms,
                         total_convert_ms, total_write_ms, total_total_ms);
}

bool RegisterBenchmarks() {
  const std::vector<BenchmarkConfig> initialization_configs =
      BuildBenchmarkConfigs(InitializationConfigs());
  for (const BenchmarkConfig& config : initialization_configs) {
    benchmark::RegisterBenchmark(
        ("BM_Initialization/" + config.name).c_str(),
        [config](benchmark::State& state) { BM_Initialization(state, config); })
        ->Unit(benchmark::kMicrosecond);
  }

  const std::vector<BenchmarkConfig> conversion_configs =
      BuildBenchmarkConfigs(ConversionConfigs());
  for (const BenchmarkConfig& config : conversion_configs) {
    benchmark::RegisterBenchmark(
        ("BM_ConvertLongText/" + config.name).c_str(),
        [config](benchmark::State& state) { BM_ConvertLongText(state, config); })
        ->Unit(benchmark::kMillisecond);
  }

  for (const BenchmarkConfig& config : conversion_configs) {
    for (const int iteration : {100, 1000, 10000, 100000}) {
      benchmark::RegisterBenchmark(
          ("BM_Convert/" + config.name + "/" + std::to_string(iteration))
              .c_str(),
          [config, iteration](benchmark::State& state) {
            BM_Convert(state, config, iteration);
          })
          ->Unit(benchmark::kMillisecond);
    }
  }

  for (const BenchmarkConfig& config : conversion_configs) {
    benchmark::RegisterBenchmark(
        ("BM_CommandLineLongText/" + config.name).c_str(),
        [config](benchmark::State& state) {
          BM_CommandLineLongText(state, config);
        })
        ->Unit(benchmark::kMillisecond)
        ->MinTime(0.01);
  }

  for (const BenchmarkConfig& config : conversion_configs) {
    for (const int iteration : {100, 1000, 10000, 100000}) {
      benchmark::RegisterBenchmark(
          ("BM_CommandLine/" + config.name + "/" + std::to_string(iteration))
              .c_str(),
          [config, iteration](benchmark::State& state) {
            BM_CommandLine(state, config, iteration);
          })
          ->Unit(benchmark::kMillisecond)
          ->MinTime(0.01);
    }
  }
  return true;
}

const bool kBenchmarksRegistered = RegisterBenchmarks();

} // namespace

} // namespace opencc

int main(int argc, char** argv) {
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
    return 1;
  }
  opencc::ThroughputConsoleReporter reporter;
  ::benchmark::RunSpecifiedBenchmarks(&reporter);
  ::benchmark::Shutdown();
  return 0;
}
