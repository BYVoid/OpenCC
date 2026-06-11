#ifndef CPPJIEBA_UTILS_HPP
#define CPPJIEBA_UTILS_HPP

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
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

enum LogLevel {
  LL_DEBUG = 0,
  LL_INFO = 1,
  LL_WARNING = 2,
  LL_ERROR = 3,
  LL_FATAL = 4,
};

class Logger {
 public:
  Logger(size_t level, const char* filename, int lineno)
      : level_(level) {
#ifdef LOGGING_LEVEL
    if (level_ < LOGGING_LEVEL) {
      return;
    }
#endif
    static const char* const kLogLevelNames[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    assert(level_ < sizeof(kLogLevelNames) / sizeof(*kLogLevelNames));

    char buf[32];
    time_t time_now;
    time(&time_now);
    struct tm tm_now;

#if defined(_WIN32) || defined(_WIN64)
    errno_t e = localtime_s(&tm_now, &time_now);
    assert(e == 0);
    (void)e;
#else
    struct tm* tm_tmp = localtime_r(&time_now, &tm_now);
    assert(tm_tmp != NULL);
    (void)tm_tmp;
#endif

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_now);
    stream_ << buf << " " << filename << ":" << lineno << " " << kLogLevelNames[level_] << " ";
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

template <class Iterator>
std::string Join(Iterator begin, Iterator end, const std::string& connector) {
  if (begin == end) {
    return std::string();
  }
  std::ostringstream os;
  os << *begin;
  ++begin;
  while (begin != end) {
    os << connector << *begin;
    ++begin;
  }
  return os.str();
}

template <class Iterator>
void Join(Iterator begin, Iterator end, std::string& result, const std::string& connector) {
  result = Join(begin, end, connector);
}

inline std::string StringFormat(const char* format, ...) {
  va_list args;
  va_start(args, format);
  va_list args_copy;
  va_copy(args_copy, args);
  int length = std::vsnprintf(NULL, 0, format, args_copy);
  va_end(args_copy);
  if (length < 0) {
    va_end(args);
    return std::string();
  }

  std::vector<char> buffer(static_cast<size_t>(length) + 1);
  std::vsnprintf(buffer.data(), buffer.size(), format, args);
  va_end(args);
  return std::string(buffer.data(), static_cast<size_t>(length));
}

inline void Split(const std::string& src,
                  std::vector<std::string>& result,
                  const std::string& delimiters,
                  size_t maxsplit = std::string::npos) {
  result.clear();
  size_t start = 0;
  while (start < src.size()) {
    size_t end = src.find_first_of(delimiters, start);
    if (end == std::string::npos || result.size() >= maxsplit) {
      result.push_back(src.substr(start));
      return;
    }
    result.push_back(src.substr(start, end - start));
    start = end + 1;
  }
}

inline std::vector<std::string> Split(const std::string& src,
                                      const std::string& delimiters,
                                      size_t maxsplit = std::string::npos) {
  std::vector<std::string> result;
  Split(src, result, delimiters, maxsplit);
  return result;
}

inline std::string& LTrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
  return s;
}

inline std::string& RTrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
  return s;
}

inline std::string& Trim(std::string& s) {
  return LTrim(RTrim(s));
}

inline bool StartsWith(const std::string& str, const std::string& prefix) {
  return prefix.size() <= str.size() && str.compare(0, prefix.size(), prefix) == 0;
}

template <class KeyType, class ContainerType>
bool IsIn(const ContainerType& container, const KeyType& key) {
  return container.find(key) != container.end();
}

template <class T1, class T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1, T2>& value) {
  return os << value.first << ":" << value.second;
}

template <class T>
std::string FormatVector(const std::vector<T>& values) {
  if (values.empty()) {
    return "[]";
  }
  std::ostringstream os;
  os << "[" << values[0];
  for (size_t i = 1; i < values.size(); ++i) {
    os << ", " << values[i];
  }
  os << "]";
  return os.str();
}

inline std::string FormatVector(const std::vector<std::string>& values) {
  if (values.empty()) {
    return "[]";
  }
  std::ostringstream os;
  os << "[\"" << values[0];
  for (size_t i = 1; i < values.size(); ++i) {
    os << "\", \"" << values[i];
  }
  os << "\"]";
  return os.str();
}

template <class T>
std::string& operator<<(std::string& result, const T& value) {
  std::ostringstream os;
  os << value;
  result = os.str();
  return result;
}

template <class T>
std::string& operator<<(std::string& result, const std::vector<T>& values) {
  result = FormatVector(values);
  return result;
}

} // namespace cppjieba

#endif // CPPJIEBA_UTILS_HPP
