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
opencc_table = $(top_srcdir)/data/table.txt
opencc_table_refined = $(builddir)/table.refined.txt
opencc_datrie_table_h = $(builddir)/opencc_datrie_table.h
refine_opencc_table_py = $(srcdir)/refine-opencc-table.py

lib_LTLIBRARIES = $(libopencc)
bin_PROGRAMS = opencc_conv
noinst_PROGRAMS = opencc_build_datrie

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

libopencc_la_DEPENDENCIES = \
	$(opencc_datrie_table_h) \
	$(NULL)

pkgdata_DATA = \
	$(NULL)

opencc_conv_DEPENDENCIES = \
	$(libopencc) \
	$(NULL)

opencc_conv_SOURCES = \
	opencc_conv.c \
	$(libopencc_headers) \
	$(NULL)

opencc_conv_LDADD = \
	$(libopencc) \
	$(NULL)

opencc_build_datrie_SOURCES = \
	opencc_build_datrie.c \
	$(libopencc_headers) \
	$(NULL)

EXTRA_DIST = \
	$(opencc_table) \
	$(refine_opencc_table_py) \
	$(NULL)

CLEANFILES = \
	$(opencc_table_refined) \
	$(opencc_datrie_table_h) \
	$(NULL)

$(opencc_datrie_table_h): opencc_build_datrie table.refined.txt
	$(builddir)/opencc_build_datrie < table.refined.txt > $@

table.refined.txt: $(refine_opencc_table_py) $(opencc_table)
	$(PYTHON) $(refine_opencc_table_py) < $(opencc_table) > $@
