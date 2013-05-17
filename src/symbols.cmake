set(
	OPENCC_SYMBOLS
	opencc_open
	opencc_close
	opencc_convert
	opencc_convert_utf8
	opencc_convert_utf8_free
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

elseif (CMAKE_C_COMPILER_ID STREQUAL GNU)
	# Create a version script for GNU ld.
	set(_symbols "{ global: ${OPENCC_SYMBOLS}; local: *; };")
	set(_version_script "${CMAKE_CURRENT_BINARY_DIR}/version.script")
	file(WRITE ${_version_script} "${_symbols}\n")

	set(LINK_FLAGS "${LINK_FLAGS} -Wl,--version-script,'${_version_script}'")
	
endif (APPLE)

set_target_properties(
	${LIBOPENCC_TARGET}
	${LIBOPENCC_STATIC_TARGET}
	PROPERTIES
		LINK_FLAGS
			"${LINK_FLAGS}"
)
