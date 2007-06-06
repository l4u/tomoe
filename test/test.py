#!/usr/bin/env python
# coding: UTF=8
import tomoe

c = tomoe.Context()
c.load_config()

q = tomoe.Query()
q.set_utf8("池")

l = c.search(q)

print l[0].get_char().get_utf8()
