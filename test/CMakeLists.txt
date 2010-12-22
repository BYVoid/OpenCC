set(CONFIGURATIONS zhs2zht zht2zhs mix2zht mix2zhs)

foreach(CONFIG ${CONFIGURATIONS})

	add_test(
		${CONFIG}_convert
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