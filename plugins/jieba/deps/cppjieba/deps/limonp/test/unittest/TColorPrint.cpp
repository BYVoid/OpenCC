#include "gtest/gtest.h"
#include "limonp/Colors.hpp"
#include "limonp/Logging.hpp"

using namespace limonp;

TEST(ColorPrint, Test1) {
  ColorPrintln(RED, "hello %s", "world");
  ColorPrintln(RED, "hello %s", "world");
  XLOG(INFO) << "hello world";
  ColorPrintln(RED, "hello %s", "world");
  ColorPrintln(GREEN, "hello %s", "world");
  XLOG(INFO) << "hello world";
  ColorPrintln(BLACK, "hello %s", "world");
  ColorPrintln(YELLOW, "hello %s", "world");
  ColorPrintln(BLUE, "hello %s", "world");
  ColorPrintln(PURPLE, "hello %s", "world");
  ColorPrintln(PURPLE, "hello %s", "world", " and colors");
}

