# coding: utf-8
# Adapted from
# https://github.com/lepture/opencc-python/blob/master/opencc.py
# credit goes to Hsiaoming Yang <me@lepture.com>

from __future__ import absolute_import, unicode_literals

import os
import platform
import sys
from ctypes import CDLL, c_char_p, c_size_t, c_void_p, cast

try:
    from opencc.version import __version__  # noqa
except ImportError:
    pass

if sys.version_info[0] == 3:
    text_type = str
else:
    text_type = unicode  # noqa

__all__ = ['CONFIGS', 'convert', 'OpenCC']


_thisdir = os.path.dirname(os.path.abspath(__file__))
_system = platform.system()
if _system == 'Darwin':
    _libopenccfilename = 'libopencc.1.1.dylib'
elif _system == 'Linux':
    _libopenccfilename = 'libopencc.so.1.1'
elif _system == 'Windows':
    _libopenccfilename = 'opencc.dll'
else:
    raise NotImplementedError('Not tested for {}'.format(_system))

if _system == 'Windows':
    _libopenccfile = os.path.join(_thisdir, 'clib', 'bin', _libopenccfilename)
else:
    _libopenccfile = os.path.join(_thisdir, 'clib', 'lib', _libopenccfilename)

libopencc = None
if os.path.isfile(_libopenccfile):
    libopencc = CDLL(_libopenccfile, use_errno=True)
    libopencc.opencc_open.restype = c_void_p
    libopencc.opencc_convert_utf8.argtypes = [c_void_p, c_char_p, c_size_t]
    libopencc.opencc_convert_utf8.restype = c_void_p
    libopencc.opencc_convert_utf8_free.argtypes = [c_char_p]
    libopencc.opencc_convert_utf8_free.restype = c_void_p
    libopencc.opencc_close.argtypes = [c_void_p]
    libopencc.opencc_error.argtypes = []
    libopencc.opencc_error.restype = c_char_p

_opencc_share_dir = os.path.join(_thisdir, 'clib', 'share', 'opencc')
CONFIGS = []
if os.path.isdir(_opencc_share_dir):
    CONFIGS = [f for f in os.listdir(_opencc_share_dir) if f.endswith('.json')]


class OpenCC(object):

    def __init__(self, config='t2s.json'):
        assert libopencc is not None, \
            'Build package library first'

        if not config.endswith('.json'):
            config += '.json'

        if not os.path.isfile(config):
            config = os.path.join(_opencc_share_dir, config)

        if not os.path.isfile(config):
            raise ValueError('Could not find file at {}'.format(config))

        od = libopencc.opencc_open(c_char_p(config.encode('utf-8')))
        if cast(od, c_void_p) == -1:
            error = libopencc.opencc_error()
            raise Exception(error.value)

        self._od = od

    def convert(self, text):
        if isinstance(text, text_type):
            # use bytes
            text = text.encode('utf-8')

        retv_i = libopencc.opencc_convert_utf8(self._od, text, len(text))
        if retv_i == -1:
            error = libopencc.opencc_error()
            raise Exception(error.value)

        retv_c = cast(retv_i, c_char_p)
        value = retv_c.value
        libopencc.opencc_convert_utf8_free(retv_c)
        return value.decode('utf-8')

    def __del__(self):
        if hasattr(self, '_od'):
            libopencc.opencc_close(self._od)
