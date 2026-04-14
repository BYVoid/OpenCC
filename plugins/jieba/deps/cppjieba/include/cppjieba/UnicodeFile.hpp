#ifndef CPPJIEBA_UNICODE_FILE_HPP
#define CPPJIEBA_UNICODE_FILE_HPP

#include <fstream>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace cppjieba {

#if defined(_WIN32) || defined(_WIN64)
inline std::wstring WidePathFromUtf8(const std::string& path) {
  if (path.empty()) {
    return L"";
  }
  const int size =
      MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
  if (size <= 1) {
    return L"";
  }
  std::wstring wide(static_cast<size_t>(size), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wide[0], size);
  wide.resize(static_cast<size_t>(size - 1));
  return wide;
}
#endif

inline void OpenInputFile(const std::string& path, std::ifstream& ifs,
                          std::ios_base::openmode mode = std::ios::in) {
#if defined(_WIN32) || defined(_WIN64)
  ifs.open(WidePathFromUtf8(path), mode);
#else
  ifs.open(path.c_str(), mode);
#endif
}

} // namespace cppjieba

#endif // CPPJIEBA_UNICODE_FILE_HPP
