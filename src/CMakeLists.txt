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
	-DPKGDATADIR="${DIR_PREFIX}/${DIR_SHARE}"
	-DVERSION="${OPENCC_VERSION}"
	-DBYTEORDER=${BYTEORDER}
	-DPACKAGE_NAME="${PACKAGE_NAME}"
)

if (GETTEXT_FOUND)
	add_definitions(
		-DLOCALEDIR="${DIR_PREFIX}/${DIR_SHARE}/locale"
		-DHAVE_GETTEXT
	)
endif (GETTEXT_FOUND)


link_directories(
	${GETTEXT_LIBRARIES}
)

include_directories(
	${GETTEXT_INCLUDE_DIR}
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


set(
	OPENCC_SYMBOLS
	opencc_open
	opencc_close
	opencc_convert
	opencc_convert_utf8
	opencc_dict_load
	opencc_set_conversion_mode
	opencc_errno
	opencc_perror
)

set (LINK_FLAGS "")
if (APPLE)
	# Create a symbols_list file for the darwin linker
	string(REPLACE ";" "\n_" _symbols "${OPENCC_SYMBOLS}")
	set(_symbols_list "${CMAKE_CURRENT_BINARY_DIR}/symbols.list")
	file(WRITE ${_symbols_list} "_${_symbols}\n")
	set(LINK_FLAGS
		"${LINK_FLAGS} -Wl,-exported_symbols_list,'${_symbols_list}'")
elseif (CMAKE_COMPILER_IS_GNUCXX)
	# Create a version script for GNU ld.
	set(_symbols "{ global: ${OPENCC_SYMBOLS}; local: *; };")
	set(_version_script "${CMAKE_CURRENT_BINARY_DIR}/version.script")
	file(WRITE ${_version_script} "${_symbols}\n")

	# Check if the linker supports version script (i.e. is GNU ld)
	check_cxx_accepts_flag(
		"-Wl,--version-script,${_version_script}"
		LD_ACCEPTS_VERSION_SCRIPT)
	if (LD_ACCEPTS_VERSION_SCRIPT)
		set(LINK_FLAGS
			"${LINK_FLAGS} -Wl,--version-script,'${_version_script}'")
	endif (LD_ACCEPTS_VERSION_SCRIPT)
endif (APPLE)

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
		LINK_FLAGS
			${LINK_FLAGS}
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
