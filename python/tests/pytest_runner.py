import os
import sys

import pytest


def _test_path(test_dir: str, arg: str) -> str:
    if os.path.isabs(arg):
        return arg
    return os.path.join(test_dir, arg)


def main() -> int:
    test_dir = os.path.dirname(os.path.abspath(__file__))
    test_paths = [_test_path(test_dir, arg) for arg in sys.argv[1:]]
    if not test_paths:
        test_paths = [test_dir]

    return pytest.main([
        *test_paths,
        f'--rootdir={test_dir}',
    ])


if __name__ == '__main__':
    sys.exit(main())
