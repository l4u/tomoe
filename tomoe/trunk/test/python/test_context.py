# encoding: utf-8
import os
import sys
import shutil
import glob
import unittest
import test_common
import tomoe

class TomoeContextTest(unittest.TestCase):

    def setUp(self):
        test_dict_name = os.path.join(test_common.test_data_dir, 'test-dict.xml')
        self.dict_name = "tomoe-test-xmldict.xml"
        shutil.copy(test_dict_name, self.dict_name)
 
        self.config_filename = "test-config"
        config_file = open(self.config_filename, "w")
        contents = """
[config]
use-system-dictionaries = false
user-dictionary = test
languages = ja;zh_CN

[test-dictionary]
type = xml
file = %s
         """ % (self.dict_name)
        config_file.write(contents)
        config_file.close()

    def tearDown(self):
        if os.access(self.config_filename, os.F_OK):
            os.unlink(self.config_filename)

        if os.access(self.dict_name, os.F_OK):
            os.unlink(self.dict_name)

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
