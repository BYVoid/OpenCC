{
  "variables": {
    "opencc_version": "1.1.9"
  },
  "target_defaults": {
    "defines": [
      "VERSION=\"<(opencc_version)\""
    ],
    "conditions": [
      [
        "OS==\"linux\"",
        {
          "cflags": [
            "-std=c++20"
          ],
          "cflags!": [
            "-fno-exceptions"
          ],
          "cflags_cc!": [
            "-fno-exceptions"
          ]
        }
      ],
      [
        "OS==\"mac\"",
        {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "MACOSX_DEPLOYMENT_TARGET": "10.7",
            "OTHER_CPLUSPLUSFLAGS": [
              "-std=c++20",
              "-stdlib=libc++"
            ],
            "OTHER_LDFLAGS": [
              "-stdlib=libc++"
            ]
          }
        }
      ],
      [
        "OS==\"win\"",
        {
          "defines": [
            "Opencc_BUILT_AS_STATIC"
          ]
        }
      ]
    ]
  }
}