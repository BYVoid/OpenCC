#include "gtest/gtest.h"
#include "limonp/Closure.hpp"

using namespace limonp;

void Foo0() {
}

void Foo1(int x) {
}

void Foo2(int x, float y) {
}

void Foo3(int x, float y, double z) {
}

class Obj {
 public:
  void Foo0() {
  }
  void Foo1(int x) {
  }
  void Foo2(int x, float y) {
  }
  void Foo3(int x, float y, double z) {
  }
}; 

TEST(Closure, Test0) {
  ClosureInterface* c;

  c = NewClosure(&Foo0);
  ASSERT_TRUE(c != NULL);
  c->Run();
  delete c;
  c = NULL;

  c = NewClosure(&Foo1, 1);
  ASSERT_TRUE(c != NULL);
  c->Run();
  delete c;
  c = NULL;

  c = NewClosure(&Foo2, 1, float(2));
  ASSERT_TRUE(c != NULL);
  c->Run();
  delete c;
  c = NULL;

  c = NewClosure(&Foo3, 1, float(2), double(3));
  ASSERT_TRUE(c != NULL);
  c->Run();
  delete c;
  c = NULL;
}

TEST(Closure, Test1) {
  ClosureInterface* c;

  Obj obj;
  c = NewClosure(&obj, &Obj::Foo0);
  ASSERT_TRUE(c != NULL);
  c->Run();
  delete c;
  c = NULL;

  c = NewClosure(&obj, &Obj::Foo1, 1);
  ASSERT_TRUE(c != NULL);
  c->Run();
  delete c;
  c = NULL;

  c = NewClosure(&obj, &Obj::Foo2, 1, float(2));
  ASSERT_TRUE(c != NULL);
  c->Run();
  delete c;
  c = NULL;

  c = NewClosure(&obj, &Obj::Foo3, 1, float(2), double(3));
  ASSERT_TRUE(c != NULL);
  c->Run();
  delete c;
  c = NULL;
}
