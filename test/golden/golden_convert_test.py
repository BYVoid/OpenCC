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
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest import mock


CONFIGS = [
    "s2hk",
    "s2t",
    "s2tw",
    "s2twp",
    "s2twp_jieba",
]

INPUTS = [
    "us_constitution_zhs.txt",
]


def _workspace_root() -> Path:
    return Path(__file__).resolve().parents[2]


class Runfiles:
    def __init__(self) -> None:
        self.manifest = {}
        manifest_path = os.environ.get("RUNFILES_MANIFEST_FILE")
        if manifest_path and Path(manifest_path).is_file():
            with open(manifest_path, encoding="utf-8") as manifest:
                for line in manifest:
                    line = line.rstrip("\n")
                    if not line:
                        continue
                    key, value = self._parse_manifest_line(line)
                    if value:
                        self.manifest[key] = value

        self.runfiles_dirs = [
            Path(value)
            for value in (
                os.environ.get("TEST_SRCDIR"),
                os.environ.get("RUNFILES_DIR"),
            )
            if value
        ]
        self.workspace_names = [
            name
            for name in (
                os.environ.get("TEST_WORKSPACE"),
                "_main",
                "opencc",
            )
            if name
        ]

    @staticmethod
    def _parse_manifest_line(line: str) -> tuple[str, str]:
        if not line.startswith(" "):
            key, _, value = line.partition(" ")
            return key, value

        parts = []
        current = []
        escaped = False
        for char in line[1:]:
            if escaped:
                current.append("\\" + char)
                escaped = False
                continue
            if char == "\\":
                escaped = True
                continue
            if char == " " and len(parts) == 0:
                parts.append("".join(current))
                current = []
                continue
            current.append(char)
        parts.append("".join(current))

        if len(parts) != 2:
            return "", ""
        return tuple(Runfiles._unescape_manifest_field(part) for part in parts)

    @staticmethod
    def _unescape_manifest_field(value: str) -> str:
        return (
            value.replace("\\s", " ")
            .replace("\\n", "\n")
            .replace("\\b", "\\")
        )

    def rlocation(self, relative_path: str) -> Path:
        candidates = []
        for workspace in self.workspace_names:
            candidates.append(f"{workspace}/{relative_path}")
        candidates.append(relative_path)

        for candidate in candidates:
            if candidate in self.manifest:
                return Path(self.manifest[candidate])

        for runfiles_dir in self.runfiles_dirs:
            for candidate in candidates:
                path = runfiles_dir / candidate
                if path.exists():
                    return path

        source_path = _workspace_root() / relative_path
        if source_path.exists():
            return source_path

        raise FileNotFoundError(f"Could not locate runfile: {relative_path}")


def _default_opencc_command(runfiles: Runfiles) -> Path:
    env_value = os.environ.get("OPENCC_COMMAND")
    if env_value:
        return Path(env_value)

    for candidate in [
        "src/tools/command_line.exe",
        "src/tools/command_line",
    ]:
        try:
            command = runfiles.rlocation(candidate)
        except FileNotFoundError:
            continue
        if command.is_file():
            return command

    for command in [
        _workspace_root() / "bazel-bin/src/tools/command_line.exe",
        _workspace_root() / "bazel-bin/src/tools/command_line",
    ]:
        if command.exists():
            return command

    raise FileNotFoundError("Could not locate command_line executable")


def _default_dict_dir(runfiles: Runfiles) -> Path:
    env_value = os.environ.get("OPENCC_DICT_DIR")
    if env_value:
        return Path(env_value)
    try:
        return runfiles.rlocation("data/dictionary/STCharacters.ocd2").parent
    except FileNotFoundError:
        dict_file = _workspace_root() / "bazel-bin/data/dictionary/STCharacters.ocd2"
        if dict_file.exists():
            return dict_file.parent
        raise


def _default_config_dir(runfiles: Runfiles) -> Path:
    env_value = os.environ.get("OPENCC_CONFIG_DIR")
    if env_value:
        return Path(env_value)
    return runfiles.rlocation("data/config/s2t.json").parent


