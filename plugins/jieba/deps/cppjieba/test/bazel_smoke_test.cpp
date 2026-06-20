#include "cppjieba/Jieba.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::string RunfilePath(const std::string& relative_path) {
  const char* test_srcdir = std::getenv("TEST_SRCDIR");
  const char* test_workspace = std::getenv("TEST_WORKSPACE");
  if (test_srcdir == NULL || test_workspace == NULL) {
    return relative_path;
  }

  return std::string(test_srcdir) + "/" + test_workspace + "/" + relative_path;
}

}  // namespace

int main() {
  cppjieba::Jieba jieba(
      RunfilePath("dict/jieba.dict.utf8"),
      RunfilePath("dict/hmm_model.utf8"),
      RunfilePath("dict/user.dict.utf8"),
      RunfilePath("dict/idf.utf8"),
      RunfilePath("dict/stop_words.utf8"));

  std::vector<std::string> words;
  jieba.Cut("南京市长江大桥", words);
  if (words.empty()) {
    std::cerr << "Expected cppjieba to segment the smoke-test sentence.\n";
    return 1;
  }

  return 0;
}
