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

    def testUnknownKunReadingSearch(self):
        char_codes = [ '池', '垳', '汪', '溏', '潢' ]
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        for char, candidate in zip(sorted(char_codes), sorted(self.dict.search(tomoe_query))):
            self.assertEqual(candidate.get_char().get_utf8(), char)

    def testKunReadingSearch(self):
        char_codes = [ '池', '垳', '汪', '溏', '潢' ]
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_KUN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        for char, candidate in zip(sorted(char_codes), sorted(self.dict.search(tomoe_query))):
            self.assertEqual(candidate.get_char().get_utf8(), char)

    def testOnReadingSearch(self):
        char_codes = [ '垳' ]
        reading = 'ガケ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        for char, candidate in zip(sorted(char_codes), sorted(self.dict.search(tomoe_query))):
            self.assertEqual(candidate.get_char().get_utf8(), char)

    def testUnknownOnReadingSearch(self):
        char_codes = [ '垳' ]
        reading = 'ガケ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        for char, candidate in zip(sorted(char_codes), sorted(self.dict.search(tomoe_query))):
            self.assertEqual(candidate.get_char().get_utf8(), char)

# vi:ts=4:nowrap:ai:expandtab
