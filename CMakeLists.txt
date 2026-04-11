#
# Open Chinese Convert
#
# Copyright 2010-2020 Carbo Kuo <byvoid@byvoid.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

######## Project settings
cmake_minimum_required(VERSION 3.5)
set (PACKAGE_NAME opencc)
project (${PACKAGE_NAME} CXX)
include (CTest)
include(GNUInstallDirs)

######## Options
option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" OFF)
option(BUILD_SHARED_LIBS "Build opencc as shared library" ON)
option(ENABLE_GTEST "Build all tests." OFF)
option(ENABLE_BENCHMARK "Build benchmark tests." OFF)
option(ENABLE_DARTS "Build DartsDict (ocd format)." ON)
option(ENABLE_JIEBA "Deprecated compatibility option. Use BUILD_OPENCC_JIEBA_PLUGIN." OFF)
option(BUILD_OPENCC_JIEBA_PLUGIN "Build Jieba segmentation as a loadable plugin." OFF)
option(BUILD_PYTHON "Build python library" OFF)
option(USE_SYSTEM_DARTS "Use system version of Darts" OFF)
option(USE_SYSTEM_GOOGLE_BENCHMARK "Use system version of Google Benchmark" OFF)
option(USE_SYSTEM_GTEST "Use system version of GoogleTest" OFF)
option(USE_SYSTEM_MARISA "Use system version of Marisa" OFF)
option(USE_SYSTEM_PYBIND11 "Use system version of pybind11" OFF)
option(USE_SYSTEM_RAPIDJSON "Use system version of RapidJSON" OFF)
option(USE_SYSTEM_TCLAP "Use system version of TCLAP" OFF)

######## Package information
set (PACKAGE_URL https://github.com/BYVoid/OpenCC)
set (PACKAGE_BUGREPORT https://github.com/BYVoid/OpenCC/issues)

# Derive version from Git tags (falls back to hardcoded default when Git is unavailable)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GitVersion)

set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${PACKAGE_NAME}-${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}.${OPENCC_VERSION_REVISION}"
)
set(CPACK_SOURCE_IGNORE_FILES
  "/build/;/test/dict.ocd;/test/dict.txt;/test/dict.bin;/other/;/opencc.xcodeproj/;/.git/;.gitignore;~$;.pyc;/bazel*;/node_modules;/.github;/.pytest_cache;/.vscode;${CPACK_SOURCE_IGNORE_FILES}"
)
include(CPack)

######## Windows

#if (WIN32)
#  set(CMAKE_SHARED_LIBRARY_PREFIX ${CMAKE_INSTALL_PREFIX})
#  set(CMAKE_STATIC_LIBRARY_PREFIX ${CMAKE_INSTALL_PREFIX})
#endif (WIN32)

######## macOS

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

######## Directory
# Installation directories follow GNU Coding Standards via CMake's GNUInstallDirs module:
# https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
#
# Default values and parity with previously hardcoded paths
# (shown for CMAKE_INSTALL_PREFIX=/usr/local and /usr):
#
#   CMAKE_INSTALL_INCLUDEDIR     "include"  -> /usr/local/include  or  /usr/include
#     (replaces: ${CMAKE_INSTALL_PREFIX}/include)
#
#   CMAKE_INSTALL_SYSCONFDIR     "etc"      -> /usr/local/etc      or  /etc
#     (replaces: ${CMAKE_INSTALL_PREFIX}/etc)
#     NOTE: when prefix is /usr the full path is /etc per GNU Coding Standards,
#           whereas the old code would have produced /usr/etc.
#
#   CMAKE_INSTALL_LIBDIR         "lib"      -> /usr/local/lib      or  /usr/lib
#     (also resolves to lib64 or lib/<multiarch> on applicable platforms)
#     (replaces: ${CMAKE_INSTALL_PREFIX}/lib[64])
#
#   CMAKE_INSTALL_DATAROOTDIR    "share"    -> /usr/local/share    or  /usr/share
#     (replaces: ${CMAKE_INSTALL_PREFIX}/share)

# All DIR_* variables use relative paths so that install() destinations and
# CMake package config files remain relocatable.
set (DIR_INCLUDE ${CMAKE_INSTALL_INCLUDEDIR})
set (DIR_ETC ${CMAKE_INSTALL_SYSCONFDIR})
set (DIR_LIBRARY ${CMAKE_INSTALL_LIBDIR})
set (DIR_SHARE ${CMAKE_INSTALL_DATAROOTDIR})

if (DEFINED SHARE_INSTALL_PREFIX)
  set (DIR_SHARE ${SHARE_INSTALL_PREFIX})
endif (DEFINED SHARE_INSTALL_PREFIX)

if (DEFINED INCLUDE_INSTALL_DIR)
  set (DIR_INCLUDE ${INCLUDE_INSTALL_DIR})
endif (DEFINED INCLUDE_INSTALL_DIR)

if (DEFINED SYSCONF_INSTALL_DIR)
  set (DIR_ETC ${SYSCONF_INSTALL_DIR})
endif (DEFINED SYSCONF_INSTALL_DIR)

if (DEFINED LIB_INSTALL_DIR)
  set (DIR_LIBRARY ${LIB_INSTALL_DIR})
endif (DEFINED LIB_INSTALL_DIR)

set (DIR_SHARE_OPENCC ${DIR_SHARE}/opencc)
if (WIN32)
  set (DIR_PLUGIN bin/opencc/plugins)
else ()
  set (DIR_PLUGIN ${DIR_LIBRARY}/opencc/plugins)
endif ()

# Compute absolute paths needed for compile-time macros (PKGDATADIR/LOCALEDIR)
# and for the pkg-config file.  We must handle both the GNUInstallDirs relative
# defaults and user-supplied absolute override paths (SHARE_INSTALL_PREFIX etc.).
foreach (_gnu_dir SHARE INCLUDE LIBRARY)
  if (IS_ABSOLUTE "${DIR_${_gnu_dir}}")
    set (DIR_${_gnu_dir}_FULL "${DIR_${_gnu_dir}}")
  else ()
    set (DIR_${_gnu_dir}_FULL "${CMAKE_INSTALL_PREFIX}/${DIR_${_gnu_dir}}")
  endif ()
endforeach ()

set (DIR_SHARE_OPENCC_FULL ${DIR_SHARE_FULL}/opencc)
set (DIR_SHARE_LOCALE_FULL ${DIR_SHARE_FULL}/locale)
if (IS_ABSOLUTE "${DIR_PLUGIN}")
  set (DIR_PLUGIN_FULL "${DIR_PLUGIN}")
else ()
  set (DIR_PLUGIN_FULL "${CMAKE_INSTALL_PREFIX}/${DIR_PLUGIN}")
endif ()

######## Configuration

include(CMakePackageConfigHelpers)

set(targets_export_name OpenCCTargets)

configure_file(
  opencc.pc.in
  opencc.pc
  @ONLY
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/opencc.pc
  DESTINATION
    ${DIR_LIBRARY}/pkgconfig
)

write_basic_package_version_file(
  OpenCCConfigVersion.cmake
  VERSION ${OPENCC_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  OpenCCConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/OpenCCConfig.cmake
  INSTALL_DESTINATION ${DIR_LIBRARY}/cmake/opencc
  PATH_VARS DIR_INCLUDE DIR_PLUGIN DIR_SHARE_OPENCC
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/OpenCCConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/OpenCCConfigVersion.cmake
  DESTINATION
    ${DIR_LIBRARY}/cmake/opencc
)

######## Compiler flags

add_definitions(
  -DPKGDATADIR="${DIR_SHARE_OPENCC_FULL}"
  -DLOCALEDIR="${DIR_SHARE_LOCALE_FULL}"
  -DVERSION="${OPENCC_VERSION}"
  -DPACKAGE_NAME="${PACKAGE_NAME}"
)

set(CMAKE_CXX_STANDARD 17) # default C++ version for new target afterward
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  add_definitions(
    -Wall
  )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-O0 -g3)
  endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(
    -Wall
  )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-O0 -g3)
  endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(
    /W4
    /D "_CRT_SECURE_NO_WARNINGS"
  )
