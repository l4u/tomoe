# -*- coding: UTF=8 -*-
import os
import sys
import glob
import unittest
import test_common
import tomoe

class TomoeContextTest(unittest.TestCase):

    def setUp(self):
        self.config_filename = "test-config"
        config_file = open(self.config_filename, "w")
        contents = """
[config]
user-dictionary = user-dict
languages = ja;zh_CN
         """
        config_file.write(contents)
        config_file.close()

    def tearDown(self):
        if os.access(self.config_filename, os.F_OK):
            os.unlink(self.config_filename)

    def testLoadConfig(self):
        context = tomoe.Context()
        context.load_config(self.config_filename)
        self.assert_(False)

    def testSearch(self):
        self.assert_(False)

    def testGetChar(self):
        context = tomoe.Context()
        context.load_config(self.config_filename)

        char = context.get_char('池')
        self.assertEqual('池', char.get_utf8())

    def testRegister(self):
        self.assert_(False)

    def testUnregister(self):
        self.assert_(False)

# vi:ts=4:nowrap:ai:expandtab
