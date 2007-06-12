# -*- coding: UTF=8 -*-
import os
import sys
import glob
import unittest
import tomoe

class TomoeRecognizerTest(unittest.TestCase):

    def setUp(self):
        self.recognizer = tomoe.Recognizer('Simple', language = 'ja')
        self.writings = []
        self.results = []
        for file in glob.glob('../data/*.data'):
            results, writing = self.parseStrokeData(file)
            self.writings.append(writing)
            self.results.append(results)

    def parseStrokeData(self, file):
        writing = tomoe.Writing()
        results = []
        l = 0
        for line in open(file, 'r'):
            if l == 0:
                results = line
            else:
                points = line.split(',')
                first_point = points.pop(0)
                x, y = first_point.split()
                writing.move_to(int(x), int(y))
                for point in points:
                    x, y = point.split()
                    writing.line_to(int(x), int(y))
            l+=1

        return results, writing

    def testStrokeSearch(self):
        for writing, result in zip(self.writings, self.results):
            for a, b in zip(self.recognizer.search(writing), result.split()):
                self.assertEqual(a.get_char().get_utf8(), b)


# vi:ts=4:nowrap:ai:expandtab
