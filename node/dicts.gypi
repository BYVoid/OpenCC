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
      "action_name": "TSPhrases",
      "variables": {
        "input": "<(input_prefix)TSPhrases.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TSPhrases.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "CNVocabulary.txt",
      "inputs": ["<(cmd)"],
      "outputs": ["<(output_prefix)CNVocabulary.txt"],
      "action": ["python", "<(dict_merge)", "<(input_prefix)CNVocabToponym.txt", "<(input_prefix)CNVocabSciTech.txt", "<(input_prefix)CNVocabOthers.txt", "<@(_outputs)"]
    }, {
      "action_name": "CNVocabulary",
      "variables": {
        "input": "<(output_prefix)CNVocabulary.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)CNVocabulary.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "HKVocabulary.txt",
      "inputs": ["<(cmd)"],
      "outputs": ["<(output_prefix)HKVocabulary.txt"],
      "action": ["python", "<(dict_merge)", "<(input_prefix)HKVocabToponym.txt", "<(input_prefix)HKVocabSciTech.txt", "<(input_prefix)HKVocabOthers.txt", "<@(_outputs)"]
    }, {
      "action_name": "HKVocabulary",
      "variables": {
        "input": "<(output_prefix)HKVocabulary.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)HKVocabulary.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "MOVocabulary.txt",
      "inputs": ["<(cmd)"],
      "outputs": ["<(output_prefix)MOVocabulary.txt"],
      "action": ["python", "<(dict_merge)", "<(input_prefix)MOVocabToponym.txt", "<(input_prefix)MOVocabSciTech.txt", "<(input_prefix)MOVocabOthers.txt", "<@(_outputs)"]
    }, {
      "action_name": "MOVocabulary",
      "variables": {
        "input": "<(output_prefix)MOVocabulary.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)MOVocabulary.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "MYVocabulary.txt",
      "inputs": ["<(cmd)"],
      "outputs": ["<(output_prefix)MYVocabulary.txt"],
      "action": ["python", "<(dict_merge)", "<(input_prefix)MYVocabToponym.txt", "<(input_prefix)MYVocabSciTech.txt", "<(input_prefix)MYVocabOthers.txt", "<@(_outputs)"]
    }, {
      "action_name": "MYVocabulary",
      "variables": {
        "input": "<(output_prefix)MYVocabulary.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)MYVocabulary.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "SGVocabulary.txt",
      "inputs": ["<(cmd)"],
      "outputs": ["<(output_prefix)SGVocabulary.txt"],
      "action": ["python", "<(dict_merge)", "<(input_prefix)SGVocabToponym.txt", "<(input_prefix)SGVocabSciTech.txt", "<(input_prefix)SGVocabOthers.txt", "<@(_outputs)"]
    }, {
      "action_name": "SGVocabulary",
      "variables": {
        "input": "<(output_prefix)SGVocabulary.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)SGVocabulary.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVocabulary.txt",
      "inputs": ["<(cmd)"],
      "outputs": ["<(output_prefix)TWVocabulary.txt"],
      "action": ["python", "<(dict_merge)", "<(input_prefix)TWVocabToponym.txt", "<(input_prefix)TWVocabSciTech.txt", "<(input_prefix)TWVocabOthers.txt", "<@(_outputs)"]
    }, {
      "action_name": "TWVocabulary",
      "variables": {
        "input": "<(output_prefix)TWVocabulary.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVocabulary.ocd2"],
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
      "action_name": "JPVariants",
      "variables": {
        "input": "<(input_prefix)JPVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)JPVariants.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariantsRev.txt",
      "variables": {
        "input": "<(input_prefix)TWVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariantsRev.txt"],
      "action": ["python", "<(dict_reverse)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "TWVariantsRev",
      "variables": {
        "input": "<(output_prefix)TWVariantsRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)TWVariantsRev.ocd2"],
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
      "outputs": ["<(output_prefix)HKVariantsRev.ocd2"],
      "action": ["node", "<(cmd)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "JPVariantsRev.txt",
      "variables": {
        "input": "<(input_prefix)JPVariants.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)JPVariantsRev.txt"],
      "action": ["python", "<(dict_reverse)", "<(input)", "<@(_outputs)"]
    }, {
      "action_name": "JPVariantsRev",
      "variables": {
        "input": "<(output_prefix)JPVariantsRev.txt",
      },
      "inputs": ["<(input)"],
      "outputs": ["<(output_prefix)JPVariantsRev.ocd2"],
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
