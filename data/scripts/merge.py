#!/usr/bin/env python
#coding: utf-8
import codecs
import sys
from common import sort_items

if len(sys.argv) < 4:
  print("Merge and sort all text dictionaries")
  print(("Usage: ", sys.argv[0], "[input1] [input2] ... [inputN] [output]"))
  exit(1)

all_lines = []
for i in range(1, len(sys.argv) - 1):
  input_file = codecs.open(sys.argv[i], "r", encoding="utf-8")
  for line in input_file:
    all_lines += line
  input_file.close()
  all_lines += '\n'

output_filename = sys.argv[-1]
output_file = open(output_filename, "wb")
for line in all_lines:
  output_file.write(line.encode('utf-8'))
output_file.close()

sort_items(output_filename, output_filename)
