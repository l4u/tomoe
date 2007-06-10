# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe
import test_dict

class TomoeDictEstTest(test_dict.TomoeDictTest):
    def setUp(self):
        self.dict = tomoe.Dict("Est", editable = True)

# vi:ts=4:nowrap:ai:expandtab
