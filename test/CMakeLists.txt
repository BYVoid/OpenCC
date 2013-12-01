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
