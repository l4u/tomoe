# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe

class TomoeDictTest(unittest.TestCase):

    def testRegisterChar(self):
        char_code = '池'

        tomoe_char = self.dict.get_char(char_code)
        self.assertEqual(tomoe_char, None)

        tomoe_char = tomoe.Char(char_code)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_char = self.dict.get_char(char_code)
        self.assertNotEqual(tomoe_char, None)
        self.assertEqual(tomoe_char.get_utf8(), char_code)

    def testUnregisterChar(self):
        char_code = '池'
        tomoe_char = tomoe.Char(char_code)

        self.assert_(self.dict.register_char(tomoe_char))
        self.assert_(self.dict.unregister_char(char_code))

        self.assertEqual(self.dict.get_char(char_code), None)

    def testGetExistChar(self):
        char_code = '池'
        n_strokes = 6
        tomoe_char = tomoe.Char(utf8 = char_code, n_strokes = n_strokes)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_char = self.dict.get_char(char_code)
        self.assertEqual(tomoe_char.get_utf8(), char_code)
        self.assertEqual(tomoe_char.get_n_strokes(), n_strokes)

    def testFailGetExistChar(self):
        char_code = '池'
        self.assertEqual(self.dict.get_char(char_code), None)

    def testReadingSearch(self):
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)

        char_code = '池'
        tomoe_char = tomoe.Char(char_code)
        tomoe_char.add_reading(tomoe_reading)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testFailReadingSearch(self):
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, 'ひげ')
        char_code = '池'
        tomoe_char = tomoe.Char(char_code)
        tomoe_char.add_reading(tomoe_reading)
        self.assert_(self.dict.register_char(tomoe_char))

        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, tomoe_reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(len(candidates), 0)

    def testKunReadingSearch(self):
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_KUN, reading)

        char_code = '池'
        tomoe_char = tomoe.Char(char_code)
        tomoe_char.add_reading(tomoe_reading)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testOnReadingSearch(self):
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)

        char_code = '池'
        tomoe_char = tomoe.Char(char_code)
        tomoe_char.add_reading(tomoe_reading)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testUnknownOnReadingSearch(self):
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)

        char_code = '池'
        tomoe_char = tomoe.Char(char_code)
        tomoe_char.add_reading(tomoe_reading)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_query = tomoe.Query()
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testUnknownKunReadingSearch(self):
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_KUN, reading)

        char_code = '池'
        tomoe_char = tomoe.Char(char_code)
        tomoe_char.add_reading(tomoe_reading)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_query = tomoe.Query()
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testOnUnknownReadingSearch(self):
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)

        char_code = '池'
        tomoe_char = tomoe.Char(char_code)
        tomoe_char.add_reading(tomoe_reading)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_query = tomoe.Query()
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testKunUnknownReadingSearch(self):
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)

        char_code = '池'
        tomoe_char = tomoe.Char(char_code)
        tomoe_char.add_reading(tomoe_reading)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_query = tomoe.Query()
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_KUN, reading)
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

# vi:ts=4:nowrap:ai:expandtab
