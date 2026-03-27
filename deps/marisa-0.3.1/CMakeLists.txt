cmake_minimum_required(VERSION 3.22)

file(STRINGS "VERSION" VERSION_STR)
project(Marisa VERSION "${VERSION_STR}" LANGUAGES C CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/finders")

# Options
option(ENABLE_NATIVE_CODE "Enable -march=native and supported instructions" OFF)
option(ENABLE_TOOLS "Enables command-line tools" ON)
option(ENABLE_ASAN "Enable address sanitizer" OFF)
option(ENABLE_UBSAN "Enable undefined behavior sanitizer" OFF)
option(ENABLE_GPERFTOOLS_PROFILER "Find and link gperftools profiler" OFF)
option(ENABLE_COVERAGE "Enable code coverage instrumentation (only enabled with BUILD_TESTING)" OFF)
option(ENABLE_STATIC_STDLIB "Link C++ stdlib statically" OFF)

function(check_macro_defined MACRO OUTPUT_VAR)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
  try_compile(result
    SOURCE_FROM_CONTENT
      "check_${OUTPUT_VAR}.cc"
      "#ifndef ${MACRO}\n#error \"${MACRO} is missing\"\n#endif\n"
    COMPILE_DEFINITIONS -march=native
  )
  set("${OUTPUT_VAR}" "${result}" PARENT_SCOPE)
  message("${OUTPUT_VAR}: ${result}")
endfunction()
if(ENABLE_NATIVE_CODE)
  check_macro_defined(__SSE2__ HAVE_SSE2)
  check_macro_defined(__SSE3__ HAVE_SSE3)
  check_macro_defined(__SSSE3__ HAVE_SSSE3)
  check_macro_defined(__SSE4_1__ HAVE_SSE4_1)
  check_macro_defined(__SSE4_2__ HAVE_SSE4_2)
  check_macro_defined(__POPCNT__ HAVE_POPCNT)
  check_macro_defined(__BMI__ HAVE_BMI)
  check_macro_defined(__BMI2__ HAVE_BMI2)
endif()

include(CMakeDependentOption)
cmake_dependent_option(ENABLE_SSE2 "Use SSE2 instructions" ON "ENABLE_NATIVE_CODE;HAVE_SSE2" OFF)
cmake_dependent_option(ENABLE_SSE3 "Use SSE3 instructions" ON "ENABLE_NATIVE_CODE;HAVE_SSE3" OFF)
cmake_dependent_option(ENABLE_SSSE3 "Use SSSE3 instructions" ON "ENABLE_NATIVE_CODE;HAVE_SSSE3" OFF)
cmake_dependent_option(ENABLE_SSE4_1 "Use SSE4_1 instructions" ON "ENABLE_NATIVE_CODE;HAVE_SSE4_1" OFF)
cmake_dependent_option(ENABLE_SSE4_2 "Use SSE4_2 instructions" ON "ENABLE_NATIVE_CODE;HAVE_SSE4_2" OFF)
cmake_dependent_option(ENABLE_POPCNT "Use POPCNT instructions" ON "ENABLE_NATIVE_CODE;HAVE_POPCNT" OFF)
cmake_dependent_option(ENABLE_BMI "Use BMI instructions" ON "ENABLE_NATIVE_CODE;HAVE_BMI" OFF)
cmake_dependent_option(ENABLE_BMI2 "Use BMI2 instructions" ON "ENABLE_NATIVE_CODE;HAVE_BMI2" OFF)
function(add_native_code TARGET)
  if(ENABLE_NATIVE_CODE)
    target_compile_options("${TARGET}" PRIVATE -march=native)
    if(ENABLE_BMI2)
      target_compile_options("${TARGET}" PRIVATE -DMARISA_USE_BMI2 -mbmi2 -msse4)
    elseif(ENABLE_BMI)
      target_compile_options("${TARGET}" PRIVATE -DMARISA_USE_BMI -mbmi -msse4)
    elseif(ENABLE_SSE4A)
      target_compile_options("${TARGET}" PRIVATE -DMARISA_USE_SSE4A -msse4a)
    elseif(ENABLE_SSE4_2 AND ENABLE_POPCNT)
      target_compile_options("${TARGET}" PRIVATE -DMARISA_USE_SSE4 -msse4)
    elseif(ENABLE_SSE4_2)
      target_compile_options("${TARGET}" PRIVATE -DMARISA_USE_SSE4_2 -msse4.2)
    elseif(ENABLE_SSE4_1)
      target_compile_options("${TARGET}" PRIVATE -DMARISA_USE_SSE4_1 -msse4.1)
    elseif(ENABLE_SSSE3)
      target_compile_options("${TARGET}" PRIVATE -DMARISA_USE_SSSE3 -mssse3)
    elseif(ENABLE_SSE3)
      target_compile_options("${TARGET}" PRIVATE -DMARISA_USE_SSE3 -msse3)
    elseif(ENABLE_SSE2)
      target_compile_options("${TARGET}" PRIVATE -DMARISA_USE_SSE2 -msse2)
    endif()
  endif()
endfunction()

include(CTest)

# We allow C++20 features if the compiler supports them
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clang-tidy

# Target Windows 8 for PrefetchVirtualMemory
if (MINGW)
    add_compile_definitions(_WIN32_WINNT=_WIN32_WINNT_WIN8)
endif()

# MSVC Configuration
# https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=msvc-170
# https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-170
add_compile_options("$<$<COMPILE_LANG_AND_ID:C,MSVC>:/utf-8>")
add_compile_options("$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8;/Zc:__cplusplus>")

# Sanitizers support
function(add_sanitizers TARGET)
  if(NOT MSVC)
    if(ENABLE_ASAN)
      target_compile_options(${TARGET} PUBLIC -fsanitize=address)
      target_link_libraries(${TARGET} PUBLIC -fsanitize=address)
    endif()
    if(ENABLE_UBSAN)
      target_compile_options(${TARGET} PUBLIC -fsanitize=undefined)
      target_link_libraries(${TARGET} PUBLIC -fsanitize=undefined)
    endif()
  endif()
endfunction()

# gperftools support (https://github.com/gperftools/gperftools)
function(add_gperftools TARGET)
  if(ENABLE_GPERFTOOLS_PROFILER)
    if(NOT Gperftools_FOUND)
      find_package(Gperftools REQUIRED)
      set(ENABLE_STATIC_STDLIB OFF)
    endif()

    # Compile with information about file and line numbers for everything
    # even in non-Debug build types.
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      target_compile_options(${TARGET} PRIVATE -g2)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      # Use the more size-efficient `-gmlt` option on clang.
      target_compile_options(${TARGET} PRIVATE -gmlt)
    endif()
    target_link_libraries(${TARGET} PUBLIC ${GPERFTOOLS_LIBRARIES})
  endif()
endfunction()

# Applies configuration to the target based on options.
function(configure_target_from_options TARGET)
  add_sanitizers(${TARGET})
  add_gperftools(${TARGET})
  target_compile_definitions(${TARGET} PRIVATE $<$<CONFIG:DEBUG>:_DEBUG>)
  if(MARISA_ENABLE_STATIC_STDLIB)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      target_link_libraries(marisa PUBLIC -static-libgcc -static-libstdc++)
    endif()
  endif()
endfunction()

# Marisa Library
set(MARISA_HEADERS
  include/marisa.h
  include/marisa/agent.h
  include/marisa/base.h
  include/marisa/iostream.h
  include/marisa/key.h
  include/marisa/keyset.h
  include/marisa/query.h
  include/marisa/stdio.h
  include/marisa/trie.h
)
add_library(marisa
  ${MARISA_HEADERS}
  lib/marisa/agent.cc
  lib/marisa/grimoire/algorithm/sort.h
  lib/marisa/grimoire/intrin.h
  lib/marisa/grimoire/io.h
  lib/marisa/grimoire/io/mapper.cc
  lib/marisa/grimoire/io/mapper.h
  lib/marisa/grimoire/io/reader.cc
  lib/marisa/grimoire/io/reader.h
  lib/marisa/grimoire/io/writer.cc
  lib/marisa/grimoire/io/writer.h
  lib/marisa/grimoire/trie.h
  lib/marisa/grimoire/trie/cache.h
  lib/marisa/grimoire/trie/config.h
  lib/marisa/grimoire/trie/entry.h
  lib/marisa/grimoire/trie/header.h
  lib/marisa/grimoire/trie/history.h
  lib/marisa/grimoire/trie/key.h
  lib/marisa/grimoire/trie/louds-trie.cc
  lib/marisa/grimoire/trie/louds-trie.h
  lib/marisa/grimoire/trie/range.h
  lib/marisa/grimoire/trie/state.h
  lib/marisa/grimoire/trie/tail.cc
  lib/marisa/grimoire/trie/tail.h
  lib/marisa/grimoire/vector.h
  lib/marisa/grimoire/vector/bit-vector.cc
  lib/marisa/grimoire/vector/bit-vector.h
  lib/marisa/grimoire/vector/flat-vector.h
  lib/marisa/grimoire/vector/pop-count.h
  lib/marisa/grimoire/vector/rank-index.h
  lib/marisa/grimoire/vector/vector.h
  lib/marisa/keyset.cc
  lib/marisa/trie.cc
)
target_include_directories(marisa
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    lib
)
set_target_properties(marisa PROPERTIES
  VERSION "${Marisa_VERSION}"
  SOVERSION "${Marisa_VERSION_MAJOR}"
)
configure_target_from_options(marisa)
add_native_code(marisa)
add_library(Marisa::marisa ALIAS marisa)

# Tools
set(MARISA_TOOLS
  marisa-build
  marisa-lookup
  marisa-reverse-lookup
  marisa-common-prefix-search
  marisa-predictive-search
  marisa-dump
  marisa-benchmark
)
if(ENABLE_TOOLS)
  add_library(cmdopt STATIC tools/cmdopt.h tools/cmdopt.cc)
  target_include_directories(cmdopt PUBLIC tools)

  foreach(_tool ${MARISA_TOOLS})
    add_executable(${_tool} "tools/${_tool}.cc")
    target_link_libraries(${_tool} PRIVATE marisa cmdopt)
    configure_target_from_options(${_tool})
  endforeach()
endif()

# Testing
if(BUILD_TESTING)
  foreach(_test
    base-test
    io-test
    vector-test
    trie-test
    marisa-test
  )
    add_executable(${_test} "tests/${_test}.cc")
    target_link_libraries(${_test} PRIVATE marisa)
    target_include_directories(${_test} PRIVATE tests lib)
    configure_target_from_options(${_test})
    add_native_code(${_test})
    add_test(
      NAME ${_test}
      COMMAND ${_test}
    )
  endforeach()

  if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      message(WARNING "Code coverage is not supported with MSVC")
    else()
      target_compile_options(marisa PUBLIC --coverage)
      target_link_options(marisa PUBLIC --coverage)
    endif()
  endif()
endif()

# Install configuration

if(NOT DEFINED LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR lib)
endif()

# We do not pass PUBLIC_HEADER because it doesn't respect subdirectories.
# Instead, we install the headers manually via a second call to `install`.
install(
  TARGETS marisa
  EXPORT MarisaTargets
  CONFIGURATIONS Release
  DESTINATION ${LIB_INSTALL_DIR}
  COMPONENT Library
)
install(
  DIRECTORY include/
  DESTINATION include
  COMPONENT Library
  FILES_MATCHING PATTERN "*.h"
)

if(ENABLE_TOOLS)
  install(
    TARGETS ${MARISA_TOOLS}
    CONFIGURATIONS Release
    COMPONENT Binaries
  )
endif()

set(ConfigPackageLocation "${LIB_INSTALL_DIR}/cmake/Marisa")

include(CMakePackageConfigHelpers)
configure_package_config_file(Marisa.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/Marisa/MarisaConfig.cmake"
  INSTALL_DESTINATION ${ConfigPackageLocation}
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/Marisa/MarisaConfigVersion.cmake"
  VERSION ${Marisa_VERSION}
  COMPATIBILITY SameMajorVersion
)

export(EXPORT MarisaTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/Marisa/MarisaTargets.cmake"
  NAMESPACE Marisa::
)

install(EXPORT MarisaTargets
  FILE MarisaTargets.cmake
  NAMESPACE Marisa::
  DESTINATION ${ConfigPackageLocation}
  COMPONENT Library
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/Marisa/MarisaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/Marisa/MarisaConfigVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
  COMPONENT Library
)

configure_file(marisa.pc.in ${CMAKE_CURRENT_BINARY_DIR}/marisa.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/marisa.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
