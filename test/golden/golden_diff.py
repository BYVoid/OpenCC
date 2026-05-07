#!/usr/bin/env python3
#
# Open Chinese Convert
#
# Copyright 2026 Carbo Kuo and contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import difflib
from collections import Counter
from itertools import combinations
from pathlib import Path
import sys
from typing import Iterable


def workspace_root() -> Path:
    return Path(__file__).resolve().parents[2]


def output_dir() -> Path:
    return workspace_root() / "test/golden/output"


def parse_output_name(path: Path) -> tuple[str, str]:
    stem = path.stem
    input_name, _, config = stem.rpartition(".")
    if not input_name or not config:
        raise ValueError(f"Unexpected output file name: {path.name}")
    return input_name, config


def load_outputs() -> dict[str, dict[str, list[str]]]:
    grouped: dict[str, dict[str, list[str]]] = {}
    for path in sorted(output_dir().glob("*.txt")):
        input_name, config = parse_output_name(path)
        grouped.setdefault(input_name, {})[config] = path.read_text(
            encoding="utf-8"
        ).splitlines()
    return grouped


def choose_input(all_outputs: dict[str, dict[str, list[str]]], input_name: str | None) -> str:
    if input_name is not None:
        if input_name not in all_outputs:
            raise SystemExit(f"Unknown input: {input_name}")
        return input_name

    names = sorted(all_outputs)
    if len(names) != 1:
        joined = ", ".join(names)
        raise SystemExit(f"Multiple inputs found; pass --input. Available: {joined}")
    return names[0]


def diff_lines(left: list[str], right: list[str]) -> list[int]:
    limit = min(len(left), len(right))
    differing = [i + 1 for i in range(limit) if left[i] != right[i]]
    if len(left) != len(right):
        differing.extend(range(limit + 1, max(len(left), len(right)) + 1))
    return differing


def use_color(mode: str) -> bool:
    if mode == "always":
        return True
    if mode == "never":
        return False
    return sys.stdout.isatty() and "NO_COLOR" not in os_environ()


def os_environ() -> dict[str, str]:
    import os

    return os.environ


def colorize(text: str, enabled: bool, role: str = "minor") -> str:
    if not enabled or not text:
        return text
    colors = {
        "major": "\033[1;34m",
        "minor": "\033[1;31m",
    }
    return f"{colors.get(role, colors['minor'])}{text}\033[0m"


def common_prefix(strings: list[str]) -> str:
    if not strings:
        return ""
    prefix = strings[0]
    for text in strings[1:]:
        limit = min(len(prefix), len(text))
        index = 0
        while index < limit and prefix[index] == text[index]:
            index += 1
        prefix = prefix[:index]
        if not prefix:
            break
    return prefix


def common_suffix(strings: list[str], prefix_len: int) -> str:
    if not strings:
        return ""
    suffix = strings[0][prefix_len:]
    for text in strings[1:]:
        candidate = text[prefix_len:]
        limit = min(len(suffix), len(candidate))
        index = 0
        while index < limit and suffix[-(index + 1)] == candidate[-(index + 1)]:
            index += 1
        suffix = suffix[len(suffix) - index :] if index else ""
        if not suffix:
            break
    return suffix


def highlight_spans(
    text: str, spans: list[tuple[int, int, str]], enabled: bool
) -> str:
    if not enabled or not spans:
        return text

    merged: list[tuple[int, int, str]] = []
    for start, end, role in sorted(spans):
        if start >= end:
            continue
        if not merged or start > merged[-1][1] or role != merged[-1][2]:
            merged.append((start, end, role))
        else:
            merged[-1] = (merged[-1][0], max(merged[-1][1], end), role)

    parts: list[str] = []
    cursor = 0
    for start, end, role in merged:
        parts.append(text[cursor:start])
        parts.append(colorize(text[start:end], enabled, role))
        cursor = end
    parts.append(text[cursor:])
    return "".join(parts)


