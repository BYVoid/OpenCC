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

libopencc_headers = \
	opencc.h \
	opencc_types.h \
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
	opencc_config.c \
	opencc_converter.c \
	opencc_dictionary.c \
	opencc_encoding.c \
	opencc_utils.c \
	opencc.c \
	opencc_config.h \
	opencc_converter.h \
	opencc_dictionary.h \
	opencc_encoding.h \
	opencc_utils.h \
	$(libopencc_dictionary_sources) \
	$(NULL)

libopencc_ladir = $(pkgincludedir)
libopencc_la_HEADERS = \
	$(libopencc_headers) \
	$(NULL)

libopencc_la_CFLAGS = \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	$(NULL)

libopencc_la_LDFLAGS = \
	-export-symbols-regex "opencc_.*" \
	$(NULL)

opencc_DEPENDENCIES = \
	$(libopencc) \
	$(NULL)

opencc_SOURCES = \
	tools/opencc.c \
	opencc_utils.c \
	opencc_utils.h \
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
	opencc_dictionary.c \
	opencc_dictionary.h \
	opencc_encoding.c \
	opencc_encoding.h \
	opencc_utils.c \
	opencc_utils.h \
	$(libopencc_dictionary_sources) \
	$(libopencc_headers) \
	$(NULL)
	
opencc_dict_CFLAGS = \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	$(NULL)

opencc_dict_LDADD = \
	$(libopencc) \
	$(NULL)

pkgdata_DATA = \
	simp_to_trad.ocd \
	trad_to_simp.ocd \
	$(NULL)

EXTRA_DIST = \
	$(NULL)

CLEANFILES = \
	$(pkgdata_DATA) \
	$(NULL)

simp_to_trad_txt = $(top_srcdir)/data/simp_to_trad.txt
trad_to_simp_txt = $(top_srcdir)/data/trad_to_simp.txt

simp_to_trad.ocd: $(simp_to_trad_txt) opencc_dict
	$(AM_V_GEN) \
	./opencc_dict -i $(simp_to_trad_txt) -o $@

trad_to_simp.ocd: $(trad_to_simp_txt) opencc_dict
	$(AM_V_GEN) \
	./opencc_dict -i $(trad_to_simp_txt) -o $@
