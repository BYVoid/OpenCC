"""Minify a JSON file and write it as a C++ raw string literal .inc file.

Usage: minify_json_to_inc.py <input.json> <output.inc>

The output file contains a single raw string literal expression suitable for
use with #include inside a C++ variable declaration:

    static const char kFoo[] =
    #include "output.inc"
    ;
"""

import json
import sys


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input.json> <output.inc>", file=sys.stderr)
        sys.exit(1)

    with open(sys.argv[1], encoding="utf-8") as f:
        data = json.load(f)

    minified = json.dumps(data, ensure_ascii=False, separators=(",", ":"))

    # Use a named raw-string delimiter that cannot appear in JSON schema content.
    with open(sys.argv[2], "w", encoding="utf-8") as f:
        f.write(
            "// Generated from opencc_config.schema.json by minify_json_to_inc.py.\n"
            "// Do not edit manually; run the script or `bazel build //src:config_schema_inc`\n"
            "// and copy bazel-bin/src/generated/opencc_config_schema.inc here to update.\n"
            f'R"schema({minified})schema"'
        )


if __name__ == "__main__":
    main()
