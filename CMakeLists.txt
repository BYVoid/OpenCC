#
# Open Chinese Convert
#
# Copyright 2010-2013 BYVoid <byvoid@byvoid.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

######## Project settings
cmake_minimum_required(VERSION 2.8)
set (PACKAGE_NAME opencc)
project (${PACKAGE_NAME} C)
include (CTest)
enable_testing()

######## Package information
set (PACKAGE_URL http://code.google.com/p/opencc)
set (PACKAGE_BUGREPORT http://code.google.com/p/opencc/issues/entry)
set (OPENCC_VERSION_MAJOR 0)
set (OPENCC_VERSION_MINOR 4)
set (OPENCC_VERSION_REVISION 3)

if (CMAKE_BUILD_TYPE MATCHES Debug)
	set (version_suffix .Debug)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

set (
	OPENCC_VERSION
	${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}.${OPENCC_VERSION_REVISION}${version_suffix}
)

set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${PACKAGE_NAME}-${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}.${OPENCC_VERSION_REVISION}"
)
set(CPACK_SOURCE_IGNORE_FILES
  "/build/;/release/;/debug/;/other/;/opencc.xcodeproj/;/.git/;.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}"
)
include(CPack)

######## Validation
option(BUILD_DOCUMENTATION "Use Doxygen to create the HTML based API documentation" OFF)

if (ENABLE_GETTEXT)
	find_package(Gettext REQUIRED)
endif (ENABLE_GETTEXT)

include (TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)
if (BIGENDIAN)
	set (BYTEORDER BIG_ENDIAN)
else (BIGENDIAN)
	set (BYTEORDER LITTLE_ENDIAN)
endif (BIGENDIAN)

######## Windows

if (WIN32)
	set(CMAKE_SHARED_LIBRARY_PREFIX ${CMAKE_INSTALL_PREFIX})
	set(CMAKE_STATIC_LIBRARY_PREFIX ${CMAKE_INSTALL_PREFIX})
endif (WIN32)

######## Directory

set (DIR_PREFIX ${CMAKE_INSTALL_PREFIX})
set (DIR_LIBRARY ${DIR_PREFIX}/${CMAKE_SHARED_LIBRARY_PREFIX})
set (DIR_LIBRARY_STATIC ${DIR_PREFIX}/${CMAKE_STATIC_LIBRARY_PREFIX})
set (DIR_INCLUDE ${DIR_PREFIX}/include)
set (DIR_SHARE ${DIR_PREFIX}/share)
set (DIR_BIN ${DIR_PREFIX}/bin)
set (DIR_ETC ${DIR_PREFIX}/etc)

if (DEFINED CMAKE_INSTALL_LIBDIR)
	set (DIR_LIBRARY ${CMAKE_INSTALL_LIBDIR})
	set (DIR_LIBRARY_STATIC ${CMAKE_INSTALL_LIBDIR})
endif (DEFINED CMAKE_INSTALL_LIBDIR)

if (DEFINED SHARE_INSTALL_PREFIX)
	set (DIR_SHARE ${SHARE_INSTALL_PREFIX})
endif (DEFINED SHARE_INSTALL_PREFIX)

if (DEFINED INCLUDE_INSTALL_DIR)
	set (DIR_INCLUDE ${INCLUDE_INSTALL_DIR})
endif (DEFINED INCLUDE_INSTALL_DIR)

if (DEFINED SYSCONF_INSTALL_DIR)
	set (DIR_ETC ${SYSCONF_INSTALL_DIR})
endif (DEFINED SYSCONF_INSTALL_DIR)

set (DIR_SHARE_OPENCC ${DIR_SHARE}/opencc)
set (DIR_SHARE_LOCALE ${DIR_SHARE}/locale)

######## Configuration

configure_file(
	opencc.pc.in
	opencc.pc
	@ONLY
)

install(
	FILES
		${CMAKE_BINARY_DIR}/opencc.pc
	DESTINATION
		${DIR_LIBRARY}/pkgconfig
)

######## Subdirectories

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(data)

if (GETTEXT_FOUND AND ENABLE_GETTEXT)
	add_subdirectory(po)
endif (GETTEXT_FOUND AND ENABLE_GETTEXT)

add_subdirectory(test)
