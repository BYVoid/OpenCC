find_package(PythonInterp REQUIRED)

set(OPENCC_DICT_BIN opencc_dict)
set(DICT_MERGE_BIN "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/merge.py")
set(DICT_REVERSE_BIN "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/reverse.py")
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
  HKVariants
  HKVariantsRevPhrases
  JPVariants
  JPShinjitaiCharacters
  JPShinjitaiPhrases
)

set(
  DICTS_GENERATED
  CNVocabulary
  HKVocabulary
  MOVocabulary
  MYVocabulary
  SGVocabulary
  TWVocabulary
  TWVariantsRev
  HKVariantsRev
  JPVariantsRev
)

set(DICTS ${DICTS_RAW} ${DICTS_GENERATED})

foreach(DICT ${DICTS})
  set(DICT_TARGETS ${DICT_TARGETS} ${DICT}.ocd2)
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
  DICT_CNVocabulary_GENERATING_INPUT
  ${DICT_DIR}/CNVocabSciTech.txt
  ${DICT_DIR}/CNVocabToponym.txt
  ${DICT_DIR}/CNVocabOthers.txt
)
set(
  DICT_CNVocabulary_GENERATING_COMMAND
  ${DICT_MERGE_BIN} ${DICT_CNVocabulary_GENERATING_INPUT} CNVocabulary.txt
)

set(
  DICT_HKVocabulary_GENERATING_INPUT
  ${DICT_DIR}/HKVocabSciTech.txt
  ${DICT_DIR}/HKVocabToponym.txt
  ${DICT_DIR}/HKVocabOthers.txt
)
set(
  DICT_HKVocabulary_GENERATING_COMMAND
  ${DICT_MERGE_BIN} ${DICT_HKVocabulary_GENERATING_INPUT} HKVocabulary.txt
)

set(
  DICT_MOVocabulary_GENERATING_INPUT
  ${DICT_DIR}/MOVocabSciTech.txt
  ${DICT_DIR}/MOVocabToponym.txt
  ${DICT_DIR}/MOVocabOthers.txt
)
set(
  DICT_MOVocabulary_GENERATING_COMMAND
  ${DICT_MERGE_BIN} ${DICT_MOVocabulary_GENERATING_INPUT} MOVocabulary.txt
)

set(
  DICT_MYVocabulary_GENERATING_INPUT
  ${DICT_DIR}/MYVocabSciTech.txt
  ${DICT_DIR}/MYVocabToponym.txt
  ${DICT_DIR}/MYVocabOthers.txt
)
set(
  DICT_MYVocabulary_GENERATING_COMMAND
  ${DICT_MERGE_BIN} ${DICT_MYVocabulary_GENERATING_INPUT} MYVocabulary.txt
)

set(
  DICT_SGVocabulary_GENERATING_INPUT
  ${DICT_DIR}/SGVocabSciTech.txt
  ${DICT_DIR}/SGVocabToponym.txt
  ${DICT_DIR}/SGVocabOthers.txt
)
set(
  DICT_SGVocabulary_GENERATING_COMMAND
  ${DICT_MERGE_BIN} ${DICT_SGVocabulary_GENERATING_INPUT} SGVocabulary.txt
)

set(
  DICT_TWVocabulary_GENERATING_INPUT
  ${DICT_DIR}/TWVocabSciTech.txt
  ${DICT_DIR}/TWVocabToponym.txt
  ${DICT_DIR}/TWVocabOthers.txt
)
set(
  DICT_TWVocabulary_GENERATING_COMMAND
  ${DICT_MERGE_BIN} ${DICT_TWVocabulary_GENERATING_INPUT} TWVocabulary.txt
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
  DICT_HKVariantsRev_GENERATING_INPUT
  ${DICT_DIR}/HKVariants.txt
)
set(
  DICT_HKVariantsRev_GENERATING_COMMAND
  ${DICT_REVERSE_BIN} ${DICT_HKVariantsRev_GENERATING_INPUT} HKVariantsRev.txt
)

set(
  DICT_JPVariantsRev_GENERATING_INPUT
  ${DICT_DIR}/JPVariants.txt
)
set(
  DICT_JPVariantsRev_GENERATING_COMMAND
  ${DICT_REVERSE_BIN} ${DICT_JPVariantsRev_GENERATING_INPUT} JPVariantsRev.txt
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

add_custom_target(
  copy_libopencc_to_dir_of_opencc_dict
  COMMENT
    "Copying libopencc to directory of opencc_dict"
  COMMAND
    ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:libopencc>" "$<TARGET_FILE_DIR:${OPENCC_DICT_BIN}>"
)
if (WIN32)
  set(DICT_WIN32_DEPENDS copy_libopencc_to_dir_of_opencc_dict)
else()
  set(DICT_WIN32_DEPENDS)
endif()

foreach(DICT ${DICTS})
  add_custom_command(
    OUTPUT
      ${DICT}.ocd2
    COMMENT
      "Building ${DICT}.ocd2"
    COMMAND
      ${OPENCC_DICT_BIN}
        --input ${DICT_${DICT}_INPUT}
        --output ${DICT}.ocd2
        --from text
        --to ocd2
    DEPENDS
      ${DICT_WIN32_DEPENDS}
      ${OPENCC_DICT_BIN}
      ${DICT_${DICT}_INPUT}
  )

  install(
    FILES
      ${DICT_GENERATED_DIR}/${DICT}.ocd2
    DESTINATION
      ${DIR_SHARE_OPENCC}
  )

  set_directory_properties(
    PROPERTIES
      ADDITIONAL_MAKE_CLEAN_FILES
        "${DICT_GENERATED_DIR}/${DICT}.ocd2"
  )
endforeach(DICT)

set(CONFIG_FILES
  config/hk2cnp.json
  config/hk2myp.json
  config/hk2s.json
  config/hk2sgp.json
  config/hk2t.json
  config/jp2t.json
  config/s2cnp.json
  config/s2hk.json
  config/s2hkp.json
  config/s2mop.json
  config/s2myp.json
  config/s2sgp.json
  config/s2t.json
  config/s2tw.json
  config/s2twp.json
  config/t2cnp.json
  config/t2hk.json
  config/t2hkp.json
  config/t2jp.json
  config/t2mop.json
  config/t2myp.json
  config/t2s.json
  config/t2sgp.json
  config/t2tw.json
  config/t2twp.json
  config/tw2cnp.json
  config/tw2myp.json
  config/tw2s.json
  config/tw2sgp.json
  config/tw2t.json
)

install(
  FILES
    ${CONFIG_FILES}
  DESTINATION
    ${DIR_SHARE_OPENCC}
)
