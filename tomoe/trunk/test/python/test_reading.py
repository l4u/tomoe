# -*- coding: UTF=8 -*-
import os
import sys
import unittest
sys.path.append('../../bindings/python/.libs')
import tomoe

class TomoeReadingTest(unittest.TestCase):
    #def setUp(self):

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
        reading1 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        self.assertEqual(tomoe.tomoe_reading_compare(reading1, reading2), 0)
        self.assertEqual(tomoe.tomoe_reading_compare(reading2, reading1), 0)

        reading1 = tomoe.Reading(tomoe.READING_JA_KUN, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        self.assertEqual(tomoe.tomoe_reading_compare(reading1, reading2), 0)
        self.assertEqual(tomoe.tomoe_reading_compare(reading2, reading1), 0)

        reading1 = tomoe.Reading(tomoe.READING_JA_ON, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_UNKNOWN, reading_string)
        self.assertEqual(tomoe.tomoe_reading_compare(reading1, reading2), 0)
        self.assertEqual(tomoe.tomoe_reading_compare(reading2, reading1), 0)

        reading1 = tomoe.Reading(tomoe.READING_JA, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_JA, reading_string)
        self.assertEqual(tomoe.tomoe_reading_compare(reading1, reading2), 0)

        reading1 = tomoe.Reading(tomoe.READING_ON, reading_string)
        reading2 = tomoe.Reading(tomoe.READING_ON, reading_string)
        self.assertEqual(tomoe.tomoe_reading_compare(reading1, reading2), 0)

if __name__ == '__main__':
    suite = unittest.makeSuite(TomoeReadingTest)
    unittest.TextTestRunner(verbosity=2).run(suite)
    #unittest.main()

# vi:ts=4:nowrap:ai:expandtab
