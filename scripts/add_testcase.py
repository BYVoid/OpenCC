#!/usr/bin/env python3
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Optional


CONFIGS = [
    "s2t",
    "s2tw",
    "s2twp",
    "s2hk",
    "t2s",
    "t2tw",
    "t2hk",
    "tw2s",
    "tw2sp",
    "tw2t",
    "hk2s",
    "hk2t",
]


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def prompt_if_missing(value: Optional[str], prompt: str) -> str:
    if value:
        return value
    return input(prompt).strip()


def normalize_kind(value: str) -> str:
    value = value.strip().lower()
    if value in {"pr", "pull request", "pull-request"}:
        return "PR"
    if value in {"issue", "i"}:
        return "Issue"
    raise ValueError("PR or Issue must be either 'PR' or 'Issue'")


def sanitize_description(value: str) -> str:
    value = value.strip()
    if not value:
        return ""
    value = re.sub(r"[^0-9A-Za-z]+", "_", value)
    return value.strip("_")


def testcase_id(kind: str, number: str, description: str) -> str:
    suffix = sanitize_description(description)
    base = f"BYVoid_OpenCC_{kind}_{number}"
    if suffix:
        return f"{base}_{suffix}"
    return base


def default_opencc_path(root: Path) -> Path:
    candidates = [
        root / "bazel-bin/src/tools/command_line",
        root / "bazel-bin/src/tools/command_line.exe",
        root / "build/python/src/tools/opencc",
        root / "build/Release/opencc",
        root / "build/Release/opencc.exe",
    ]
    for path in candidates:
        if path.exists():
            return path
    return candidates[0]


def default_dict_dir(root: Path) -> Path:
    candidates = [
        root / "bazel-bin/data/config/config_dict_validation_t2tw.runfiles/_main/data/dictionary",
        root / "build/python/data",
        root / "build/Release",
    ]
    for path in candidates:
        if path.is_dir():
            return path
    return candidates[0]


def build_current_artifacts(root: Path) -> None:
    subprocess.run(
        [
            "bazel",
            "build",
            "//src/tools:command_line",
            "//data/config:config_dict_validation_t2tw",
        ],
        cwd=root,
        check=True,
    )


def convert(opencc: Path, config: Path, dict_dir: Path, text: str) -> str:
    result = subprocess.run(
        [str(opencc), "-c", str(config), "--path", str(dict_dir) + "/"],
        input=text,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"{config.stem} conversion failed:\n{result.stderr.strip()}"
        )
    return result.stdout.rstrip("\n")


def load_testcases(path: Path) -> dict:
    import re
    with path.open(encoding="utf-8") as stream:
        content = stream.read()
    # Strip comments (single-line // and multi-line /* */)
    pattern = re.compile(r'"(?:[^"\\]|\\.)*"|(/\*[\s\S]*?\*/|//.*)')
    clean_content = pattern.sub(lambda m: "" if m.group(1) else m.group(0), content)
    return json.loads(clean_content)


def write_testcases(path: Path, data: dict) -> None:
    with path.open("w", encoding="utf-8") as stream:
        json.dump(data, stream, ensure_ascii=False, indent=2)
        stream.write("\n")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate an OpenCC testcase from current conversion output."
    )
    parser.add_argument("--kind", "--pr-or-issue", dest="kind")
    parser.add_argument("--number")
    parser.add_argument("--brief-description", "--brief_description", dest="brief")
    parser.add_argument("--input", dest="input_text")
    parser.add_argument("--testcases", type=Path)
    parser.add_argument("--opencc", type=Path)
    parser.add_argument("--config-dir", type=Path)
    parser.add_argument("--dict-dir", type=Path)
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--replace", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root = repo_root()

    try:
        kind = normalize_kind(prompt_if_missing(args.kind, "PR or Issue: "))
    except ValueError as error:
        print(error, file=sys.stderr)
        return 2

    number = prompt_if_missing(args.number, "Number: ")
    if not number:
        print("Number is required", file=sys.stderr)
        return 2

    brief = prompt_if_missing(args.brief, "brief_description: ")
    input_text = prompt_if_missing(args.input_text, "input text: ")
    if not input_text:
        print("input text is required", file=sys.stderr)
        return 2

    if not args.no_build:
        build_current_artifacts(root)

    testcases_path = args.testcases or root / "test/testcases/testcases.json"
    config_dir = args.config_dir or root / "data/config"
    dict_dir = args.dict_dir or default_dict_dir(root)
    opencc = args.opencc or default_opencc_path(root)

    if not opencc.exists():
        print(f"opencc binary not found: {opencc}", file=sys.stderr)
        return 1
    if not dict_dir.is_dir():
        print(f"dictionary directory not found: {dict_dir}", file=sys.stderr)
        return 1

    expected = {}
    for config in CONFIGS:
        expected[config] = convert(
            opencc=opencc,
            config=config_dir / f"{config}.json",
            dict_dir=dict_dir,
            text=input_text,
        )

    entry = {
        "id": testcase_id(kind, number, brief),
        "input": input_text,
        "expected": expected,
    }

    data = load_testcases(testcases_path)
    cases = data.setdefault("cases", [])
    existing_index = next(
        (index for index, case in enumerate(cases) if case.get("id") == entry["id"]),
        None,
    )
    if existing_index is not None:
        if not args.replace:
            print(
                f"testcase id already exists: {entry['id']} "
                "(use --replace to update it)",
                file=sys.stderr,
            )
            return 1
        cases[existing_index] = entry
    else:
        cases.append(entry)

    if args.dry_run:
        print(json.dumps(entry, ensure_ascii=False, indent=2))
        return 0

    write_testcases(testcases_path, data)
    print(f"Added testcase {entry['id']} to {testcases_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
