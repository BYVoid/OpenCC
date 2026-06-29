# Example OpenCC Configurations

[繁體中文](README.zh.md)

This directory contains example configurations for learning and customization.
They are not official built-in OpenCC configurations and are not listed by
`opencc --help`.

Use them by passing the file path explicitly and pointing `--path` to the
directory that contains the compiled dictionary files:

```sh
opencc -c examples/config/s2twp_taiwan_word.json --path /path/to/opencc/dictionary/
```

When using Bazel build artifacts:

```sh
bazel-bin/src/tools/command_line \
  -c examples/config/s2twp_taiwan_word.json \
  --path bazel-bin/data/dictionary/
```

The examples are based on `data/config/s2twp.json` and add small inline
dictionaries to show how local overrides work without editing the official
dictionary files. You can modify these examples as needed to fit your own
conversion requirements.

The phrase example also adds the corresponding inline entries to
`segmentation.dict` before `STPhrases.ocd2`. This makes it safer to adapt: when
you add your own multi-character phrase override, the segmentation entry keeps
the source phrase from being split before the conversion stages can match it.

The quote-mark example does not modify `segmentation`, because it only converts
single-character punctuation marks.

Segmentation placement:

```json
"segmentation": {
  "type": "mmseg",
  "dict": {
    "type": "group",
    "match_policy": "short_circuit",
    "dicts": [
      {
        "type": "inline",
        "entries": {
          "custom source phrase": "custom source phrase"
        }
      },
      { "type": "ocd2", "file": "STPhrases.ocd2" }
    ]
  }
}
```

`match_policy: short_circuit` (the default when the field is absent) stops at
the first dictionary in the group that has a match for the current prefix. Use
it when your inline dict should take priority and you do not want later
dictionaries to override it.
