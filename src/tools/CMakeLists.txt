set(
	LIBOPENCC_DICTIONARY_SOURCES
	../dict.c
	../dictionary/datrie.c
	../dictionary/text.c
	../dict.h
	../dictionary/datrie.h
	../dictionary/text.h
)

set(
	OPENCC_DCIT_SOURCES
	${LIBOPENCC_DICTIONARY_SOURCES}
	opencc_dict.c
	../dict_group.c
	../dict_group.h
	../dict_chain.c
	../dict_chain.h
	../config_reader.c
	../config_reader.h
	../encoding.c
	../encoding.h
	../utils.c
	../utils.h
)

add_executable(
	opencc_dict
	${OPENCC_DCIT_SOURCES}
)

target_link_libraries(
	opencc_dict
	${LIBOPENCC_TARGET}
)

install(
	TARGETS
		opencc_dict
	RUNTIME
	DESTINATION
		${DIR_BIN}
)


set(
	OPENCC_SOURCES
	opencc.c
	../utils.c
	../utils.h
)

add_executable(
	opencc
	${OPENCC_SOURCES}
)

add_dependencies(
	opencc
	ocds
)

target_link_libraries(
	opencc
	${LIBOPENCC_TARGET}
)

install(
	TARGETS
		opencc
	RUNTIME
	DESTINATION
		${DIR_BIN}
)
