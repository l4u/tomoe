# -*- coding: UTF=8 -*-
import os
import sys
import glob
import unittest
import test_common
import tomoe

class TomoeConfigTest(unittest.TestCase):

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
        self.config = tomoe.Config(self.config_filename)

    def testGetFilename(self):
        self.assertEqual(self.config_filename, self.config.get_filename())

    def testGetUserDictName(self):
        self.assertEqual('user-dict', self.config.get_user_dict_name())

    def testGetLanguages(self):
        self.assertEqual(sorted(['ja', 'zh_CN']), sorted(self.config.get_languages()))

    def testMakeShelf(self):
        self.assert_(False)

# vi:ts=4:nowrap:ai:expandtab
