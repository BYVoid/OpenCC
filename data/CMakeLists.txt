set(
	OPENCC_DICT_BIN
	${CMAKE_BINARY_DIR}/src/tools/opencc_dict
)

add_custom_target(
	ocds
	DEPENDS opencc_dict
)

add_custom_command(
	TARGET
		ocds
	POST_BUILD
	COMMENT
		"Building simp_to_trad_characters.ocd"
	COMMAND
		${OPENCC_DICT_BIN} -i ${CMAKE_SOURCE_DIR}/data/simp_to_trad/characters.txt -o simp_to_trad_characters.ocd
)

add_custom_command(
	TARGET
		ocds
	POST_BUILD
	COMMENT
		"Building simp_to_trad_phrases.ocd"
	COMMAND
		${OPENCC_DICT_BIN} -i ${CMAKE_SOURCE_DIR}/data/simp_to_trad/phrases.txt -o simp_to_trad_phrases.ocd
)

add_custom_command(
	TARGET
		ocds
	POST_BUILD
	COMMENT
		"Building trad_to_simp_characters.ocd"
	COMMAND
		${OPENCC_DICT_BIN} -i ${CMAKE_SOURCE_DIR}/data/trad_to_simp/characters.txt -o trad_to_simp_characters.ocd
)

add_custom_command(
	TARGET
		ocds
	POST_BUILD
	COMMENT
		"Building trad_to_simp_phrases.ocd"
	COMMAND
		${OPENCC_DICT_BIN} -i ${CMAKE_SOURCE_DIR}/data/trad_to_simp/phrases.txt -o trad_to_simp_phrases.ocd
)

set(
	ocdfiles 
	${CMAKE_BINARY_DIR}/data/simp_to_trad_characters.ocd
	${CMAKE_BINARY_DIR}/data/simp_to_trad_phrases.ocd
	${CMAKE_BINARY_DIR}/data/trad_to_simp_characters.ocd
	${CMAKE_BINARY_DIR}/data/trad_to_simp_phrases.ocd
)

set_directory_properties(
	PROPERTIES
		ADDITIONAL_MAKE_CLEAN_FILES
			"${ocdfiles}"
)

install(
	FILES
		${ocdfiles}
	DESTINATION
		${DIR_SHARE}/opencc
)

set(
	CONFIG_FILES
	config/zhs2zht.ini
	config/zht2zhs.ini
	config/mix2zht.ini
	config/mix2zhs.ini
)

install(
	FILES
		${CONFIG_FILES}
	DESTINATION
		${DIR_SHARE}/opencc
)
