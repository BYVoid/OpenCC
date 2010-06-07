#
# Open Chinese Convert
#
# Copyright 2010 BYVoid <byvoid1@gmail.com>
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

lib_LTLIBRARIES = libopencc.la
bin_PROGRAMS = opencc_conv

libopencc_headers = \
	opencc.h \
	opencc_datrie.h \
	opencc_convert.h \
	opencc_utils.h \
	$(NULL)

libopencc_la_SOURCES = \
	opencc_datrie.c \
	opencc_convert.c \
	$(libopencc_headers) \
	$(NULL)

libopencc_la_DATA = \
	datrie.txt \
	$(NULL)

libopencc_ladir = .

opencc_conv_DEPENDENCIES = \
	$(lib_LTLIBRARIES) \
	$(NULL)

opencc_conv_SOURCES = \
	opencc_conv.c \
	$(libopencc_headers) \
	$(NULL)

opencc_conv_LDADD = \
	$(lib_LTLIBRARIES) \
	$(NULL)

CLEANFILES = \
	datrie.txt \
	opencc_build_datrie \
	$(NULL)

datrie.txt: datrie.in opencc_build_datrie.c opencc_datrie.c
	gcc opencc_build_datrie.c opencc_datrie.c -o opencc_build_datrie
	./opencc_build_datrie
