# -*- coding: UTF=8 -*-
import os
import sys
import unittest
import tomoe
import test_dict

class TomoeDictXMLTest(test_dict.TomoeDictTest):
    def setUp(self):
        self.dict_name = "tomoe-test-xmldict.xml"
        dict_file = open(self.dict_name, "w")
        contents = """
<?xml version="1.0" standalone="no"?>
<!DOCTYPE dictionary SYSTEM "/usr/share/tomoe/dict.dtd">
<dictionary>
  <character>
    <utf8>池</utf8>
    <number-of-strokes>6</number-of-strokes>
    <readings>
      <reading type="ja_on">チ</reading>
      <reading type="ja_kun">いけ</reading>
      <reading type="ja_unknown">あんのうん</reading>
    </readings>
    <meta>
      <jis208>35-51</jis208>
      <ucs>6c60</ucs>
      <jouyou>2</jouyou>
      <meaning>pond, cistern, pool, reservoir</meaning>
    </meta>
  </character>
</dictionary>
         """
        dict_file.write(contents)
        dict_file.close()
        self.dict = tomoe.Dict("XML", filename = self.dict_name, editable = True)

    def tearDown(self):
        self.dict.flush()
        self.dict = None
        if os.access(self.dict_name, os.F_OK):
            os.unlink(self.dict_name)

# vi:ts=4:nowrap:ai:expandtab
