#ifndef CPPJIEBA_UNICODE_FILE_HPP
#define CPPJIEBA_UNICODE_FILE_HPP

#include <fstream>
#include <limits>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace cppjieba {

#ifdef _WIN32
inline bool Utf8ToWidePath(const std::string& path, std::wstring& widePath) {
  if (path.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    return false;
  }
  if (path.empty()) {
    widePath.clear();
    return true;
  }

  const int pathSize = static_cast<int>(path.size());
  const int wideSize = MultiByteToWideChar(
      CP_UTF8,
      MB_ERR_INVALID_CHARS,
      path.data(),
      pathSize,
      NULL,
      0);
  if (wideSize <= 0) {
    return false;
  }

  widePath.resize(static_cast<size_t>(wideSize));
  return MultiByteToWideChar(
             CP_UTF8,
             MB_ERR_INVALID_CHARS,
             path.data(),
             pathSize,
             &widePath[0],
             wideSize) == wideSize;
}
#endif

inline void OpenInputFile(std::ifstream& ifs, const std::string& path) {
#ifdef _WIN32
  std::wstring widePath;
  if (Utf8ToWidePath(path, widePath)) {
    ifs.open(widePath.c_str());
    return;
  }
  ifs.setstate(std::ios::failbit);
  return;
#endif
  ifs.open(path.c_str());
}

} // namespace cppjieba

#endif
