include (GenerateExportHeader)

include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src")

if(NOT USE_SYSTEM_MARISA)
  include_directories(../deps/marisa-0.2.6/include)
endif()
if(NOT USE_SYSTEM_RAPIDJSON)
  include_directories(../deps/rapidjson-1.1.0)
endif()
if(NOT USE_SYSTEM_TCLAP)
  include_directories(../deps/tclap-1.2.2)
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
    include_directories(../deps/darts-clone)
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

add_library(libopencc ${LIBOPENCC_SOURCES} ${LIBOPENCC_HEADERS})
set_target_properties(libopencc PROPERTIES POSITION_INDEPENDENT_CODE ON)
source_group(libopencc FILES ${LIBOPENCC_SOURCES} ${LIBOPENCC_HEADERS})
target_link_libraries(libopencc marisa)

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
    VERSION
      ${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}.${OPENCC_VERSION_REVISION}
    SOVERSION
      ${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}
)

# Installation

install(
  TARGETS libopencc
  LIBRARY DESTINATION ${DIR_LIBRARY}
  ARCHIVE DESTINATION ${DIR_LIBRARY}
  RUNTIME DESTINATION bin
)

install(
  FILES ${LIBOPENCC_HEADERS}
  DESTINATION ${DIR_INCLUDE}/opencc
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
