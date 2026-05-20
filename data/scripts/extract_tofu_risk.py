#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys


TOFU_RISK_PREFIX = "# @tofu-risk:"


def extract_tofu_risk(input_path, output_path):
    with open(input_path, encoding="utf-8") as input_file:
        lines = input_file.readlines()

    extracted = []
    for index, line in enumerate(lines):
        if not line.startswith(TOFU_RISK_PREFIX):
            continue
        if index + 1 >= len(lines):
            raise ValueError(f"Missing mapping after {TOFU_RISK_PREFIX} at line {index + 1}")
        mapping = lines[index + 1]
        if not mapping.strip() or mapping.startswith("#"):
            raise ValueError(f"Expected mapping after {TOFU_RISK_PREFIX} at line {index + 1}")
        key, values_text = mapping.rstrip("\n").split("\t", 1)
        values = values_text.split()
        if values and values[0] == key:
            values = values[1:]
        if not values:
            raise ValueError(f"Expected extension mapping after {TOFU_RISK_PREFIX} at line {index + 1}")
        extracted.append(f"{key}\t{' '.join(values)}\n")

    with open(output_path, "w", encoding="utf-8") as output_file:
        output_file.writelines(extracted)


if len(sys.argv) != 3:
    print("Extract mappings following @tofu-risk annotations")
    print(("Usage: ", sys.argv[0], "[input] [output]"))
    exit(1)

extract_tofu_risk(sys.argv[1], sys.argv[2])
