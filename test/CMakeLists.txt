include_directories(../deps/libdarts/src)
include_directories(../src)

add_executable(
  UnitTest
  UnitTest.cpp
)
target_link_libraries(
  UnitTest
  libopencc
)
add_test(
  UnitTest
  UnitTest
)
set(CONFIG_TEST
  config_test/config_test.json
  config_test/config_test_characters.txt
  config_test/config_test_phrases.txt
)

if (BUILD_TESTING)
  foreach (CONFIG_TEST_FILE ${CONFIG_TEST})
    configure_file(${CONFIG_TEST_FILE} ${PROJECT_BINARY_DIR}/test/config_test COPYONLY)
  endforeach (CONFIG_TEST_FILE)
endif (BUILD_TESTING)

set(CONFIGURATIONS
  s2t
  t2s
  s2tw
)

foreach(CONFIG ${CONFIGURATIONS})
  add_test(
    ${CONFIG}_convert
    ${CMAKE_COMMAND} -E chdir ${PROJECT_BINARY_DIR}/data
    ${PROJECT_BINARY_DIR}/src/opencc
    -i ${CMAKE_SOURCE_DIR}/test/testcases/${CONFIG}.in
    -o ${PROJECT_BINARY_DIR}/test/${CONFIG}.out
    -c ${CMAKE_SOURCE_DIR}/data/config/${CONFIG}.json
  )
  add_test(
    ${CONFIG}_compare
    diff
    ${PROJECT_BINARY_DIR}/test/${CONFIG}.out
    ${CMAKE_SOURCE_DIR}/test/testcases/${CONFIG}.ans
  )
  set_property(
    TEST ${CONFIG}_compare
    APPEND PROPERTY
      DEPENDS ${CONFIG}_convert)
endforeach(CONFIG)
