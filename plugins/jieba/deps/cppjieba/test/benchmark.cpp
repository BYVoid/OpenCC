#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#ifdef __APPLE__
#include <mach/mach.h>
#elif defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#include "cppjieba/DictTrie.hpp"
#include "cppjieba/HMMModel.hpp"
#include "cppjieba/MPSegment.hpp"
#include "cppjieba/MixSegment.hpp"
#include "test_paths.h"

using namespace cppjieba;

namespace {

typedef std::chrono::steady_clock BenchmarkClock;

std::string ReadFile(const std::string& path) {
  std::ifstream ifs(path.c_str(), std::ifstream::in | std::ifstream::binary);
  if (!ifs.is_open()) {
    return std::string();
  }
  return std::string((std::istreambuf_iterator<char>(ifs)),
      std::istreambuf_iterator<char>());
}

std::vector<std::string> LoadDictWords(const std::string& path, size_t limit) {
  std::ifstream ifs(path.c_str());
  std::vector<std::string> words;
  words.reserve(limit);
  std::string word;
  while (words.size() < limit && ifs >> word) {
    words.push_back(word);
    std::string ignore;
    std::getline(ifs, ignore);
  }
  return words;
}

double GetCurrentRSSInMB() {
#ifdef __APPLE__
  mach_task_basic_info info;
  mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
        reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS) {
    return -1.0;
  }
  return static_cast<double>(info.resident_size) / (1024.0 * 1024.0);
#elif defined(__linux__)
  long total_pages = 0;
  long rss_pages = 0;
  std::ifstream ifs("/proc/self/statm");
  if (!(ifs >> total_pages >> rss_pages)) {
    return -1.0;
  }
  const long page_size = sysconf(_SC_PAGESIZE);
  return static_cast<double>(rss_pages) * static_cast<double>(page_size) / (1024.0 * 1024.0);
#elif defined(_WIN32)
  PROCESS_MEMORY_COUNTERS counters;
  if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) {
    return -1.0;
  }
  return static_cast<double>(counters.WorkingSetSize) / (1024.0 * 1024.0);
#else
  return -1.0;
#endif
}

template <typename Func>
double MeasureMillis(Func func) {
  const BenchmarkClock::time_point begin = BenchmarkClock::now();
  func();
  const BenchmarkClock::time_point end = BenchmarkClock::now();
  return std::chrono::duration_cast<std::chrono::duration<double, std::milli> >(end - begin).count();
}

void PrintMetric(const std::string& name, double millis, const std::string& details) {
  std::cout << std::fixed << std::setprecision(3)
            << "BENCH " << name
            << " ms=" << millis;
  if (!details.empty()) {
    std::cout << " " << details;
  }
  std::cout << std::endl;
}

std::string FormatRSSDetails(double before_rss_mb, double after_rss_mb) {
  if (after_rss_mb < 0.0) {
    return std::string();
  }
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(3)
      << "rss_mb=" << after_rss_mb;
  if (before_rss_mb >= 0.0) {
    oss << " rss_delta_mb=" << (after_rss_mb - before_rss_mb);
  }
  return oss.str();
}

std::string FormatThroughputDetails(size_t iterations, size_t bytes, size_t output_words, double millis) {
  const double seconds = millis / 1000.0;
  const double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
  const double docs_per_sec = seconds > 0.0 ? static_cast<double>(iterations) / seconds : 0.0;
  const double mb_per_sec = seconds > 0.0 ? mb * iterations / seconds : 0.0;
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(3)
      << "iters=" << iterations
      << " bytes=" << bytes
      << " output_words=" << output_words
      << " docs_per_sec=" << docs_per_sec
      << " mb_per_sec=" << mb_per_sec;
  return oss.str();
}

std::string FormatLookupDetails(size_t iterations, size_t queries, size_t hits, double millis) {
  const double seconds = millis / 1000.0;
  const double qps = seconds > 0.0 ? static_cast<double>(iterations * queries) / seconds : 0.0;
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(3)
      << "iters=" << iterations
      << " queries=" << queries
      << " hits=" << hits
      << " qps=" << qps;
  return oss.str();
}

}  // namespace

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  const std::string dict_path = DICT_DIR "/jieba.dict.utf8";
  const std::string model_path = DICT_DIR "/hmm_model.utf8";
  const std::string user_dict_path = DICT_DIR "/user.dict.utf8";
  const std::string cut_doc = ReadFile(TEST_DATA_DIR "/synthetic_doc.utf8");
  const std::vector<std::string> queries = LoadDictWords(dict_path, 50000);

  if (cut_doc.empty()) {
    std::cerr << "Failed to load benchmark document." << std::endl;
    return 1;
  }
  if (queries.empty()) {
    std::cerr << "Failed to load benchmark queries." << std::endl;
    return 1;
  }

  const size_t mp_cut_iterations = 200;
  const size_t mix_cut_iterations = 200;
  const size_t find_iterations = 20;

  double rss_before = GetCurrentRSSInMB();
  DictTrie* dict_trie = NULL;
  const double dict_load_ms = MeasureMillis([&dict_trie, &dict_path, &user_dict_path]() {
    dict_trie = new DictTrie(dict_path, user_dict_path);
  });
  double rss_after_dict = GetCurrentRSSInMB();
  PrintMetric("DictTrieLoad", dict_load_ms, FormatRSSDetails(rss_before, rss_after_dict));

  HMMModel* hmm_model = NULL;
  const double hmm_load_ms = MeasureMillis([&hmm_model, &model_path]() {
    hmm_model = new HMMModel(model_path);
  });
  double rss_after_hmm = GetCurrentRSSInMB();
  PrintMetric("HMMModelLoad", hmm_load_ms, FormatRSSDetails(rss_after_dict, rss_after_hmm));

  MPSegment mp_segment(dict_trie);
  MixSegment mix_segment(dict_trie, hmm_model);

  size_t mp_words = 0;
  const double mp_cut_ms = MeasureMillis([&]() {
    std::vector<std::string> words;
    for (size_t i = 0; i < mp_cut_iterations; ++i) {
      words.clear();
      mp_segment.Cut(cut_doc, words);
    }
    mp_words = words.size();
  });
  PrintMetric("MPCut", mp_cut_ms,
      FormatThroughputDetails(mp_cut_iterations, cut_doc.size(), mp_words, mp_cut_ms));

  size_t mix_words = 0;
  const double mix_cut_ms = MeasureMillis([&]() {
    std::vector<std::string> words;
    for (size_t i = 0; i < mix_cut_iterations; ++i) {
      words.clear();
      mix_segment.Cut(cut_doc, words);
    }
    mix_words = words.size();
  });
  PrintMetric("MixCut", mix_cut_ms,
      FormatThroughputDetails(mix_cut_iterations, cut_doc.size(), mix_words, mix_cut_ms));

  size_t hits = 0;
  const double find_ms = MeasureMillis([&]() {
    for (size_t i = 0; i < find_iterations; ++i) {
      for (size_t j = 0; j < queries.size(); ++j) {
        if (dict_trie->Find(queries[j])) {
          ++hits;
        }
      }
    }
  });
  PrintMetric("DictFind", find_ms,
      FormatLookupDetails(find_iterations, queries.size(), hits, find_ms));

  delete hmm_model;
  delete dict_trie;
  return 0;
}
