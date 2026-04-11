#include "cppjieba/DictTrie.hpp"
#include "cppjieba/MPSegment.hpp"
#include "gtest/gtest.h"
#include "test_paths.h"

using namespace cppjieba;

static const char* const DICT_FILE = TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8";

TEST(TrieTest, Empty) {
  vector<Unicode> keys;
  vector<const DictUnit*> values;
  Trie trie(keys, values);
}

TEST(TrieTest, Construct) {
  vector<Unicode> keys;
  vector<const DictUnit*> values;
  keys.push_back(DecodeUTF8RunesInString("你"));
  values.push_back((const DictUnit*)(NULL));
  Trie trie(keys, values);
}

TEST(DictTrieTest, NewAndDelete) {
  DictTrie * trie;
  trie = new DictTrie(DICT_FILE);
  delete trie;
}

TEST(DictTrieTest, Test1) {
  string s1, s2;
  DictTrie trie(DICT_FILE);
  ASSERT_LT(trie.GetMinWeight() + 15.6479, 0.001);
  string word("来到");
  cppjieba::RuneStrArray uni;
  ASSERT_TRUE(DecodeUTF8RunesInString(word, uni));
  const DictUnit* du = trie.Find(uni.begin(), uni.end());
  ASSERT_TRUE(du != NULL);
  ASSERT_EQ(2u, du->word.size());
  ASSERT_EQ(26469u, du->word[0]);
  ASSERT_EQ(21040u, du->word[1]);
  ASSERT_EQ("v", du->tag);
  ASSERT_NEAR(-8.870, du->weight, 0.001);

  word = "清华大学";
  LocalVector<pair<size_t, const DictUnit*> > res;
  const char * words[] = {"清", "清华", "清华大学"};
  for (size_t i = 0; i < sizeof(words)/sizeof(words[0]); i++) {
    ASSERT_TRUE(DecodeUTF8RunesInString(words[i], uni));
    res.push_back(make_pair(uni.size() - 1, trie.Find(uni.begin(), uni.end())));
  }
  vector<pair<size_t, const DictUnit*> > vec;
  vector<struct Dag> dags;
  ASSERT_TRUE(DecodeUTF8RunesInString(word, uni));
  trie.Find(uni.begin(), uni.end(), dags);
  ASSERT_EQ(dags.size(), uni.size());
  ASSERT_NE(dags.size(), 0u);
  s1 << res;
  s2 << dags[0].nexts;
  ASSERT_EQ(s1, s2);
}

TEST(DictTrieTest, UserDict) {
  DictTrie trie(DICT_FILE, TEST_DATA_DIR "/userdict.utf8");
  string word = "云计算";
  cppjieba::RuneStrArray unicode;
  ASSERT_TRUE(DecodeUTF8RunesInString(word, unicode));
  const DictUnit * unit = trie.Find(unicode.begin(), unicode.end());
  ASSERT_TRUE(unit != NULL);
  ASSERT_NEAR(unit->weight, -14.100, 0.001);

  word = "蓝翔";
  ASSERT_TRUE(DecodeUTF8RunesInString(word, unicode));
  unit = trie.Find(unicode.begin(), unicode.end());
  ASSERT_TRUE(unit != NULL);
  ASSERT_EQ(unit->tag, "nz");
  ASSERT_NEAR(unit->weight, -14.100, 0.001);

  word = "区块链";
  ASSERT_TRUE(DecodeUTF8RunesInString(word, unicode));
  unit = trie.Find(unicode.begin(), unicode.end());
  ASSERT_TRUE(unit != NULL);
  ASSERT_EQ(unit->tag, "nz");
  ASSERT_NEAR(unit->weight, -15.6478, 0.001);
}

TEST(DictTrieTest, UserDictWithMaxWeight) {
  DictTrie trie(DICT_FILE, TEST_DATA_DIR "/userdict.utf8", DictTrie::WordWeightMax);
  string word = "云计算";
  cppjieba::RuneStrArray unicode;
  ASSERT_TRUE(DecodeUTF8RunesInString(word, unicode));
  const DictUnit * unit = trie.Find(unicode.begin(), unicode.end());
  ASSERT_TRUE(unit);
  ASSERT_NEAR(unit->weight, -2.975, 0.001);
}

TEST(DictTrieTest, Dag) {
  DictTrie trie(DICT_FILE, TEST_DATA_DIR "/userdict.utf8");

  {
    string word = "清华大学";
    cppjieba::RuneStrArray unicode;
    ASSERT_TRUE(DecodeUTF8RunesInString(word, unicode));
    vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res);

    size_t nexts_sizes[] = {3, 2, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }

  {
    string word = "北京邮电大学";
    cppjieba::RuneStrArray unicode;
    ASSERT_TRUE(DecodeUTF8RunesInString(word, unicode));
    vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res);

    size_t nexts_sizes[] = {3, 1, 2, 2, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }

  {
    string word = "长江大桥";
    cppjieba::RuneStrArray unicode;
    ASSERT_TRUE(DecodeUTF8RunesInString(word, unicode));
    vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res);

    size_t nexts_sizes[] = {3, 1, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }

  {
    string word = "长江大桥";
    cppjieba::RuneStrArray unicode;
    ASSERT_TRUE(DecodeUTF8RunesInString(word, unicode));
    vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res, 3);

    size_t nexts_sizes[] = {2, 1, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }

  {
    string word = "长江大桥";
    cppjieba::RuneStrArray unicode;
    ASSERT_TRUE(DecodeUTF8RunesInString(word, unicode));
    vector<struct Dag> res;
    trie.Find(unicode.begin(), unicode.end(), res, 4);

    size_t nexts_sizes[] = {3, 1, 2, 1};
    ASSERT_EQ(res.size(), sizeof(nexts_sizes)/sizeof(nexts_sizes[0]));
    for (size_t i = 0; i < res.size(); i++) {
      ASSERT_EQ(res[i].nexts.size(), nexts_sizes[i]);
    }
  }
}
