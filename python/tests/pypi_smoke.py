import opencc


def _check(actual, expected):
    if actual != expected:
        raise AssertionError('expected {!r}, got {!r}'.format(expected, actual))


def main():
    print('opencc module:', opencc.__file__)
    print('opencc version:', opencc.__version__)
    assert opencc.__version__, '__version__ must be non-empty'

    # Verify that key built-in configs are present in the installed package
    for config in ('s2t', 's2tw', 's2twp', 's2hk', 's2hkp', 't2s', 't2tw', 't2hk'):
        config_file = config + '.json'
        assert (
            config in opencc.CONFIGS or config_file in opencc.CONFIGS
        ), '{} or {} not in opencc.CONFIGS'.format(config, config_file)

    # Simplified → Traditional
    _check(opencc.OpenCC('s2t').convert('汉字'), '漢字')

    # Simplified → Taiwan (with phrase replacement)
    _check(opencc.OpenCC('s2twp').convert('打印机和鼠标'), '印表機和滑鼠')

    # Simplified → Hong Kong
    _check(opencc.OpenCC('s2hk').convert('鼠标'), '鼠標')

    # Traditional → Simplified
    _check(opencc.OpenCC('t2s').convert('開放中文轉換'), '开放中文转换')

    # Taiwan → Simplified: character-variant conversion only (no vocabulary replacement)
    _check(opencc.OpenCC('tw2s').convert('台灣'), '台湾')

    # Taiwan → Simplified: with Mainland vocabulary replacement (tw2sp)
    _check(opencc.OpenCC('tw2sp').convert('滑鼠'), '鼠标')

    # Config stem without .json suffix resolves correctly
    _check(opencc.OpenCC('s2t').convert('汉字'), '漢字')

    print('All checks passed.')


if __name__ == '__main__':
    main()
