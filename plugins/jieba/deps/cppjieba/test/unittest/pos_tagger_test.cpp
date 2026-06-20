#include "cppjieba/MixSegment.hpp"
#include "gtest/gtest.h"
#include "test_paths.h"

using namespace cppjieba;

static const char * const QUERY_TEST1 = "我是蓝翔技工拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上总经理，出任CEO，迎娶白富美，走上人生巅峰。";
static const char * const ANS_TEST1 = "[我:r, 是:v, 蓝翔:x, 技工:n, 拖拉机:n, 学院:n, 手扶拖拉机:n, 专业:n, 的:uj, 。:x, 不用:v, 多久:m, ，:x, 我:r, 就:d, 会:v, 升职:v, 加薪:nr, ，:x, 当上:t, 总经理:n, ，:x, 出任:v, CEO:eng, ，:x, 迎娶:v, 白富:x, 美:ns, ，:x, 走上:v, 人生:n, 巅峰:n, 。:x]";
static const char * const QUERY_TEST2 = "我是蓝翔技工拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上总经理，出任CEO，迎娶白富美，走上人生巅峰。";
static const char * const ANS_TEST2 = "[我:r, 是:v, 蓝翔:nz, 技工:n, 拖拉机:n, 学院:n, 手扶拖拉机:n, 专业:n, 的:uj, 。:x, 不用:v, 多久:m, ，:x, 我:r, 就:d, 会:v, 升职:v, 加薪:nr, ，:x, 当上:t, 总经理:n, ，:x, 出任:v, CEO:eng, ，:x, 迎娶:v, 白富:x, 美:ns, ，:x, 走上:v, 人生:n, 巅峰:n, 。:x]";

static const char * const QUERY_TEST3 = "iPhone6手机的最大特点是很容易弯曲。";
static const char * const ANS_TEST3 = "[iPhone6:eng, 手机:n, 的:uj, 最大:a, 特点:n, 是:v, 很:zg, 容易:a, 弯曲:v, 。:x]";
//static const char * const ANS_TEST3 = "";

TEST(PosTaggerTest, Test) {
  MixSegment tagger(DICT_DIR "/jieba.dict.utf8", DICT_DIR "/hmm_model.utf8");
  {
    vector<pair<string, string> > res;
    tagger.Tag(QUERY_TEST1, res);
    string s;
    s << res;
    ASSERT_TRUE(s == ANS_TEST1);
  }
}
TEST(PosTagger, TestUserDict) {
  MixSegment tagger(DICT_DIR "/jieba.dict.utf8", DICT_DIR "/hmm_model.utf8", TEST_DATA_DIR "/userdict.utf8");
  {
    vector<pair<string, string> > res;
    tagger.Tag(QUERY_TEST2, res);
    string s;
    s << res;
    ASSERT_EQ(s, ANS_TEST2);
  }
  {
    vector<pair<string, string> > res;
    tagger.Tag(QUERY_TEST3, res);
    string s;
    s << res;
    ASSERT_EQ(s, ANS_TEST3);
  }
}
