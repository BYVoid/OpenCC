cmake_minimum_required(VERSION 2.8)
project (opencc C)

find_package(PkgConfig)

configure_file(
	opencc.pc.in
	opencc.pc
	@ONLY
)

install(
	FILES
		${CMAKE_BINARY_DIR}/opencc.pc
	DESTINATION
		lib/pkgconfig
)

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(data)
