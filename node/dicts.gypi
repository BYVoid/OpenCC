{
  "targets": [{
    "target_name": "dicts",
    "type": "none",
    "variables": {
      "cmd": "<(module_root_dir)/node/dict.js",
      "dict_merge": "<(module_root_dir)/data/scripts/merge.py",
      "dict_reverse": "<(module_root_dir)/data/scripts/reverse.py",
      "input_prefix": "<(module_root_dir)/data/dictionary/",
      "output_prefix": "<(PRODUCT_DIR)/"
    },
    "actions": [{
      "action_name": "STCharacters",
      "variables": {
        "input": "<(input_prefix)STCharacters.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)STCharacters.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "STPhrases",
      "variables": {
        "input": "<(input_prefix)STPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)STPhrases.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TSCharacters",
      "variables": {
        "input": "<(input_prefix)TSCharacters.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TSCharacters.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TSPhrases",
      "variables": {
        "input": "<(input_prefix)TSPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TSPhrases.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariants",
      "variables": {
        "input": "<(input_prefix)TWVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariants.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariantsRevPhrases",
      "variables": {
        "input": "<(input_prefix)TWVariantsRevPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariantsRevPhrases.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "JPVariants",
      "variables": {
        "input": "<(input_prefix)JPVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)JPVariants.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWPhrases.txt",
      "inputs": ["<(cmd)"],
      "outputs": ["<(output_prefix)TWPhrases.txt"],
      "action": ["python", "<(dict_merge)", "<(input_prefix)TWPhrasesIT.txt", "<(input_prefix)TWPhrasesName.txt", "<(input_prefix)TWPhrasesOther.txt", "<@(_outputs)"]
    }, {
      "action_name": "TWVariantsRev.txt",
      "variables": {
        "input": "<(input_prefix)TWVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariantsRev.txt"],
      "action": ["python", "<(dict_reverse)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWPhrasesRev.txt",
      "variables": {
        "input": "<(output_prefix)TWPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWPhrasesRev.txt"],
      "action": ["python", "<(dict_reverse)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWPhrases",
      "variables": {
        "input": "<(output_prefix)TWPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWPhrases.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariantsRev",
      "variables": {
        "input": "<(output_prefix)TWVariantsRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariantsRev.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWPhrasesRev",
      "variables": {
        "input": "<(output_prefix)TWPhrasesRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWPhrasesRev.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariants",
      "variables": {
        "input": "<(input_prefix)HKVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariants.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariantsPhrases",
      "variables": {
        "input": "<(input_prefix)HKVariantsPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariantsPhrases.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariantsRevPhrases",
      "variables": {
        "input": "<(input_prefix)HKVariantsRevPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariantsRevPhrases.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariantsRev.txt",
      "variables": {
        "input": "<(input_prefix)HKVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariantsRev.txt"],
      "action": ["python", "<(dict_reverse)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVariantsRev",
      "variables": {
        "input": "<(output_prefix)HKVariantsRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVariantsRev.ocd"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }],
    "dependencies": [
      "opencc"
    ]
  }]
}
