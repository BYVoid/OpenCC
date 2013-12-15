{
  "targets": [{
    "target_name": "dicts",
    "type": "none",
    "variables": {
      "cmd": "<(PRODUCT_DIR)/opencc_dict",
      "input_prefix": "data/dictionary/",
      "output_prefix": "<(PRODUCT_DIR)/"
    },
    "actions": [{
      "action_name": "STCharacters",
      "variables": {
        "input": "<(input_prefix)STCharacters.txt",
      },
      "inputs": ["<(cmd)", "<(input)"],
      "outputs": ["<(output_prefix)STCharacters.ocd"],
      "action": ["<(cmd)", "-i", "<(input)", "-o", "<@(_outputs)", "--from", "text", "--to", "ocd"]
    }, {
      "action_name": "STPhrases",
      "variables": {
        "input": "<(input_prefix)STPhrases.txt",
      },
      "inputs": ["<(cmd)", "<(input)"],
      "outputs": ["<(output_prefix)STPhrases.ocd"],
      "action": ["<(cmd)", "-i", "<(input)", "-o", "<@(_outputs)", "--from", "text", "--to", "ocd"]
    }],
    "dependencies": [
      "opencc_dict"
    ]
  }]
}
