from __future__ import absolute_import, unicode_literals

import os
import platform
import sys

__all__ = ['OpenCC', 'CONFIGS']

_thisdir = os.path.dirname(os.path.abspath(__file__))
_opencc_lib_dir = os.path.join(_thisdir, 'clib', 'lib')
_opencc_share_dir = os.path.join(_thisdir, 'clib', 'share', 'opencc')

if sys.version_info.major == 2:
    text_type = unicode  # noqa
else:
    text_type = str

if os.path.isdir(_opencc_share_dir):
    CONFIGS = [f for f in os.listdir(_opencc_share_dir) if f.endswith('.json')]
else:
    CONFIGS = []


def _append_path_to_env(name, path):
    value = os.environ.get(name, '')
    if path in value:  # Path already exists
        return
    if value == '':
        value = path
    else:
        value += ':' + path
    os.environ[name] = value


def _import_opencc_clib():
    _system = platform.system()
    if _system == 'Darwin':  # Mac OS X
        _append_path_to_env('DYLD_LIBRARY_PATH', _opencc_lib_dir)
    elif _system == 'Linux':
        _append_path_to_env('LD_LIBRARY_PATH', _opencc_lib_dir)
    else:
        raise NotImplementedError('Not tested for {}'.format(_system))

    from opencc.clib import opencc_clib
    return opencc_clib


opencc_clib = _import_opencc_clib()


class OpenCC(opencc_clib._OpenCC):

    def __init__(self, config):
        if not config.endswith('.json'):
            config += '.json'
        if not os.path.isfile(config):
            config = os.path.join(_opencc_share_dir, config)
        super(OpenCC, self).__init__(config)

    def convert(self, text):
        if isinstance(text, text_type):
            text = text.encode('utf-8')
        return super(OpenCC, self).convert(text, len(text))
