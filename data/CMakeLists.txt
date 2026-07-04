if(DEFINED PYTHON_EXECUTABLE AND NOT DEFINED Python3_EXECUTABLE)
  set(Python3_EXECUTABLE "${PYTHON_EXECUTABLE}")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(OPENCC_DICT_BIN opencc_dict)
set(OPENCC_BIN opencc)
set(DICT_REVERSE_BIN "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/reverse.py")
set(DICT_EXTRACT_TOFU_RISK_BIN "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/extract_tofu_risk.py")
set(DICT_GENERATE_ST_PHRASES_BIN "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_st_phrases_from_regional_phrases.py")
set(CONFIG_CONVERT_DICT_FORMAT_BIN "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/convert_config_dict_format.py")
set(DICT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dictionary)
set(DICT_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(
  DICTS_RAW
  CJK_Compatibility_Ideographs
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
  DICTS_GENERATED
  TSCharactersExt
  STPhrases_GeneratedFromRegionalPhrases
  TWVariantsRev
  HKVariantsRev
  JPShinjitaiCharactersRev
)

set(DICTS ${DICTS_RAW} ${DICTS_GENERATED})

foreach(DICT ${DICTS_RAW})
  set(DICT_${DICT}_INPUT ${DICT_DIR}/${DICT}.txt)
endforeach(DICT)

foreach(DICT ${DICTS_GENERATED})
  set(DICT_${DICT}_INPUT ${DICT_GENERATED_DIR}/${DICT}.txt)
endforeach(DICT)

# In "text" format, the Dictionaries target depends on the .txt dictionaries
# (copied into the build tree alongside the generated ones, so the build
# tree stays self-contained and runnable like the ocd2 build tree) instead of
# building/installing compiled .ocd2/.ocd dictionaries. STPhrases_GeneratedFrom
# RegionalPhrases still needs a handful of .ocd2 dictionaries to bootstrap
# itself (see below) regardless of this setting; those are built on demand
# since they remain explicit command dependencies.
if(OPENCC_DICT_FORMAT STREQUAL "text")
  foreach(DICT ${DICTS})
    set(DICT_TARGETS ${DICT_TARGETS} ${DICT}.txt)
  endforeach(DICT)
else()
  foreach(DICT ${DICTS})
    set(DICT_TARGETS ${DICT_TARGETS} ${DICT}.${OPENCC_DICT_FORMAT})
  endforeach(DICT)
endif()

add_custom_target(
  Dictionaries
  ALL
  DEPENDS
    ${DICT_TARGETS}
)

set(
  DICT_TSCharactersExt_GENERATING_INPUT
  ${DICT_DIR}/TSCharacters.txt
)
set(
  DICT_TSCharactersExt_GENERATING_COMMAND
  ${DICT_EXTRACT_TOFU_RISK_BIN} ${DICT_TSCharactersExt_GENERATING_INPUT} TSCharactersExt.txt
)

set(
  DICT_STPhrases_GeneratedFromRegionalPhrases_GENERATING_INPUT
  ${DICT_DIR}/HKPhrases.txt
  ${DICT_DIR}/TWPhrases.txt
  ${DICT_GENERATED_DIR}/CJK_Compatibility_Ideographs.ocd2
  ${DICT_GENERATED_DIR}/TSCharacters.ocd2
  ${DICT_GENERATED_DIR}/TSCharactersExt.ocd2
  ${DICT_GENERATED_DIR}/TSPhrases.ocd2
  ${CMAKE_CURRENT_SOURCE_DIR}/config/t2s.json
)
set(
  DICT_STPhrases_GeneratedFromRegionalPhrases_GENERATING_COMMAND
  ${DICT_GENERATE_ST_PHRASES_BIN}
    --input ${DICT_DIR}/HKPhrases.txt
    --input ${DICT_DIR}/TWPhrases.txt
    --output STPhrases_GeneratedFromRegionalPhrases.txt
    --opencc $<TARGET_FILE:${OPENCC_BIN}>
    --config ${CMAKE_CURRENT_SOURCE_DIR}/config/t2s.json
    --dict-dir ${DICT_GENERATED_DIR}
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

foreach(DICT ${DICTS_GENERATED})
  add_custom_command(
    OUTPUT
      ${DICT}.txt
    COMMENT
      "Generating ${DICT}.txt"
    COMMAND
      ${DICT_${DICT}_GENERATING_COMMAND}
    DEPENDS
      ${DICT_WIN32_DEPENDS}
      ${OPENCC_BIN}
      ${DICT_${DICT}_GENERATING_INPUT}
  )
  set_directory_properties(
    PROPERTIES
      ADDITIONAL_MAKE_CLEAN_FILES
        "${DICT_GENERATED_DIR}/${DICT}.txt"
  )
endforeach(DICT)

if(OPENCC_DICT_FORMAT STREQUAL "text")
  foreach(DICT ${DICTS_RAW})
    add_custom_command(
      OUTPUT
        ${DICT}.txt
      COMMENT
        "Copying ${DICT}.txt"
      COMMAND
        ${CMAKE_COMMAND} -E copy ${DICT_${DICT}_INPUT} ${DICT_GENERATED_DIR}/${DICT}.txt
      DEPENDS
        ${DICT_${DICT}_INPUT}
    )
    set_directory_properties(
      PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES
          "${DICT_GENERATED_DIR}/${DICT}.txt"
    )
  endforeach(DICT)
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

  if(OPENCC_ENABLE_INSTALL AND OPENCC_DICT_FORMAT STREQUAL "ocd2")
  install(
    FILES
      ${DICT_GENERATED_DIR}/${DICT}.ocd2
    DESTINATION
      ${DIR_SHARE_OPENCC}
  )
  endif()

  if(OPENCC_ENABLE_INSTALL AND OPENCC_DICT_FORMAT STREQUAL "text")
  install(
    FILES
      ${DICT_GENERATED_DIR}/${DICT}.txt
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

if(OPENCC_DICT_FORMAT STREQUAL "ocd")
  foreach(DICT ${DICTS})
    add_custom_command(
      OUTPUT
        ${DICT}.ocd
      COMMENT
        "Building ${DICT}.ocd"
      COMMAND
        ${OPENCC_DICT_BIN}
          --input ${DICT_${DICT}_INPUT}
          --output ${DICT}.ocd
          --from text
          --to ocd
      DEPENDS
        ${DICT_WIN32_DEPENDS}
        ${OPENCC_DICT_BIN}
        ${DICT_${DICT}_INPUT}
    )

    if(OPENCC_ENABLE_INSTALL)
    install(
      FILES
        ${DICT_GENERATED_DIR}/${DICT}.ocd
      DESTINATION
        ${DIR_SHARE_OPENCC}
    )
    endif()

    set_directory_properties(
      PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES
          "${DICT_GENERATED_DIR}/${DICT}.ocd"
    )
  endforeach(DICT)
endif()

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

if(NOT OPENCC_DICT_FORMAT STREQUAL "ocd2")
  set(CONFIG_CONVERTED_DIR ${CMAKE_CURRENT_BINARY_DIR}/config)
  file(MAKE_DIRECTORY ${CONFIG_CONVERTED_DIR})

  foreach(CONFIG_FILE ${CONFIG_FILES})
    get_filename_component(CONFIG_FILE_NAME ${CONFIG_FILE} NAME)
    add_custom_command(
      OUTPUT
        ${CONFIG_CONVERTED_DIR}/${CONFIG_FILE_NAME}
      COMMENT
        "Converting ${CONFIG_FILE_NAME} to reference ${OPENCC_DICT_FORMAT} dictionaries"
      COMMAND
        ${CONFIG_CONVERT_DICT_FORMAT_BIN}
          ${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_FILE}
          ${CONFIG_CONVERTED_DIR}/${CONFIG_FILE_NAME}
          ${OPENCC_DICT_FORMAT}
      DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_FILE}
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/convert_config_dict_format.py
    )
    set(CONFIG_CONVERTED_FILES ${CONFIG_CONVERTED_FILES} ${CONFIG_CONVERTED_DIR}/${CONFIG_FILE_NAME})
  endforeach(CONFIG_FILE)

  add_custom_target(
    ConvertedConfigs
    ALL
    DEPENDS
      ${CONFIG_CONVERTED_FILES}
  )

  set_directory_properties(
    PROPERTIES
      ADDITIONAL_MAKE_CLEAN_FILES
        "${CONFIG_CONVERTED_FILES}"
  )

  if(OPENCC_ENABLE_INSTALL)
  install(
    FILES
      ${CONFIG_CONVERTED_FILES}
    DESTINATION
      ${DIR_SHARE_OPENCC}
  )
  endif()
else()
  if(OPENCC_ENABLE_INSTALL)
  install(
    FILES
      ${CONFIG_FILES}
    DESTINATION
      ${DIR_SHARE_OPENCC}
  )
  endif()
endif()
