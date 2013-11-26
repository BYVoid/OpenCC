include_directories(../deps/libdarts/src)
include_directories(../src)

set(
  UNIT_TESTS
  TextDictTest
  DartsDictTest
  ConversionTest
)

foreach(UNIT_TEST ${UNIT_TESTS})
  add_executable(
    ${UNIT_TEST}
    ${UNIT_TEST}.cpp
  )
  target_link_libraries(
    ${UNIT_TEST}
    libopencc
  )
  add_test(
  	${UNIT_TEST}
  	${UNIT_TEST}
  )
endforeach(UNIT_TEST)
