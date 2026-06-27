{
  "targets": [{
    "target_name": "dicts",
    "type": "none",
    "variables": {
      "cmd": "<(module_root_dir)/node/dict.js",
      "dict_extract_tofu_risk": "<(module_root_dir)/data/scripts/extract_tofu_risk.py",
      "dict_generate_st_phrases": "<(module_root_dir)/data/scripts/generate_st_phrases_from_regional_phrases.py",
      "dict_reverse": "<(module_root_dir)/data/scripts/reverse.py",
      "input_prefix": "<(module_root_dir)/data/dictionary/",
      "output_prefix": "<(PRODUCT_DIR)/",
      "python_cmd": "python3"
    },
    "conditions": [
      [
        "OS==\"win\"",
        {
          "variables": {
            "python_cmd": "python.exe"
          }
        }
      ]
    ],
    "actions": [{
      "action_name": "STCharacters",
      "variables": {
        "input": "<(input_prefix)STCharacters.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)STCharacters.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "STPhrases",
      "variables": {
        "input": "<(input_prefix)STPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)STPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TSCharacters",
      "variables": {
        "input": "<(input_prefix)TSCharacters.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TSCharacters.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TSCharactersExt.txt",
      "variables": {
        "input": "<(input_prefix)TSCharacters.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TSCharactersExt.txt"],
      "action": ["<(python_cmd)", "<(dict_extract_tofu_risk)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TSCharactersExt",
      "variables": {
        "input": "<(output_prefix)TSCharactersExt.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TSCharactersExt.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TSPhrases",
      "variables": {
        "input": "<(input_prefix)TSPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TSPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "CJK_Compatibility_Ideographs",
      "variables": {
        "input": "<(input_prefix)CJK_Compatibility_Ideographs.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)CJK_Compatibility_Ideographs.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "STPhrases_GeneratedFromRegionalPhrases.txt",
      "inputs": [
        "<(dict_generate_st_phrases)",
        "<(input_prefix)HKPhrases.txt",
        "<(input_prefix)TWPhrases.txt",
        "<(module_root_dir)/data/config/t2s.json",
        "<(output_prefix)CJK_Compatibility_Ideographs.ocd2",
        "<(output_prefix)TSCharacters.ocd2",
        "<(output_prefix)TSCharactersExt.ocd2",
        "<(output_prefix)TSPhrases.ocd2",
        "<(output_prefix)opencc.node"
      ],
      "outputs": ["<(output_prefix)STPhrases_GeneratedFromRegionalPhrases.txt"],
      "action": [
        "<(python_cmd)",
        "<(dict_generate_st_phrases)",
        "--input",
        "<(input_prefix)HKPhrases.txt",
        "--input",
        "<(input_prefix)TWPhrases.txt",
        "--output",
        "<@(_outputs)",
        "--node-binding",
        "<(output_prefix)opencc.node",
        "--config",
        "<(module_root_dir)/data/config/t2s.json",
        "--dict-dir",
        "<(PRODUCT_DIR)"
      ]
    }, {
      "action_name": "STPhrases_GeneratedFromRegionalPhrases",
      "variables": {
        "input": "<(output_prefix)STPhrases_GeneratedFromRegionalPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)STPhrases_GeneratedFromRegionalPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariantsPhrases",
      "variables": {
        "input": "<(input_prefix)TWVariantsPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariantsPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariants",
      "variables": {
        "input": "<(input_prefix)TWVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariants.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariantsRevPhrases",
      "variables": {
        "input": "<(input_prefix)TWVariantsRevPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariantsRevPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "JPShinjitaiCharactersRev.txt",
      "variables": {
        "input": "<(input_prefix)JPShinjitaiCharacters.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)JPShinjitaiCharactersRev.txt"],
      "action": ["<(python_cmd)", "<(dict_reverse)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "JPShinjitaiCharactersRev",
      "variables": {
        "input": "<(output_prefix)JPShinjitaiCharactersRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)JPShinjitaiCharactersRev.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariantsRev.txt",
      "variables": {
        "input": "<(input_prefix)TWVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariantsRev.txt"],
      "action": ["<(python_cmd)", "<(dict_reverse)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWPhrases",
      "variables": {
        "input": "<(input_prefix)TWPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariantsRev",
      "variables": {
        "input": "<(output_prefix)TWVariantsRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariantsRev.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWPhrasesRev",
      "variables": {
        "input": "<(input_prefix)TWPhrasesRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWPhrasesRev.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariantsPhrases",
      "variables": {
        "input": "<(input_prefix)HKVariantsPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariantsPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariants",
      "variables": {
        "input": "<(input_prefix)HKVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariants.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariantsRevPhrases",
      "variables": {
        "input": "<(input_prefix)HKVariantsRevPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariantsRevPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKPhrases",
      "variables": {
        "input": "<(input_prefix)HKPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariantsRev.txt",
      "variables": {
        "input": "<(input_prefix)HKVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariantsRev.txt"],
      "action": ["<(python_cmd)", "<(dict_reverse)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariantsRev",
      "variables": {
        "input": "<(output_prefix)HKVariantsRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariantsRev.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKPhrasesRev",
      "variables": {
        "input": "<(input_prefix)HKPhrasesRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKPhrasesRev.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "JPShinjitaiCharacters",
      "variables": {
        "input": "<(input_prefix)JPShinjitaiCharacters.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)JPShinjitaiCharacters.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "JPShinjitaiPhrases",
      "variables": {
        "input": "<(input_prefix)JPShinjitaiPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)JPShinjitaiPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }],
    "dependencies": [
      "opencc"
    ]
  }]
}
