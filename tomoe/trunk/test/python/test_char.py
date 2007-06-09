# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe

class TomoeCharTest(unittest.TestCase):

    def testUTF8(self):
        utf8 = '池'

        tomoe_char = tomoe.Char()
        self.assertEqual(tomoe_char.get_utf8(), None)
        tomoe_char.set_utf8(utf8)
        self.assertEqual(tomoe_char.get_utf8(), utf8)

        tomoe_char = tomoe.Char(utf8 = utf8)
        self.assertEqual(tomoe_char.get_utf8(), utf8)

        new_utf8 = '地'
        tomoe_char.set_utf8(new_utf8)
        self.assertEqual(tomoe_char.get_utf8(), new_utf8)

    def testStrokes(self):
        n_strokes = 2
        tomoe_char = tomoe.Char()
        self.assertEqual(tomoe_char.get_n_strokes(), -1)
        tomoe_char.set_n_strokes(n_strokes)
        self.assertEqual(tomoe_char.get_n_strokes(), n_strokes)

        tomoe_char = tomoe.Char(n_strokes = n_strokes)
        self.assertEqual(tomoe_char.get_n_strokes(), n_strokes)

        n_strokes = 3
        tomoe_char.set_n_strokes(n_strokes)
        self.assertEqual(tomoe_char.get_n_strokes(), n_strokes)

    def testReading(self):
        reading_string = 'いけ'
        reading1 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)

        tomoe_char = tomoe.Char()
        self.assertEqual(len(tomoe_char.get_readings()), 0)

        tomoe_char.add_reading(reading1)
        self.assertEqual(tomoe_char.get_readings(), [reading1])

        reading2 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string)
        tomoe_char.add_reading(reading2)
        self.assertEqual(tomoe_char.get_readings().sort(), [reading1, reading2].sort())

    def testRadical(self):
        radical_string1 = 'いけ'

        tomoe_char = tomoe.Char()
        self.assertEqual(len(tomoe_char.get_radicals()), 0)

        tomoe_char.add_radical(radical_string1)
        self.assertEqual(tomoe_char.get_radicals(), [radical_string1])

        radical_string2 = 'ほげ'
        tomoe_char.add_radical(radical_string2)
        self.assertEqual(tomoe_char.get_radicals().sort(), [radical_string1, radical_string2].sort())

    def testWriting(self):
        writing = tomoe.Writing()

        tomoe_char = tomoe.Char()
        self.assertEqual(tomoe_char.get_writing(), None)
        tomoe_char.set_writing(writing)
        self.assertEqual(tomoe_char.get_writing(), writing)

        tomoe_char = tomoe.Char(writing = writing)
        self.assertEqual(tomoe_char.get_writing(), writing)

        writing.move_to(1, 1)
        writing.line_to(2, 2)
        tomoe_char.set_writing(writing)
        self.assertEqual(tomoe_char.get_writing(), writing)

    def testVariant(self):
        variant = '池'
        tomoe_char = tomoe.Char()
        self.assertEqual(tomoe_char.get_variant(), None)
        tomoe_char.set_variant(variant)
        self.assertEqual(tomoe_char.get_variant(), variant)

        tomoe_char = tomoe.Char(variant = variant)
        self.assertEqual(tomoe_char.get_variant(), variant)

        variant = '地'
        tomoe_char.set_variant(variant)
        self.assertEqual(tomoe_char.get_variant(), variant)

    def testCompare(self):
        char1 = '池'
        char2 = '地'
        
        tomoe_char1 = tomoe.Char(utf8 = char1)
        tomoe_char2 = tomoe.Char(utf8 = char1)
        self.assertEqual(tomoe_char1, tomoe_char2)
        
        tomoe_char2 = tomoe.Char(utf8 = char2)
        self.assertNotEqual(tomoe_char1, tomoe_char2, 0)

# vi:ts=4:nowrap:ai:expandtab
