include (GenerateExportHeader)

include_directories(../deps/darts-clone)
include_directories(../deps/rapidjson-0.11)
include_directories(../deps/tclap-1.2.1)

# Library

set(
  LIBOPENCC_HEADERS
  BinaryDict.hpp
  Common.hpp
  Config.hpp
  Conversion.hpp
  ConversionChain.hpp
  Converter.hpp
  DartsDict.hpp
  Dict.hpp
  DictConverter.hpp
  DictEntry.hpp
  DictGroup.hpp
  Exception.hpp
  Export.hpp
  Lexicon.hpp
  MaxMatchSegmentation.hpp
  Optional.hpp
  PhraseExtract.hpp
  Segmentation.hpp
  Segments.hpp
  SerializableDict.hpp
  SimpleConverter.hpp
  TextDict.hpp
  UTF8StringSlice.hpp
  UTF8Util.hpp
  opencc.h
)

set(
  LIBOPENCC_SOURCES
  BinaryDict.cpp
  Config.cpp
  Conversion.cpp
  ConversionChain.cpp
  Converter.cpp
  DartsDict.cpp
  Dict.cpp
  DictConverter.cpp
  DictEntry.cpp
  DictGroup.cpp
  MaxMatchSegmentation.cpp
  PhraseExtract.cpp
  SimpleConverter.cpp
  Segmentation.cpp
  TextDict.cpp
  UTF8StringSlice.cpp
  UTF8Util.cpp
)

add_library(libopencc ${LIBOPENCC_SOURCES} ${LIBOPENCC_HEADERS})
source_group(libopencc FILES ${LIBOPENCC_SOURCES} ${LIBOPENCC_HEADERS})

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
      1.0.0
    SOVERSION
      2
)

# Installation

install(
  TARGETS libopencc
  LIBRARY DESTINATION ${DIR_LIBRARY}
  ARCHIVE DESTINATION ${DIR_LIBRARY}
)

install(
  FILES ${LIBOPENCC_HEADERS}
  DESTINATION ${DIR_INCLUDE}/opencc
)

# Gtest

if (ENABLE_GTEST)
  include_directories(../deps/gtest-1.7.0/include)
  set(UNITTESTS
    BinaryDictTest.cpp
    ConfigTest.cpp
    ConversionChainTest.cpp
    ConversionTest.cpp
    DartsDictTest.cpp
    DictGroupTest.cpp
    MaxMatchSegmentationTest.cpp
    PhraseExtractTest.cpp
    SimpleConverterTest.cpp
    TextDictTest.cpp
    UTF8StringSliceTest.cpp
    UTF8UtilTest.cpp
  )
  add_executable(UnitTest ${UNITTESTS})
  target_link_libraries(UnitTest gtest gtest_main libopencc)
  add_test(UnitTest UnitTest)
endif()

# Subdir

add_subdirectory(tools)
