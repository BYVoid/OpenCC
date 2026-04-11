if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  # Standalone mode: Building the plugin outside of the OpenCC parent tree
  cmake_minimum_required(VERSION 3.10)
  project(opencc_jieba)

  find_package(OpenCC REQUIRED)

  if (NOT DEFINED DIR_PLUGIN)
    if (OPENCC_PLUGIN_DIR)
      set(DIR_PLUGIN "${OPENCC_PLUGIN_DIR}")
    elseif (WIN32)
      set(DIR_PLUGIN bin/opencc/plugins)
    else()
      set(DIR_PLUGIN lib/opencc/plugins)
    endif()
  endif()
  if (NOT DEFINED DIR_SHARE_OPENCC)
    if (OPENCC_SHARE_DIR)
      set(DIR_SHARE_OPENCC "${OPENCC_SHARE_DIR}")
    else()
      set(DIR_SHARE_OPENCC share/opencc)
    endif()
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
endif()
target_compile_definitions(opencc_jieba PRIVATE OPENCC_PLUGIN_BUILD)
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
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/data/config/s2twp_jieba.json
      ${CMAKE_CURRENT_SOURCE_DIR}/data/config/tw2sp_jieba.json
    DESTINATION
      ${DIR_SHARE_OPENCC}
  )

  install(
    DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}/data/jieba_dict/
    DESTINATION
      ${DIR_SHARE_OPENCC}/jieba_dict
    FILES_MATCHING
      PATTERN "*.utf8"
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