def _default_jieba_config_dir(runfiles: Runfiles) -> Path:
    env_value = os.environ.get("OPENCC_JIEBA_CONFIG_DIR")
    if env_value:
        return Path(env_value)
    return runfiles.rlocation("plugins/jieba/data/config/s2twp_jieba.json").parent


def _default_jieba_plugin_dir(runfiles: Runfiles) -> Path:
    env_value = os.environ.get("OPENCC_JIEBA_PLUGIN_DIR")
    if env_value:
        return Path(env_value)

    candidates = [
        "plugins/jieba/libopencc-jieba.dylib",
        "plugins/jieba/libopencc-jieba.so",
        "plugins/jieba/opencc-jieba.dll",
        "plugins/jieba/opencc-jieba",
    ]
    for candidate in candidates:
        try:
            path = runfiles.rlocation(candidate)
        except FileNotFoundError:
            continue
        if path.is_file():
            return path.parent

    for candidate in [
        _workspace_root() / "bazel-bin/plugins/jieba/libopencc-jieba.dylib",
        _workspace_root() / "bazel-bin/plugins/jieba/libopencc-jieba.so",
    ]:
        if candidate.is_file():
            return candidate.parent

    raise FileNotFoundError("Could not locate opencc-jieba plugin")


def _default_jieba_resource_dir(runfiles: Runfiles) -> Path:
    env_value = os.environ.get("OPENCC_JIEBA_RESOURCE_DIR")
    if env_value:
        return Path(env_value)
    try:
        return runfiles.rlocation("plugins/jieba/jieba_dict/jieba_merged.ocd2").parent.parent
    except FileNotFoundError:
        resource = _workspace_root() / "bazel-bin/plugins/jieba/jieba_dict/jieba_merged.ocd2"
        if resource.exists():
            return resource.parent.parent
        raise


def convert(
    command: Path,
    dict_dir: Path,
    input_path: Path,
    output_path: Path,
    config_path: Path,
    extra_paths: list[Path],
    plugin_dir: Path | None,
) -> None:
    env = os.environ.copy()
    if plugin_dir is not None:
        env["OPENCC_SEGMENTATION_PLUGIN_PATH"] = str(plugin_dir)

    subprocess.run(
        [
            str(command),
            "-i",
            str(input_path),
            "-o",
            str(output_path),
            "-c",
            str(config_path),
            "--path",
            str(dict_dir),
        ]
        + [
            item
            for path in extra_paths
            for item in ("--path", str(path))
        ],
        check=True,
        env=env,
    )


def output_name(input_name: str, config: str) -> str:
    return f"{Path(input_name).stem}.{config}.txt"


class GoldenConvertTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runfiles = Runfiles()
        cls.command = _default_opencc_command(cls.runfiles)
        cls.config_dir = _default_config_dir(cls.runfiles)
        cls.dict_dir = _default_dict_dir(cls.runfiles)
        cls.jieba_config_dir = _default_jieba_config_dir(cls.runfiles)
        cls.jieba_plugin_dir = _default_jieba_plugin_dir(cls.runfiles)
        cls.jieba_resource_dir = _default_jieba_resource_dir(cls.runfiles)

    @classmethod
    def config_path(cls, config: str) -> Path:
        if config.endswith("_jieba"):
            return cls.jieba_config_dir / f"{config}.json"
        return cls.config_dir / f"{config}.json"

    @classmethod
    def extra_paths(cls, config: str) -> list[Path]:
        paths = [cls.config_dir]
        if config.endswith("_jieba"):
            paths.extend([cls.jieba_config_dir, cls.jieba_resource_dir])
        return paths

    @classmethod
    def plugin_dir(cls, config: str) -> Path | None:
        if config.endswith("_jieba"):
            return cls.jieba_plugin_dir
        return None

    def test_golden_outputs(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)
            for input_name in INPUTS:
                input_path = self.runfiles.rlocation(f"test/golden/input/{input_name}")
                for config in CONFIGS:
                    with self.subTest(input=input_name, config=config):
                        actual_path = temp_path / output_name(input_name, config)
                        expected_path = self.runfiles.rlocation(
                            f"test/golden/output/{output_name(input_name, config)}"
                        )
                        convert(
                            self.command,
                            self.dict_dir,
                            input_path,
                            actual_path,
                            self.config_path(config),
                            self.extra_paths(config),
                            self.plugin_dir(config),
                        )
                        expected = expected_path.read_text(encoding="utf-8")
                        actual = actual_path.read_text(encoding="utf-8")
                        if expected != actual:
                            diff = "".join(
                                difflib.unified_diff(
                                    expected.splitlines(keepends=True),
                                    actual.splitlines(keepends=True),
                                    fromfile=str(expected_path),
                                    tofile=str(actual_path),
                                )
                            )
                            self.fail(diff)


