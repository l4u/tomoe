# -*- coding: UTF=8 -*-
import os
import sys
import glob
import fileinput
import unittest
import test_common
import tomoe

class TomoeRecognizerTest(unittest.TestCase):

    def setUp(self):
        dict = tomoe.Dict("XML", filename = os.path.join(test_common.data_dir, 'handwriting-ja.xml'))
        self.recognizer = tomoe.Recognizer('Simple', dictionary = dict)
        self.writings = []
        self.results = []
        for file in glob.glob(os.path.join(test_common.test_data_dir, '*.data')):
            results, writing = self.parseStrokeData(file)
            self.writings.append(writing)
            self.results.append(results)

    def parseStrokeData(self, file):
        writing = tomoe.Writing()
        results = []
        for line in fileinput.input(file):
            if fileinput.filelineno() == 1:
                results = line
            else:
                points = line.split(',')
                first_point = points.pop(0)
                x, y = first_point.split()
                writing.move_to(int(x), int(y))
                for point in points:
                    x, y = point.split()
                    writing.line_to(int(x), int(y))

        return results, writing

    def testStrokeSearch(self):
        for writing, result in zip(self.writings, self.results):
            for a, b in zip(self.recognizer.search(writing), result.split()):
                self.assertEqual(a.get_char().get_utf8(), b)


# vi:ts=4:nowrap:ai:expandtab
