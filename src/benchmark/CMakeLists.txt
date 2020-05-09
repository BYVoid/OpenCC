include_directories(..)

add_executable(performance Performance.cpp)
target_link_libraries(performance benchmark libopencc)
add_test(performance performance)
