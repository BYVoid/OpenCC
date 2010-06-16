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

libopencc = libopencc.la

lib_LTLIBRARIES = $(libopencc)
bin_PROGRAMS = opencc opencc_dict
#noinst_PROGRAMS = opencc_build_datrie

libopencc_headers = \
	opencc.h \
	wrapper/cplusplus/openccxx.h \
	$(NULL)

libopencc_dictionary_sources = \
	dictionary/opencc_dictionary_abstract.c \
	dictionary/opencc_dictionary_datrie.c \
	dictionary/opencc_dictionary_text.c \
	dictionary/opencc_dictionary_abstract.h \
	dictionary/opencc_dictionary_datrie.h \
	dictionary/opencc_dictionary_text.h \
	$(NULL)

libopencc_la_SOURCES = \
	opencc_converter.c \
	opencc_dictionary.c \
	opencc_encoding.c \
	opencc_utils.c \
	opencc.c \
	opencc_converter.h \
	opencc_dictionary.h \
	opencc_encoding.h \
	opencc_utils.h \
	$(libopencc_dictionary_sources) \
	$(NULL)

libopencc_la_HEADERS = \
	$(libopencc_headers) \
	$(NULL)

libopencc_la_CFLAGS = \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	$(NULL)

libopencc_la_LDFLAGS = \
	-export-symbols-regex "opencc_.*" \
	$(NULL)

libopencc_ladir = $(pkgincludedir)

#nodist_libopencc_la_SOURCES = \
#	$(libopencc_built_headers) \
#	$(NULL)

#BUILT_SOURCES = \
#	$(libopencc_built_headers) \
#	$(NULL)

pkgdata_DATA = \
	$(NULL)

opencc_DEPENDENCIES = \
	$(libopencc) \
	$(NULL)

opencc_SOURCES = \
	tools/opencc.c \
	$(libopencc_headers) \
	$(NULL)

opencc_LDADD = \
	$(libopencc) \
	$(NULL)

opencc_dict_DEPENDENCIES = \
	$(libopencc) \
	$(NULL)

opencc_dict_SOURCES = \
	tools/opencc_dict.c \
	opencc_utils.c \
	opencc_utils.h \
	dictionary/opencc_dictionary_datrie.c \
	dictionary/opencc_dictionary_datrie.h \
	$(libopencc_headers) \
	$(NULL)

opencc_dict_LDADD = \
	$(libopencc) \
	$(NULL)

#opencc_build_datrie_SOURCES = \
#	opencc_build_datrie.c \
#	$(libopencc_headers) \
#	$(NULL)

opencc_table = $(top_srcdir)/data/table.txt

#EXTRA_DIST = \
#	refine-opencc-table.py \
#	$(NULL)

CLEANFILES = \
	$(NULL)

opencc_datrie_table.h: table.refined.txt opencc_build_datrie
	$(AM_V_GEN) \
	./opencc_build_datrie < table.refined.txt > $@

table.refined.txt: $(opencc_table)
	$(AM_V_GEN) \
	$(PYTHON) $(srcdir)/refine-opencc-table.py < $(opencc_table) > table.refined.txt

