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
import unittest

import os
import pyopencc_pure
test_dir = '..\\..\\test\\testcases'

class OpenCCTestCase(unittest.TestCase):
    pass

def test_generator(config_name):
    def test_convert(self):
        config = pyopencc_pure.load_config(config_name+'.json')

        input_file_name = os.path.join(test_dir, config_name+'.in')
        with open(input_file_name,'r', encoding='utf-8') as input_file:
            output = pyopencc_pure.convert(config, input_file.read())

        out_file_name = os.path.join(test_dir, config_name+'.out')
        with open(out_file_name,'w+', encoding='utf-8') as out_file:
            out_file.write(output)

        ans_file_name = os.path.join(test_dir, config_name+'.ans')
        with open(ans_file_name,'r', encoding='utf-8') as ans_file:
            ans = ans_file.read()

        self.assertEqual(output, ans)
    return test_convert

if __name__ == '__main__':
    test_configs = ["s2t", "t2s", "s2tw", "s2twp", "tw2s","tw2sp", "s2hk", "hk2s"]
    suite = unittest.TestSuite()
    for config_name in test_configs:
        test_name = 'test_{}'.format(config_name)
        test = test_generator(config_name)
        setattr(OpenCCTestCase, test_name, test)
        suite.addTest(OpenCCTestCase(test_name))
    runner = unittest.TextTestRunner()
    runner.run(suite)
