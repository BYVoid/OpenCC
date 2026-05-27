#include "limonp/StdExtension.hpp"
#include "limonp/StringUtil.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <deque>

using namespace limonp;
TEST(StdOutbound, Test1) {
  ifstream ifs("../test/testdata/StdExtension.data");
  string s;
  s << ifs;
  ASSERT_EQ("key1 = val1\n##this is comment\nkey2=val2\n", s);

  const char * outFileName = "stdoutbuond.test.tmp";
  ofstream ofs(outFileName);
  s = outFileName;
  ofs << s;
  ASSERT_TRUE(!!ofs);
  ofs.close();

  ifstream ifs2(outFileName);
  s << ifs2;
  ASSERT_EQ(outFileName, s);
}

TEST(StdOutbound, Funct_IsIn) {
  map<int, int> mp;
  for(size_t i = 1; i < 5; i ++) {
    mp[i] = i+1;
  }

  ASSERT_TRUE(IsIn(mp, 1));
  ASSERT_FALSE(IsIn(mp, 0));
}

TEST(StdOutbound, Test2) {
  map<int, int> mp1;
  mp1[1] = 2;
  mp1[2] = 3;
  string s;
  ASSERT_EQ(s << mp1, "{1:2, 2:3}");
  unordered_map<int,int> mp2;
  mp2[1] = 2;
  mp2[2] = 3;
  s << mp2;
  ASSERT_TRUE( s == "{1:2, 2:3}" || s == "{2:3, 1:2}");
}

TEST(StdOutbound, Test3) {
  {
    vector<string> v;
    v.push_back("1");
    v.push_back("2");
    string s;
    ASSERT_EQ(s << v, "[\"1\", \"2\"]");
  }
  {
    deque<string> v;
    v.push_back("1");
    v.push_back("2");
    string s;
    ASSERT_EQ(s << v, "[\"1\", \"2\"]");
  }
}

struct TestWord {
  string word;
  size_t offset;
}; // struct TestWord

ostream& operator << (ostream& os, const TestWord& w) {
  return os << "{\"word\": \"" << w.word << "\", \"offset\": " << w.offset << "}";
}

TEST(StdOutbound, TestWord) {
  {
    vector<string> v;
    v.push_back("1");
    v.push_back("2");
    string s;
    ASSERT_EQ(s << v, "[\"1\", \"2\"]");
  }
  {
    vector<TestWord> v;
    TestWord w;
    w.word = "hello";
    w.offset = 0;
    v.push_back(w);
    w.word = "world";
    w.offset = 5;
    v.push_back(w);
    string s;
    ASSERT_EQ(s << v, "[{\"word\": \"hello\", \"offset\": 0}, {\"word\": \"world\", \"offset\": 5}]");
  }
}
