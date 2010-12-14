#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ctypes import cast, cdll, c_char_p, c_int, c_size_t, c_void_p
from ctypes.util import find_library
import sys

class ConvertError(Exception):
	pass

class DictType:
	TEXT,DATRIE = 0,1

class OpenCC:

	def __init__(self, config=None, verbose=True):
		self.libopencc = cdll.LoadLibrary(find_library('opencc'))
		self.libopencc.opencc_open.restype = c_void_p
		self.libopencc.opencc_convert_utf8.argtypes = [c_void_p, c_char_p, c_size_t]
		# for checking for the returned '-1' pointer in case opencc_convert() fails.
		# c_char_p always tries to convert the returned (char *) to a Python string,
		self.libopencc.opencc_convert_utf8.restype = c_void_p
		self.libopencc.opencc_close.argtypes = [c_void_p]
		self.libopencc.opencc_perror.argtypes = [c_char_p]
		self.libopencc.opencc_dict_load.argtypes = [c_void_p, c_char_p, c_int]

		self.libc = cdll.LoadLibrary(find_library('c'))
		self.libc.free.argtypes = [c_void_p]

		self.config = config
		self.verbose = verbose
		self.od = None
	
	def __enter__(self):
		if self.config is None:
			self.od = self.libopencc.opencc_open(0)
		else:
			self.od = self.libopencc.opencc_open(c_char_p(self.config))
		return self

	def __exit__(self, type, value, traceback):
		self.libopencc.opencc_close(self.od)
		self.od = None

	def __perror(self, message):
		if self.verbose:
			self.libopencc.opencc_perror(message)
	
	def convert(self, text):
		retv_c = self.libopencc.opencc_convert_utf8(self.od, text, len(text))
		if retv_c == -1:
			self.__perror('OpenCC error:')
			raise ConvertError()
		retv_c = cast(retv_c, c_char_p)
		str_buffer = retv_c.value
		self.libc.free(retv_c);
		return str_buffer
	
	def dict_load(self, filename, dicttype):
		retv = self.libopencc.opencc_dict_load(self.od, filename, dicttype)
		if retv == -1:
			self.__perror('OpenCC error:')
		return retv

if __name__ == "__main__":
	with sys.stdin as fp:
		text = fp.read()
	with OpenCC() as converter:
		for path in ['simp_to_trad_characters.ocd',
			     'simp_to_trad_phrases.ocd']:
			converter.dict_load(path, DictType.DATRIE)
		print converter.convert(text)
