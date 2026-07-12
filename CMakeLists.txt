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
cmake_minimum_required(VERSION 3.12)
set (PACKAGE_NAME opencc)
project (${PACKAGE_NAME} CXX)
include (CTest)
include(GNUInstallDirs)

######## Options

# PROJECT_IS_TOP_LEVEL is available since CMake 3.21; provide a fallback.
if(CMAKE_VERSION VERSION_LESS 3.21)
  if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(PROJECT_IS_TOP_LEVEL TRUE)
  else()
    set(PROJECT_IS_TOP_LEVEL FALSE)
  endif()
endif()

# When used via FetchContent / add_subdirectory, build a static library and
# skip install rules by default.
if(PROJECT_IS_TOP_LEVEL)
  set(_OPENCC_DEFAULT_BUILD_SHARED_LIBS ON)
  set(_OPENCC_DEFAULT_ENABLE_INSTALL ON)
else()
  set(_OPENCC_DEFAULT_BUILD_SHARED_LIBS OFF)
  set(_OPENCC_DEFAULT_ENABLE_INSTALL OFF)
endif()

option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" OFF)
option(BUILD_SHARED_LIBS "Build opencc as shared library" ${_OPENCC_DEFAULT_BUILD_SHARED_LIBS})
option(OPENCC_ENABLE_INSTALL "Generate install targets" ${_OPENCC_DEFAULT_ENABLE_INSTALL})
option(ENABLE_GTEST "Build all tests." OFF)
option(ENABLE_BENCHMARK "Build benchmark tests." OFF)
# Top-level macOS builds enable the Jieba plugin by default so package
# managers that build with default options (e.g. Homebrew) ship it. Other
# platforms and subproject builds (FetchContent / add_subdirectory) keep it
# opt-in. Trimmed source packages ship without plugins/, so only default to
# ON when the plugin sources are actually present; an explicit
# -DBUILD_OPENCC_JIEBA_PLUGIN=ON without them still fails loudly below.
if (APPLE AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR
    AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugins/CMakeLists.txt")
  set(OPENCC_JIEBA_PLUGIN_DEFAULT ON)
else()
  set(OPENCC_JIEBA_PLUGIN_DEFAULT OFF)
endif()
option(BUILD_OPENCC_JIEBA_PLUGIN "Build Jieba segmentation as a loadable plugin." ${OPENCC_JIEBA_PLUGIN_DEFAULT})
option(BUILD_PYTHON "Build python library" OFF)

# Built-in dictionaries and configs can be produced as compiled ocd2
# (marisa-trie, default) or legacy ocd (Darts) dictionaries, or as plain text
# dictionaries with configs that reference them directly (slower to load,
# but human-readable and easy to interoperate with other toolchains).
set(OPENCC_DICT_FORMAT "ocd2" CACHE STRING "Format of built-in dictionaries and configs to generate/install: ocd2, ocd, or text")
set_property(CACHE OPENCC_DICT_FORMAT PROPERTY STRINGS ocd2 ocd text)
if(NOT OPENCC_DICT_FORMAT STREQUAL "ocd2" AND NOT OPENCC_DICT_FORMAT STREQUAL "ocd" AND NOT OPENCC_DICT_FORMAT STREQUAL "text")
  message(FATAL_ERROR "OPENCC_DICT_FORMAT must be one of 'ocd2', 'ocd', or 'text', got '${OPENCC_DICT_FORMAT}'")
endif()
if(OPENCC_DICT_FORMAT STREQUAL "ocd" AND NOT ENABLE_DARTS)
  message(FATAL_ERROR "OPENCC_DICT_FORMAT=ocd requires ENABLE_DARTS=ON (DartsDict support)")
endif()
option(USE_SYSTEM_DARTS "Use system version of Darts" OFF)
option(USE_SYSTEM_GOOGLE_BENCHMARK "Use system version of Google Benchmark" OFF)
option(USE_SYSTEM_GTEST "Use system version of GoogleTest" OFF)
option(USE_SYSTEM_MARISA "Use system version of Marisa" OFF)
option(USE_SYSTEM_PYBIND11 "Use system version of pybind11" OFF)
option(USE_SYSTEM_RAPIDJSON "Use system version of RapidJSON" OFF)
option(USE_SYSTEM_TCLAP "Use system version of TCLAP" OFF)
set(OPENCC_SOURCE_PACKAGE_PROFILE "full" CACHE STRING "Source package profile: full, opencc, or opencc-jieba")
set_property(CACHE OPENCC_SOURCE_PACKAGE_PROFILE PROPERTY STRINGS full opencc opencc-jieba)

######## Package information
set (PACKAGE_URL https://github.com/BYVoid/OpenCC)
set (PACKAGE_BUGREPORT https://github.com/BYVoid/OpenCC/issues)

# Derive version from Git tags (falls back to hardcoded default when Git is unavailable)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(GitVersion)

# C++ interface/layout changes after 1.3.1 require downstream C++ consumers
# such as librime to relink instead of loading an ABI-incompatible libopencc.
set (OPENCC_ABI_VERSION 1.4)

set(_OPENCC_SOURCE_PACKAGE_SUFFIX "")
set(_OPENCC_SOURCE_PACKAGE_IGNORE_FILES "")
# Trimmed source packages keep both supported build systems (CMake and
# Bazel), so Bazel workspace files (BUILD.bazel, MODULE.bazel[.lock],
# .bazelrc, .bazelversion, .bazelignore) are intentionally NOT ignored here.
# Only .bazelrc.user is excluded (it is a local/CI override that may carry a
# BuildBuddy API key); it is dropped via CPACK_SOURCE_IGNORE_FILES below.
set(_OPENCC_TRIMMED_SOURCE_PACKAGE_IGNORE_FILES
  "/.appveyor.yml"
  "/.agents/"
  "/.claude/"
  "/.npmignore"
  "/AGENTS.md"
  "/CLAUDE.md"
  "/CONTRIBUTING.md"
  "/DESIGN_PRINCIPLES.md"
  "/MANIFEST.in"
  "/Makefile"
  "/OpenCC.egg-info/"
  "/PUBLICATIONS.md"
  "/binding.gyp"
  "/build.cmd"
  "/data/icon/"
  "/deps/google-benchmark/"
  "/deps/pybind11-2.13.1/"
  "/doc/"
  "/examples/"
  "/node/"
  "/package.json"
  "/package-lock.json"
  "/pyproject.toml"
  "/python[^/]*/"
  "/release-pypi-[^/]*"
  "${PROJECT_SOURCE_DIR}/scripts/"
  "/setup.py"
  "/test.cmd"
)
if(OPENCC_SOURCE_PACKAGE_PROFILE STREQUAL "opencc")
  set(_OPENCC_SOURCE_PACKAGE_SUFFIX "-opencc-src")
  set(_OPENCC_SOURCE_PACKAGE_IGNORE_FILES
    ${_OPENCC_TRIMMED_SOURCE_PACKAGE_IGNORE_FILES}
    "/plugins/"
  )
elseif(OPENCC_SOURCE_PACKAGE_PROFILE STREQUAL "opencc-jieba")
  set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "opencc-jieba-${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}.${OPENCC_VERSION_REVISION}-src"
  )
  set(_OPENCC_SOURCE_PACKAGE_IGNORE_FILES
    ${_OPENCC_TRIMMED_SOURCE_PACKAGE_IGNORE_FILES}
    "/plugins/jieba/node/"
  )
elseif(NOT OPENCC_SOURCE_PACKAGE_PROFILE STREQUAL "full")
  message(FATAL_ERROR "OPENCC_SOURCE_PACKAGE_PROFILE must be 'full', 'opencc', or 'opencc-jieba'.")
endif()

if(NOT CPACK_SOURCE_PACKAGE_FILE_NAME)
  set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "${PACKAGE_NAME}-${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}.${OPENCC_VERSION_REVISION}${_OPENCC_SOURCE_PACKAGE_SUFFIX}"
  )
endif()
set(CPACK_SOURCE_IGNORE_FILES
  "/build[^/]*/;/test/dict.ocd;/test/dict.txt;/test/dict.bin;/dict.ocd;/.DS_Store;/.zig-[^/]*/;/dist[^/]*/;/other/;/opencc.xcodeproj/;/.git/;.gitignore;~$;.pyc;/bazel-[^/]*;/.bazelrc.user;/node_modules;/.github;/.pytest_cache;/.vscode;/packaging/;${_OPENCC_SOURCE_PACKAGE_IGNORE_FILES};${CPACK_SOURCE_IGNORE_FILES}"
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
  set (DIR_PLUGIN bin/plugins)
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

set(OPENCC_TARGETS_EXPORT_NAME OpenCCTargets)

configure_file(
  opencc.pc.in
  opencc.pc
  @ONLY
)

if(OPENCC_ENABLE_INSTALL)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/opencc.pc
  DESTINATION
    ${DIR_LIBRARY}/pkgconfig
)
endif()

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

if(OPENCC_ENABLE_INSTALL)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/OpenCCConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/OpenCCConfigVersion.cmake
  DESTINATION
    ${DIR_LIBRARY}/cmake/opencc
)
endif()

