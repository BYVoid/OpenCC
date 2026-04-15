include (GenerateExportHeader)

include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src")

if(NOT USE_SYSTEM_MARISA)
  include_directories(SYSTEM ../deps/marisa-0.3.1/include)
endif()
if(NOT USE_SYSTEM_RAPIDJSON)
  include_directories(SYSTEM ../deps/rapidjson-1.1.0)
endif()
if(NOT USE_SYSTEM_TCLAP)
  include_directories(SYSTEM ../deps/tclap-1.2.5)
endif()

# Library

set(
  LIBOPENCC_HEADERS
  Common.hpp
  Config.hpp
  Conversion.hpp
  ConversionChain.hpp
  Converter.hpp
  Dict.hpp
  DictConverter.hpp
  DictEntry.hpp
  DictGroup.hpp
  Exception.hpp
  Export.hpp
  Lexicon.hpp
  MarisaDict.hpp
  MaxMatchSegmentation.hpp
  Optional.hpp
  PluginSegmentation.hpp
  PhraseExtract.hpp
  Segmentation.hpp
  Segments.hpp
  SerializableDict.hpp
  SerializedValues.hpp
  SimpleConverter.hpp
  TextDict.hpp
  UTF8StringSlice.hpp
  UTF8Util.hpp
  opencc.h
  "${PROJECT_BINARY_DIR}/src/opencc_config.h"
)

set(
  LIBOPENCC_SOURCES
  Config.cpp
  Conversion.cpp
  ConversionChain.cpp
  Converter.cpp
  Dict.cpp
  DictConverter.cpp
  DictEntry.cpp
  DictGroup.cpp
  Lexicon.cpp
  MarisaDict.cpp
  MaxMatchSegmentation.cpp
  PluginSegmentation.cpp
  PhraseExtract.cpp
  SerializedValues.cpp
  SimpleConverter.cpp
  Segmentation.cpp
  TextDict.cpp
  UTF8StringSlice.cpp
  UTF8Util.cpp
)

set(UNITTESTS
  ConfigTest
  ConversionChainTest
  ConversionTest
  DictGroupTest
  LexiconAnnotationTest
  MarisaDictTest
  MaxMatchSegmentationTest
  PhraseExtractTest
  SerializedValuesTest
  SimpleConverterTest
  TextDictTest
  UTF8StringSliceTest
  UTF8UtilTest
)

if (ENABLE_DARTS)
  set(OPENCC_ENABLE_DARTS 1)
  if(NOT USE_SYSTEM_DARTS)
    include_directories(SYSTEM ../deps/darts-clone-0.32)
  endif()
  set(
    LIBOPENCC_HEADERS
    ${LIBOPENCC_HEADERS}
    BinaryDict.hpp
    DartsDict.hpp
  )
  set(
    LIBOPENCC_SOURCES
    ${LIBOPENCC_SOURCES}
    BinaryDict.cpp
    DartsDict.cpp
  )
  set(
    UNITTESTS
    ${UNITTESTS}
    BinaryDictTest
    DartsDictTest
  )
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/src/opencc_config.h.in"
  "${PROJECT_BINARY_DIR}/src/opencc_config.h")

if (WIN32)
  set(RC_FILE_DESCRIPTION "Open Chinese Convert Library")
  set(RC_INTERNAL_NAME "opencc")
  set(RC_ORIGINAL_FILENAME "opencc.dll")
  set(RC_FILE_TYPE "VFT_DLL")
  configure_file(
    "${PROJECT_SOURCE_DIR}/src/version.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version_libopencc.rc"
    @ONLY
  )
  list(APPEND LIBOPENCC_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version_libopencc.rc")
endif()

add_library(libopencc ${LIBOPENCC_SOURCES} ${LIBOPENCC_HEADERS})
add_library(OpenCC::OpenCC ALIAS libopencc)
set_target_properties(libopencc PROPERTIES POSITION_INDEPENDENT_CODE ON)
source_group(libopencc FILES ${LIBOPENCC_SOURCES} ${LIBOPENCC_HEADERS})
target_link_libraries(libopencc marisa)
if (NOT BUILD_SHARED_LIBS)
  target_compile_definitions(libopencc PUBLIC Opencc_BUILT_AS_STATIC)
endif()
# PluginSegmentation.cpp uses LoadLibrary on _WIN32 (MSVC and MinGW).
# Only non-Windows targets may need libdl for dlopen.
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  include(CheckLibraryExists)
  check_library_exists(dl dlopen "" HAVE_LIBDL)
  if (HAVE_LIBDL)
    target_link_libraries(libopencc dl)
  endif ()
endif ()
target_include_directories(libopencc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${DIR_INCLUDE}/opencc>
)

GENERATE_EXPORT_HEADER(
  libopencc
  BASE_NAME OPENCC
  EXPORT_MACRO_NAME OPENCC_EXPORT
  EXPORT_FILE_NAME Opencc_Export.h
  STATIC_DEFINE Opencc_BUILT_AS_STATIC
)

set_target_properties(
  libopencc
  PROPERTIES
    LINKER_LANGUAGE
      CXX
    OUTPUT_NAME
      opencc
    EXPORT_NAME
      OpenCC
    VERSION
      ${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}.${OPENCC_VERSION_REVISION}
    SOVERSION
      ${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}
)

# Installation

if (USE_SYSTEM_MARISA)
  set(INSTALL_TARGETS libopencc)
else()
  set(INSTALL_TARGETS libopencc marisa)
endif()

install(
  TARGETS ${INSTALL_TARGETS} EXPORT ${OPENCC_TARGETS_EXPORT_NAME}
  LIBRARY DESTINATION ${DIR_LIBRARY}
  ARCHIVE DESTINATION ${DIR_LIBRARY}
  RUNTIME DESTINATION bin
)

install(
  EXPORT ${OPENCC_TARGETS_EXPORT_NAME}
  FILE ${OPENCC_TARGETS_EXPORT_NAME}.cmake
  DESTINATION ${DIR_LIBRARY}/cmake/opencc
  NAMESPACE OpenCC::
)

install(
  FILES ${LIBOPENCC_HEADERS}
  DESTINATION ${DIR_INCLUDE}/opencc
)

install(
  FILES plugin/OpenCCPlugin.h
  DESTINATION ${DIR_INCLUDE}/opencc/plugin
)

# Gtest

if (ENABLE_GTEST)
  if (WIN32)
    add_custom_target(
        copy_gtest_to_src
        ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest> ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Copying gtest to src"
    )
    add_custom_target(
        copy_gtest_main_to_src
        ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest_main> ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Copying gtest_main to src"
    )
  endif()

  foreach(TESTCASE ${UNITTESTS})
    add_executable(${TESTCASE} ${TESTCASE}.cpp)
    target_link_libraries(${TESTCASE} gtest gtest_main libopencc)
    add_test(${TESTCASE} ${TESTCASE})
    if (WIN32)
      add_dependencies(${TESTCASE} copy_gtest_to_src copy_gtest_main_to_src)
    endif()
  endforeach()
endif()

# Benchmark

if (ENABLE_BENCHMARK)
  add_subdirectory(benchmark)
endif()

# Subdir

add_subdirectory(tools)
