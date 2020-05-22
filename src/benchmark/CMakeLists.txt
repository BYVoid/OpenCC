include_directories(..)

add_executable(performance Performance.cpp)
target_link_libraries(performance benchmark libopencc)
add_test(BenchmarkTest performance)

if (WIN32)
  add_custom_target(
    copy_benchmark
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:benchmark> ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Copy benchmark"
  )
  add_custom_target(
    copy_opencc
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libopencc> ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Copy opencc"
  )
  add_dependencies(performance copy_benchmark copy_opencc)
endif()