######## Compiler flags

add_definitions(
  -DPKGDATADIR="${DIR_SHARE_OPENCC_FULL}"
  -DLOCALEDIR="${DIR_SHARE_LOCALE_FULL}"
  -DOPENCC_VERSION="${OPENCC_VERSION}"
  -DPACKAGE_NAME="${PACKAGE_NAME}"
)

set(CMAKE_CXX_STANDARD 17) # default C++ version for new target afterward
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  add_compile_options(
    -Wall
  )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-O0 -g3)
  endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(
    -Wall
  )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-O0 -g3)
  endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_compile_options(/W4)
  add_definitions(/D "_CRT_SECURE_NO_WARNINGS")
endif()

if (NOT BUILD_SHARED_LIBS)
  add_definitions(
    -DOpencc_BUILT_AS_STATIC
  )
endif (NOT BUILD_SHARED_LIBS)

if (ENABLE_GTEST)
  add_definitions(
    -DPROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
    -DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
  )
endif()

if (ENABLE_BENCHMARK)
  add_definitions(
    -DPROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
    -DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
  )
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
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/doc/CMakeLists.txt")
  add_subdirectory(doc)
elseif(BUILD_DOCUMENTATION)
  message(FATAL_ERROR "BUILD_DOCUMENTATION requires the doc source directory.")
