#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import glob
import sys

from common import sort_items

if len(sys.argv) < 2:
    print("Sort the dictionary")
    print(("Usage: ", sys.argv[0], "[directory]"))
    exit(1)

directory = sys.argv[1]
files = glob.glob(directory + "/*.txt")
for filename in files:
    print(filename)
    sort_items(filename, filename)
