if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  # Standalone mode: Building the plugin outside of the OpenCC parent tree
  cmake_minimum_required(VERSION 3.10)
  project(opencc_jieba)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

  find_package(OpenCC REQUIRED)

  set(_opencc_install_prefix_hint "")
  if (OPENCC_SHARE_DIR AND IS_ABSOLUTE "${OPENCC_SHARE_DIR}")
    get_filename_component(_opencc_share_parent "${OPENCC_SHARE_DIR}" DIRECTORY)
    get_filename_component(_opencc_install_prefix_hint "${_opencc_share_parent}" DIRECTORY)
  elseif (OPENCC_INCLUDE_DIR AND IS_ABSOLUTE "${OPENCC_INCLUDE_DIR}")
    get_filename_component(_opencc_include_parent "${OPENCC_INCLUDE_DIR}" DIRECTORY)
    get_filename_component(_opencc_install_prefix_hint "${_opencc_include_parent}" DIRECTORY)
  endif()

  function(opencc_make_install_destination_relative output_var package_path fallback_path)
    if (NOT package_path)
      set(${output_var} "${fallback_path}" PARENT_SCOPE)
      return()
    endif()
    if (NOT IS_ABSOLUTE "${package_path}")
      set(${output_var} "${package_path}" PARENT_SCOPE)
      return()
    endif()

    file(RELATIVE_PATH _relative_path "${CMAKE_INSTALL_PREFIX}" "${package_path}")
    if (NOT _relative_path MATCHES "^[.][.]/" AND NOT _relative_path STREQUAL "..")
      set(${output_var} "${_relative_path}" PARENT_SCOPE)
      return()
    endif()

    if (_opencc_install_prefix_hint AND IS_ABSOLUTE "${_opencc_install_prefix_hint}")
      file(RELATIVE_PATH _relative_path "${_opencc_install_prefix_hint}" "${package_path}")
      if (NOT _relative_path MATCHES "^[.][.]/" AND NOT _relative_path STREQUAL "..")
        set(${output_var} "${_relative_path}" PARENT_SCOPE)
        return()
      endif()
    endif()

    set(${output_var} "${fallback_path}" PARENT_SCOPE)
  endfunction()

  if (NOT DEFINED DIR_PLUGIN)
    if (WIN32)
      set(_opencc_default_plugin_dir bin/plugins)
    else()
      set(_opencc_default_plugin_dir lib/opencc/plugins)
    endif()
    opencc_make_install_destination_relative(
      DIR_PLUGIN "${OPENCC_PLUGIN_DIR}" "${_opencc_default_plugin_dir}"
    )
  endif()
  if (NOT DEFINED DIR_SHARE_OPENCC)
    opencc_make_install_destination_relative(
      DIR_SHARE_OPENCC "${OPENCC_SHARE_DIR}" "share/opencc"
    )
  endif()

  set(BUILD_OPENCC_JIEBA_PLUGIN ON)
else()
  # Integrated mode: Included from OpenCC root
  if (NOT BUILD_OPENCC_JIEBA_PLUGIN AND NOT ENABLE_GTEST)
    return()
  endif()
endif()

if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  include_directories("${PROJECT_SOURCE_DIR}")
  include_directories("${PROJECT_BINARY_DIR}/src")
  include_directories("${PROJECT_SOURCE_DIR}/src")
endif()
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/cppjieba/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/deps/cppjieba/deps/limonp/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  include_directories("${PROJECT_SOURCE_DIR}/deps/rapidjson-1.1.0")
endif()

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  # OpenCC C++ Headers are provided by find_package
  add_library(opencc_jieba SHARED
    src/JiebaSegmentation.cpp
    src/JiebaSegmentationPlugin.cpp
  )
  target_link_libraries(opencc_jieba PRIVATE OpenCC::OpenCC)
  add_executable(cppjieba_dict tools/cppjieba_dict.cpp)
  target_link_libraries(cppjieba_dict PRIVATE OpenCC::OpenCC)
else()
  add_library(opencc_jieba SHARED
    src/JiebaSegmentation.cpp
    src/JiebaSegmentationPlugin.cpp
    ${PROJECT_SOURCE_DIR}/src/plugin/OpenCCPlugin.h
  )
  target_include_directories(opencc_jieba PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src
  )
  target_link_libraries(opencc_jieba PRIVATE libopencc)
  add_executable(cppjieba_dict tools/cppjieba_dict.cpp)
  target_include_directories(cppjieba_dict PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src
  )
  target_link_libraries(cppjieba_dict PRIVATE libopencc)
endif()
target_compile_definitions(opencc_jieba PRIVATE OPENCC_PLUGIN_BUILD)

