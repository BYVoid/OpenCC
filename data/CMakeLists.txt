set(
  OPENCC_DICT_BIN
  ${CMAKE_BINARY_DIR}/src/opencc_dict
)

add_custom_target(
  Dictionaries
  ALL
  DEPENDS
    STCharacters.ocd
    STPhrases.ocd
    TSCharacters.ocd
    TSPhrases.ocd
    TWVariants.ocd
)

set(
  DICTS_RAW
  STCharacters
  STPhrases
  TSCharacters
  TSPhrases
  TWVariants
)

foreach(DICT ${DICTS_RAW})
  set(ENV{DICT_${DICT}_INPUT} ${CMAKE_SOURCE_DIR}/data/dictionary/${DICT}.txt)
endforeach(DICT)

set(DICTS ${DICTS_RAW})

foreach(DICT ${DICTS})
  add_custom_command(
    OUTPUT
      ${DICT}.ocd
    COMMENT
      "Building ${DICT}.ocd"
    COMMAND
      ${OPENCC_DICT_BIN}
        --input $ENV{DICT_${DICT}_INPUT}
        --output ${DICT}.ocd
        --from text
        --to ocd
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
endforeach(DICT)

set(CONFIG_FILES
  config/s2t.json
  config/t2s.json
  config/s2tw.json
)

install(
  FILES
    ${CONFIG_FILES}
  DESTINATION
    ${DIR_SHARE_OPENCC}
)

if (BUILD_TESTING)
  foreach (DICT ${TAIWAN_DICT_FILES} ${CHINA_DICT_FILES})
    configure_file(${DICT} ${PROJECT_BINARY_DIR}/data COPYONLY)
  endforeach (DICT)
endif (BUILD_TESTING)
