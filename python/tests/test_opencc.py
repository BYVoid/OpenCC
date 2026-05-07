import json
import os
import pytest
import sys

_this_dir = os.path.dirname(os.path.abspath(__file__))
_opencc_rootdir = os.path.abspath(os.path.join(_this_dir, '..', '..'))
_testcases_path = os.path.join(_opencc_rootdir, 'test', 'testcases', 'testcases.json')
_config_test_path = os.path.join(_opencc_rootdir, 'test', 'config_test', 'config_test.json')


def test_import():
    import opencc  # noqa


def test_init_delete_converter():
    import opencc

    for config in opencc.CONFIGS:
        converter = opencc.OpenCC(config)
        del converter


def test_jieba_configs_are_not_exposed():
    import opencc

    assert all('jieba' not in config for config in opencc.CONFIGS)


def test_custom_config_resolves_local_dictionaries():
    import opencc

    converter = opencc.OpenCC(_config_test_path)

    assert converter.convert('燕燕于飞，之子于归。') == '燕燕于飛，之子于歸。'
    assert converter.convert('远') == '遠'


def test_custom_config_resolves_absolute_ocd2_path_to_text_dictionary(tmp_path):
    import opencc

    dict_txt = tmp_path / 'CustomPhrases.txt'
    dict_txt.write_text('测试\t測試\n', encoding='utf-8')
    config_path = tmp_path / 'custom.json'
    config_path.write_text(json.dumps({
        'name': 'Custom',
        'segmentation': {
            'type': 'mmseg',
            'dict': {
                'type': 'ocd2',
                'file': str(tmp_path / 'CustomPhrases.ocd2'),
            },
        },
        'conversion_chain': [{
            'dict': {
                'type': 'ocd2',
                'file': str(tmp_path / 'CustomPhrases.ocd2'),
            },
        }],
    }), encoding='utf-8')

    converter = opencc.OpenCC(str(config_path))

    assert converter.convert('测试') == '測試'


def test_dictionary_parser_strips_utf8_bom(tmp_path):
    from opencc import _opencc_pure

    dict_txt = tmp_path / 'BomPhrases.txt'
    dict_txt.write_text('\ufeff测试\t測試\n', encoding='utf-8')
    trie = _opencc_pure._load_trie(str(dict_txt), reverse=False)

    assert trie.prefix_match('测试', 0) == (2, '測試')


def test_conversion():
    import opencc

    with open(_testcases_path, 'r', encoding='utf-8') as f:
        parsed = json.load(f)

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
