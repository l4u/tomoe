# -*- coding: UTF=8 -*-
import os
import sys
import unittest
sys.path.append('../../bindings/python/.libs')
import tomoe

class TomoeDictTest(unittest.TestCase):
    #def setUp(self):

    def setUpXMLDict(self, dict_contents):
        dict_name = "tomoe-test-xmldict.xml"
        dict_file = open(dict_name, "w")
        dict_file.write(dict_contents)
        dict_file.close()

        return tomoe.Dict("XML", filename = dict_name, editable = True)

    def getDictContents(self):
        dict_name = "tomoe-test-xmldict.xml"
        dict_file = open(dict_name, "r")
        contents = dict_file.read()
        dict_file.close()

        return contents

    def testRegisterChar(self):
        dict_contents = """
        <?xml version="1.0" encoding="UTF-8" standalone="no"?>
        <!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
        <dictionary>
        </dictionary>
        """
        dict = self.setUpXMLDict(dict_contents)
        char_code ='池'
        tomoe_char = dict.get_char(char_code)
        self.assertEqual(tomoe_char, None)

        tomoe_char = tomoe.Char()
        tomoe_char.set_utf8(char_code)
        ret = dict.register_char(tomoe_char)
        self.assertEqual(ret, True)

        tomoe_char = dict.get_char(char_code)
        self.assertNotEqual(tomoe_char, None)
        self.assertEqual(tomoe_char.get_utf8(), char_code)

        dict.flush()
        contents = """\
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
<dictionary>
  <character>
    <utf8>池</utf8>
  </character>
</dictionary>
"""
        self.assertEqual(contents, self.getDictContents())

    def testUnregisterChar(self):
        char_code ='池'
        dict_contents = """
        <?xml version="1.0" standalone="no"?>
        <!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
        <dictionary>
          <character>
            <utf8>%s</utf8>
          </character>
        </dictionary>
        """ % (char_code)
        dict = self.setUpXMLDict(dict_contents)
        ret = dict.unregister_char(char_code)
        self.assertEqual(ret, True)

        tomoe_char = dict.get_char(char_code)
        self.assertEqual(tomoe_char, None)

    def testGetExistChar(self):
        char_code ='池'
        n_strokes = 6
        dict_contents = """
        <?xml version="1.0" standalone="no"?>
        <!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
        <dictionary>
          <character>
            <utf8>%s</utf8>
            <number-of-strokes>%d</number-of-strokes>
            <readings>
              <reading type="ja_on">チ</reading>
              <reading type="ja_kun">いけ</reading>
            </readings>
            <meta>
              <jis208>35-51</jis208>
              <ucs>6c60</ucs>
              <jouyou>2</jouyou>
              <meaning>pond, cistern, pool, reservoir</meaning>
            </meta>
          </character>
        </dictionary>
        """ % (char_code, n_strokes)
        dict = self.setUpXMLDict(dict_contents)
        tomoe_char = dict.get_char(char_code)
        self.assertEqual(tomoe_char.get_utf8(), char_code)
        self.assertEqual(tomoe_char.get_n_strokes(), n_strokes)

    def testFailGetExistChar(self):
        char_code ='池'
        n_strokes = 6
        dict_contents = """
        <?xml version="1.0" standalone="no"?>
        <!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
        <dictionary>
          <character>
            <utf8>%s</utf8>
          </character>
        </dictionary>
        """ % ("地")
        dict = self.setUpXMLDict(dict_contents)
        tomoe_char = dict.get_char(char_code)
        self.assertEqual(tomoe_char, None)

    def testReadingSearch(self):
        char_code ='池'
        reading = 'いけ'
        dict_contents = """
        <?xml version="1.0" standalone="no"?>
        <!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
        <dictionary>
          <character>
            <utf8>%s</utf8>
            <readings>
              <reading type="ja_kun">%s</reading>
            </readings>
          </character>
        </dictionary>
        """ % (char_code, reading)
        dict = self.setUpXMLDict(dict_contents)
        tomoe_query = tomoe.Query()
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, reading)
        tomoe_query.add_reading(tomoe_reading)
        candidates = dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testFailReadingSearch(self):
        char_code ='池'
        reading = 'いけ'
        dict_contents = """
        <?xml version="1.0" standalone="no"?>
        <!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
        <dictionary>
          <character>
            <utf8>%s</utf8>
            <readings>
              <reading type="ja_kun">%s</reading>
            </readings>
          </character>
        </dictionary>
        """ % (char_code, reading)
        dict = self.setUpXMLDict(dict_contents)
        tomoe_query = tomoe.Query()
        tomoe_reading = tomoe.Reading(tomoe.READING_UNKNOWN, 'ひげ')
        tomoe_query.add_reading(tomoe_reading)
        candidates = dict.search(tomoe_query)
        self.assertEqual(len(candidates), 0)

    def testKunReadingSearch(self):
        char_code ='池'
        reading = 'いけ'
        dict_contents = """
        <?xml version="1.0" standalone="no"?>
        <!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
        <dictionary>
          <character>
            <utf8>%s</utf8>
            <readings>
              <reading type="ja_kun">%s</reading>
            </readings>
          </character>
        </dictionary>
        """ % (char_code, reading)
        dict = self.setUpXMLDict(dict_contents)
        tomoe_query = tomoe.Query()
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_KUN, reading)
        tomoe_query.add_reading(tomoe_reading)
        candidates = dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

    def testOnReadingSearch(self):
        char_code ='池'
        reading = 'チ'
        dict_contents = """
        <?xml version="1.0" standalone="no"?>
        <!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
        <dictionary>
          <character>
            <utf8>%s</utf8>
            <readings>
              <reading type="ja_on">%s</reading>
            </readings>
          </character>
        </dictionary>
        """ % (char_code, reading)
        dict = self.setUpXMLDict(dict_contents)
        tomoe_query = tomoe.Query()
        tomoe_reading = tomoe.Reading(tomoe.READING_JA_ON, reading)
        tomoe_query.add_reading(tomoe_reading)
        candidates = dict.search(tomoe_query)
        self.assertEqual(candidates[0].get_char().get_utf8(), char_code)

if __name__ == '__main__':
    suite = unittest.makeSuite(TomoeDictTest)
    unittest.TextTestRunner(verbosity=2).run(suite)
    #unittest.main()

# vi:ts=4:nowrap:ai:expandtab
