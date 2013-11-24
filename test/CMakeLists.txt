add_executable(
  TextDictTest
  TextDictTest.cpp
  ${CMAKE_SOURCE_DIR}/src/TextDict.cpp
  ${CMAKE_SOURCE_DIR}/src/UTF8Util.cpp
)

add_definitions(
  -I${CMAKE_SOURCE_DIR}/src
  -std=c++11
  -Wall
)

## FIXME C++ link
target_link_libraries(
  TextDictTest
  ${LIBOPENCC_TARGET}
)

add_test(
	TextDictTest
	${PROJECT_BINARY_DIR}/test/TextDictTest
)
