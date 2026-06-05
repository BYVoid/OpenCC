find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(OPENCC_DICT_BIN opencc_dict)
set(DICT_REVERSE_BIN "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/reverse.py")
set(DICT_TOFU_BIN "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/tofu.py")
set(DICT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dictionary)
set(DICT_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(SCHEME_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scheme)

set(
  DICTS_RAW
  STCharacters
  STPhrases
  TSCharacters
  TSPhrases
  TWPhrases
  TWPhrasesRev
  TWVariantsPhrases
  TWVariants
  TWVariantsRevPhrases
  HKVariantsPhrases
  HKVariants
  HKVariantsRevPhrases
  HKPhrases
  HKPhrasesRev
  JPShinjitaiCharacters
  JPShinjitaiPhrases
)

set(
  DICTS_GENERATED_REV
  TWVariantsRev
  HKVariantsRev
  JPShinjitaiCharactersRev
)

set(
  DICTS_GENERATED_TOFU
  TSCharactersBase
  TSCharactersExt
  TSPhrasesBase
  TSPhrasesExt
)

set(DICTS_GENERATED ${DICTS_GENERATED_REV} ${DICTS_GENERATED_TOFU})

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
  SCHEME_ALLOWED_SMP
  ${SCHEME_DIR}/AllowedSmpChars.txt
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
  DICT_JPShinjitaiCharactersRev_GENERATING_INPUT
  ${DICT_DIR}/JPShinjitaiCharacters.txt
)
set(
  DICT_JPShinjitaiCharactersRev_GENERATING_COMMAND
  ${DICT_REVERSE_BIN} ${DICT_JPShinjitaiCharactersRev_GENERATING_INPUT} JPShinjitaiCharactersRev.txt
)

foreach(DICT ${DICTS_GENERATED_REV})
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

add_custom_command(
  OUTPUT
    "${DICT_GENERATED_DIR}/TSCharactersBase.txt"
    "${DICT_GENERATED_DIR}/TSCharactersExt.txt"
  COMMENT
    "Generating TSCharactersBase.txt and TSCharactersExt.txt"
  COMMAND
    ${DICT_TOFU_BIN}
    "${DICT_DIR}/TSCharacters.txt"
    "${SCHEME_ALLOWED_SMP}"
    "${DICT_GENERATED_DIR}/TSCharactersBase.txt"
    "${DICT_GENERATED_DIR}/TSCharactersExt.txt"
  DEPENDS
    "${DICT_DIR}/TSCharacters.txt"
    "${SCHEME_ALLOWED_SMP}"
)

add_custom_command(
  OUTPUT
    "${DICT_GENERATED_DIR}/TSPhrasesBase.txt"
    "${DICT_GENERATED_DIR}/TSPhrasesExt.txt"
  COMMENT
    "Generating TSPhrasesBase.txt and TSPhrasesExt.txt"
  COMMAND
    ${DICT_TOFU_BIN}
    "${DICT_DIR}/TSPhrases.txt"
    "${SCHEME_ALLOWED_SMP}"
    "${DICT_GENERATED_DIR}/TSPhrasesBase.txt"
    "${DICT_GENERATED_DIR}/TSPhrasesExt.txt"
  DEPENDS
    "${DICT_DIR}/TSPhrases.txt"
    "${SCHEME_ALLOWED_SMP}"
)

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

  if(OPENCC_ENABLE_INSTALL)
  install(
    FILES
      ${DICT_GENERATED_DIR}/${DICT}.ocd2
    DESTINATION
      ${DIR_SHARE_OPENCC}
  )
  endif()

  set_directory_properties(
    PROPERTIES
      ADDITIONAL_MAKE_CLEAN_FILES
        "${DICT_GENERATED_DIR}/${DICT}.ocd2"
  )
endforeach(DICT)

set(CONFIG_FILES
  config/hk2s.json
  config/hk2sp.json
  config/hk2t.json
  config/jp2t.json
  config/s2hk.json
  config/s2hkp.json
  config/s2t.json
  config/s2tw.json
  config/s2twp.json
  config/t2hk.json
  config/t2jp.json
  config/t2s.json
  config/t2tw.json
  config/tw2s.json
  config/tw2sp.json
  config/tw2t.json
)

if(OPENCC_ENABLE_INSTALL)
install(
  FILES
    ${CONFIG_FILES}
  DESTINATION
    ${DIR_SHARE_OPENCC}
)
endif()
