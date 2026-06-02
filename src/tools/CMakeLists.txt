# Executables

include_directories("${PROJECT_SOURCE_DIR}")

if (WIN32)
  set(RC_FILE_DESCRIPTION "OpenCC Command Line")
  set(RC_INTERNAL_NAME "opencc")
  set(RC_ORIGINAL_FILENAME "opencc.exe")
  set(RC_FILE_TYPE "VFT_APP")
  configure_file(
    "${PROJECT_SOURCE_DIR}/src/version.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version_opencc.rc"
    @ONLY
  )

  set(RC_FILE_DESCRIPTION "OpenCC Dictionary Converter")
  set(RC_INTERNAL_NAME "opencc_dict")
  set(RC_ORIGINAL_FILENAME "opencc_dict.exe")
  configure_file(
    "${PROJECT_SOURCE_DIR}/src/version.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version_opencc_dict.rc"
    @ONLY
  )

  set(RC_FILE_DESCRIPTION "OpenCC Phrase Extractor")
  set(RC_INTERNAL_NAME "opencc_phrase_extract")
  set(RC_ORIGINAL_FILENAME "opencc_phrase_extract.exe")
  configure_file(
    "${PROJECT_SOURCE_DIR}/src/version.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version_opencc_phrase_extract.rc"
    @ONLY
  )
endif()

## opencc
add_executable(opencc
  CommandLine.cpp
  CommandLineMain.cpp
  PlatformIO.cpp
)
target_link_libraries(opencc libopencc)
if (WIN32)
  target_link_libraries(opencc shell32)
endif()

## opencc_dict
add_executable(opencc_dict DictConverter.cpp)
target_link_libraries(opencc_dict libopencc)

## opencc_phrase_extract
add_executable(opencc_phrase_extract PhraseExtract.cpp)
target_link_libraries(opencc_phrase_extract libopencc)

if (WIN32)
  target_sources(opencc PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/version_opencc.rc")
  target_sources(opencc_dict PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/version_opencc_dict.rc")
  target_sources(opencc_phrase_extract PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/version_opencc_phrase_extract.rc")
endif()

# Installation

if(OPENCC_ENABLE_INSTALL)
install(
  TARGETS opencc opencc_dict opencc_phrase_extract
  RUNTIME DESTINATION bin
)
endif()
