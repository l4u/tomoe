# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe

class TomoeQueryTest(unittest.TestCase):

    def testUTF8(self):
        utf8 = '池'
        query = tomoe.Query()
        self.assertEqual(query.get_utf8(), None)
        query.set_utf8(utf8)
        self.assertEqual(query.get_utf8(), utf8)

        query = tomoe.Query(utf8 = utf8)
        self.assertEqual(query.get_utf8(), utf8)

    def testReading(self):
        query = tomoe.Query()
        self.assertEqual(len(query.get_readings()), 0)

        reading_string = "いけ"
        reading1 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        query.add_reading(reading1)
        self.assertEqual(query.get_readings(), [reading1])

        reading_string = "ひげ"
        reading2 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string)
        query.add_reading(reading2)
        self.assertEqual(query.get_readings().sort(), [reading1, reading2].sort())

    def testRadical(self):
        query = tomoe.Query()
        self.assertEqual(len(query.get_radicals()), 0)

        radical1 = "いけ"
        query.add_radical(radical1)
        self.assertEqual(query.get_radicals(), [radical1])

        radical2 = "ひげ"
        query.add_radical(radical2)
        self.assertEqual(query.get_radicals().sort(), [radical1, radical2].sort())

    def testRadical(self):
        query = tomoe.Query()
        self.assertEqual(len(query.get_radicals()), 0)

        radical1 = "いけ"
        query.add_radical(radical1)
        self.assertEqual(query.get_radicals(), [radical1])

        radical2 = "ひげ"
        query.add_radical(radical2)
        self.assertEqual(query.get_radicals().sort(), [radical1, radical2].sort())

    def testVariant(self):
        query = tomoe.Query()
        self.assertEqual(query.get_variant(), None)

        variant = "池"
        query.set_variant(variant)
        self.assertEqual(query.get_variant(), variant)

        variant = "地"
        query.set_variant(variant)
        self.assertEqual(query.get_variant(), variant)

    def testMinStroke(self):
        query = tomoe.Query()
        self.assertEqual(query.get_min_n_strokes(), -1)

        n_strokes = 2
        query.set_min_n_strokes(n_strokes)
        self.assertEqual(query.get_min_n_strokes(), n_strokes)

        n_strokes = 3
        query.set_min_n_strokes(n_strokes)
        self.assertEqual(query.get_min_n_strokes(), n_strokes)

    def testMaxStroke(self):
        query = tomoe.Query()
        self.assertEqual(query.get_max_n_strokes(), -1)

        n_strokes = 2
        query.set_max_n_strokes(n_strokes)
        self.assertEqual(query.get_max_n_strokes(), n_strokes)

        n_strokes = 3
        query.set_max_n_strokes(n_strokes)
        self.assertEqual(query.get_max_n_strokes(), n_strokes)

    def testEmpty(self):
        query = tomoe.Query()
        self.assert_(query.is_empty())

        query = tomoe.Query(utf8 = '池')
        self.assert_(not query.is_empty())

# vi:ts=4:nowrap:ai:expandtab
