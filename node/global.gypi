{
  "variables": {
    "opencc_version": "1.0.5"
  },
  "target_defaults": {
    "defines": [
      "VERSION=\"<(opencc_version)\""
    ],
    "conditions": [
      ["OS=='linux'", {
        "cflags": [
          "-std=c++0x"
        ],
        "cflags!": ["-fno-exceptions"],
        "cflags_cc!": ["-fno-exceptions"],
      }],
      ["OS=='mac'", {
        'xcode_settings': {
          'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
          'MACOSX_DEPLOYMENT_TARGET': '10.7',
          'OTHER_CPLUSPLUSFLAGS': ["-std=c++11", "-stdlib=libc++"],
          'OTHER_LDFLAGS': ["-stdlib=libc++"]
        }
      }],
	  ["OS=='win'", {
        "defines": ["Opencc_BUILT_AS_STATIC"]
	  }]
    ]
  }
}
