# coding: utf-8
# Adapted from
# https://github.com/lepture/opencc-python/blob/master/opencc.py
# credit goes to Hsiaoming Yang <me@lepture.com>

from __future__ import absolute_import, unicode_literals

import os
import sys
from ctypes import CDLL, c_char_p, c_size_t, c_void_p, cast
from ctypes.util import find_library

if sys.version_info[0] == 3:
    text_type = str
else:
    text_type = unicode  # noqa

__all__ = ['CONFIGS', 'convert', 'OpenCC']

_libcfile = find_library('c') or 'libc.so.6'
libc = CDLL(_libcfile, use_errno=True)

_thisdir = os.path.dirname(os.path.abspath(__file__))
_libopenccfile = os.path.join(_thisdir, 'clib', 'lib', 'libopencc.so.2')
assert os.path.isfile(_libopenccfile), 'Build package library first'
libopencc = CDLL(_libopenccfile, use_errno=True)

libc.free.argtypes = [c_void_p]

libopencc.opencc_open.restype = c_void_p
libopencc.opencc_convert_utf8.argtypes = [c_void_p, c_char_p, c_size_t]
libopencc.opencc_convert_utf8.restype = c_void_p
libopencc.opencc_close.argtypes = [c_void_p]

_opencc_share_dir = os.path.join(_thisdir, 'clib', 'share', 'opencc')
CONFIGS = [
    os.path.splitext(f)[0]
    for f in os.listdir(_opencc_share_dir)
    if f.endswith('.json')
]


class OpenCC(object):

    def __init__(self, config='t2s.json'):
        self._od = libopencc.opencc_open(c_char_p(config.encode('utf-8')))

    def convert(self, text):
        if isinstance(text, text_type):
            # use bytes
            text = text.encode('utf-8')

        retv_i = libopencc.opencc_convert_utf8(self._od, text, len(text))
        if retv_i == -1:
            raise Exception('OpenCC Convert Error')
        retv_c = cast(retv_i, c_char_p)
        value = retv_c.value
        libc.free(retv_c)
        return value.decode('utf-8')

    def __del__(self):
        libopencc.opencc_close(self._od)


def convert(text, config='t2s.json'):
    cc = OpenCC(config)
    return cc.convert(text)
