#define LOGGING_LEVEL LL_WARNING
#include "limonp/Logging.hpp"
#include "gtest/gtest.h"

TEST(Logging, Test1) {
  XLOG(DEBUG) << "xxx" << " yyy";
  XLOG(INFO) << "hello";
  XLOG(WARNING) << "hello";
  XLOG(ERROR) << "hello";
  //XCHECK(false) << "hello, " << "world";
  //XLOG(FATAL) << "hello";
}
