file(STRINGS LINGUAS LANGUAGES)
separate_arguments(LANGUAGES)
set(DOMAIN ${PACKAGE_NAME})

foreach(LANG ${LANGUAGES})

	add_custom_target(
		${LANG}_mo
		ALL
		DEPENDS
			${LANG}.mo
	)

	add_custom_command(
		OUTPUT ${LANG}.mo
		COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
				${GETTEXT_MSGFMT_PARAMETER}
				-o ${LANG}.mo ${CMAKE_SOURCE_DIR}/po/${LANG}.po
		DEPENDS 
			${LANG}.po
		COMMENT "mo-update [${LANG}]: Creating mo file."
	)

	install(
		FILES
			${CMAKE_BINARY_DIR}/po/${LANG}.mo
		RENAME
			${DOMAIN}.mo
		DESTINATION
			${DIR_SHARE_LOCALE}/${LANG}/LC_MESSAGES
	)

endforeach(LANG ${LANGUAGES})