#include <iostream>
#include "opencc.h"

int main() {
  const opencc::SimpleConverter converter("s2twp.json");
  std::cout << converter.Convert("网络") << std::endl;  // 網路
  return 0;
}
