# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe
import test_dict

class TomoeDictXMLTest(test_dict.TomoeDictTest):
    def setUp(self):
        self.dict_name = "tomoe-test-xmldict.xml"
        self.dict = tomoe.Dict("XML", filename = self.dict_name, editable = True)

    def tearDown(self):
        self.dict.flush()
        self.dict = None
        if os.access(self.dict_name, os.F_OK):
            os.unlink(self.dict_name)

# vi:ts=4:nowrap:ai:expandtab
