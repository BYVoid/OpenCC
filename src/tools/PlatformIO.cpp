/*
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "src/tools/PlatformIO.hpp"

#include <cstdlib>
#include <cwchar>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#include "src/WinUtil.hpp"
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace opencc {
namespace tools {

namespace {

#ifdef _WIN32
std::wstring ParentDirectory(const std::wstring& fileName) {
  const size_t slash = fileName.find_last_of(L"\\/");
  if (slash == std::wstring::npos) {
    return L".\\";
  }
  return fileName.substr(0, slash + 1);
}
#else
std::string ParentDirectory(const std::string& fileName) {
  const size_t slash = fileName.find_last_of('/');
  if (slash == std::string::npos) {
    return ".";
  }
  if (slash == 0) {
    return "/";
  }
  return fileName.substr(0, slash);
}
#endif

} // namespace

FILE* OpenFileUtf8(const std::string& fileName, const char* mode) {
#ifdef _WIN32
  return _wfopen(internal::WideFromUtf8(fileName).c_str(),
                 internal::WideFromUtf8(mode).c_str());
#else
  return fopen(fileName.c_str(), mode);
#endif
}

FILE* CreateTempFileNearUtf8(const std::string& targetFileName,
                             std::string* fileName) {
#ifdef _WIN32
  if (fileName == nullptr) {
    return nullptr;
  }

  const std::wstring target = internal::WideFromUtf8(targetFileName);
  const std::wstring parent = ParentDirectory(target);
  const DWORD processId = GetCurrentProcessId();
  const ULONGLONG tick = GetTickCount64();

  for (int i = 0; i < 100; i++) {
    const std::wstring tempFileName =
        parent + L".opencc-" + std::to_wstring(processId) + L"-" +
        std::to_wstring(tick) + L"-" + std::to_wstring(i) + L".tmp";
    const int fd = _wopen(tempFileName.c_str(),
                          _O_CREAT | _O_EXCL | _O_BINARY | _O_WRONLY,
                          _S_IREAD | _S_IWRITE);
    if (fd == -1) {
      continue;
    }
    FILE* file = _wfdopen(fd, L"wb");
    if (file == nullptr) {
      _close(fd);
      DeleteFileW(tempFileName.c_str());
      return nullptr;
    }
    *fileName = internal::Utf8FromWide(tempFileName);
    return file;
  }
  return nullptr;
#else
  if (fileName == nullptr) {
    return nullptr;
  }

  std::string tempFileName = ParentDirectory(targetFileName);
  tempFileName += "/";
  tempFileName += "openccXXXXXX";

  std::vector<char> pathBuffer(tempFileName.begin(), tempFileName.end());
  pathBuffer.push_back('\0');
  const int fd = mkstemp(pathBuffer.data());
  if (fd == -1) {
    return nullptr;
  }
  FILE* file = fdopen(fd, "wb");
  if (file == nullptr) {
    close(fd);
    std::remove(pathBuffer.data());
    return nullptr;
  }
  *fileName = pathBuffer.data();
  return file;
#endif
}

bool GetRealPathUtf8(const std::string& fileName, std::string* realPath) {
#ifdef _WIN32
  if (realPath == nullptr) {
    return false;
  }

  const std::wstring wideFileName = internal::WideFromUtf8(fileName);
  HANDLE handle =
      CreateFileW(wideFileName.c_str(), 0,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    return false;
  }

  const DWORD flags = FILE_NAME_NORMALIZED | VOLUME_NAME_DOS;
  const DWORD required = GetFinalPathNameByHandleW(handle, nullptr, 0, flags);
  if (required == 0) {
    CloseHandle(handle);
    return false;
  }
  std::wstring resolved(static_cast<size_t>(required) + 1, L'\0');
  const DWORD copied =
      GetFinalPathNameByHandleW(handle, &resolved[0],
                                static_cast<DWORD>(resolved.size()), flags);
  CloseHandle(handle);
  if (copied == 0 || copied >= resolved.size()) {
    return false;
  }
  resolved.resize(copied);
  *realPath = internal::Utf8FromWide(resolved);
  return true;
#else
  if (realPath == nullptr) {
    return false;
  }
  char* resolved = realpath(fileName.c_str(), nullptr);
  if (resolved == nullptr) {
    return false;
  }
  *realPath = resolved;
  std::free(resolved);
  return true;
#endif
}

bool HasMultipleLinksUtf8(const std::string& fileName) {
#ifdef _WIN32
  const std::wstring wideFileName = internal::WideFromUtf8(fileName);
  HANDLE handle =
      CreateFileW(wideFileName.c_str(), 0,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    return false;
  }
  BY_HANDLE_FILE_INFORMATION fileInfo;
  const bool success = GetFileInformationByHandle(handle, &fileInfo) != 0;
  CloseHandle(handle);
  return success && fileInfo.nNumberOfLinks > 1;
#else
  struct stat fileStat;
  if (stat(fileName.c_str(), &fileStat) != 0) {
    return false;
  }
  return fileStat.st_nlink > 1;
#endif
}

bool IsSameFileUtf8(const std::string& first, const std::string& second) {
#ifdef _WIN32
  const std::wstring wideFirst = internal::WideFromUtf8(first);
  const std::wstring wideSecond = internal::WideFromUtf8(second);

  HANDLE firstHandle =
      CreateFileW(wideFirst.c_str(), 0,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (firstHandle == INVALID_HANDLE_VALUE) {
    return false;
  }
  HANDLE secondHandle =
      CreateFileW(wideSecond.c_str(), 0,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (secondHandle == INVALID_HANDLE_VALUE) {
    CloseHandle(firstHandle);
    return false;
  }

  BY_HANDLE_FILE_INFORMATION firstInfo;
  BY_HANDLE_FILE_INFORMATION secondInfo;
  const bool success =
      GetFileInformationByHandle(firstHandle, &firstInfo) != 0 &&
      GetFileInformationByHandle(secondHandle, &secondInfo) != 0;
  CloseHandle(secondHandle);
  CloseHandle(firstHandle);

  if (!success) {
    return false;
  }
  return firstInfo.dwVolumeSerialNumber == secondInfo.dwVolumeSerialNumber &&
         firstInfo.nFileIndexHigh == secondInfo.nFileIndexHigh &&
         firstInfo.nFileIndexLow == secondInfo.nFileIndexLow;
#else
  struct stat firstStat;
  struct stat secondStat;
  if (stat(first.c_str(), &firstStat) != 0 ||
      stat(second.c_str(), &secondStat) != 0) {
    return false;
  }
  return firstStat.st_dev == secondStat.st_dev &&
         firstStat.st_ino == secondStat.st_ino;
#endif
}

int PreserveMetadataUtf8(const std::string& source,
                         const std::string& target) {
#ifdef _WIN32
  (void)source;
  (void)target;
  return 0;
#else
  struct stat sourceStat;
  if (stat(source.c_str(), &sourceStat) != 0) {
    return -1;
  }
  if (chmod(target.c_str(), sourceStat.st_mode & 07777) != 0) {
    return -1;
  }
  return 0;
#endif
}

int ReplaceFileUtf8(const std::string& source, const std::string& target) {
#ifdef _WIN32
  return ReplaceFileW(internal::WideFromUtf8(target).c_str(),
                      internal::WideFromUtf8(source).c_str(), nullptr, 0,
                      nullptr, nullptr)
             ? 0
             : -1;
#else
  return rename(source.c_str(), target.c_str());
#endif
}

int RemoveFileUtf8(const std::string& fileName) {
#ifdef _WIN32
  return _wremove(internal::WideFromUtf8(fileName).c_str());
#else
  return std::remove(fileName.c_str());
#endif
}

#ifdef _WIN32
std::vector<std::string> GetUtf8CommandLineArgs() {
  int argc = 0;
  wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  std::vector<std::string> args;
  if (argv == nullptr) {
    return args;
  }
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; i++) {
    args.push_back(internal::Utf8FromWide(argv[i]));
  }
  LocalFree(argv);
  return args;
}
#endif

} // namespace tools
} // namespace opencc