endif()
add_subdirectory(data)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/plugins/CMakeLists.txt")
  add_subdirectory(plugins)
elseif(BUILD_OPENCC_JIEBA_PLUGIN)
  message(FATAL_ERROR "BUILD_OPENCC_JIEBA_PLUGIN requires the plugins source directory.")
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
  add_subdirectory(test)
elseif(ENABLE_GTEST)
  message(FATAL_ERROR "ENABLE_GTEST requires the test source directory.")
endif()

if (ENABLE_BENCHMARK AND TARGET performance AND TARGET opencc_jieba)
  target_compile_definitions(performance PRIVATE
    OPENCC_BENCHMARK_JIEBA_CONFIG_DIR="${PROJECT_SOURCE_DIR}/plugins/jieba/data/config"
    OPENCC_BENCHMARK_JIEBA_PLUGIN_DIR="$<TARGET_FILE_DIR:opencc_jieba>"
  )
  add_dependencies(performance opencc_jieba)
  if (WIN32)
    add_custom_target(
      copy_opencc_jieba_to_benchmark
      ${CMAKE_COMMAND} -E copy $<TARGET_FILE:opencc_jieba> ${PROJECT_BINARY_DIR}/src/benchmark
      COMMENT "Copying opencc_jieba to src/benchmark"
    )
    add_dependencies(performance copy_opencc_jieba_to_benchmark)
  endif()
endif()

######## Testing

if (ENABLE_GTEST)
  if(NOT USE_SYSTEM_GTEST AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/googletest-1.15.0/CMakeLists.txt")
    message(FATAL_ERROR "ENABLE_GTEST with bundled GoogleTest requires deps/googletest-1.15.0.")
  endif()
  if(NOT USE_SYSTEM_GTEST)
    add_subdirectory(deps/googletest-1.15.0)
  endif()
  enable_testing()
endif()

if (ENABLE_BENCHMARK)
  if(NOT USE_SYSTEM_GOOGLE_BENCHMARK AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/google-benchmark/CMakeLists.txt")
    message(FATAL_ERROR "ENABLE_BENCHMARK with bundled Google Benchmark requires deps/google-benchmark.")
  endif()
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
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/pybind11-2.13.1/CMakeLists.txt")
      message(FATAL_ERROR "BUILD_PYTHON with bundled pybind11 requires deps/pybind11-2.13.1.")
    endif()
    add_subdirectory(deps/pybind11-2.13.1)
  endif()
  pybind11_add_module(opencc_clib src/py_opencc.cpp)
  target_link_libraries(opencc_clib PRIVATE libopencc)
endif()
