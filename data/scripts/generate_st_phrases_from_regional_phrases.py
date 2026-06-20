#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import subprocess
import sys
from collections import defaultdict

from common import Dict


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate ST phrase entries from regional phrase keys."
    )
    parser.add_argument("--input", action="append", required=True)
    parser.add_argument("--output", required=True)
    backend = parser.add_mutually_exclusive_group(required=True)
    backend.add_argument("--opencc")
    backend.add_argument("--node-binding")
    parser.add_argument("--config", required=True)
    parser.add_argument("--dict-dir", required=True)
    return parser.parse_args()


def clean_path_arg(path):
    return path.replace('"', "")


def convert_keys(opencc, config, dict_dir, keys):
    result = subprocess.run(
        [
            opencc,
            "-c",
            config,
            "--path",
            dict_dir,
            "--include-tofu-risk-dictionaries",
        ],
        input="\n".join(keys) + "\n",
        encoding="utf-8",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        print(result.stderr, file=sys.stderr, end="")
        return None

    converted = result.stdout.splitlines()
    if len(converted) != len(keys):
        print(
            "Converted key count mismatch: "
            f"expected {len(keys)}, got {len(converted)}",
            file=sys.stderr,
        )
        return None
    return converted


def convert_keys_node(node_binding, config, dict_dir, keys):
    script = r"""
const fs = require('fs');
const path = require('path');
const binding = require(process.argv[1]);
const config = fs.readFileSync(process.argv[2], 'utf8');
let dictDir = process.argv[3].replace(/"/g, '');
if (!dictDir.endsWith(path.sep)) {
  dictDir += path.sep;
}
const converter = new binding.Opencc(config, dictDir);
let input = '';
process.stdin.setEncoding('utf8');
process.stdin.on('data', chunk => { input += chunk; });
process.stdin.on('end', () => {
  const lines = input.endsWith('\n') ? input.slice(0, -1).split('\n') : input.split('\n');
  for (const line of lines) {
    process.stdout.write(converter.convertSync(line) + '\n');
  }
});
"""
    result = subprocess.run(
        ["node", "-e", script, node_binding, config, dict_dir],
        input="\n".join(keys) + "\n",
        encoding="utf-8",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        print(result.stderr, file=sys.stderr, end="")
        return None

    converted = result.stdout.splitlines()
    if len(converted) != len(keys):
        print(
            "Converted key count mismatch: "
            f"expected {len(keys)}, got {len(converted)}",
            file=sys.stderr,
        )
        return None
    return converted


def main():
    args = parse_args()
    if args.opencc:
        args.opencc = clean_path_arg(args.opencc)
    if args.node_binding:
        args.node_binding = clean_path_arg(args.node_binding)
    args.config = clean_path_arg(args.config)
    args.dict_dir = clean_path_arg(args.dict_dir)
    input_names = [input_path.rsplit("/", 1)[-1] for input_path in args.input]
    output_name = args.output.rsplit("/", 1)[-1]

    collisions = defaultdict(list)
    for input_path in args.input:
        entries = list(Dict().iter(input_path))
        keys = [entry["key"] for entry in entries]
        if args.opencc:
            converted_keys = convert_keys(args.opencc, args.config, args.dict_dir, keys)
        else:
            converted_keys = convert_keys_node(
                args.node_binding, args.config, args.dict_dir, keys
            )
        if converted_keys is None:
            return 1

        for converted_key, original_key in zip(converted_keys, keys):
            # Short regional vocabulary keys can split longer Simplified words
            # before STPhrases gets a chance to match them.
            if len(converted_key) < 3:
                continue
            collisions[converted_key].append(original_key)

    conflicts = {
        key: values for key, values in collisions.items() if len(set(values)) > 1
    }
    if conflicts:
        print("Conflicting regional phrase simplified projections:", file=sys.stderr)
        for key in sorted(conflicts):
            print(f"  {key}: {' '.join(conflicts[key])}", file=sys.stderr)
        return 1

    generated = [
        {"key": key, "values": [values[0]]}
        for key, values in collisions.items()
    ]
    generated.sort(key=lambda entry: entry["key"])

    with open(args.output, "w", encoding="utf-8", newline="\n") as stream:
        stream.write("# Open Chinese Convert (OpenCC) Dictionary\n")
        stream.write(f"# File: {output_name}\n")
        stream.write("# Format: key\tvalue(s) (values separated by spaces)\n")
        stream.write("# License: Apache-2.0 (see LICENSE)\n")
        stream.write(
            "# Source: generated from " + ", ".join(input_names) +
            " keys via t2s.json\n"
        )
        stream.write("# Used in configs: s2t.json, s2hk.json, s2hkp.json, s2tw.json, s2twp.json\n")
        stream.write("#\n")
        stream.write("# This generated ST phrase dictionary preserves Simplified-input spans\n")
        stream.write("# before applying regional phrase vocabulary.\n")
        stream.write("\n")
        for entry in generated:
            stream.write(Dict.get_line(entry))

    return 0


if __name__ == "__main__":
    sys.exit(main())
