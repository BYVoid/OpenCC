#coding: utf-8
import sys
from common import reverse_items

if len(sys.argv) != 3:
	print "Usage: ", sys.argv[0], "[input] [output]"
	exit(1)

reverse_items(sys.argv[1], sys.argv[2])