def map_reference_span(
    opcodes: list[tuple[str, int, int, int, int]], start: int, end: int
) -> tuple[int, int]:
    mapped_start: int | None = None
    mapped_end: int | None = None

    for tag, i1, i2, j1, j2 in opcodes:
        if end <= i1:
            break
        if start >= i2:
            continue
        overlap_start = max(start, i1)
        overlap_end = min(end, i2)
        if overlap_start >= overlap_end:
            continue

        if tag == "equal":
            candidate_start = j1 + (overlap_start - i1)
            candidate_end = j1 + (overlap_end - i1)
        elif tag == "replace":
            candidate_start = j1
            candidate_end = j2
        elif tag == "delete":
            candidate_start = j1
            candidate_end = j1
        else:
            continue

        if mapped_start is None:
            mapped_start = candidate_start
        mapped_end = candidate_end

    if mapped_start is None or mapped_end is None:
        for tag, i1, i2, j1, j2 in opcodes:
            if tag == "insert" and i1 == start:
                return j1, j2
        return 0, 0

    return mapped_start, mapped_end


def highlight_matrix_values(
    configs: list[str],
    values: dict[str, str],
    enabled: bool,
    reference_name: str | None = None,
) -> dict[str, str]:
    unique_values = list(dict.fromkeys(values.values()))
    if len(unique_values) <= 1:
        return values

    if reference_name is None:
        counts = Counter(values.values())
        reference_text = max(unique_values, key=lambda value: (counts[value], -len(value)))
        reference_name = next(
            config for config in configs if values[config] == reference_text
        )
    else:
        reference_text = values[reference_name]

    config_opcodes: dict[str, list[tuple[str, int, int, int, int]]] = {}
    reference_spans: list[tuple[int, int]] = []
    for config in configs:
        if config == reference_name:
            continue
        opcodes = difflib.SequenceMatcher(a=reference_text, b=values[config]).get_opcodes()
        config_opcodes[config] = opcodes
        for tag, i1, i2, j1, j2 in opcodes:
            if tag == "equal":
                continue
            if tag == "insert":
                reference_spans.append((i1, i1))
            else:
                reference_spans.append((i1, i2))

    merged_reference_spans: list[tuple[int, int]] = []
    for start, end in sorted(reference_spans):
        if not merged_reference_spans or start > merged_reference_spans[-1][1]:
            merged_reference_spans.append((start, end))
        else:
            merged_reference_spans[-1] = (
                merged_reference_spans[-1][0],
                max(merged_reference_spans[-1][1], end),
            )

    spans: dict[str, list[tuple[int, int, str]]] = {config: [] for config in configs}
    for start, end in merged_reference_spans:
        reference_segment = reference_text[start:end]
        spans[reference_name].append((start, end, "major"))
        for config in configs:
            if config == reference_name:
                continue
            mapped_start, mapped_end = map_reference_span(
                config_opcodes[config], start, end
            )
            segment = values[config][mapped_start:mapped_end]
            role = "major" if segment == reference_segment else "minor"
            spans[config].append((mapped_start, mapped_end, role))

    highlighted: dict[str, str] = {}
    for config in configs:
        highlighted[config] = highlight_spans(values[config], spans[config], enabled)
    return highlighted


def highlight_pair(left: str, right: str, enabled: bool) -> tuple[str, str]:
    if not enabled or left == right:
        return left, right

    left_role = "major"
    right_role = "minor"
    left_parts: list[str] = []
    right_parts: list[str] = []
    matcher = difflib.SequenceMatcher(a=left, b=right)
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        left_chunk = left[i1:i2]
        right_chunk = right[j1:j2]
        if tag == "equal":
            left_parts.append(left_chunk)
            right_parts.append(right_chunk)
            continue
        left_parts.append(colorize(left_chunk, True, left_role))
        right_parts.append(colorize(right_chunk, True, right_role))
    return "".join(left_parts), "".join(right_parts)


