SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

# Configure test paths
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_paths.h.in" "${CMAKE_BINARY_DIR}/test/test_paths.h")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/test
)

ADD_EXECUTABLE(load_test load_test.cpp)
ADD_EXECUTABLE(benchmark_bin benchmark.cpp)
if(WIN32)
    target_link_libraries(benchmark_bin Psapi)
endif()
set_target_properties(benchmark_bin PROPERTIES OUTPUT_NAME benchmark_bin)
ADD_CUSTOM_TARGET(benchmark
    COMMAND $<TARGET_FILE:benchmark_bin>
    DEPENDS benchmark_bin
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    USES_TERMINAL)
ADD_SUBDIRECTORY(unittest)
