set(
  LIBOPENCC_HEADERS
  Common.hpp
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

set (LIBOPENCC_TARGET libopencc)
set (LIBOPENCC_STATIC_TARGET libopencc_static)

add_definitions(
  -I${CMAKE_SOURCE_DIR}/deps/libdarts/src
  -std=c++11
  -DPKGDATADIR="${DIR_SHARE_OPENCC}"
  -DLOCALEDIR="${DIR_SHARE_LOCALE}"
  -DVERSION="${OPENCC_VERSION}"
  -DPACKAGE_NAME="${PACKAGE_NAME}"
  -Wall
)

add_library(
  ${LIBOPENCC_TARGET}
  SHARED
  ${LIBOPENCC_SOURCES}
)

# add_library(
#   ${LIBOPENCC_STATIC_TARGET}
#   STATIC
#   ${LIBOPENCC_SOURCES}
# )

set_target_properties(
  ${LIBOPENCC_TARGET}
  # ${LIBOPENCC_STATIC_TARGET}
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
    ${LIBOPENCC_TARGET}
  LIBRARY DESTINATION
    ${DIR_LIBRARY}
)

# install(
#   TARGETS
#     ${LIBOPENCC_STATIC_TARGET}
#   ARCHIVE DESTINATION
#     ${DIR_LIBRARY_STATIC}
# )

install(
  FILES
    ${LIBOPENCC_HEADERS}
  DESTINATION
    ${DIR_INCLUDE}/opencc
)
