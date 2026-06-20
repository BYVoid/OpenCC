#ifndef CPPJIEBA_UTILS_HPP
#define CPPJIEBA_UTILS_HPP

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef XLOG
#error "XLOG has been defined already"
#endif
#ifdef XCHECK
#error "XCHECK has been defined already"
#endif

#define XLOG(level) cppjieba::Logger(cppjieba::LL_##level, __FILE__, __LINE__).Stream()
#define XCHECK(exp) if (!(exp)) XLOG(FATAL) << "exp: [" #exp << "] false. "

namespace cppjieba {

using std::deque;
using std::ifstream;
using std::make_pair;
using std::map;
using std::min;
using std::ofstream;
using std::ostream;
using std::pair;
using std::set;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::unordered_set;
using std::vector;

enum {
  LL_DEBUG = 0,
  LL_INFO = 1,
  LL_WARNING = 2,
  LL_ERROR = 3,
  LL_FATAL = 4,
};

static const char* const LOG_LEVEL_ARRAY[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
static const char* const LOG_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

class Logger {
 public:
  Logger(size_t level, const char* filename, int lineno)
      : level_(level) {
#ifdef LOGGING_LEVEL
    if (level_ < LOGGING_LEVEL) {
      return;
    }
#endif
    assert(level_ <= sizeof(LOG_LEVEL_ARRAY) / sizeof(*LOG_LEVEL_ARRAY));

    char buf[32];
    time_t timeNow;
    time(&timeNow);

    struct tm tmNow;
#if defined(_WIN32) || defined(_WIN64)
    errno_t e = localtime_s(&tmNow, &timeNow);
    assert(e == 0);
    (void)e;
#else
    struct tm* tm_tmp = localtime_r(&timeNow, &tmNow);
    assert(tm_tmp != NULL);
    (void)tm_tmp;
#endif

    strftime(buf, sizeof(buf), LOG_TIME_FORMAT, &tmNow);

    stream_ << buf
            << " " << filename
            << ":" << lineno
            << " " << LOG_LEVEL_ARRAY[level_]
            << " ";
  }

  ~Logger() {
#ifdef LOGGING_LEVEL
    if (level_ < LOGGING_LEVEL) {
      return;
    }
#endif
    std::cerr << stream_.str() << std::endl;
    if (level_ == LL_FATAL) {
      abort();
    }
  }

  std::ostream& Stream() {
    return stream_;
  }

 private:
  std::ostringstream stream_;
  size_t level_;
};

template <class T>
class LocalVector {
 public:
  typedef const T* const_iterator;
  typedef T value_type;
  typedef size_t size_type;

  LocalVector() {}
  LocalVector(const_iterator begin, const_iterator end) : data_(begin, end) {}
  LocalVector(size_t size, const T& value) : data_(size, value) {}

  T& operator[](size_t i) {
    return data_[i];
  }
  const T& operator[](size_t i) const {
    return data_[i];
  }

  void push_back(const T& value) {
    data_.push_back(value);
  }
  void reserve(size_t size) {
    data_.reserve(size);
  }
  bool empty() const {
    return data_.empty();
  }
  size_t size() const {
    return data_.size();
  }
  size_t capacity() const {
    return data_.capacity();
  }
  const_iterator begin() const {
    return data_.data();
  }
  const_iterator end() const {
    return data_.data() + data_.size();
  }
  void clear() {
    data_.clear();
  }

  bool operator==(const LocalVector<T>& rhs) const {
    return data_ == rhs.data_;
  }
  bool operator!=(const LocalVector<T>& rhs) const {
    return !(*this == rhs);
  }

 private:
  vector<T> data_;
};

inline string StringFormat(const char* fmt, ...) {
  int size = 256;
  string str;
  va_list ap;
  while (true) {
    str.resize(size);
    va_start(ap, fmt);
    int n = vsnprintf(&str[0], size, fmt, ap);
    va_end(ap);
    if (n > -1 && n < size) {
      str.resize(n);
      return str;
    }
    if (n > -1) {
      size = n + 1;
    } else {
      size *= 2;
    }
  }
}

template <class T>
void Join(T begin, T end, string& res, const string& connector) {
  if (begin == end) {
    return;
  }
  stringstream ss;
  ss << *begin;
  ++begin;
  while (begin != end) {
    ss << connector << *begin;
    ++begin;
  }
  res = ss.str();
}

template <class T>
string Join(T begin, T end, const string& connector) {
  string res;
  Join(begin, end, res, connector);
  return res;
}

inline string& LTrim(string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
  return s;
}

inline string& RTrim(string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
  return s;
}

inline string& Trim(string& s) {
  return LTrim(RTrim(s));
}

inline bool StartsWith(const string& str, const string& prefix) {
  if (prefix.length() > str.length()) {
    return false;
  }
  return str.compare(0, prefix.length(), prefix) == 0;
}

inline void Split(const string& src, vector<string>& res, const string& pattern, size_t maxsplit = string::npos) {
  res.clear();
  size_t start = 0;
  size_t end = 0;
  while (start < src.size()) {
    end = src.find_first_of(pattern, start);
    if (end == string::npos || res.size() >= maxsplit) {
      res.push_back(src.substr(start));
      return;
    }
    res.push_back(src.substr(start, end - start));
    start = end + 1;
  }
}

inline vector<string> Split(const string& src, const string& pattern, size_t maxsplit = string::npos) {
  vector<string> res;
  Split(src, res, pattern, maxsplit);
  return res;
}

template <class KeyType, class ContainType>
bool IsIn(const ContainType& contain, const KeyType& key) {
  return contain.end() != contain.find(key);
}

template <class T1, class T2>
ostream& operator<<(ostream& os, const pair<T1, T2>& pr) {
  os << pr.first << ":" << pr.second;
  return os;
}

template <typename T>
ostream& operator<<(ostream& os, const vector<T>& v) {
  if (v.empty()) {
    return os << "[]";
  }
  os << "[" << v[0];
  for (size_t i = 1; i < v.size(); i++) {
    os << ", " << v[i];
  }
  os << "]";
  return os;
}

template <>
inline ostream& operator<<(ostream& os, const vector<string>& v) {
  if (v.empty()) {
    return os << "[]";
  }
  os << "[\"" << v[0];
  for (size_t i = 1; i < v.size(); i++) {
    os << "\", \"" << v[i];
  }
  os << "\"]";
  return os;
}

template <typename T>
ostream& operator<<(ostream& os, const deque<T>& dq) {
  if (dq.empty()) {
    return os << "[]";
  }
  os << "[\"" << dq[0];
  for (size_t i = 1; i < dq.size(); i++) {
    os << "\", \"" << dq[i];
  }
  os << "\"]";
  return os;
}

template <class T>
ostream& operator<<(ostream& os, const LocalVector<T>& vec) {
  if (vec.empty()) {
    return os << "[]";
  }
  os << "[\"" << vec[0];
  for (size_t i = 1; i < vec.size(); i++) {
    os << "\", \"" << vec[i];
  }
  os << "\"]";
  return os;
}

template <class T>
string& operator<<(string& str, const T& obj) {
  stringstream ss;
  ss << obj;
  return str = ss.str();
}

inline string& operator<<(string& s, ifstream& ifs) {
  return s.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

} // namespace cppjieba

#endif // CPPJIEBA_UTILS_HPP
