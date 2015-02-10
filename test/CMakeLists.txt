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

set(CONFIG_TEST_TARGET_DIR ${PROJECT_BINARY_DIR}/test/config_test)
make_directory(${CONFIG_TEST_TARGET_DIR})
foreach (CONFIG_TEST_FILE ${CONFIG_TEST})
  configure_file(${CONFIG_TEST_FILE} ${CONFIG_TEST_TARGET_DIR} COPYONLY)
endforeach (CONFIG_TEST_FILE)

if (ENABLE_GTEST)
  add_definitions(
    -DPROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
    -DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
  )
  include_directories(../deps/gtest-1.7.0/include)
  set(UNITTESTS
    CommandLineConvertTest
  )
  foreach(UNITTEST ${UNITTESTS})
    add_executable(${UNITTEST} ${UNITTEST}.cpp)
    target_link_libraries(${UNITTEST} gtest gtest_main libopencc)
    add_test(${UNITTEST} ${UNITTEST})
  endforeach(UNITTEST)
endif()
