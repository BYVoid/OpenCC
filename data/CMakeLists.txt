set(OPENCC_DICT_BIN ${CMAKE_BINARY_DIR}/src/opencc_dict)
set(DICT_MERGE_BIN python ${CMAKE_SOURCE_DIR}/data/scripts/merge.py)
set(DICT_DIR ${CMAKE_SOURCE_DIR}/data/dictionary)
set(DICT_GENERATED_DIR ${CMAKE_BINARY_DIR}/data)

add_custom_target(
  Dictionaries
  ALL
  DEPENDS
    STCharacters.ocd
    STPhrases.ocd
    TSCharacters.ocd
    TSPhrases.ocd
    TWVariants.ocd
    TWPhrases.ocd
)

set(
  DICTS_RAW
  STCharacters
  STPhrases
  TSCharacters
  TSPhrases
  TWVariants
)

add_custom_command(
  OUTPUT
    TWPhrases.txt
  COMMENT
    "Generating TWPhrases.txt"
  COMMAND
    ${DICT_MERGE_BIN}
      ${DICT_DIR}/TWPhrasesIT.txt
      ${DICT_DIR}/TWPhrasesName.txt
      ${DICT_DIR}/TWPhrasesOther.txt
      TWPhrases.txt
)
set_directory_properties(
  PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES
      "${CMAKE_BINARY_DIR}/data/TWPhrases.txt"
)

set(
  DICTS_GENERATED
  TWPhrases
)

foreach(DICT ${DICTS_RAW})
  set(ENV{DICT_${DICT}_INPUT} ${DICT_DIR}/${DICT}.txt)
endforeach(DICT)

foreach(DICT ${DICTS_GENERATED})
  set(ENV{DICT_${DICT}_INPUT} ${DICT_GENERATED_DIR}/${DICT}.txt)
endforeach(DICT)

set(DICTS ${DICTS_RAW} ${DICTS_GENERATED})

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
    DEPENDS
      opencc_dict
      $ENV{DICT_${DICT}_INPUT}
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
  config/s2twp.json
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
