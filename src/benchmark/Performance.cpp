/*
 * Open Chinese Convert
 *
 * Copyright 2020-2021 Carbo Kuo <byvoid@byvoid.com>
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

#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "SimpleConverter.hpp"
#include "TestUtilsUTF8.hpp"

namespace opencc {

SimpleConverter* Initialize(const std::string& config_name) {
  chdir(PROJECT_BINARY_DIR "/data");
  const std::string config_dir = CMAKE_SOURCE_DIR "/data/config/";
  const std::string config_path = config_dir + config_name + ".json";
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
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, hk2t, "hk2t")
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, jp2t, "jp2t")
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, s2hk, "s2hk")
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, s2t, "s2t")->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, s2tw, "s2tw")
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, s2twp, "s2twp")
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, t2hk, "t2hk")
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, t2jp, "t2jp")
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, t2s, "t2s")->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, tw2s, "tw2s")
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, tw2sp, "tw2sp")
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Initialization, tw2t, "tw2t")
    ->Unit(benchmark::kMillisecond);

static void BM_Convert2M(benchmark::State& state) {
  const std::string config_name = "s2t";
  const std::string text = ReadText("zuozhuan.txt");
  const std::unique_ptr<SimpleConverter> converter(Initialize(config_name));
  for (auto _ : state) {
    Convert(converter.get(), text);
  }
}
BENCHMARK(BM_Convert2M)->Unit(benchmark::kMillisecond);

static void BM_Convert(benchmark::State& state, int iteration) {
  std::ostringstream os;
  for (int i = 0; i < iteration; i++) {
    os << "Open Chinese Convert 開放中文轉換" << i << std::endl;
  }
  const std::string text = os.str();
  const std::string config_name = "s2t";
  const std::unique_ptr<SimpleConverter> converter(Initialize(config_name));
  for (auto _ : state) {
    Convert(converter.get(), text);
  }
}
BENCHMARK_CAPTURE(BM_Convert, 100, 100)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, 1000, 1000)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, 10000, 10000)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_Convert, 100000, 100000)->Unit(benchmark::kMillisecond);

} // namespace opencc

BENCHMARK_MAIN();
