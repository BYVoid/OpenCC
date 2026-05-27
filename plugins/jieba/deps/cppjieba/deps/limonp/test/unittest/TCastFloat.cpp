#include "limonp/CastFloat.hpp"
#include "gtest/gtest.h"
#include <cmath>

using namespace std;
using namespace limonp;
using namespace CastFloat;

TEST(CastFunctTest, Test1) {
  ASSERT_LT(abs(shortBitsToFloat(127) - 3.43025e-05), 0.0001);
  ASSERT_LT(abs(-122880 - shortBitsToFloat(-128)), 0.0001);
  ASSERT_EQ(shortBitsToFloat(0), 0);
  //for(short i = -128 ; i <= 127; i++)
  //{
  //    float f = shortBitsToFloat(i);
  //    cout<<i<<" "<<f<<endl;
  //}
}

