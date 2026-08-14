cmake_minimum_required(VERSION 3.18)
project (opencc-integration CXX)
include (CTest)
enable_testing()
set(CTEST_OUTPUT_ON_FAILURE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The test includes private headers (src/Common.hpp) from the top-level
# source tree, which autopkgtest provides as the working tree.
include_directories(${CMAKE_SOURCE_DIR}/../..)

find_package(PkgConfig REQUIRED)
find_program (OPENCC_TOOL opencc REQUIRED)
pkg_check_modules(OPENCC REQUIRED IMPORTED_TARGET opencc)

add_definitions(
  -DCMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}"
  -DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
  -DPROJECT_BINARY_PATH="${OPENCC_TOOL}"
  -DPROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
)
make_directory(${CMAKE_BINARY_DIR}/test)
add_subdirectory(/usr/src/googletest/googletest ${CMAKE_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)
set(UNITTESTS
  CommandLineConvertTest
)
foreach(UNITTEST ${UNITTESTS})
  add_executable(${UNITTEST} ${UNITTEST}.cpp)
  target_link_libraries(${UNITTEST} gtest gtest_main PkgConfig::OPENCC)
  add_test(${UNITTEST} ${UNITTEST})
endforeach(UNITTEST)
