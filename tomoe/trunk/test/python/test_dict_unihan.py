# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe
import test_dict

#class TestDictUnihan(tomoe.Dict):
#    def register_char(self, char):
#        return True
#
#    def unregister_char(self, char):
#        return True

class TomoeDictUnihanTest(test_dict.TomoeDictTest):
    def setUp(self):
        self.dict = tomoe.Dict("Unihan")

    def testRegisterChar(self):
        return

    def testUnregisterChar(self):
        return

    def testUnknownReadingSearch(self):
        """
        Unihan dicitionary has no unknown-reading character
        """
        return

    def testUnknownKunReadingSearch(self):
        """
        Unihan dicitionary has no unknown-reading character
        """
        return

    def testOnUnknownReadingSearch(self):
        """
        Unihan dicitionary has no unknown-reading character
        """
        return

    def testKunUnknownReadingSearch(self):
        """
        Unihan dicitionary has no unknown-reading character
        """
        return

# vi:ts=4:nowrap:ai:expandtab