class PathResolutionTest(unittest.TestCase):
    def test_default_opencc_command_accepts_windows_exe_runfile(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            command = Path(temp_dir) / "command_line.exe"
            command.write_text("", encoding="utf-8")
            runfiles = Runfiles()
            runfiles.manifest = {
                "_main/src/tools/command_line.exe": str(command),
            }
            runfiles.runfiles_dirs = []
            runfiles.workspace_names = ["_main"]

            with mock.patch.dict(os.environ, {}, clear=True):
                self.assertEqual(command, _default_opencc_command(runfiles))

    def test_parse_manifest_line_unescapes_escaped_entries(self) -> None:
        key, value = Runfiles._parse_manifest_line(
            r" _main/src/tools/command_line.exe C:\btmp\bpath\swith\sspaces\bbin\bcommand_line.exe"
        )
        self.assertEqual("_main/src/tools/command_line.exe", key)
        self.assertEqual(r"C:\tmp\path with spaces\bin\command_line.exe", value)


def update_outputs() -> None:
    runfiles = Runfiles()
    command = _default_opencc_command(runfiles)
    config_dir = _default_config_dir(runfiles)
    dict_dir = _default_dict_dir(runfiles)
    jieba_config_dir = _default_jieba_config_dir(runfiles)
    jieba_plugin_dir = _default_jieba_plugin_dir(runfiles)
    jieba_resource_dir = _default_jieba_resource_dir(runfiles)
    output_dir = _workspace_root() / "test/golden/output"
    output_dir.mkdir(parents=True, exist_ok=True)

    for input_name in INPUTS:
        input_path = _workspace_root() / "test/golden/input" / input_name
        expected_names = {output_name(input_name, config) for config in CONFIGS}
        for stale_path in output_dir.glob(f"{input_path.stem}.*.txt"):
            if stale_path.name not in expected_names:
                stale_path.unlink()

        for config in CONFIGS:
            if config.endswith("_jieba"):
                config_path = jieba_config_dir / f"{config}.json"
                extra_paths = [config_dir, jieba_config_dir, jieba_resource_dir]
                plugin_dir = jieba_plugin_dir
            else:
                config_path = config_dir / f"{config}.json"
                extra_paths = [config_dir]
                plugin_dir = None
            output_path = output_dir / output_name(input_name, config)
            with tempfile.NamedTemporaryFile(delete=False) as temp_file:
                temp_path = Path(temp_file.name)
            try:
                convert(
                    command,
                    dict_dir,
                    input_path,
                    temp_path,
                    config_path,
                    extra_paths,
                    plugin_dir,
                )
                shutil.move(str(temp_path), output_path)
                output_path.chmod(0o644)
            finally:
                if temp_path.exists():
                    temp_path.unlink()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--update",
        action="store_true",
        help="regenerate files under test/golden/output",
    )
    args, remaining = parser.parse_known_args()

    if args.update:
        update_outputs()
    else:
        unittest.main(argv=[parser.prog] + remaining)


if __name__ == "__main__":
    main()
