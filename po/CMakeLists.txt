file(STRINGS LINGUAS LANGUAGES)
separate_arguments(LANGUAGES)
set(DOMAIN ${PACKAGE_NAME})
set(LOCALE_INSTALL ${DIR_SHARE}/locale)
set(LOCALEDIR po)

############ Validation. ###########

find_program(GETTEXT_MSGINIT_EXECUTABLE msginit)
if(NOT GETTEXT_MSGINIT_EXECUTABLE)
	message(FATAL_ERROR "msginit not found")
endif(NOT GETTEXT_MSGINIT_EXECUTABLE)

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTABLE )
	message(FATAL_ERROR "xgettext not found")
endif(NOT GETTEXT_XGETTEXT_EXECUTABLE )

find_program(GETTEXT_MSGCAT_EXECUTABLE msgcat)
if(NOT GETTEXT_MSGCAT_EXECUTABLE )
	message(FATAL_ERROR "msgcat not found")
endif(NOT GETTEXT_MSGCAT_EXECUTABLE )

find_program(GETTEXT_MSGATTRIB_EXECUTABLE msgattrib)
if(NOT GETTEXT_MSGATTRIB_EXECUTABLE)
	message(FATAL_ERROR "msgattrib not found")
endif(NOT GETTEXT_MSGATTRIB_EXECUTABLE)

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
			${LOCALE_INSTALL}/${LANG}/LC_MESSAGES
	)

endforeach(LANG ${LANGUAGES})