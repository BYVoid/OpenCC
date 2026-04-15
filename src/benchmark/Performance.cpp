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
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>

#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "SimpleConverter.hpp"
#include "TestUtilsUTF8.hpp"

namespace opencc {

namespace {

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

} // namespace

SimpleConverter* Initialize(const std::string& config_name) {
  chdir(PROJECT_BINARY_DIR "/data");
  const std::string config_path = ResolveConfigPath(config_name);
  return new SimpleConverter(config_path);
}

void Convert(const SimpleConverter* converter, const std::string& text) {
  converter->Convert(text);
}

std::string ReadText(const std::string& filename) {
  const std::string benchmark_data_dir = CMAKE_SOURCE_DIR "/test/benchmark/";
  const std::string data_path = benchmark_data_dir + filename;
  std::ifstream stream(data_path);
  return std::string((std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>());
}

static void BM_Initialization(benchmark::State& state,
                              std::string config_name) {
  for (auto _ : state) {
    const SimpleConverter* converter = Initialize(config_name);
    // Do not count the destruction time.
    state.PauseTiming();
    delete converter;
    state.ResumeTiming();
  }
}
BENCHMARK_CAPTURE(BM_Initialization, hk2s, "hk2s")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, hk2t, "hk2t")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, jp2t, "jp2t")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, s2hk, "s2hk")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, s2t, "s2t")->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, s2tw, "s2tw")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, s2twp, "s2twp")
    ->Unit(benchmark::kMicrosecond);
#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
BENCHMARK_CAPTURE(BM_Initialization, s2twp_jieba, "s2twp_jieba")
    ->Unit(benchmark::kMicrosecond);
#endif
BENCHMARK_CAPTURE(BM_Initialization, t2hk, "t2hk")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, t2jp, "t2jp")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, t2s, "t2s")->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, t2tw, "t2tw")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, tw2s, "tw2s")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, tw2sp, "tw2sp")
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(BM_Initialization, tw2t, "tw2t")
    ->Unit(benchmark::kMicrosecond);

static void BM_ConvertLongText(benchmark::State& state,
                               std::string config_name) {
  const std::string text = ReadText("zuozhuan.txt");
  const std::unique_ptr<SimpleConverter> converter(Initialize(config_name));
  for (auto _ : state) {
    Convert(converter.get(), text);
  }
  state.SetBytesProcessed(
      static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(text.size()));
}
BENCHMARK_CAPTURE(BM_ConvertLongText, s2t, "s2t")->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_ConvertLongText, s2twp, "s2twp")
    ->Unit(benchmark::kMillisecond);
#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
BENCHMARK_CAPTURE(BM_ConvertLongText, s2twp_jieba, "s2twp_jieba")
    ->Unit(benchmark::kMillisecond);
#endif

static void BM_Convert(benchmark::State& state, std::string config_name,
                       int iteration) {
  std::ostringstream os;
  for (int i = 0; i < iteration; i++) {
    os << "Open Chinese Convert 開放中文轉換" << i << std::endl;
  }
  const std::string text = os.str();
  const std::unique_ptr<SimpleConverter> converter(Initialize(config_name));
  for (auto _ : state) {
    Convert(converter.get(), text);
  }
  state.SetBytesProcessed(
      static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(text.size()));
}
BENCHMARK_CAPTURE(BM_Convert, s2t_100, "s2t", 100)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2t_1000, "s2t", 1000)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2t_10000, "s2t", 10000)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2t_100000, "s2t", 100000)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2twp_100, "s2twp", 100)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2twp_1000, "s2twp", 1000)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2twp_10000, "s2twp", 10000)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2twp_100000, "s2twp", 100000)
    ->Unit(benchmark::kMillisecond);
#ifdef OPENCC_BENCHMARK_JIEBA_CONFIG_DIR
BENCHMARK_CAPTURE(BM_Convert, s2twp_jieba_100, "s2twp_jieba", 100)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2twp_jieba_1000, "s2twp_jieba", 1000)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2twp_jieba_10000, "s2twp_jieba", 10000)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, s2twp_jieba_100000, "s2twp_jieba", 100000)
    ->Unit(benchmark::kMillisecond);
#endif

} // namespace opencc

BENCHMARK_MAIN();
