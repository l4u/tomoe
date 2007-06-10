# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe
import test_dict

class TomoeDictUnihanTest(test_dict.TomoeDictTest):
    def setUp(self):
        self.dict = tomoe.Dict("Unihan")

# vi:ts=4:nowrap:ai:expandtab
