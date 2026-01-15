# -*- coding: utf-8 -*-

import codecs
import sys


def sort_items(input_filename, output_filename):
    input_file = codecs.open(input_filename, "r", encoding="utf-8")

    lines = [line.rstrip("\r\n") for line in input_file]
    input_file.close()

    def line_type(line):
        if line == "" or line.strip() == "":
            return "empty"
        if line.startswith("#"):
            return "comment"
        if "\t" in line:
            return "entry"
        raise ValueError("Invalid dictionary line: " + line)

    parsed = []
    for line in lines:
        parsed.append({"type": line_type(line), "content": line})

    entry_lines = [i for i, p in enumerate(parsed) if p["type"] == "entry"]
    if not entry_lines:
        header_blocks = []
        current = []
        for p in parsed:
            if p["type"] == "comment":
                current.append(p["content"])
            elif p["type"] == "empty":
                if current:
                    header_blocks.append(list(current))
                    current = []
        if current:
            header_blocks.append(list(current))

        output_file = open(output_filename, "wb")
        for idx, block in enumerate(header_blocks):
            for line in block:
                output_file.write((line + "\n").encode("utf-8"))
            if idx < len(header_blocks) - 1:
                output_file.write(b"\n")
        if header_blocks:
            output_file.write(b"\n")
        output_file.close()
        return

    first_entry = entry_lines[0]
    last_entry = entry_lines[-1]

    header_end = -1
    for i in range(first_entry - 1, -1, -1):
        if parsed[i]["type"] == "empty":
            header_end = i
            break

    header_blocks = []
    current = []
    for i in range(0, header_end + 1):
        if parsed[i]["type"] == "comment":
            current.append(parsed[i]["content"])
        elif parsed[i]["type"] == "empty":
            if current:
                header_blocks.append(list(current))
                current = []
    if current:
        header_blocks.append(list(current))

    footer_blocks = []
    current = []
    for i in range(last_entry + 1, len(parsed)):
        if parsed[i]["type"] == "comment":
            current.append(parsed[i]["content"])
        elif parsed[i]["type"] == "empty":
            if current:
                footer_blocks.append(list(current))
                current = []
    if current:
        footer_blocks.append(list(current))

    annotated_entries = []
    floating_blocks = []
    current = []
    entry_index = 0
    for i in range(header_end + 1, last_entry + 1):
        p = parsed[i]
        if p["type"] == "comment":
            current.append(p["content"])
            continue
        if p["type"] == "empty":
            if current:
                floating_blocks.append({"anchor": entry_index, "lines": list(current)})
                current = []
            continue
        if p["type"] == "entry":
            attached = None
            if current:
                has_empty = False
                for j in range(i - 1, -1, -1):
                    if parsed[j]["type"] == "entry":
                        break
                    if parsed[j]["type"] == "empty":
                        has_empty = True
                        break
                if has_empty:
                    floating_blocks.append({"anchor": entry_index, "lines": list(current)})
                else:
                    attached = list(current)
                current = []

            key, value = p["content"].split("\t", 1)
            annotated_entries.append(
                {
                    "key": key,
                    "value": value,
                    "attached": attached,
                    "original_index": entry_index,
                }
            )
            entry_index += 1

    if current:
        floating_blocks.append({"anchor": entry_index, "lines": list(current)})

    annotated_entries.sort(key=lambda e: e["key"])
    index_map = {e["original_index"]: i for i, e in enumerate(annotated_entries)}
    for block in floating_blocks:
        if block["anchor"] in index_map:
            block["anchor"] = index_map[block["anchor"]]
        else:
            block["anchor"] = len(annotated_entries)

    floating_by_anchor = {}
    for block in floating_blocks:
        floating_by_anchor.setdefault(block["anchor"], []).append(block["lines"])

    output_file = open(output_filename, "wb")

    for idx, block in enumerate(header_blocks):
        for line in block:
            output_file.write((line + "\n").encode("utf-8"))
        if idx < len(header_blocks) - 1:
            output_file.write(b"\n")
    if header_blocks and annotated_entries:
        output_file.write(b"\n")

    for i, entry in enumerate(annotated_entries):
        for block in floating_by_anchor.get(i, []):
            output_file.write(b"\n")
            for line in block:
                output_file.write((line + "\n").encode("utf-8"))
            output_file.write(b"\n")

        if entry["attached"]:
            for line in entry["attached"]:
                output_file.write((line + "\n").encode("utf-8"))
        output_file.write(
            (entry["key"] + "\t" + entry["value"] + "\n").encode("utf-8")
        )

    for block in floating_by_anchor.get(len(annotated_entries), []):
        output_file.write(b"\n")
        for line in block:
            output_file.write((line + "\n").encode("utf-8"))

    if footer_blocks:
        if annotated_entries:
            output_file.write(b"\n")
        for idx, block in enumerate(footer_blocks):
            for line in block:
                output_file.write((line + "\n").encode("utf-8"))
            if idx < len(footer_blocks) - 1:
                output_file.write(b"\n")

    output_file.close()


def reverse_items(input_filename, output_filename):
    input_file = codecs.open(input_filename, "r", encoding="utf-8")
    dic = {}

    for line in input_file:
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        key, value = line.split("\t")
        while value[-1] == "\n" or value[-1] == "\r":
            value = value[:-1]

        value_list = value.split(" ")
        for value in value_list:
            if value in dic:
                dic[value].append(key)
            else:
                dic[value] = [key]

    input_file.close()

    output_file = open(output_filename, "wb")

    for key in sorted(dic.keys()):
        line = key + "\t" + " ".join(dic[key]) + "\n"
        output_file.write(line.encode('utf-8'))

    output_file.close()


def find_target_items(input_filename, keyword):
    input_file = codecs.open(input_filename, "r", encoding="utf-8")
    for line in input_file:
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        key, value = line.split("\t")
        while value[-1] == "\n" or value[-1] == "\r":
            value = value[:-1]

        value_list = value.split(" ")
        for value in value_list:
            if keyword in value:
                sys.stdout.write(line)

    input_file.close()
