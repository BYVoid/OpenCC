#include "limonp/Config.hpp"
#include "gtest/gtest.h"

using namespace limonp;
TEST(COnfigTest, Test1) {
  Config conf("../test/testdata/1.conf");
  string res;
  ASSERT_EQ("{key1:val1, key2:val2}", res << conf);
  res = conf.Get("key1", "");
  ASSERT_EQ("val1", res);
  res = conf.Get("key2", "");
  ASSERT_EQ("val2", res);
  const char * str = conf["key3"];
  ASSERT_FALSE(str);
  ASSERT_TRUE(0 == strcmp(conf["key2"], "val2"));
}

