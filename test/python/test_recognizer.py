# -*- coding: UTF=8 -*-
import os
import sys
import glob
import unittest
import tomoe

class TomoeRecognizerTest(unittest.TestCase):

    def setUp(self):
        self.recognizer = tomoe.Recognizer('Simple')
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
                i = 0
                for point in line.split(','):
                    if i == 0:
                        writing.move_to(int(point.split()[0]), int(point.split()[1]))
                    else:
                        writing.line_to(int(point.split()[0]), int(point.split()[1]))
                    i+=1
            l+=1

        return results, writing

    def testStrokeSearch(self):
        for writing, result in zip(self.writings, self.results):
            self.recognizer.search(writing)
            for a, b in zip(self.recognizer.search(writing), result.split()):
                self.assertEqual(a.get_char().get_utf8(), b)


# vi:ts=4:nowrap:ai:expandtab
