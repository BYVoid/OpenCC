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

libopencc = libopencc.la

lib_LTLIBRARIES = $(libopencc)
bin_PROGRAMS = opencc opencc_dict

macro_cflags = \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	$(NULL)

libopencc_headers = \
	opencc.h \
	opencc_types.h \
	wrapper/cplusplus/openccxx.h \
	$(NULL)

libopencc_dictionary_sources = \
	dictionary/abstract.c \
	dictionary/datrie.c \
	dictionary/text.c \
	dictionary/abstract.h \
	dictionary/datrie.h \
	dictionary/text.h \
	$(NULL)

libopencc_la_SOURCES = \
	config_reader.c \
	converter.c \
	dictionary_group.c \
	dictionary_set.c \
	encoding.c \
	utils.c \
	opencc.c \
	config_reader.h \
	converter.h \
	dictionary_group.h \
	dictionary_set.h \
	encoding.h \
	utils.h \
	$(libopencc_dictionary_sources) \
		$(libopencc_headers) \
	$(NULL)

libopencc_ladir = $(pkgincludedir)
libopencc_la_HEADERS = \
	$(libopencc_headers) \
	$(NULL)

libopencc_la_CFLAGS = \
	$(macro_cflags) \
	$(NULL)

libopencc_la_LDFLAGS = \
	-export-symbols-regex "opencc_.*" \
	-version-info @LT_VERSION_INFO@ \
	$(NULL)

opencc_DEPENDENCIES = \
	$(libopencc) \
	$(NULL)

opencc_SOURCES = \
	tools/opencc.c \
	utils.c \
	utils.h \
	$(libopencc_headers) \
	$(NULL)

opencc_CFLAGS = \
	$(macro_cflags) \
	$(NULL)

opencc_LDADD = \
	$(libopencc) \
	$(NULL)

opencc_dict_DEPENDENCIES = \
	$(libopencc) \
	$(NULL)

opencc_dict_SOURCES = \
	tools/opencc_dict.c \
	dictionary_group.c \
	dictionary_group.h \
	encoding.c \
	encoding.h \
	utils.c \
	utils.h \
	$(libopencc_dictionary_sources) \
	$(libopencc_headers) \
	$(NULL)
	
opencc_dict_CFLAGS = \
	$(macro_cflags) \
	$(NULL)

opencc_dict_LDADD = \
	$(libopencc) \
	$(NULL)

pkgdata_DATA = \
	simp_to_trad_characters.ocd \
	simp_to_trad_phrases.ocd \
	trad_to_simp_characters.ocd \
	trad_to_simp_phrases.ocd \
	$(NULL)

EXTRA_DIST = \
	$(NULL)

CLEANFILES = \
	$(pkgdata_DATA) \
	$(NULL)

simp_to_trad_characters_txt = $(top_srcdir)/data/simp_to_trad/characters.txt
simp_to_trad_phrases_txt = $(top_srcdir)/data/simp_to_trad/phrases.txt
trad_to_simp_characters_txt = $(top_srcdir)/data/trad_to_simp/characters.txt
trad_to_simp_phrases_txt = $(top_srcdir)/data/trad_to_simp/phrases.txt

simp_to_trad_characters.ocd: $(simp_to_trad_characters_txt) opencc_dict
	$(AM_V_GEN) \
	./opencc_dict -i $(simp_to_trad_characters_txt) -o $@

simp_to_trad_phrases.ocd: $(simp_to_trad_phrases_txt) opencc_dict
	$(AM_V_GEN) \
	./opencc_dict -i $(simp_to_trad_phrases_txt) -o $@

trad_to_simp_characters.ocd: $(trad_to_simp_characters_txt) opencc_dict
	$(AM_V_GEN) \
	./opencc_dict -i $(trad_to_simp_characters_txt) -o $@

trad_to_simp_phrases.ocd: $(trad_to_simp_phrases_txt) opencc_dict
	$(AM_V_GEN) \
	./opencc_dict -i $(trad_to_simp_phrases_txt) -o $@
