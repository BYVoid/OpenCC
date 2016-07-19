set(OPENCC_DICT_BIN opencc_dict)
set(DICT_MERGE_BIN python ${CMAKE_CURRENT_SOURCE_DIR}/scripts/merge.py)
set(DICT_REVERSE_BIN python ${CMAKE_CURRENT_SOURCE_DIR}/scripts/reverse.py)
set(DICT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dictionary)
set(DICT_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(
  DICTS_RAW
  STCharacters
  STPhrases
  TSCharacters
  TSPhrases
  TWVariants
  TWVariantsRevPhrases
  JPVariants
  HKVariants
  HKVariantsPhrases
  HKVariantsRevPhrases
)

set(
  DICTS_GENERATED
  TWPhrases
  TWPhrasesRev
  TWVariantsRev
  HKVariantsRev
)

set(DICTS ${DICTS_RAW} ${DICTS_GENERATED})

foreach(DICT ${DICTS})
  set(DICT_TARGETS ${DICT_TARGETS} ${DICT}.ocd)
endforeach(DICT)

add_custom_target(
  Dictionaries
  ALL
  DEPENDS
    ${DICT_TARGETS}
)

foreach(DICT ${DICTS_RAW})
  set(DICT_${DICT}_INPUT ${DICT_DIR}/${DICT}.txt)
endforeach(DICT)

foreach(DICT ${DICTS_GENERATED})
  set(DICT_${DICT}_INPUT ${DICT_GENERATED_DIR}/${DICT}.txt)
endforeach(DICT)

set(
  DICT_TWPhrases_GENERATING_INPUT
  ${DICT_DIR}/TWPhrasesIT.txt
  ${DICT_DIR}/TWPhrasesName.txt
  ${DICT_DIR}/TWPhrasesOther.txt
)
set(
  DICT_TWPhrases_GENERATING_COMMAND
  ${DICT_MERGE_BIN} ${DICT_TWPhrases_GENERATING_INPUT} TWPhrases.txt
)

set(
  DICT_TWVariantsRev_GENERATING_INPUT
  ${DICT_DIR}/TWVariants.txt
)
set(
  DICT_TWVariantsRev_GENERATING_COMMAND
  ${DICT_REVERSE_BIN} ${DICT_TWVariantsRev_GENERATING_INPUT} TWVariantsRev.txt
)

set(
  DICT_TWPhrasesRev_GENERATING_INPUT
  ${DICT_GENERATED_DIR}/TWPhrases.txt
)
set(
  DICT_TWPhrasesRev_GENERATING_COMMAND
  ${DICT_REVERSE_BIN} ${DICT_TWPhrasesRev_GENERATING_INPUT} TWPhrasesRev.txt
)

set(
  DICT_HKVariantsRev_GENERATING_INPUT
  ${DICT_DIR}/HKVariants.txt
)
set(
  DICT_HKVariantsRev_GENERATING_COMMAND
  ${DICT_REVERSE_BIN} ${DICT_HKVariantsRev_GENERATING_INPUT} HKVariantsRev.txt
)

foreach(DICT ${DICTS_GENERATED})
  add_custom_command(
    OUTPUT
      ${DICT}.txt
    COMMENT
      "Generating ${DICT}.txt"
    COMMAND
      ${DICT_${DICT}_GENERATING_COMMAND}
    DEPENDS
      ${DICT_${DICT}_GENERATING_INPUT}
  )
  set_directory_properties(
    PROPERTIES
      ADDITIONAL_MAKE_CLEAN_FILES
        "${DICT_GENERATED_DIR}/${DICT}.txt"
  )
endforeach(DICT)

foreach(DICT ${DICTS})
  add_custom_command(
    OUTPUT
      ${DICT}.ocd
    COMMENT
      "Building ${DICT}.ocd"
    COMMAND
      ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:libopencc>" "$<TARGET_FILE_DIR:${OPENCC_DICT_BIN}>"
    COMMAND
      ${OPENCC_DICT_BIN}
        --input ${DICT_${DICT}_INPUT}
        --output ${DICT}.ocd
        --from text
        --to ocd
    DEPENDS
      ${OPENCC_DICT_BIN}
      ${DICT_${DICT}_INPUT}
  )

  install(
    FILES
      ${DICT_GENERATED_DIR}/${DICT}.ocd
    DESTINATION
      ${DIR_SHARE_OPENCC}
  )

  set_directory_properties(
    PROPERTIES
      ADDITIONAL_MAKE_CLEAN_FILES
        "${DICT_GENERATED_DIR}/${DICT}.ocd"
  )
endforeach(DICT)

set(CONFIG_FILES
  config/s2t.json
  config/t2s.json
  config/s2tw.json
  config/s2twp.json
  config/tw2s.json
  config/tw2sp.json
  config/t2tw.json
  config/s2hk.json
  config/hk2s.json
  config/t2hk.json
)

install(
  FILES
    ${CONFIG_FILES}
  DESTINATION
    ${DIR_SHARE_OPENCC}
)
