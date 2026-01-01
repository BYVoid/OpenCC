#!/usr/bin/env python3
"""
Generate a consolidated testcases JSON from OpenCC/test/testcases.

Usage:
  python gen_testcases_json.py ../test/testcases > public/testcases.json

It scans for files ending with ".in" and expects a sibling ".out" with the
same basename. The directory name is treated as the config filename
(e.g. t2s/t2s.in -> config "t2s.json").
"""

import json
import os
import sys


def collect_cases(root: str):
    cases = []
    for dirpath, _, filenames in os.walk(root):
        for name in filenames:
            if not name.endswith(".in"):
                continue
            in_path = os.path.join(dirpath, name)
            out_path = os.path.join(dirpath, name[:-3] + ".ans")
            if not os.path.exists(out_path):
                continue
            # config name derives from file basename (e.g. t2s.in -> t2s.json)
            cfg_base = os.path.splitext(name)[0]
            config = f"{cfg_base}.json"
            with open(in_path, "r", encoding="utf-8") as fin:
                in_lines = fin.read().splitlines()
            with open(out_path, "r", encoding="utf-8") as fout:
                out_lines = fout.read().splitlines()
            if len(in_lines) != len(out_lines):
                raise ValueError(
                    f"Line count mismatch for {in_path} ({len(in_lines)}) vs {out_path} ({len(out_lines)})"
                )
            for inp, ans in zip(in_lines, out_lines):
                cases.append(
                    {
                        "config": config,
                        "input": inp,
                        "expected": ans,
                    }
                )
    return cases


def main():
    if len(sys.argv) < 2:
        print("Usage: gen_testcases_json.py <path-to-testcases>", file=sys.stderr)
        sys.exit(1)
    root = sys.argv[1]
    cases = collect_cases(root)
    json.dump(cases, sys.stdout, ensure_ascii=False, indent=2)


if __name__ == "__main__":
    main()
