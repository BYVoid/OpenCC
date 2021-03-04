include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src")

set(CONFIG_TEST
  config_test/config_test.json
  config_test/config_test_characters.txt
  config_test/config_test_phrases.txt
)

set(CONFIG_TEST_TARGET_DIR ${PROJECT_BINARY_DIR}/test/config_test)
make_directory(${CONFIG_TEST_TARGET_DIR})
foreach (CONFIG_TEST_FILE ${CONFIG_TEST})
  configure_file(${CONFIG_TEST_FILE} ${CONFIG_TEST_TARGET_DIR} COPYONLY)
endforeach (CONFIG_TEST_FILE)

if (ENABLE_GTEST)
  if (WIN32)
    add_custom_target(
        copy_gtest_to_test
        ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest> ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Copying gtest to test"
    )
    add_custom_target(
        copy_gtest_main_to_test
        ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gtest_main> ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Copying gtest_main to test"
    )
  endif()

  if(NOT USE_SYSTEM_GTEST)
    include_directories(../deps/gtest-1.7.0/include)
  endif()
  set(UNITTESTS
    CommandLineConvertTest
  )
  foreach(UNITTEST ${UNITTESTS})
    add_executable(${UNITTEST} ${UNITTEST}.cpp)
    target_link_libraries(${UNITTEST} gtest gtest_main libopencc)
    add_test(${UNITTEST} ${UNITTEST})
    if (WIN32)
      add_dependencies(${UNITTEST} copy_gtest_to_test copy_gtest_main_to_test)
    endif()
  endforeach(UNITTEST)
endif()
