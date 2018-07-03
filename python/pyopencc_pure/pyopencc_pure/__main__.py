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

from . opencc_convert import load_config, convert

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(prog = 'pyopencc_pure', description='convert string or file according to rules in config file')
    parser.add_argument('--config', help='json config file name', metavar='CONFIG.json', required=True)
    parser.add_argument('string', help='input text')
    parser.add_argument('--file', help='input file', metavar='Input.txt')
    parser.add_argument('--out', help='output file', metavar='Output.txt')
    args = parser.parse_args()
    if args.string:
        string_in = args.string
    if args.file:
        file_in = open(args.file, 'r', encoding='utf-8')
        file_in.close()
        string_in = file_in.read()
    
    config = load_config(args.config) 
    string_out = convert(config, string_in)
    if args.string:
        print(string_out)
    if args.file and args.out:
        f_out = open(args.out, 'w+', encoding='utf-8',newline='\n' )
        f_out.write(string_out)
        f_out.close()
    if args.file:
        print(string_out)
