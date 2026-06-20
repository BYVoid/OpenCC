#include "cppjieba/KeywordExtractor.hpp"
#include "gtest/gtest.h"
#include "test_paths.h"

using namespace cppjieba;

TEST(KeywordExtractorTest, Test1) {
  KeywordExtractor Extractor(TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8", 
                            DICT_DIR "/hmm_model.utf8", 
                            DICT_DIR "/idf.utf8", 
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
      ASSERT_EQ(res, "[世界:8.73506, 你好:7.95788]");
    }

    {
      vector<KeywordExtractor::Word> words;
      Extractor.Extract(s, words, topN);
      res << words;
      ASSERT_EQ(res, "[{\"word\": \"\xE4\xB8\x96\xE7\x95\x8C\", \"offset\": [6, 12], \"weight\": 8.73506}, {\"word\": \"\xE4\xBD\xA0\xE5\xA5\xBD\", \"offset\": [0], \"weight\": 7.95788}]");
    }
  }

  {
    string s("我是拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上CEO，走上人生巅峰。");
    string res;
    vector<KeywordExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res << wordweights;
    ASSERT_EQ(res, "[{\"word\": \"CEO\", \"offset\": [93], \"weight\": 11.7392}, {\"word\": \"\xE5\x8D\x87\xE8\x81\x8C\", \"offset\": [72], \"weight\": 10.8562}, {\"word\": \"\xE5\x8A\xA0\xE8\x96\xAA\", \"offset\": [78], \"weight\": 10.6426}, {\"word\": \"\xE6\x89\x8B\xE6\x89\xB6\xE6\x8B\x96\xE6\x8B\x89\xE6\x9C\xBA\", \"offset\": [21], \"weight\": 10.0089}, {\"word\": \"\xE5\xB7\x85\xE5\xB3\xB0\", \"offset\": [111], \"weight\": 9.49396}]");
  }

  {
    string s("一部iPhone6");
    string res;
    vector<KeywordExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res << wordweights;
    ASSERT_EQ(res, "[{\"word\": \"iPhone6\", \"offset\": [6], \"weight\": 11.7392}, {\"word\": \"\xE4\xB8\x80\xE9\x83\xA8\", \"offset\": [0], \"weight\": 6.47592}]");
  }
}

TEST(KeywordExtractorTest, Test2) {
  KeywordExtractor Extractor(TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8", 
                            DICT_DIR "/hmm_model.utf8", 
                            DICT_DIR "/idf.utf8", 
                            DICT_DIR "/stop_words.utf8", 
                            TEST_DATA_DIR "/userdict.utf8");

  {
    string s("蓝翔优秀毕业生");
    string res;
    vector<KeywordExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res << wordweights;
    ASSERT_EQ(res, "[{\"word\": \"\xE8\x93\x9D\xE7\xBF\x94\", \"offset\": [0], \"weight\": 11.7392}, {\"word\": \"\xE6\xAF\x95\xE4\xB8\x9A\xE7\x94\x9F\", \"offset\": [12], \"weight\": 8.13549}, {\"word\": \"\xE4\xBC\x98\xE7\xA7\x80\", \"offset\": [6], \"weight\": 6.78347}]");
  }

  {
    string s("一部iPhone6");
    string res;
    vector<KeywordExtractor::Word> wordweights;
    size_t topN = 5;
    Extractor.Extract(s, wordweights, topN);
    res << wordweights;
    ASSERT_EQ(res, "[{\"word\": \"iPhone6\", \"offset\": [6], \"weight\": 11.7392}, {\"word\": \"\xE4\xB8\x80\xE9\x83\xA8\", \"offset\": [0], \"weight\": 6.47592}]");
  }
}