endif()

if (NOT BUILD_SHARED_LIBS)
  add_definitions(
    -DOpencc_BUILT_AS_STATIC
  )
endif (NOT BUILD_SHARED_LIBS)

if (ENABLE_GTEST)
  add_definitions(
    -DPROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
    -DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
  )
endif()

if (ENABLE_BENCHMARK)
  add_definitions(
    -DPROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
    -DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
  )
endif()

if (ENABLE_DARTS)
  add_definitions(
    -DENABLE_DARTS
  )
endif()

if (ENABLE_JIEBA)
  message(WARNING "ENABLE_JIEBA is deprecated; enabling BUILD_OPENCC_JIEBA_PLUGIN for compatibility.")
  set(BUILD_OPENCC_JIEBA_PLUGIN ON)
endif()

add_definitions(
  -DOPENCC_SEGMENTATION_PLUGIN_DIR="${DIR_PLUGIN_FULL}"
)


######## Dependencies

if(NOT USE_SYSTEM_MARISA)
  message(STATUS "Use bundled marisa library.")
  add_subdirectory(deps/marisa-0.3.1)
else()
  find_library(LIBMARISA NAMES marisa)
  if (LIBMARISA)
    message(STATUS "libmarisa found: ${LIBMARISA}")
  else()
    message(FATAL_ERROR "libmarisa not found.")
  endif()
endif()

######## Subdirectories

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(data)
add_subdirectory(plugins)
add_subdirectory(test)

######## Testing

if (ENABLE_GTEST)
  if(NOT USE_SYSTEM_GTEST)
    add_subdirectory(deps/googletest-1.15.0)
  endif()
  enable_testing()
endif()

if (ENABLE_BENCHMARK)
  set(BENCHMARK_ENABLE_TESTING OFF)
  if(NOT USE_SYSTEM_GOOGLE_BENCHMARK)
    add_subdirectory(deps/google-benchmark)
  endif()
  enable_testing()
endif()

######## Python

if (BUILD_PYTHON)
  if(USE_SYSTEM_PYBIND11)
    find_package(pybind11 CONFIG)
  else()
    add_subdirectory(deps/pybind11-2.13.1)
  endif()
  pybind11_add_module(opencc_clib src/py_opencc.cpp)
  target_link_libraries(opencc_clib PRIVATE libopencc)
endif()
