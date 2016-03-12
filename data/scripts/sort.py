#!/usr/bin/env python
#coding: utf-8
import sys
from common import sort_items

if len(sys.argv) < 2:
  print("Sort the dictionary")
  print(("Usage: ", sys.argv[0], "[input] ([output])"))
  exit(1)

input = sys.argv[1]

if len(sys.argv) < 3:
  output = input
else:
  output = sys.argv[2]

sort_items(input, output)
