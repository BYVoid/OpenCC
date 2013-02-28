{
  "variables": {
    "opencc_version": "0.4.0",
  },
  "target_defaults": {
    "defines": [
      "VERSION=\"<(opencc_version)\""
    ]
  },
  "targets": [{
    "target_name": "libopencc",
    "type": "<(library)",
    "sources": [
			"src/config_reader.c",
			"src/converter.c",
			"src/dictionary_group.c",
			"src/dictionary_set.c",
			"src/encoding.c",
			"src/utils.c",
			"src/opencc.c",
			"src/config_reader.h",
			"src/converter.h",
			"src/dictionary_group.h",
			"src/dictionary_set.h",
			"src/encoding.h",
			"src/utils.h",
			"src/dictionary/abstract.c",
			"src/dictionary/datrie.c",
			"src/dictionary/text.c",
			"src/dictionary/abstract.h",
			"src/dictionary/datrie.h",
			"src/dictionary/text.h",
    ],
    "defines": [
      "PKGDATADIR=\"\""
    ],
    "conditions": [
      ["OS=='linux'", {
        "cflags": [
          "-fPIC"
        ]
      }]
    ]
  }, {
    "target_name": "opencc",
    "type": "executable",
    "sources": [
      "src/tools/opencc.c"
    ],
    "dependencies": [
      "libopencc"
    ]
  }, {
    "target_name": "opencc_dict",
    "type": "executable",
    "sources": [
      "src/tools/opencc_dict.c"
    ],
    "dependencies": [
      "libopencc"
    ]
  }, {
    "target_name": "configs",
    "type": "none",
    "copies": [{
      "destination": "<(PRODUCT_DIR)",
      "files": [
				"data/config/mix2zhs.ini",
				"data/config/mix2zht.ini",
				"data/config/zhs2zht.ini",
				"data/config/zhs2zhtw_p.ini",
				"data/config/zhs2zhtw_v.ini",
				"data/config/zhs2zhtw_vp.ini",
				"data/config/zht2zhs.ini",
				"data/config/zht2zhtw_p.ini",
				"data/config/zht2zhtw_v.ini",
				"data/config/zht2zhtw_vp.ini",
				"data/config/zhtw2zhcn_s.ini",
				"data/config/zhtw2zhcn_t.ini",
				"data/config/zhtw2zhs.ini",
				"data/config/zhtw2zht.ini"
      ]
    }]
  }, {
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
