set(
	LIBOPENCC_HEADERS
	opencc.h
	opencc_types.h
	wrapper/cplusplus/openccxx.h
)

set(
	LIBOPENCC_DICTIONARY_SOURCES
	dict.c
	dictionary/datrie.c
	dictionary/text.c
	dict.h
	dictionary/datrie.h
	dictionary/text.h
)

set(
	LIBOPENCC_SOURCES
	${LIBOPENCC_DICTIONARY_SOURCES}
	config_reader.c
	converter.c
	dict_group.c
	dict_chain.c
	encoding.c
	utils.c
	opencc.c
	config_reader.h
	converter.h
	dict_group.h
	dict_chain.h
	encoding.h
	utils.h
)

set (LIBOPENCC_TARGET libopencc)
set (LIBOPENCC_STATIC_TARGET libopencc_static)

add_definitions(
	-DPKGDATADIR="${DIR_SHARE_OPENCC}"
	-DLOCALEDIR="${DIR_SHARE_LOCALE}"
	-DVERSION="${OPENCC_VERSION}"
	-DBYTEORDER=${BYTEORDER}
	-DPACKAGE_NAME="${PACKAGE_NAME}"
	-Wall
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

if (ENABLE_GETTEXT)

	add_definitions(
		-DENABLE_GETTEXT
	)
	
	link_directories(
	${GETTEXT_LIBRARIES}
	)

	include_directories(
		${GETTEXT_INCLUDE_DIR}
	)
	
endif (ENABLE_GETTEXT)

if (CMAKE_BUILD_TYPE MATCHES Debug)

	add_definitions(
		-O0
		-g3
	)

endif (CMAKE_BUILD_TYPE MATCHES Debug)

if (NOT WIN32)
install(
	TARGETS
		${LIBOPENCC_TARGET}
	LIBRARY DESTINATION
		${DIR_LIBRARY}
)
endif (NOT WIN32)

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

include(symbols.cmake)

add_subdirectory(tools)
