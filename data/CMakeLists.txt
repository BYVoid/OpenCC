set(
	OPENCC_DICT_BIN
	${CMAKE_BINARY_DIR}/src/tools/opencc_dict
)

add_custom_target(
	ocds
	ALL
	DEPENDS
		simp_to_trad_characters.ocd
		simp_to_trad_phrases.ocd
		trad_to_simp_characters.ocd
		trad_to_simp_phrases.ocd
)

foreach(DICT_PREFIX simp_to_trad trad_to_simp)

	foreach(DICT_SURFIX characters phrases)

		set (DICT ${DICT_PREFIX}_${DICT_SURFIX})

		add_custom_command(
			OUTPUT
				${DICT}.ocd
			COMMENT
				"Building ${DICT}.ocd"
			COMMAND
				${OPENCC_DICT_BIN} -i ${CMAKE_SOURCE_DIR}/data/${DICT_PREFIX}/${DICT_SURFIX}.txt -o ${DICT}.ocd
			DEPENDS opencc_dict
		)

		install(
			FILES
				${CMAKE_BINARY_DIR}/data/${DICT}.ocd
			DESTINATION
				${DIR_SHARE_OPENCC}
		)

		set_directory_properties(
			PROPERTIES
				ADDITIONAL_MAKE_CLEAN_FILES
					"${CMAKE_BINARY_DIR}/data/${DICT}.ocd"
		)

	endforeach(DICT_SURFIX)
endforeach(DICT_PREFIX)

set(CONFIG_FILES
	config/zhs2zht.ini
	config/zht2zhs.ini
	config/mix2zht.ini
	config/mix2zhs.ini
	config/zhs2zhtw_p.ini
	config/zhs2zhtw_v.ini
	config/zhs2zhtw_vp.ini
	config/zht2zhtw_p.ini
	config/zht2zhtw_v.ini
	config/zht2zhtw_vp.ini
	config/zhtw2zht.ini
	config/zhtw2zhs.ini
	config/zhtw2zhcn_s.ini
	config/zhtw2zhcn_t.ini
)

install(
	FILES
		${CONFIG_FILES}
	DESTINATION
		${DIR_SHARE_OPENCC}
)

set(TAIWAN_DICT_FILES
	tw/to_tw_variants.txt
	tw/to_tw_phrases.txt
	tw/from_tw_variants.txt
	tw/from_tw_phrases.txt
)

install(
	FILES
		${TAIWAN_DICT_FILES}
	DESTINATION
		${DIR_SHARE_OPENCC}
)

set(CHINA_DICT_FILES
	cn/to_cn_phrases.txt
)

install(
	FILES
		${CHINA_DICT_FILES}
	DESTINATION
		${DIR_SHARE_OPENCC}
)

if (BUILD_TESTING)
	foreach (DICT ${TAIWAN_DICT_FILES} ${CHINA_DICT_FILES})
		configure_file(${DICT} ${PROJECT_BINARY_DIR}/data COPYONLY)
	endforeach (DICT)
endif (BUILD_TESTING)
