#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Rewrite an OpenCC config JSON so that its dictionary references point at
# a different dictionary format (text or ocd) instead of ocd2. Used to build
# the OPENCC_DICT_FORMAT=text/ocd CMake outputs, mirroring the ocd2<->text
# config conversion already used by the Bazel opencc_resources_zip /
# convert_resource_zip tools.

import json
import sys

TARGET_EXTENSIONS = {
    "text": ".txt",
    "ocd": ".ocd",
}


def convert_dict_references(value, target_format, target_extension):
    if isinstance(value, dict):
        converted = {}
        for key, child in value.items():
            if key == "type" and child == "ocd2":
                converted[key] = target_format
            elif key == "file" and isinstance(child, str) and child.endswith(".ocd2"):
                converted[key] = child[:-len(".ocd2")] + target_extension
            else:
                converted[key] = convert_dict_references(
                    child, target_format, target_extension
                )
        return converted
    if isinstance(value, list):
        return [
            convert_dict_references(child, target_format, target_extension)
            for child in value
        ]
    return value


if len(sys.argv) != 4 or sys.argv[3] not in TARGET_EXTENSIONS:
    print("Rewrite ocd2 dictionary references in a config JSON to another format")
    print(f"Usage: {sys.argv[0]} [input] [output] [text|ocd]")
    sys.exit(1)

target_format = sys.argv[3]
with open(sys.argv[1], encoding="utf-8") as f:
    config = json.load(f)

converted = convert_dict_references(config, target_format, TARGET_EXTENSIONS[target_format])
with open(sys.argv[2], "w", encoding="utf-8") as f:
    f.write(json.dumps(converted, ensure_ascii=False, indent=2) + "\n")
