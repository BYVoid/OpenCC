#!/usr/bin/env python
# coding=utf8
#
# Open Chinese Convert
#
# Copyright 2010 BYVoid <byvoid1@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import sys

def assign_map(records, maxlen):
	new_records = []
	for lens in range(1, maxlen + 1):
		for simp,trads in records:
			if len(simp) != lens:
				continue
			if len(trads) == 1:
				new_records.append( (simp, trads[0]) )
			else:
				freq = list()
				for i in range(0,len(trads)):
					trad = trads[i]
					freq.append(0)
					for sc,tcs in records:
						pos = sc.find(simp)
						if pos == -1:
							continue
						for tc in tcs:
							post = tc.find(trad)
							if pos != post:
								continue
							freq[i] += 1
				
				maxf = 0
				maxt = ""
				
				for i in range(0,len(trads)):
					trad = trads[i]
					if freq[i] > maxf:
						maxf = freq[i]
						maxt = trad

				new_records.append( (simp, maxt) )
	return new_records

def convert(s, d, n):
	out = u""
	end = len(s)
	begin = 0
	while begin < end:
		for i in range(min(n, end - begin), 0, -1):
			t = s[begin:begin+i]
			t = d.get(t, t if i == 1 else None)
			if t:
				break
		out = out + t
		begin += i
	return out

def filter_more(records, n):
	han = filter(lambda (k, v): len(k) <= n, records)
	hand = dict(han)
	hanm = filter(lambda (k, v): convert(k, hand, n) != v, records)
	return hanm + han

def have_extb(cstr):
	for i in range(0,len(cstr)):
		ch = cstr[i]
		if ord(ch) > 65535:
			return True
	return False

def get_records():
	pfile = sys.stdin
	items = pfile.readlines()
	records = []
	for item in items:
		item = unicode(item,"utf8")
		item = (item.split('\n'))[0].split('\t')
		sc = item[0]
		tc = item[1]
		if have_extb(sc) or have_extb(tc):
			continue;
		tc = tc.split(' ')
		records.append( (sc,tc) )
	
	maxlen = max(map(lambda (k,v): len(k), records))
	
	records = assign_map(records, maxlen)
	
	for i in range(1,  maxlen - 1):
		records = filter_more(records, i)

	records.sort()
	
	return maxlen,records

def main():

	maxlen, records = get_records()
	for sc, tc in records:
		print sc.encode("utf8") + "\t" + tc.encode("utf8")


if __name__ == "__main__":
	main()

