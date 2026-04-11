#include "limonp/ArgvContext.hpp"
#include "gtest/gtest.h"

using namespace limonp;

TEST(ArgvContextTest, Test1) {
  const char * argv[] = {"./exe1", "--hehe", "11", "key2", "-k", "val"};
  string s;
  ArgvContext arg(sizeof(argv)/sizeof(argv[0]), argv);
  s<<arg;
  ASSERT_EQ(s, "[\"./exe1\", \"key2\"]{--hehe:11, -k:val}{}");
  ASSERT_EQ("key2", arg[1]);
  ASSERT_EQ("11", arg["--hehe"]);
  ASSERT_FALSE(arg.HasKey("-help"));
}

