#include <benchmark/benchmark.h>
#include <fstream>
#include <iostream>
#include <memory>
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
BENCHMARK_CAPTURE(BM_Initialization, hk2s, "hk2s");
BENCHMARK_CAPTURE(BM_Initialization, hk2t, "hk2t");
BENCHMARK_CAPTURE(BM_Initialization, jp2t, "jp2t");
BENCHMARK_CAPTURE(BM_Initialization, s2hk, "s2hk");
BENCHMARK_CAPTURE(BM_Initialization, s2t, "s2t");
BENCHMARK_CAPTURE(BM_Initialization, s2tw, "s2tw");
BENCHMARK_CAPTURE(BM_Initialization, s2twp, "s2twp");
BENCHMARK_CAPTURE(BM_Initialization, t2hk, "t2hk");
BENCHMARK_CAPTURE(BM_Initialization, t2jp, "t2jp");
BENCHMARK_CAPTURE(BM_Initialization, t2s, "t2s");
BENCHMARK_CAPTURE(BM_Initialization, tw2s, "tw2s");
BENCHMARK_CAPTURE(BM_Initialization, tw2sp, "tw2sp");
BENCHMARK_CAPTURE(BM_Initialization, tw2t, "tw2t");

static void BM_Convert(benchmark::State& state) {
  const std::string config_name = "s2t";
  const std::string text = ReadText("zuozhuan.txt");
  const std::unique_ptr<SimpleConverter> converter(Initialize(config_name));
  for (auto _ : state) {
    Convert(converter.get(), text);
  }
}
BENCHMARK(BM_Convert)->Unit(benchmark::kMillisecond);

} // namespace opencc

BENCHMARK_MAIN();
