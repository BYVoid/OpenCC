{
  "includes": [
    "opencc_dict.gypi",
  ],
  "targets": [{
    "target_name": "dicts",
    "type": "none",
    "variables": {
      "cmd": "<(PRODUCT_DIR)/opencc_dict",
      "input_prefix": "data/",
      "output_prefix": "<(PRODUCT_DIR)/"
    },
    "actions": [{
      "action_name": "simp_to_trad_characters",
      "variables": {
        "input": "<(input_prefix)simp_to_trad/characters.txt",
      },
      "inputs": ["<(cmd)", "<(input)"],
      "outputs": ["<(output_prefix)simp_to_trad_characters.ocd"],
      "action": ["<(cmd)", "-i", "<(input)", "-o", "<@(_outputs)"]
    }, {
      "action_name": "simp_to_trad_phrases",
      "variables": {
        "input": "<(input_prefix)simp_to_trad/phrases.txt",
      },
      "inputs": ["<(cmd)", "<(input)"],
      "outputs": ["<(output_prefix)simp_to_trad_phrases.ocd"],
      "action": ["<(cmd)", "-i", "<(input)", "-o", "<@(_outputs)"]
    }, {
      "action_name": "trad_to_simp_characters",
      "variables": {
        "input": "<(input_prefix)trad_to_simp/characters.txt",
      },
      "inputs": ["<(cmd)", "<(input)"],
      "outputs": ["<(output_prefix)trad_to_simp_characters.ocd"],
      "action": ["<(cmd)", "-i", "<(input)", "-o", "<@(_outputs)"]
    }, {
      "action_name": "trad_to_simp_phrases",
      "variables": {
        "input": "<(input_prefix)trad_to_simp/phrases.txt",
      },
      "inputs": ["<(cmd)", "<(input)"],
      "outputs": ["<(output_prefix)trad_to_simp_phrases.ocd"],
      "action": ["<(cmd)", "-i", "<(input)", "-o", "<@(_outputs)"]
    }],
    "dependencies": [
      "opencc_dict"
    ]
  }]
}
