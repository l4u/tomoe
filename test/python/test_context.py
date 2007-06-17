# encoding: utf-8
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
use-system-dictionaries = false
user-dictionary = user-dict
languages = ja;zh_CN

[test-dictionary]
type = xml
file = test-dict.xml
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

        char_code = '池'
        tomoe_char = context.get_char(char_code)
        self.assertEqual(char_code, tomoe_char.get_utf8())

    def testRegister(self):
        context = tomoe.Context()
        context.load_config(self.config_filename)

        char_code = '地'
        tomoe_char = context.get_char(char_code)
        self.assertEqual(None, tomoe_char)

        tomoe_char = tomoe.Char(char_code)
        self.assert_(context.register(tomoe_char))

        tomoe_char = context.get_char(char_code)
        self.assertNotEqual(None, tomoe_char)
        self.assertEqual(char_code, tomoe_char.get_utf8())

    def testUnregister(self):
        context = tomoe.Context()
        context.load_config(self.config_filename)

        char_code = '池'
        tomoe_char = context.get_char(char_code)
        self.assertEqual(char_code, tomoe_char.get_utf8())

        self.assert_(context.unregister(char_code))
        self.assertEqual(None, context.get_char(char_code))

# vi:ts=4:nowrap:ai:expandtab
