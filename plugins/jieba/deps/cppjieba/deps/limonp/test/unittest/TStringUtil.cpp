#include "limonp/StringUtil.hpp"
#include "gtest/gtest.h"
using namespace limonp;

TEST(StringUtilTest, Test1) {
  vector<string> vec;
  string s;
  Split("\t1\t3\t4\t", vec, "\t");
  ASSERT_EQ(s << vec, "[\"\", \"1\", \"3\", \"4\"]");
  s = " \t\n ni hao ad \r\n";
  ASSERT_EQ("ni hao ad", Trim(s));
  ASSERT_EQ("select * from table1  limit 1;" ,StringFormat("select %s from %s  %s;", "*","table1","limit 1"));
  s = StringFormat("select %s from %s  %s;", "*","table1","limit 1");
  ASSERT_EQ("select * from table1  limit 1;" ,s);
  vec.clear();
  vec.push_back("1");
  vec.push_back("2");
  vec.push_back("3");
  s.clear();
  Join(vec.begin(), vec.end(), s,",");
  ASSERT_EQ("1,2,3",s);
  s = Join(vec.begin(), vec.end(), "..");
  ASSERT_EQ("1..2..3", s);
  const char* arr[] = {"2","3","5"};
  ASSERT_EQ("2,3,5", Join(arr, arr + sizeof(arr)/sizeof(arr[0]), ","));
  map<string , int> mp;
  mp["key1"] =2;
  ASSERT_EQ("{key1:2}", s <<  mp);
  std::unordered_map<int,int> hmp;
  hmp[1]=2;
  ASSERT_EQ("{1:2}", s <<  hmp);
}

TEST(StringUtilTest, Test2) {
  string s, gbks;
  ifstream ifs("../test/testdata/dict.gbk");
  ASSERT_TRUE(!!ifs);

  vector<uint16_t> uni;
  while(getline(ifs, s)) {
    GBKTrans(s, uni);
    GBKTrans(uni.begin(), uni.end(), gbks);
    ASSERT_EQ(s, gbks);
  }
}

TEST(StringUtilTest, Test3) {
  string s, utf8;
  ifstream ifs("../test/testdata/dict.utf8");
  ASSERT_TRUE(!!ifs);

  vector<uint16_t> uni;
  while(getline(ifs, s)) {
    ASSERT_TRUE(Utf8ToUnicode(s, uni));
    UnicodeToUtf8(uni.begin(), uni.end(), utf8);
    ASSERT_EQ(s, utf8);
  }
}

TEST(StringUtilTest, Test4) {
  //ASSERT_TRUE(StartsWith("--help",NULL));
  ASSERT_TRUE(StartsWith("--help","--"));
  ASSERT_TRUE(StartsWith("--help","-"));
  ASSERT_FALSE(StartsWith("--help","he"));
  ASSERT_TRUE(StartsWith("help","help"));
  ASSERT_FALSE(StartsWith("","help"));
  ASSERT_TRUE(StartsWith("hel",""));
  ASSERT_TRUE(EndsWith("hel",""));
  ASSERT_TRUE(EndsWith("hel","el"));
}

TEST(StringUtilTest, Test5) {
  const char* str = "1,2,3,4";
  vector<string> vec;
  string res;
  Split(str, vec, ",");
  ASSERT_EQ("[\"1\", \"2\", \"3\", \"4\"]", res << vec);
  Split("1,2,3,4,", vec, ",");
  ASSERT_EQ("[\"1\", \"2\", \"3\", \"4\"]", res << vec);
  Split(str, vec, ",", 3);
  ASSERT_EQ("[\"1\", \"2\", \"3\", \"4\"]", res << vec);

  Split("1", vec, ",");
  ASSERT_EQ("[\"1\"]", res << vec);

  Split(str, vec, ",", 1);
  ASSERT_EQ("[\"1\", \"2,3,4\"]", res << vec);

  Split("", vec, ",");
  ASSERT_EQ("[]", res << vec);
  
  Split("1, 2", vec, ",");
  ASSERT_EQ("[\"1\", \" 2\"]", res << vec);

  Split("1==2", vec, "==");
  ASSERT_EQ("[\"1\", \"\", \"2\"]", res << vec);

  Split("1,", vec, ",");
  ASSERT_EQ("[\"1\"]", res << vec);

  Split(",1,", vec, ",");
  ASSERT_EQ("[\"\", \"1\"]", res << vec);

  Split("1, ", vec, ",");
  ASSERT_EQ("[\"1\", \" \"]", res << vec);

  res << Split("1|2,3", "|,");
  ASSERT_EQ("[\"1\", \"2\", \"3\"]", res);
}

TEST(StringUtilTest, Trim) {
  string s;
  s = "xxxyyyxx";
  ASSERT_EQ(RTrim(s, 'x'), "xxxyyy");
  ASSERT_EQ(LTrim(s, 'x'), "yyy");
  s = "xxxyyyxx";
  ASSERT_EQ(Trim(s, 'x'), "yyy");

  s = " x y ";
  ASSERT_EQ(Trim(s), "x y");

  // check if it core dump when using isalpha
  wchar_t w = 1000024;
  ASSERT_FALSE(IsSpace(w));
  w = 0x20;
  ASSERT_TRUE(IsSpace(w));
}

TEST(StringUtilTest, GetTime) {
  string s;
  GetTime("%Y-%m-%d %H:%M:%S", s);
  //print(s);
}

TEST(StringUtilTest, PathJoin) {
  const char * path1 = "/home/foo/dir";
  const char * path2 = "file";
  const char * path3 = "/home/foo/dir/";
  const char * path4 = "file";
  const char * answer = "/home/foo/dir/file";

  ASSERT_EQ(answer, PathJoin(path1, path2));
  ASSERT_EQ(answer, PathJoin(path3, path4));
}

TEST(StringUtilTest, JapaneseUnicode) {
  // Japanese
  const char* s = "がんば";
  vector<uint16_t> unicode;
  ASSERT_TRUE(Utf8ToUnicode(s, unicode));
  ASSERT_EQ(3u, unicode.size());
}

TEST(StringUtilTest, RareChinese) {
  //U+10000 – U+10FFFF
  const char* s = "𪚥";
  vector<uint16_t> unicode;
  ASSERT_FALSE(Utf8ToUnicode(s, unicode));
  ASSERT_EQ(0u, unicode.size());
}

TEST(StringUtilTest, RareChineseUnicode32) {
  //U+10000 – U+10FFFF
  const char* s = "𪚥";
  vector<uint32_t> unicode;
  ASSERT_TRUE(Utf8ToUnicode32(s, unicode));
  ASSERT_EQ(1u, unicode.size());

  string s2;
  Unicode32ToUtf8(unicode.begin(), unicode.end(), s2);
  ASSERT_EQ(s2, s);
}

TEST(StringUtilTest, Unicode32) {
  const char* s = "1+1=2你好世界，。";
  vector<uint32_t> unicode;
  ASSERT_TRUE(Utf8ToUnicode32(s, unicode));
  ASSERT_EQ(unicode.size(), 11u);

  string s2;
  Unicode32ToUtf8(unicode.begin(), unicode.end(), s2);
  ASSERT_EQ(s2, s);
}
