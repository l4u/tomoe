# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe

class TomoeReadingTest(unittest.TestCase):

    def testGetReadingType(self):
        reading_string = "いけ"

        reading = tomoe.Reading(tomoe.READING_INVALID, reading_string)
        self.assertEqual(reading.get_reading_type(), tomoe.READING_INVALID)

        reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        self.assertEqual(reading.get_reading_type(), tomoe.READING_UNKNOWN)

        reading = tomoe.Reading(tomoe.READING_JA_KUN, reading_string)
        self.assertEqual(reading.get_reading_type(), tomoe.READING_JA_KUN)

        reading = tomoe.Reading(tomoe.READING_JA_ON, reading_string)
        self.assertEqual(reading.get_reading_type(), tomoe.READING_JA_ON)

    def testGetReading(self):
        reading_string = "いけ"
        reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        self.assertEqual(reading.get_reading(), reading_string)

    def testToXML(self):
        reading_string = "いけ"
        reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        reading_data = """\
      <reading type="unknown">%s</reading>
""" % (reading_string)
        self.assertEqual(reading.to_xml(), reading_data)

    def testCompare(self):
        reading_string = "いけ"
        reading_string2 = "ほげ"
        reading1 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        reading3 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string2)
        self.assertEqual(reading1.compare(reading2), 0)
        self.assertNotEqual(reading1.compare(reading3), 0)

        reading1 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        reading3 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string2)
        reading4 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string2)
        self.assertEqual(reading1.compare(reading2), 0)
        self.assertEqual(reading2.compare(reading1), 0)
        self.assertNotEqual(reading2.compare(reading3), 0)
        self.assertNotEqual(reading1.compare(reading4), 0)

        reading1 = tomoe.Reading(tomoe.READING_JA_ON, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        reading3 = tomoe.Reading(tomoe.READING_JA_ON, reading_string2)
        reading4 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string2)
        self.assertEqual(reading1.compare(reading2), 0)
        self.assertEqual(reading2.compare(reading1), 0)
        self.assertNotEqual(reading2.compare(reading3), 0)
        self.assertNotEqual(reading1.compare(reading4), 0)

        reading1 = tomoe.Reading(tomoe.READING_JA_ON, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_JA_ON, reading_string)
        reading3 = tomoe.Reading(tomoe.READING_JA_ON, reading_string2)
        self.assertEqual(reading1.compare(reading2), 0)
        self.assertNotEqual(reading1.compare(reading3), 0)

        reading1 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string)
        reading3 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string2)
        self.assertEqual(reading1.compare(reading2), 0)
        self.assertNotEqual(reading1.compare(reading3), 0)

        reading1 = tomoe.Reading(tomoe.READING_JA_ON, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string)
        reading3 = tomoe.Reading(tomoe.READING_JA_ON, reading_string2)
        reading4 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string2)
        self.assertNotEqual(reading1.compare(reading2), 0)
        self.assertNotEqual(reading2.compare(reading1), 0)
        self.assertNotEqual(reading2.compare(reading3), 0)
        self.assertNotEqual(reading1.compare(reading4), 0)

# vi:ts=4:nowrap:ai:expandtab
