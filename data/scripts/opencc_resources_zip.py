#!/usr/bin/env python3

import argparse
import json
import os
import time
import zipfile


def parse_args():
    parser = argparse.ArgumentParser(
        description="Build an OpenCC share archive backed by text dictionaries."
    )
    parser.add_argument("--output", required=True)
    parser.add_argument("--configs", nargs="+", required=True)
    parser.add_argument("--dicts", nargs="+", required=True)
    return parser.parse_args()


def convert_dict_references(value):
    if isinstance(value, dict):
        converted = {}
        for key, child in value.items():
            if key == "type" and child == "ocd2":
                converted[key] = "text"
            elif key == "file" and isinstance(child, str) and child.endswith(".ocd2"):
                converted[key] = child[:-5] + ".txt"
            else:
                converted[key] = convert_dict_references(child)
        return converted
    if isinstance(value, list):
        return [convert_dict_references(child) for child in value]
    return value


def read_text_config(path):
    with open(path, encoding="utf-8") as file:
        config = json.load(file)
    converted = convert_dict_references(config)
    return json.dumps(converted, ensure_ascii=False, indent=2) + "\n"


def read_clean_dictionary(path):
    cleaned = []
    with open(path, encoding="utf-8") as file:
        for line in file:
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            cleaned.append(line)
    return "".join(cleaned)


def write_entry(archive, name, content, date_time):
    info = zipfile.ZipInfo(name)
    info.date_time = date_time
    info.compress_type = zipfile.ZIP_STORED
    info.external_attr = 0o644 << 16
    archive.writestr(info, content.encode("utf-8"))


def main():
    args = parse_args()
    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    date_time = time.localtime()[:6]
    with zipfile.ZipFile(args.output, "w") as archive:
        for path in sorted(args.configs, key=os.path.basename):
            write_entry(
                archive, os.path.basename(path), read_text_config(path), date_time
            )
        for path in sorted(args.dicts, key=os.path.basename):
            write_entry(
                archive, os.path.basename(path), read_clean_dictionary(path), date_time
            )


if __name__ == "__main__":
    main()
