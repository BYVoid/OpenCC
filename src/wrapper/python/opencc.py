#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ctypes import *
import sys

class opencc:
	def __init__(self, convert_direction):
		self.libopencc = cdll.LoadLibrary("libopencc.so")
		self.od = self.libopencc.opencc_open(c_int(convert_direction))
		self.libc = cdll.LoadLibrary("libc.so.6")
	
	def __del__(self):
		self.libopencc.opencc_close(self.od)
	
	def convert(self, text):
		text_c = c_char_p(text)
		len_c = c_int(len(text))
		retv_c = c_char_p(self.libopencc.opencc_convert_utf8(self.od, text_c, len_c))
		str_buffer = retv_c.value
		self.libc.free(retv_c);
		return str_buffer
	
	def dict_load(self, filename, dicttype):
		filename_c = c_char_p(filename)
		dicttype_c = c_int(dicttype)
		retv = c_int(self.libopencc.opencc_dict_load(self.od, filename_c, dicttype_c));
		return retv

if __name__ == "__main__":
	converter = opencc(0)
	fp = sys.stdin
	text = fp.read()
	fp.close()
	print converter.convert(text)
