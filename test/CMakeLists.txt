set(CONFIGURATIONS
	zhs2zht
	zht2zhs
	mix2zht
	mix2zhs
	zhs2zhtw_p
	zhs2zhtw_vp
	zhtw2zhcn_t
	zhtw2zhcn_s
)

foreach(CONFIG ${CONFIGURATIONS})

	add_test(
		${CONFIG}_convert
		${CMAKE_COMMAND} -E chdir ${PROJECT_BINARY_DIR}/data
		${PROJECT_BINARY_DIR}/src/tools/opencc
		-i ${CMAKE_SOURCE_DIR}/test/testcases/${CONFIG}.in
		-o ${PROJECT_BINARY_DIR}/test/${CONFIG}.out
		-c ${CMAKE_SOURCE_DIR}/data/config/${CONFIG}.ini
	)
	
	add_test(
		${CONFIG}_compare
		diff
		${PROJECT_BINARY_DIR}/test/${CONFIG}.out
		${CMAKE_SOURCE_DIR}/test/testcases/${CONFIG}.ans
	)
	
	set_property(
		TEST ${CONFIG}_compare
		APPEND PROPERTY
			DEPENDS ${CONFIG}_convert)

endforeach(CONFIG)