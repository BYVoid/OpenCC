set(
  LIBOPENCC_HEADERS
  Common.hpp
  Config.hpp
  Conversion.hpp
  ConversionChain.hpp
  DartsDict.hpp
  Dict.hpp
  DictEntry.hpp
  DictGroup.hpp
  MaxMatchSegmentation.hpp
  Optional.hpp
  Segmentation.hpp
  TextDict.hpp
  UTF8Util.hpp
)

set(
  LIBOPENCC_SOURCES
  Config.cpp
  Conversion.cpp
  ConversionChain.cpp
  DartsDict.cpp
  Dict.cpp
  DictGroup.cpp
  MaxMatchSegmentation.cpp
  Opencc.cpp
  Segmentation.cpp
  TextDict.cpp
  UTF8Util.cpp
)

include_directories(../deps/libdarts/src)
include_directories(../deps/rapidjson-0.11)

add_library(
  libopencc
  SHARED
  ${LIBOPENCC_SOURCES}
)

# add_library(
#   libopencc_static
#   STATIC
#   ${LIBOPENCC_SOURCES}
# )

set_target_properties(
  libopencc
  # libopencc_static
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

if (ENABLE_GETTEXT)
  add_definitions(
    -DENABLE_GETTEXT
  )
  link_directories(
    ${GETTEXT_LIBRARIES}
  )
  include_directories(
    ${GETTEXT_INCLUDE_DIR}
  )
endif (ENABLE_GETTEXT)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(
    -O0
    -g3
  )
endif (CMAKE_BUILD_TYPE MATCHES Debug)

install(
  TARGETS
    libopencc
  LIBRARY DESTINATION
    ${DIR_LIBRARY}
)

# install(
#   TARGETS
#     libopencc_static
#   ARCHIVE DESTINATION
#     ${DIR_LIBRARY_STATIC}
# )

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
  RUNTIME
  DESTINATION
    ${DIR_BIN}
)
