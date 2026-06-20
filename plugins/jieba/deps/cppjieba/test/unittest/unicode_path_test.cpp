#include <cerrno>
#include <cstdio>
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "cppjieba/UnicodeFile.hpp"
#include "cppjieba/Jieba.hpp"
#include "cppjieba/KeywordExtractor.hpp"
#include "cppjieba/TextRankExtractor.hpp"
#include "gtest/gtest.h"
#include "test_paths.h"

using namespace cppjieba;

namespace {

string TestPathJoin(const string& lhs, const string& rhs) {
  return lhs + "/" + rhs;
}

void MakeDir(const string& path) {
#ifdef _WIN32
  std::wstring widePath;
  ASSERT_TRUE(Utf8ToWidePath(path, widePath)) << path;
  ASSERT_TRUE(_wmkdir(widePath.c_str()) == 0 || errno == EEXIST) << path;
#else
  ASSERT_TRUE(mkdir(path.c_str(), 0755) == 0 || errno == EEXIST) << path;
#endif
}

void OpenOutputFile(std::ofstream& ofs, const string& path) {
#ifdef _WIN32
  std::wstring widePath;
  ASSERT_TRUE(Utf8ToWidePath(path, widePath)) << path;
  ofs.open(widePath.c_str(), std::ios::binary);
#else
  ofs.open(path.c_str(), std::ios::binary);
#endif
}

void CopyFile(const string& src, const string& dst) {
  std::ifstream ifs;
  OpenInputFile(ifs, src);
  ASSERT_TRUE(ifs.is_open()) << src;
  std::ofstream ofs;
  OpenOutputFile(ofs, dst);
  ASSERT_TRUE(ofs.is_open()) << dst;
  ofs << ifs.rdbuf();
  ASSERT_TRUE(ofs.good()) << dst;
}

string PrepareUnicodeDictDir() {
  const string baseDir = "./cppjieba_unicode_path_test";
  const string unicodeDir = TestPathJoin(baseDir, "中文路径");
  const string dictDir = TestPathJoin(unicodeDir, "dict");
  MakeDir(baseDir);
  MakeDir(unicodeDir);
  MakeDir(dictDir);

  CopyFile(DICT_DIR "/jieba.dict.utf8", TestPathJoin(dictDir, "jieba.dict.utf8"));
  CopyFile(DICT_DIR "/hmm_model.utf8", TestPathJoin(dictDir, "hmm_model.utf8"));
  CopyFile(DICT_DIR "/user.dict.utf8", TestPathJoin(dictDir, "user.dict.utf8"));
  CopyFile(DICT_DIR "/idf.utf8", TestPathJoin(dictDir, "idf.utf8"));
  CopyFile(DICT_DIR "/stop_words.utf8", TestPathJoin(dictDir, "stop_words.utf8"));
  CopyFile(TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8", TestPathJoin(dictDir, "jieba.dict.small.utf8"));
  return dictDir;
}

} // namespace

TEST(UnicodePathTest, LoadsResourcesFromUnicodePath) {
  const string dictDir = PrepareUnicodeDictDir();

  {
    cppjieba::Jieba jieba(TestPathJoin(dictDir, "jieba.dict.utf8"),
                          TestPathJoin(dictDir, "hmm_model.utf8"),
                          TestPathJoin(dictDir, "user.dict.utf8"),
                          TestPathJoin(dictDir, "idf.utf8"),
                          TestPathJoin(dictDir, "stop_words.utf8"));
    vector<string> words;
    string result;
    jieba.Cut("他来到了网易杭研大厦", words);
    result << words;
    ASSERT_EQ("[\"他\", \"来到\", \"了\", \"网易\", \"杭研\", \"大厦\"]", result);
  }

  {
    KeywordExtractor extractor(TestPathJoin(dictDir, "jieba.dict.small.utf8"),
                               TestPathJoin(dictDir, "hmm_model.utf8"),
                               TestPathJoin(dictDir, "idf.utf8"),
                               TestPathJoin(dictDir, "stop_words.utf8"));
    vector<string> words;
    extractor.Extract("你好世界世界而且而且", words, 5);
    ASSERT_EQ(2u, words.size());
    ASSERT_EQ("世界", words[0]);
    ASSERT_EQ("你好", words[1]);
  }

  {
    TextRankExtractor extractor(TestPathJoin(dictDir, "jieba.dict.small.utf8"),
                                TestPathJoin(dictDir, "hmm_model.utf8"),
                                TestPathJoin(dictDir, "stop_words.utf8"));
    vector<string> words;
    extractor.Extract("你好世界世界而且而且", words, 5);
    ASSERT_EQ(2u, words.size());
    ASSERT_EQ("世界", words[0]);
    ASSERT_EQ("你好", words[1]);
  }
}
