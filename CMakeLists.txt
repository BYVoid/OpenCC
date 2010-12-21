#
# Open Chinese Convert
#
# Copyright 2010 BYVoid <byvoid.kcp@gmail.com>
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

cmake_minimum_required(VERSION 2.8)
set (PACKAGE_NAME opencc)
project (${PACKAGE_NAME} C)

set (PACKAGE_URL http://code.google.com/p/open-chinese-convert)
set (PACKAGE_BUGREPORT http://code.google.com/p/open-chinese-convert/issues/entry)
set (OPENCC_VERSION_MAJOR 0)
set (OPENCC_VERSION_MINOR 1)
set (OPENCC_VERSION_REVISION 2)
set (OPENCC_RELEASED 0)

if (NOT OPENCC_RELEASED)
	set (version_suffix .debug)
endif (NOT OPENCC_RELEASED)

set (
	OPENCC_VERSION
	${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}.${OPENCC_VERSION_REVISION}${version_suffix}
)

set (DIR_PREFIX ${CMAKE_INSTALL_PREFIX})
set (DIR_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX})
set (DIR_LIBRARY_STATIC ${CMAKE_STATIC_LIBRARY_PREFIX})
set (DIR_INCLUDE include)
set (DIR_SHARE share)
set (DIR_BIN bin)

find_package(Gettext REQUIRED)

include (TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)
if (BIGENDIAN)
	set (BYTEORDER BIG_ENDIAN)
else (BIGENDIAN)
	set (BYTEORDER LITTLE_ENDIAN)
endif (BIGENDIAN)

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

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(data)
if (GETTEXT_FOUND)
	add_subdirectory(po)
endif (GETTEXT_FOUND)