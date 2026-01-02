#!/usr/bin/env python3
"""
Build a consolidated testcases.json from *.in/*.ans pairs.

Usage (Bazel genrule):
  python3 gen_testcases_json.py <file1> <file2> ... > testcases.json

Input files should include matching *.in and *.ans files. The script will
combine lines by input text and emit:
{
  "cases": [
    {
      "id": "case_001",
      "input": "...",
      "expected": {
        "s2t": "...",
        "t2s": "..."
      }
    },
    ...
  ]
}
"""

from __future__ import annotations

import json
import os
import sys
from collections import OrderedDict, defaultdict
from typing import Dict, List


def collect_pairs(paths: List[str]) -> Dict[str, Dict[str, str]]:
  """Collect matching .in/.ans files keyed by basename."""
  pairs: Dict[str, Dict[str, str]] = defaultdict(dict)
  for path in paths:
    base = os.path.basename(path)
    if base.endswith(".in"):
      pairs[base[:-3]]["in"] = path
    elif base.endswith(".ans"):
      pairs[base[:-4]]["ans"] = path
  return pairs


def load_cases(pairs: Dict[str, Dict[str, str]]) -> OrderedDict:
  """Load input/expected lines and merge by input string."""
  case_map: "OrderedDict[str, Dict[str, str]]" = OrderedDict()
  for cfg in sorted(pairs.keys()):
    entry = pairs[cfg]
    if "in" not in entry or "ans" not in entry:
      continue
    with open(entry["in"], "r", encoding="utf-8") as fin:
      inputs = fin.read().splitlines()
    with open(entry["ans"], "r", encoding="utf-8") as fans:
      answers = fans.read().splitlines()
    if len(inputs) != len(answers):
      raise ValueError(
          f"Line count mismatch for {entry['in']} ({len(inputs)}) vs "
          f"{entry['ans']} ({len(answers)})")
    for text, expected in zip(inputs, answers):
      if text not in case_map:
        case_map[text] = {}
      if cfg in case_map[text] and case_map[text][cfg] != expected:
        raise ValueError(
            f"Conflicting expectation for '{text}' in {cfg}: "
            f"'{case_map[text][cfg]}' vs '{expected}'")
      case_map[text][cfg] = expected
  return case_map


def main(argv: List[str]) -> int:
  if len(argv) < 2:
    print("Usage: gen_testcases_json.py <files...>", file=sys.stderr)
    return 1

  pairs = collect_pairs(argv[1:])
  case_map = load_cases(pairs)

  cases = []
  for idx, (text, expectations) in enumerate(case_map.items(), start=1):
    cases.append({
        "id": f"case_{idx:03d}",
        "input": text,
        "expected": expectations,
    })

  json.dump({"cases": cases}, sys.stdout, ensure_ascii=False, indent=2)
  sys.stdout.write("\n")
  return 0


if __name__ == "__main__":
  sys.exit(main(sys.argv))
