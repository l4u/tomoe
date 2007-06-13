# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe

class TomoeDictTest(unittest.TestCase):

    def testRegisterChar(self):
        char_code = '地'
        tomoe_char = self.dict.get_char(char_code)
        self.assertEqual(None, tomoe_char)

        tomoe_char = tomoe.Char(char_code)
        self.assert_(self.dict.register_char(tomoe_char))

        tomoe_char = self.dict.get_char(char_code)
        self.assertNotEqual(None, tomoe_char)
        self.assertEqual(char_code, tomoe_char.get_utf8())

    def testUnregisterChar(self):
        char_code = '池'
        tomoe_char = self.dict.get_char(char_code)
        self.assertEqual(char_code, tomoe_char.get_utf8())

        self.assert_(self.dict.unregister_char(char_code))
        self.assertEqual(None, self.dict.get_char(char_code))

    def testGetExistChar(self):
        char_code = '池'
        n_strokes = 6
        tomoe_char = self.dict.get_char(char_code)
        self.assertEqual(char_code, tomoe_char.get_utf8())
        self.assertEqual(n_strokes, tomoe_char.get_n_strokes())

    def testFailGetExistChar(self):
        char_code = '存在しない'
        self.assertEqual(None, self.dict.get_char(char_code))

    def testFailReadingSearch(self):
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, 'そんなよみかたありません')
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)
        candidates = self.dict.search(tomoe_query)
        self.assertEqual(0, len(candidates))

    def testUnknownReadingSearch(self):
        char_code = '池'
        reading = 'あんのうん'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(char_code, candidates[0].get_char().get_utf8())

    def testKunReadingSearch(self):
        char_code = '池'
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_KUN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(char_code, candidates[0].get_char().get_utf8())

    def testOnReadingSearch(self):
        char_code = '池'
        reading = 'チ'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(char_code, candidates[0].get_char().get_utf8())

    def testUnknownOnReadingSearch(self):
        char_code = '池'
        reading = 'チ'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(char_code, candidates[0].get_char().get_utf8())

    def testUnknownKunReadingSearch(self):
        char_code = '池'
        reading = 'いけ'
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(char_code, candidates[0].get_char().get_utf8())

    def testOnUnknownReadingSearch(self):
        char_code = '池'
        reading = 'あんのうん'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(char_code, candidates[0].get_char().get_utf8())

    def testKunUnknownReadingSearch(self):
        char_code = '池'
        reading = 'あんのうん'
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_KUN, reading)
        tomoe_query = tomoe.Query()
        tomoe_query.add_reading(tomoe_reading)

        candidates = self.dict.search(tomoe_query)
        self.assertEqual(char_code, candidates[0].get_char().get_utf8())

# vi:ts=4:nowrap:ai:expandtab
