#!/usr/bin/env python
# -*- coding: UTF=8 -*-
import glob
import os
import sys
import unittest
import test_common

SKIP_FILES = ['runtests', 'test_dict', 'test_common']

dir = os.path.split(os.path.abspath(__file__))[0]
os.chdir(dir)

def gettestnames():
    files = glob.glob('*.py')
    names = map(lambda x: x[:-3], files)
    map(names.remove, SKIP_FILES)
    return names

suite = unittest.TestSuite()
loader = unittest.TestLoader()

for name in gettestnames():
    suite.addTest(loader.loadTestsFromName(name))

testRunner = unittest.TextTestRunner(verbosity=1)
testRunner.run(suite)

# vi:ts=4:nowrap:ai:expandtab
