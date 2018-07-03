
from . opencc_convert import load_config, convert

import time
import sys
if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='convert string or file according to rules in config file')
    parser.prog = 'pyopencc_pure'
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


