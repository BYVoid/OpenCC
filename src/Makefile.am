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
bin_PROGRAMS = opencc
noinst_PROGRAMS = opencc_build_datrie

libopencc_headers = \
	opencc.h \
	opencc_convert.h \
	$(NULL)
libopencc_built_headers = \
	opencc_datrie_table.h \
	$(NULL)
libopencc_la_SOURCES = \
	opencc_datrie.c \
	opencc_convert.c \
	opencc_datrie.h \
	opencc_utils.h \
	$(NULL)
libopencc_la_HEADERS = \
	$(libopencc_headers) \
	$(NULL)
libopencc_ladir = $(pkgincludedir)
nodist_libopencc_la_SOURCES = \
	$(libopencc_built_headers) \
	$(NULL)
BUILT_SOURCES = \
	$(libopencc_built_headers) \
	$(NULL)

pkgdata_DATA = \
	$(NULL)

opencc_DEPENDENCIES = \
	$(libopencc) \
	$(NULL)

opencc_SOURCES = \
	opencc.c \
	$(libopencc_headers) \
	$(NULL)

opencc_LDADD = \
	$(libopencc) \
	$(NULL)

opencc_build_datrie_SOURCES = \
	opencc_build_datrie.c \
	$(libopencc_headers) \
	$(NULL)

opencc_table = $(top_srcdir)/data/table.txt

EXTRA_DIST = \
	refine-opencc-table.py \
	$(NULL)

CLEANFILES = \
	opencc_datrie_table.h \
	table.refined.txt \
	$(NULL)

opencc_datrie_table.h: table.refined.txt opencc_build_datrie
	$(AM_V_GEN) \
	./opencc_build_datrie < table.refined.txt > $@

table.refined.txt: $(opencc_table)
	$(AM_V_GEN) \
	$(PYTHON) $(srcdir)/refine-opencc-table.py < $(opencc_table) > table.refined.txt

