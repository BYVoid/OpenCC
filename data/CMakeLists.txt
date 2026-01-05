find_package(PythonInterp REQUIRED)

set(OPENCC_DICT_BIN opencc_dict)
set(DICT_REVERSE_BIN "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/reverse.py")
set(DICT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dictionary)
set(DICT_CNGOV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dictionary/cngov)
set(DICT_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(
  DICTS_RAW
  STCharacters
  STPhrases
  TSCharacters
  TSPhrases
  TWPhrases
  TWPhrasesRev
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
  TWVariantsRev
  HKVariantsRev
  JPVariantsRev
)

# ==============================================================================
# CN Government Standard Conversion Dictionaries (Third-party)
# Copyright 2024 TerryTian-tech
# Source: https://github.com/TerryTian-tech/OpenCC-Traditional-Chinese-characters-according-to-Chinese-government-standards
# License: Apache License 2.0
# Reference: 《通用规范汉字表》(2013)
# ==============================================================================
set(
  DICTS_CNGOV
  TGCharacters
  TGCharacters_keep_simp
  TGPhrases
)

set(DICTS ${DICTS_RAW} ${DICTS_GENERATED} ${DICTS_CNGOV})

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

foreach(DICT ${DICTS_CNGOV})
  set(DICT_${DICT}_INPUT ${DICT_CNGOV_DIR}/${DICT}.txt)
endforeach(DICT)

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

# Create cngov subdirectory and copy files for testing
add_custom_command(
  TARGET Dictionaries POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/cngov
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_BINARY_DIR}/TGCharacters.ocd2
          ${CMAKE_CURRENT_BINARY_DIR}/cngov/TGCharacters.ocd2
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_BINARY_DIR}/TGCharacters_keep_simp.ocd2
          ${CMAKE_CURRENT_BINARY_DIR}/cngov/TGCharacters_keep_simp.ocd2
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_BINARY_DIR}/TGPhrases.ocd2
          ${CMAKE_CURRENT_BINARY_DIR}/cngov/TGPhrases.ocd2
  COMMENT "Copying cngov dictionaries to subdirectory for testing"
)

set(CONFIG_FILES
  config/hk2s.json
  config/hk2t.json
  config/jp2t.json
  config/s2hk.json
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
  config/t2cngov.json
  config/t2cngov_keep_simp.json
)

install(
  FILES
    ${CONFIG_FILES}
  DESTINATION
    ${DIR_SHARE_OPENCC}
)

# Install cngov dictionaries to subdirectory
foreach(DICT ${DICTS_CNGOV})
  install(
    FILES
      ${DICT_GENERATED_DIR}/${DICT}.ocd2
    DESTINATION
      ${DIR_SHARE_OPENCC}/cngov
  )
endforeach(DICT)
