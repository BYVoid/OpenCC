set(
	LIBOPENCC_HEADERS
	opencc.h
	opencc_types.h
	wrapper/cplusplus/openccxx.h
)

set(
	LIBOPENCC_DICTIONARY_SOURCES
	dictionary/abstract.c
	dictionary/datrie.c
	dictionary/text.c
	dictionary/abstract.h
	dictionary/datrie.h
	dictionary/text.h
)

set(
	LIBOPENCC_SOURCES
	${LIBOPENCC_DICTIONARY_SOURCES}
	config_reader.c
	converter.c
	dictionary_group.c
	dictionary_set.c
	encoding.c
	utils.c
	opencc.c
	config_reader.h
	converter.h
	dictionary_group.h
	dictionary_set.h
	encoding.h
	utils.h
)

set (LIBOPENCC_TARGET libopencc)
set (LIBOPENCC_STATIC_TARGET libopencc_static)

add_definitions(
	-DPKGDATADIR="/usr/share"
	-DLOCALEDIR="/usr/share/locale"
	-DVERSION="0.1.2"
	-DBYTEORDER=LITTLE_ENDIAN
)

add_library(
	${LIBOPENCC_TARGET}
	SHARED
	${LIBOPENCC_SOURCES}
)

add_library(
	${LIBOPENCC_STATIC_TARGET}
	STATIC
	${LIBOPENCC_SOURCES}
)

set_target_properties(
	${LIBOPENCC_TARGET}
	${LIBOPENCC_STATIC_TARGET}
	PROPERTIES
		OUTPUT_NAME
			opencc
		VERSION
			1.0.0
		SOVERSION
			1
)

install(
	TARGETS
		${LIBOPENCC_TARGET}
	LIBRARY DESTINATION
		${DIR_LIBRARY}
)

install(
	TARGETS
		${LIBOPENCC_STATIC_TARGET}
	ARCHIVE DESTINATION
		${DIR_LIBRARY_STATIC}
)

install(
	FILES
		${LIBOPENCC_HEADERS}
	DESTINATION
		${DIR_INCLUDE}/opencc
)

add_subdirectory(tools)