set(JIEBA_MERGED_DICT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/jieba_dict")
set(JIEBA_MERGED_DICT "${JIEBA_MERGED_DICT_DIR}/jieba_merged.ocd2")
set(OPENCC_CAN_RUN_CPPJIEBA_DICT TRUE)

if (CMAKE_CROSSCOMPILING)
  set(OPENCC_CAN_RUN_CPPJIEBA_DICT FALSE)
endif()

if (WIN32 AND OPENCC_CAN_RUN_CPPJIEBA_DICT)
  if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(_opencc_runtime_target OpenCC::OpenCC)
  else()
    set(_opencc_runtime_target libopencc)
  endif()

  add_custom_target(
    copy_opencc_to_cppjieba_dict
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${_opencc_runtime_target}>
            $<TARGET_FILE_DIR:cppjieba_dict>
    DEPENDS cppjieba_dict
    COMMENT "Copying OpenCC runtime to directory of cppjieba_dict"
  )
  set(JIEBA_MERGED_DICT_WIN32_DEPENDS copy_opencc_to_cppjieba_dict)
else()
  set(JIEBA_MERGED_DICT_WIN32_DEPENDS)
endif()

if (OPENCC_CAN_RUN_CPPJIEBA_DICT)
  add_custom_command(
    OUTPUT "${JIEBA_MERGED_DICT}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${JIEBA_MERGED_DICT_DIR}"
    COMMAND $<TARGET_FILE:cppjieba_dict>
            -i "${CMAKE_CURRENT_SOURCE_DIR}/deps/cppjieba/dict/jieba.dict.utf8"
            -i "${CMAKE_CURRENT_SOURCE_DIR}/deps/cppjieba/dict/user.dict.utf8"
            -o "${JIEBA_MERGED_DICT}"
    DEPENDS
      ${JIEBA_MERGED_DICT_WIN32_DEPENDS}
      cppjieba_dict
      "${CMAKE_CURRENT_SOURCE_DIR}/deps/cppjieba/dict/jieba.dict.utf8"
      "${CMAKE_CURRENT_SOURCE_DIR}/deps/cppjieba/dict/user.dict.utf8"
    VERBATIM
  )

  add_custom_target(jieba_merged_dict ALL DEPENDS "${JIEBA_MERGED_DICT}")
  add_dependencies(opencc_jieba jieba_merged_dict)
endif()

# On Windows, OpenCC headers must not use dllimport when the plugin links
# against a static OpenCC library. This affects both integrated builds and
# standalone plugin builds via find_package(OpenCC).
if (WIN32)
  if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    get_target_property(_opencc_target_type OpenCC::OpenCC TYPE)
  else()
    get_target_property(_opencc_target_type libopencc TYPE)
  endif()
  if (_opencc_target_type STREQUAL "STATIC_LIBRARY")
    target_compile_definitions(opencc_jieba PRIVATE Opencc_BUILT_AS_STATIC)
    target_compile_definitions(cppjieba_dict PRIVATE Opencc_BUILT_AS_STATIC)
  endif()
endif()

set_target_properties(opencc_jieba PROPERTIES
  OUTPUT_NAME opencc-jieba
  POSITION_INDEPENDENT_CODE ON
)
target_include_directories(opencc_jieba PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/cppjieba/include
  ${CMAKE_CURRENT_SOURCE_DIR}/deps/cppjieba/deps/limonp/include
)

if (BUILD_OPENCC_JIEBA_PLUGIN)
  install(
    TARGETS opencc_jieba
    LIBRARY DESTINATION ${DIR_PLUGIN}
    ARCHIVE DESTINATION ${DIR_PLUGIN}
    RUNTIME DESTINATION ${DIR_PLUGIN}
  )

  install(
    TARGETS cppjieba_dict
    RUNTIME DESTINATION bin
  )

  if (OPENCC_CAN_RUN_CPPJIEBA_DICT)
    install(
      FILES "${JIEBA_MERGED_DICT}"
      DESTINATION ${DIR_SHARE_OPENCC}/jieba_dict
    )
  endif()

  install(
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/data/config/s2twp_jieba.json
      ${CMAKE_CURRENT_SOURCE_DIR}/data/config/tw2sp_jieba.json
    DESTINATION
      ${DIR_SHARE_OPENCC}
  )

  install(
    DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}/deps/cppjieba/dict/
    DESTINATION
      ${DIR_SHARE_OPENCC}/jieba_dict
    FILES_MATCHING
      PATTERN "*.utf8"
      PATTERN "pos_dict" EXCLUDE
      PATTERN "README.md" EXCLUDE
  )
endif()

if (ENABLE_GTEST AND NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  if (WIN32)
    add_custom_target(
      copy_gtest_to_plugins_jieba
      ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest> ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Copying gtest to plugins/jieba"
    )
    add_custom_target(
      copy_gtest_main_to_plugins_jieba
      ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest_main> ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Copying gtest_main to plugins/jieba"
    )
  endif()

  add_executable(JiebaPluginIntegrationTest tests/JiebaPluginIntegrationTest.cpp)
  target_compile_definitions(JiebaPluginIntegrationTest PRIVATE
    OPENCC_TEST_COMMAND="$<TARGET_FILE:opencc>"
    OPENCC_PLUGIN_TEST_DIR="$<TARGET_FILE_DIR:opencc_jieba>"
  )
  target_link_libraries(JiebaPluginIntegrationTest gtest gtest_main)
  add_dependencies(JiebaPluginIntegrationTest opencc opencc_jieba Dictionaries)
  add_test(JiebaPluginIntegrationTest JiebaPluginIntegrationTest)
  set_tests_properties(
    JiebaPluginIntegrationTest
    PROPERTIES
      ENVIRONMENT "OPENCC_SEGMENTATION_PLUGIN_PATH=$<TARGET_FILE_DIR:opencc_jieba>"
  )
  if (WIN32)
    add_dependencies(JiebaPluginIntegrationTest
      copy_gtest_to_plugins_jieba
      copy_gtest_main_to_plugins_jieba
    )
  endif()
endif()
