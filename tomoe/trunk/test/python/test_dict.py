# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe

class TomoeDictTest(unittest.TestCase):

    def testRegisterChar(self):
        char_code = '地'
        tomoe_char = self.dict.get_char(char_code)
        self.assertEqual(tomoe_char, None)

        tomoe_char = tomoe.Char(char_code)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_char = self.dict.get_char(char_code)
        self.assertNotEqual(tomoe_char, None)
        self.assertEqual(tomoe_char.get_utf8(), char_code)

    def testUnregisterChar(self):
        char_code = '池'
        tomoe_char = self.dict.get_char(char_code)
        self.assertEqual(tomoe_char.get_utf8(), char_code)

        self.assert_(self.dict.unregister_char(char_code))
        self.assertEqual(self.dict.get_char(char_code), None)

    def testGetExistChar(self):
        char_code = '池'
        n_strokes = 6
        tomoe_char = self.dict.get_char(char_code)
        self.assertEqual(tomoe_char.get_utf8(), char_code)
        self.assertEqual(tomoe_char.get_n_strokes(), n_strokes)

    def testFailGetExistChar(self):
        char_code = '存在しない'
        self.assertEqual(self.dict.get_char(char_code), None)

    def testFailReadingSearch(self):
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, 'そんなよみかたありません')
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(len(candidates), 0)

    def testUnknownReadingSearch(self):
        char_code = '池'
        reading = 'あんのうん'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testKunReadingSearch(self):
        char_code = '池'
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_KUN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testOnReadingSearch(self):
        char_code = '池'
        reading = 'チ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testUnknownOnReadingSearch(self):
        char_code = '池'
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testUnknownKunReadingSearch(self):
        char_code = '池'
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testOnUnknownReadingSearch(self):
        char_code = '池'
        reading = 'あんのうん'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testKunUnknownReadingSearch(self):
        char_code = '池'
        reading = 'あんのうん'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_KUN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

# vi:ts=4:nowrap:ai:expandtab
