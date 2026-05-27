#include "cppjieba/SegmentBase.hpp"
#include "cppjieba/MixSegment.hpp"
#include "cppjieba/MPSegment.hpp"
#include "cppjieba/HMMSegment.hpp"
#include "cppjieba/FullSegment.hpp"
#include "cppjieba/QuerySegment.hpp"
#include "gtest/gtest.h"
#include "test_paths.h"

using namespace cppjieba;

TEST(MixSegmentTest, Test1) {
  MixSegment segment(DICT_DIR "/jieba.dict.utf8", DICT_DIR "/hmm_model.utf8");
  string sentence;
  vector<string> words;
  string actual;
  string expected;

  {
    sentence = "我来自北京邮电大学。。。学号123456，用AK47";
    expected = "我/来自/北京邮电大学/。/。/。/学号/123456/，/用/AK47";
    segment.Cut(sentence, words);
    actual = Join(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }

  {
    sentence = "B超 T恤";
    expected = "B超/ /T恤";
    segment.Cut(sentence, words);
    actual = Join(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }

  {
    // Digit+letter combinations should be kept together, matching Python jieba behavior
    sentence = "5G网络速度快";
    expected = "5G/网络/速度/快";
    segment.Cut(sentence, words);
    actual = Join(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }

  {
    // Decimal number followed by letters: decimal and letters remain separate
    sentence = "3.5KG重量";
    expected = "3.5/KG/重量";
    segment.Cut(sentence, words);
    actual = Join(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }

  {
    sentence = "他来到了网易杭研大厦";
    expected = "他/来到/了/网易/杭/研/大厦";
    segment.Cut(sentence, words, false);
    actual = Join(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }

  {
    sentence = "他来到了网易杭研大厦";
    expected = "他/来到/了/网易/杭研/大厦";
    segment.Cut(sentence, words);
    actual = Join(words.begin(), words.end(), "/");
    ASSERT_EQ(actual, expected);
  }
}

TEST(MixSegmentTest, NoUserDict) {
  MixSegment segment(TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8", DICT_DIR "/hmm_model.utf8");
  const char* str = "令狐冲是云计算方面的专家";
  vector<string> words;
  segment.Cut(str, words);
  string res;
  ASSERT_EQ("[\"令狐冲\", \"是\", \"云\", \"计算\", \"方面\", \"的\", \"专家\"]", res << words);
}

TEST(MixSegmentTest, UserDict) {
  MixSegment segment(TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8", 
                    DICT_DIR "/hmm_model.utf8", 
                    DICT_DIR "/user.dict.utf8");
  {
    const char* str = "令狐冲是云计算方面的专家";
    vector<string> words;
    segment.Cut(str, words);
    string res;
    ASSERT_EQ("[\"令狐冲\", \"是\", \"云计算\", \"方面\", \"的\", \"专家\"]", res << words);
  }
  {
    const char* str = "小明先就职于IBM,后在日本京都大学深造";
    vector<string> words;
    segment.Cut(str, words);
    string res;
    res << words;
    ASSERT_EQ("[\"小明\", \"先\", \"就职\", \"于\", \"IBM\", \",\", \"后\", \"在\", \"日本\", \"京都大学\", \"深造\"]", res);
  }
  {
    const char* str = "IBM,3.14";
    vector<string> words;
    segment.Cut(str, words);
    string res;
    res << words;
    ASSERT_EQ("[\"IBM\", \",\", \"3.14\"]", res);
  }
}

TEST(MixSegmentTest, TestUserDict) {
  MixSegment segment(TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8", DICT_DIR "/hmm_model.utf8", 
        TEST_DATA_DIR "/userdict.utf8");
  vector<string> words;
  string res;

  segment.Cut("令狐冲是云计算方面的专家", words);
  ASSERT_EQ("[\"令狐冲\", \"是\", \"云计算\", \"方面\", \"的\", \"专家\"]", res << words);

  segment.Cut("小明先就职于IBM,后在日本京都大学深造", words);
  res << words;
  ASSERT_EQ("[\"小明\", \"先\", \"就职\", \"于\", \"I\", \"B\", \"M\", \",\", \"后\", \"在\", \"日本\", \"京都大学\", \"深造\"]", res);

  segment.Cut("IBM,3.14", words);
  res << words;
  ASSERT_EQ("[\"I\", \"B\", \"M\", \",\", \"3.14\"]", res);

  segment.Cut("忽如一夜春风来，千树万树梨花开", words);
  res = limonp::Join(words.begin(), words.end(), "/");
  ASSERT_EQ("忽如一夜春风来/，/千树/万树/梨花/开", res);

  // rand input
  {
    const size_t ITERATION = 16;
    const size_t MAX_LEN = 256;
    string s;
    srand(time(NULL));

    for (size_t i = 0; i < ITERATION; i++) {
      size_t len = rand() % MAX_LEN;
      s.resize(len);
      for (size_t j = 0; j < len; j++) {
        s[rand() % len] = rand();
      }
      segment.Cut(s, words);
    }
  }
}

TEST(MixSegmentTest, TestMultiUserDict) {
  MixSegment segment(TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8", DICT_DIR "/hmm_model.utf8", 
        TEST_DATA_DIR "/userdict.utf8;" TEST_DATA_DIR "/userdict.2.utf8");
  vector<string> words;
  string res;

  segment.Cut("忽如一夜春风来，千树万树梨花开", words);
  res = limonp::Join(words.begin(), words.end(), "/");
  ASSERT_EQ("忽如一夜春风来/，/千树万树梨花开", res);
}

TEST(MPSegmentTest, Test1) {
  MPSegment segment(DICT_DIR "/jieba.dict.utf8");
  string s;
  vector<string> words;
  segment.Cut("我来自北京邮电大学。", words);
  ASSERT_EQ("[\"我\", \"来自\", \"北京邮电大学\", \"。\"]", s << words);

  segment.Cut("B超 T恤", words);
  ASSERT_EQ(s << words, "[\"B超\", \" \", \"T恤\"]");

  segment.Cut("南京市长江大桥", words);
  ASSERT_EQ("[\"南京市\", \"长江大桥\"]", s << words);

  // MaxWordLen
  segment.Cut("南京市长江大桥", words, 3);
  ASSERT_EQ("[\"南京市\", \"长江\", \"大桥\"]", s << words);

  segment.Cut("南京市长江大桥", words, 0);
  ASSERT_EQ("[\"南\", \"京\", \"市\", \"长\", \"江\", \"大\", \"桥\"]", s << words);

  segment.Cut("湖南长沙市天心区", words);
  s = Join(words.begin(), words.end(), "/");
  ASSERT_EQ("湖南长沙市/天心区", s);

  segment.Cut("湖南长沙市天心区", words, 3);
  s = Join(words.begin(), words.end(), "/");
  ASSERT_EQ("湖南/长沙市/天心区", s);
}

TEST(HMMSegmentTest, Test1) {
  HMMSegment segment(DICT_DIR "/hmm_model.utf8");
  {
    const char* str = "我来自北京邮电大学。。。学号123456";
    const char* res[] = {"我来", "自北京", "邮电大学", "。", "。", "。", "学号", "123456"};
    vector<string> words;
    segment.Cut(str, words);
    ASSERT_EQ(words, vector<string>(res, res + sizeof(res)/sizeof(res[0])));
  }

  {
    const char* str = "IBM,1.2,123";
    const char* res[] = {"IBM", ",", "1.2", ",", "123"};
    vector<string> words;
    segment.Cut(str, words);
    ASSERT_EQ(words, vector<string>(res, res + sizeof(res)/sizeof(res[0])));
  }
}

TEST(HMMSegmentTest, AlphanumericCombinations) {
  // Tests for digit+letter and letter+decimal combinations to align with
  // Python jieba finalseg behavior: re_skip = "[a-zA-Z0-9]+(?:\.\d+)?"
  HMMSegment segment(DICT_DIR "/hmm_model.utf8");
  vector<string> words;

  {
    // digit followed by letter (no decimal) should stay together
    const char* str = "5G";
    const char* res[] = {"5G"};
    segment.Cut(str, words);
    ASSERT_EQ(words, vector<string>(res, res + sizeof(res)/sizeof(res[0])));
  }

  {
    // digit followed by letter then Chinese
    const char* str = "3D打印";
    const char* res[] = {"3D", "打印"};
    segment.Cut(str, words);
    ASSERT_EQ(words, vector<string>(res, res + sizeof(res)/sizeof(res[0])));
  }

  {
    // decimal number followed by letters: decimal and letters are separate
    const char* str = "3.5KG";
    const char* res[] = {"3.5", "KG"};
    segment.Cut(str, words);
    ASSERT_EQ(words, vector<string>(res, res + sizeof(res)/sizeof(res[0])));
  }

  {
    // letter followed by digit followed by decimal
    const char* str = "v1.2";
    const char* res[] = {"v1.2"};
    segment.Cut(str, words);
    ASSERT_EQ(words, vector<string>(res, res + sizeof(res)/sizeof(res[0])));
  }
}

TEST(FullSegment, Test1) {
  FullSegment segment(TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8");
  vector<string> words;
  string s;

  segment.Cut("我来自北京邮电大学", words);
  s << words;
  ASSERT_EQ(s, "[\"我\", \"来自\", \"北京\", \"北京邮电大学\", \"邮电\", \"电大\", \"大学\"]");

  
  segment.Cut("上市公司CEO", words);
  s << words;
  ASSERT_EQ(s, "[\"上市\", \"公司\", \"C\", \"E\", \"O\"]");
}

TEST(FullSegment, NullDictUnitDoesNotSkipFollowingRune) {
  FullSegment segment(DICT_DIR "/jieba.dict.utf8");
  vector<string> words;

  segment.Cut("崎岖的牙齿", words);
  ASSERT_EQ("崎岖/的/牙齿", Join(words.begin(), words.end(), "/"));
}

TEST(QuerySegment, Test1) {
  QuerySegment segment(DICT_DIR "/jieba.dict.utf8", DICT_DIR "/hmm_model.utf8", "");
  vector<string> words;
  string s1, s2;

  segment.Cut("小明硕士毕业于中国科学院计算所，后在日本京都大学深造", words);
  s1 = Join(words.begin(), words.end(), "/");
  s2 = "小明/硕士/毕业/于/中国/科学/学院/科学院/中国科学院/计算/计算所/，/后/在/日本/京都/大学/日本京都大学/深造";
  ASSERT_EQ(s1, s2);

  segment.Cut("亲口交代", words);
  s1 = Join(words.begin(), words.end(), "/");
  s2 = "亲口/交代";
  ASSERT_EQ(s1, s2);

  segment.Cut("他心理健康", words);
  s1 = Join(words.begin(), words.end(), "/");
  s2 = "他/心理/健康/心理健康";
  ASSERT_EQ(s1, s2);
}

TEST(QuerySegment, Test2) {
  QuerySegment segment(TEST_DATA_DIR "/extra_dict/jieba.dict.small.utf8", 
                      DICT_DIR "/hmm_model.utf8", 
                      TEST_DATA_DIR "/userdict.utf8|" TEST_DATA_DIR "/userdict.english");
  vector<string> words;
  string s1, s2;

  {
    segment.Cut("小明硕士毕业于中国科学院计算所，后在日本京都大学深造", words);
    s1 = Join(words.begin(), words.end(), "/");
    s2 = "小明/硕士/毕业/于/中国/科学/学院/科学院/中国科学院/计算/计算所/，/后/在/日本/京都/大学/京都大学/深造";
    ASSERT_EQ(s1, s2);
  }

  {
    segment.Cut("小明硕士毕业于中国科学院计算所iPhone6", words);
    s1 = Join(words.begin(), words.end(), "/");
    s2 = "小明/硕士/毕业/于/中国/科学/学院/科学院/中国科学院/计算/计算所/iPhone6";
    ASSERT_EQ(s1, s2);
  }

  {
    segment.Cut("中国科学院", words);
    s1 = Join(words.begin(), words.end(), "/");
    s2 = "中国/科学/学院/科学院/中国科学院";
    ASSERT_EQ(s1, s2);
  }
}

TEST(MPSegmentTest, Unicode32) {
  string s("天气很好，🙋 我们去郊游。");
  vector<string> words;

  MPSegment segment(DICT_DIR "/jieba.dict.utf8");
  segment.Cut(s, words);

  ASSERT_EQ(Join(words.begin(), words.end(), "/"), "天气/很/好/，/🙋/ /我们/去/郊游/。");
}

// Regression test for heap corruption ("corrupted size vs. prev_size") with
// input strings larger than 2114 bytes. The bug was caused by using
// LocalVector<RuneStr> (only safe for primitive types) for RuneStrArray.
TEST(MixSegmentTest, LongInput) {
  // 2114 is the byte length threshold beyond which the original heap
  // corruption was triggered when RuneStrArray used LocalVector<RuneStr>.
  const size_t HEAP_CORRUPTION_THRESHOLD = 2114;

  MixSegment segment(DICT_DIR "/jieba.dict.utf8", DICT_DIR "/hmm_model.utf8");
  vector<string> words;

  // Test with a long Chinese string (> HEAP_CORRUPTION_THRESHOLD bytes)
  string phrase = "我来到北京清华大学进行学习和研究工作，非常愉快，让我有了很大的收获。";
  string long_chinese;
  while (long_chinese.size() < HEAP_CORRUPTION_THRESHOLD + 1000) {
    long_chinese += phrase;
  }
  ASSERT_GT(long_chinese.size(), HEAP_CORRUPTION_THRESHOLD);
  segment.Cut(long_chinese, words);
  ASSERT_GT(words.size(), size_t(0));

  // Test with a long ASCII string (> HEAP_CORRUPTION_THRESHOLD bytes)
  string long_ascii(HEAP_CORRUPTION_THRESHOLD + 1000, 'a');
  words.clear();
  segment.Cut(long_ascii, words);
  ASSERT_GT(words.size(), size_t(0));

  // Test with a very long string (> 6000 bytes)
  string very_long_chinese;
  while (very_long_chinese.size() < 6000) {
    very_long_chinese += phrase;
  }
  ASSERT_GT(very_long_chinese.size(), size_t(6000));
  words.clear();
  segment.Cut(very_long_chinese, words);
  ASSERT_GT(words.size(), size_t(0));
}
