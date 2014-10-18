include (GenerateExportHeader)

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
  DictEntry.hpp
  DictGroup.hpp
  Exception.hpp
  Export.hpp
  Lexicon.hpp
  MaxMatchSegmentation.hpp
  Optional.hpp
  Segmentation.hpp
  Segments.hpp
  SerializableDict.hpp
  TextDict.hpp
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
  DictEntry.cpp
  DictGroup.cpp
  MaxMatchSegmentation.cpp
  SimpleConverter.cpp
  Segmentation.cpp
  TextDict.cpp
  UTF8Util.cpp
)

include_directories(../deps/darts-clone)
include_directories(../deps/rapidjson-0.11)
include_directories(../deps/tclap-1.2.1)

add_library(
  libopencc
  ${LIBOPENCC_SOURCES}
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
    VERSION
      1.0.0
    SOVERSION
      2
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(
    -O0
    -g3
  )
endif (CMAKE_BUILD_TYPE MATCHES Debug)

install(
  TARGETS
    libopencc
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static
)

install(
  FILES
    ${LIBOPENCC_HEADERS}
  DESTINATION
    ${DIR_INCLUDE}/opencc
)

add_executable(
  opencc
  CommandLine.cpp
)
target_link_libraries(
  opencc
  libopencc
)

add_executable(
  opencc_dict
  DictConverter.cpp
)
target_link_libraries(
  opencc_dict
  libopencc
)

install(
  TARGETS
    opencc
    opencc_dict
  RUNTIME DESTINATION bin
)
