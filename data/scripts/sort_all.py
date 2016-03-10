#!/usr/bin/env python
#coding: utf-8
import glob
import sys
from common import sort_items

if len(sys.argv) < 2:
  print("Sort the dictionary")
  print(("Usage: ", sys.argv[0], "[directory]"))
  exit(1)

dirtectory = sys.argv[1]
files = glob.glob(dirtectory + "/*")
for filename in files:
  print(filename)
  sort_items(filename, filename)
