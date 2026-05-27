#include "cppjieba/TextRankExtractor.hpp"
#include "gtest/gtest.h"
#include "test_paths.h"

using namespace cppjieba;

TEST(TextRankExtractorTest, Test1) {
  TextRankExtractor Extractor(
    TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8",
    DICT_DIR "/hmm_model.utf8", 
    DICT_DIR "/stop_words.utf8");
  {
    string s("你好世界世界而且而且");
    string res;
    size_t topN = 5;

    {
      vector<string> words;
      Extractor.Extract(s, words, topN);
      res << words;
      ASSERT_EQ(res, "[\"世界\", \"你好\"]");
    }

    {
      vector<pair<string, double> > words;
      Extractor.Extract(s, words, topN);
      res << words;
      ASSERT_EQ(res, "[世界:1, 你好:0.519787]");
    }

    {
      vector<TextRankExtractor::Word> words;
      Extractor.Extract(s, words, topN);
      res << words;
      ASSERT_EQ(res, "[{\"word\": \"世界\", \"offset\": [6, 12], \"weight\": 1}, {\"word\": \"你好\", \"offset\": [0], \"weight\": 0.519787}]");
    }
  }

  { 
    string s("\xe6\x88\x91\xe6\x98\xaf\xe6\x8b\x96\xe6\x8b\x89\xe6\x9c\xba\xe5\xad\xa6\xe9\x99\xa2\xe6\x89\x8b\xe6\x89\xb6\xe6\x8b\x96\xe6\x8b\x89\xe6\x9c\xba\xe4\xb8\x93\xe4\xb8\x9a\xe7\x9a\x84\xe3\x80\x82\xe4\xb8\x8d\xe7\x94\xa8\xe5\xa4\x9a\xe4\xb9\x85\xef\xbc\x8c\xe6\x88\x91\xe5\xb0\xb1\xe4\xbc\x9a\xe5\x8d\x87\xe8\x81\x8c\xe5\x8a\xa0\xe8\x96\xaa\xef\xbc\x8c\xe5\xbd\x93\xe4\xb8\x8a CEO\xef\xbc\x8c\xe8\xb5\xb0\xe4\xb8\x8a\xe4\xba\xba\xe7\x94\x9f\xe5\xb7\x85\xe5\xb3\xb0");
    string res;
    vector<TextRankExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res << wordweights;
    ASSERT_EQ(res, "[{\"word\": \"当上\", \"offset\": [87], \"weight\": 1}, {\"word\": \"不用\", \"offset\": [48], \"weight\": 0.989848}, {\"word\": \"多久\", \"offset\": [54], \"weight\": 0.985126}, {\"word\": \"加薪\", \"offset\": [78], \"weight\": 0.983046}, {\"word\": \"升职\", \"offset\": [72], \"weight\": 0.980278}]");
    //ASSERT_EQ(res, "[{\"word\": \"专业\", \"offset\": [36], \"weight\": 1}, {\"word\": \"CEO\", \"offset\": [94], \"weight\": 0.95375}, {\"word\": \"手扶拖拉机\", \"offset\": [21], \"weight\": 0.801701}, {\"word\": \"当上\", \"offset\": [87], \"weight\": 0.798968}, {\"word\": \"走上\", \"offset\": [100], \"weight\": 0.775505}]");
  }

  {
    string s("一部iPhone6");
    string res;
    vector<TextRankExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res << wordweights;
    ASSERT_EQ(res, "[{\"word\": \"一部\", \"offset\": [0], \"weight\": 1}, {\"word\": \"iPhone6\", \"offset\": [6], \"weight\": 0.996126}]");
  }
}

TEST(TextRankExtractorTest, Test2) {
  TextRankExtractor Extractor(
    TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8",
    DICT_DIR "/hmm_model.utf8",
    DICT_DIR "/stop_words.utf8",
    TEST_DATA_DIR "/userdict.utf8");

  {
    string s("\xe8\x93\x9d\xe7\xbf\x94\xe4\xbc\x98\xe7\xa7\x80\xe6\xaf\x95\xe4\xb8\x9a\xe7\x94\x9f");
    string res;
    vector<TextRankExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res << wordweights;
    ASSERT_EQ(res, "[{\"word\": \"蓝翔\", \"offset\": [0], \"weight\": 1}, {\"word\": \"毕业生\", \"offset\": [12], \"weight\": 0.996685}, {\"word\": \"优秀\", \"offset\": [6], \"weight\": 0.992994}]");
  }

  {
    string s("一部iPhone6");
    string res;
    vector<TextRankExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res << wordweights;
    ASSERT_EQ(res, "[{\"word\": \"一部\", \"offset\": [0], \"weight\": 1}, {\"word\": \"iPhone6\", \"offset\": [6], \"weight\": 0.996126}]");
  }
}
