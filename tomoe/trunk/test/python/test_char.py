# -*- coding: UTF=8 -*-
import os
import sys
import unittest
sys.path.append('../../bindings/python/.libs')
import tomoe

class TomoeCharTest(unittest.TestCase):
    #def setUp(self):

    def testUTF8(self):
        utf8 = '池'

        tomoe_char = tomoe.Char()
        self.assertEqual(tomoe_char.get_utf8(), None)

        tomoe_char.set_utf8(utf8)
        self.assertEqual(tomoe_char.get_utf8(), utf8)

    def testStrokes(self):
        n_strokes = 2

        tomoe_char = tomoe.Char()
        self.assertEqual(tomoe_char.get_n_strokes(), -1)

        tomoe_char.set_n_strokes(n_strokes)
        self.assertEqual(tomoe_char.get_n_strokes(), n_strokes)

    def testReading(self):
        reading_string = 'いけ'
        reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)

        tomoe_char = tomoe.Char()
        self.assertEqual(len(tomoe_char.get_readings()), 0)

        tomoe_char.add_reading(reading)

        self.fail()

    def testRadical(self):
        radical_string = 'いけ'

        tomoe_char = tomoe.Char()
        self.assertEqual(len(tomoe_char.get_radicals()), 0)

        tomoe_char.add_radical(radical_string)

        self.fail()

    def testWriting(self):
        self.fail()

    def testVariant(self):
        variant = '池'
        tomoe_char = tomoe.Char()
        self.assertEqual(tomoe_char.get_variant(), None)

        tomoe_char.set_variant(variant)
        self.assertEqual(tomoe_char.get_variant(), variant)

    def testCompare(self):
        char1 = '池'
        char2 = '地'
        
        tomoe_char1 = tomoe.Char()
        tomoe_char1.set_utf8(char1)
        tomoe_char2 = tomoe.Char()
        tomoe_char2.set_utf8(char1)

        self.assertEqual(tomoe_char1.compare(tomoe_char2), 0)
        
        tomoe_char2 = tomoe.Char()
        tomoe_char2.set_utf8(char2)
        self.assertNotEqual(tomoe_char1.compare(tomoe_char2), 0)

if __name__ == '__main__':
    unittest.main()

# vi:ts=4:nowrap:ai:expandtab
