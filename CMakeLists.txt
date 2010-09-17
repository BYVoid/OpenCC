cmake_minimum_required(VERSION 2.8)
project (opencc C)

find_package(PkgConfig)

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(data)
