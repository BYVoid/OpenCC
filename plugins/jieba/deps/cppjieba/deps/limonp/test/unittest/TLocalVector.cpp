#include "limonp/LocalVector.hpp"
#include "limonp/StdExtension.hpp"
#include <fstream>
#include "gtest/gtest.h"

using namespace limonp;

TEST(LocalVector, test1) {
  LocalVector<size_t> vec;
  ASSERT_EQ(vec.size(), 0u);
  ASSERT_EQ(vec.capacity(), LOCAL_VECTOR_BUFFER_SIZE);
  ASSERT_TRUE(vec.empty());
  size_t size = 129;
  for(size_t i = 0; i < size; i++) {
    vec.push_back(i);
  }
  ASSERT_EQ(vec.size(), size);
  ASSERT_EQ(vec.capacity(), 256u);
  ASSERT_FALSE(vec.empty());
  LocalVector<size_t> vec2(vec);
  ASSERT_EQ(vec2.capacity(), vec.capacity());
  ASSERT_EQ(vec2.size(), vec.size());
}

TEST(LocalVector, test2) {
  LocalVector<size_t> vec;
  ASSERT_EQ(vec.size(), 0u);
  ASSERT_EQ(vec.capacity(), LOCAL_VECTOR_BUFFER_SIZE);
  ASSERT_TRUE(vec.empty());
  size_t size = 1;
  for(size_t i = 0; i < size; i++) {
    vec.push_back(i);
  }
  ASSERT_EQ(vec.size(), size);
  ASSERT_EQ(vec.capacity(), LOCAL_VECTOR_BUFFER_SIZE);
  ASSERT_FALSE(vec.empty());
  LocalVector<size_t> vec2;
  vec2 = vec;
  ASSERT_EQ(vec2.capacity(), vec.capacity());
  ASSERT_EQ(vec2.size(), vec.size());
}
