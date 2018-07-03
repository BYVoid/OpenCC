#!/usr/bin/env python
#coding: utf-8
# Open Chinese Convert
#
# Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
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

import os
this_dir, this_filename = os.path.split(__file__)
dict_path=os.path.join(this_dir, 'data\\dictionary\\')

def load_dict(dict_type):
    # only Txt format is supported
    dict_file = dict_type['file']
    dict_file = dict_file.split('.', 1)
    f = open(dict_path+dict_file[0]+'.txt', 'r', encoding='utf-8')
    dict_ret = dict()
    for line in f:
        chars = line.split('\t')
        if chars:
            char_to = chars[1].split(' ', 1)
            dict_ret[chars[0]] = char_to[0].replace('\n','')
    return dict_ret

import json
config_path=os.path.join(this_dir, 'data\\config\\')

def load_config(config_file):
    f = open(config_path+config_file, 'r', encoding='utf-8')
    config = json.load(f)
    # put python dict data in key 'table' 
    config['segmentation']['dict']['table'] = load_dict(config['segmentation']['dict'])

    for chain in config['conversion_chain']:
        if chain['dict']['type'] == 'group':
            for dict_i in chain['dict']['dicts']:
                # put python dict data in key 'table'
                dict_i['table'] = load_dict(dict_i)
        elif chain['dict']['type'] == 'ocd':
            # put python dict data in key 'table'
            chain['dict']['table'] =  load_dict(chain['dict'])
    return config

import re
def segment(config, string):
    #  only Max Match Segmentation are implimented
    segkeys = config['segmentation']['dict']['table'].keys()
    segs = list()
    for key in segkeys:
        matches_iter = re.finditer(key, string)
        matches = [match for match in matches_iter]
        segs = segs + matches
    segs = sorted(segs, key= lambda x: x.start() )
    return segs

def replace(string, table):
    segs = list()
    for key in table.keys():
        matches_iter = re.finditer(key, string)
        matches = [match for match in matches_iter]
        segs = segs + matches
    segs = sorted(segs, key= lambda x: x.start() )
    string_out = ''
    last_end = 0
    if segs:
        for seg in segs:
            string_out = string_out + string[last_end:seg.start()] + table[seg.group()]
            last_end = seg.end()
        string_out = string_out + string[last_end:]
        return string_out
    else:
        return string

def convert(config, string):
    for chain in config['conversion_chain']:
        if chain['dict']['type'] == 'group':
            for dict_i in chain['dict']['dicts']:
                table = dict_i['table']
                string = replace(string, table) 
        elif chain['dict']['type'] == 'ocd':
            table = chain['dict']['table']
            string = replace(string, table)
    return string

import time
import sys
if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='load config')
    parser.add_argument('--config', help='config file name')
    parser.add_argument('--string', help='input text')
    parser.add_argument('--file', help='input file')
    parser.add_argument('--out', help='output file')
    args = parser.parse_args()
    if args.string:
        string_in = args.string
    if args.file:
        file_in = open(args.file, 'r', encoding='utf-8')
        file_in.close()
        string_in = file_in.read()
    
    # t0 = time.time()
    config = load_config(args.config) 
    # t1 = time.time()
    string_out = convert(config, string_in)
    # t2 = time.time()
    if args.string:
        print(string_out)
    if args.file and args.out:
        f_out = open(args.out, 'w+', encoding='utf-8',newline='\n' )
        f_out.write(string_out)
        f_out.close()
    if args.file:
        print(string_out)
    # print(t1-t0, t2-t1)