def print_summary(outputs: dict[str, list[str]]) -> None:
    configs = sorted(outputs)
    width = max(len(config) for config in configs)
    for left_name, right_name in combinations(configs, 2):
        differing = diff_lines(outputs[left_name], outputs[right_name])
        first = differing[0] if differing else "-"
        print(
            f"{left_name:<{width}} vs {right_name:<{width}} "
            f"diff_lines={len(differing):<3} first={first}"
        )


def print_matrix(
    outputs: dict[str, list[str]],
    only_diffs: bool,
    line_numbers: Iterable[int] | None,
    color: bool,
    reference_name: str | None,
) -> None:
    configs = sorted(outputs)
    if reference_name is not None and reference_name not in outputs:
        raise SystemExit(f"Unknown config: {reference_name}")
    width = max(len(config) for config in configs)
    total_lines = max(len(lines) for lines in outputs.values())
    selected = set(line_numbers or [])

    for line_number in range(1, total_lines + 1):
        values = {
            outputs[config][line_number - 1] if line_number - 1 < len(outputs[config]) else ""
            for config in configs
        }
        if selected and line_number not in selected:
            continue
        if only_diffs and len(values) <= 1:
            continue

        print(f"LINE {line_number}")
        rendered = highlight_matrix_values(
            configs,
            {
                config: (
                    outputs[config][line_number - 1]
                    if line_number - 1 < len(outputs[config])
                    else ""
                )
                for config in configs
            },
            color,
            reference_name=reference_name,
        )
        for config in configs:
            print(f"{config:<{width}}  {rendered[config]}")
        print()


def print_pair(
    outputs: dict[str, list[str]], left_name: str, right_name: str, color: bool
) -> None:
    if left_name not in outputs or right_name not in outputs:
        missing = [name for name in (left_name, right_name) if name not in outputs]
        raise SystemExit(f"Unknown config: {', '.join(missing)}")

    left = outputs[left_name]
    right = outputs[right_name]
    width = max(len(left_name), len(right_name))

    for line_number in diff_lines(left, right):
        left_value = left[line_number - 1] if line_number - 1 < len(left) else ""
        right_value = right[line_number - 1] if line_number - 1 < len(right) else ""
        left_value, right_value = highlight_pair(left_value, right_value, color)
        print(f"LINE {line_number}")
        print(f"{left_name:<{width}}  {left_value}")
        print(f"{right_name:<{width}}  {right_value}")
        print()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Inspect differences between OpenCC golden outputs."
    )
    parser.add_argument("--input", help="Input stem, for example us_constitution_zhs")
    parser.add_argument(
        "--color",
        choices=["auto", "always", "never"],
        default="auto",
        help="Whether to use ANSI colors in diff output",
    )

    subparsers = parser.add_subparsers(dest="command", required=True)

    subparsers.add_parser("summary", help="Show pairwise diff-line counts")

    matrix_parser = subparsers.add_parser(
        "matrix", help="Show all configs side by side for differing lines"
    )
    matrix_parser.add_argument(
        "--all-lines",
        action="store_true",
        help="Show every line instead of only differing lines",
    )
    matrix_parser.add_argument(
        "--line",
        dest="lines",
        action="append",
        type=int,
        help="Show a specific line number; repeatable",
    )
    matrix_parser.add_argument(
        "--against",
        dest="reference_name",
        help="Highlight each config against this reference config",
    )

    pair_parser = subparsers.add_parser(
        "pair", help="Show differing lines for two configs"
    )
    pair_parser.add_argument("left")
    pair_parser.add_argument("right")

    return parser.parse_args()


def main() -> None:
    args = parse_args()
    color = use_color(args.color)
    all_outputs = load_outputs()
    input_name = choose_input(all_outputs, args.input)
    outputs = all_outputs[input_name]

    if args.command == "summary":
        print_summary(outputs)
        return

    if args.command == "matrix":
        print_matrix(
            outputs,
            only_diffs=not args.all_lines,
            line_numbers=args.lines,
            color=color,
            reference_name=args.reference_name,
        )
        return

    if args.command == "pair":
        print_pair(outputs, args.left, args.right, color)
        return

    raise SystemExit(f"Unknown command: {args.command}")


if __name__ == "__main__":
    main()
