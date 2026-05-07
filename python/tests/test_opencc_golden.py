import difflib
from pathlib import Path

import pytest

import opencc

_this_dir = Path(__file__).resolve().parent
_opencc_rootdir = _this_dir.parents[1]
_golden_input_dir = _opencc_rootdir / 'test' / 'golden' / 'input'
_golden_output_dir = _opencc_rootdir / 'test' / 'golden' / 'output'

_GOLDEN_INPUTS = [
    'us_constitution_zhs.txt',
]


def _available_python_golden_cases():
    configs = [
        config[:-5] if config.endswith('.json') else config
        for config in opencc.CONFIGS
        if 'jieba' not in config
    ]
    cases = []
    for input_name in _GOLDEN_INPUTS:
        input_stem = Path(input_name).stem
        for config in configs:
            expected_path = _golden_output_dir / f'{input_stem}.{config}.txt'
            if expected_path.is_file():
                cases.append((input_name, config, expected_path))
    return cases


_GOLDEN_CASES = _available_python_golden_cases()


def test_python_golden_cases_are_available():
    assert _GOLDEN_CASES


@pytest.mark.parametrize(
    'input_name,config,expected_path',
    _GOLDEN_CASES,
)
def test_python_matches_project_golden_output(input_name, config, expected_path):
    input_path = _golden_input_dir / input_name
    source_text = input_path.read_text(encoding='utf-8')
    expected = expected_path.read_text(encoding='utf-8')

    actual = opencc.OpenCC(config).convert(source_text)

    assert actual == expected, ''.join(
        difflib.unified_diff(
            expected.splitlines(keepends=True),
            actual.splitlines(keepends=True),
            fromfile=str(expected_path),
            tofile=f'opencc.OpenCC({config!r})',
        )
    )
