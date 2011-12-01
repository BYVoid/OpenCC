#coding: utf-8
import sys
from common import find_target_items

if len(sys.argv) != 3:
	print "Usage: ", sys.argv[0], "[input] [keyword]"
	exit(1)

find_target_items(sys.argv[1], sys.argv[2])
