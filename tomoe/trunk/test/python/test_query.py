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

# vi:ts=4:nowrap:ai:expandtab
