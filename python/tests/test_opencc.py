import json
import os
import pytest
import sys

_this_dir = os.path.dirname(os.path.abspath(__file__))
_opencc_rootdir = os.path.abspath(os.path.join(_this_dir, '..', '..'))
_testcases_path = os.path.join(_opencc_rootdir, 'test', 'testcases', 'testcases.json')


def test_import():
    import opencc  # noqa


def test_init_delete_converter():
    import opencc

    for config in opencc.CONFIGS:
        converter = opencc.OpenCC(config)
        del converter


def test_conversion():
    import opencc
    import re

    with open(_testcases_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Strip comments (single-line // and multi-line /* */)
    pattern = re.compile(r'"(?:[^"\\]|\\.)*"|(/\*[\s\S]*?\*/|//.*)')
    clean_content = pattern.sub(lambda m: "" if m.group(1) else m.group(0), content)
    parsed = json.loads(clean_content)

    for case in parsed.get('cases', []):
        input_text = case.get('input')
        expected = case.get('expected', {})
        if not input_text or not isinstance(expected, dict):
            continue
        for cfg, ans in expected.items():
            converter = opencc.OpenCC(f'{cfg}.json')
            assert converter.convert(input_text) == ans, \
                'Failed to convert {} for {} -> {}'.format(cfg, input_text, ans)


if __name__ == "__main__":
    sys.exit(pytest.main(sys.argv[1:]))
