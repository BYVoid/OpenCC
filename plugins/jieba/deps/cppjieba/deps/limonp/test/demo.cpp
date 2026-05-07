#include "limonp/StringUtil.hpp"
#include "limonp/Logging.hpp"

using namespace std;

#define print(x) std::cout << x << std::endl

int main(int argc, char** argv) {
  string strname = "hello, world";
  print(strname); //hello, world
  map<string, int> mp;
  mp["hello"] = 1;
  mp["world"] = 2;
  print(mp); // {"hello":1, "world":2}

  string res;
  res << mp;
  print(res); // {"hello":1, "world":2}

  string str;
  str = limonp::StringFormat("%s, %s", "hello", "world");
  print(str); //hello, world

  const char * a[] = {"hello", "world"};
  str = limonp::Join(a, a + sizeof(a)/sizeof(*a), ",");
  print(str); //hello,world

  str = "hello, world";
  vector<string> buf;
  limonp::Split(str, buf, ",");
  print(buf); //["hello", " world"]

  int arr[] = {1,10,100,1000};
  vector<int> vec_i(arr, arr + sizeof(arr)/sizeof(arr[0]));
  print(vec_i); //[1, 10, 100, 1000]

  XLOG(INFO) << "hello, world"; //2014-04-05 20:52:37 demo.cpp:35 INFO hello, world
  //In the same way, `LOG(DEBUG),LOG(WARNING),LOG(ERROR),LOG(FATAL)`.
  return EXIT_SUCCESS;
}
