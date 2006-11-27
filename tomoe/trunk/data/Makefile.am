# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation; either
## version 2 of the License, or (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public
## License along with this program; if not, write to the
## Free Software Foundation, Inc., 59 Temple Place, Suite 330,
## Boston, MA  02111-1307  USA

CLEANFILES = *~ *.bak *.valid

dicdir = $(datadir)/$(PACKAGE)
dic_DATA = tomoe-dict.dtd all.xml kanjidic2.xml

confdir = $(sysconfdir)/$(PACKAGE)
conf_DATA = config

EXTRA_DIST = \
	kanjidic2-original.xml kanjidic2-original.xsl \
	$(dic_DATA) \
	$(conf_DATA)

kanjidic2.xml: kanjidic2-original.xsl kanjidic2-original.xml
	$(XSLTPROC) kanjidic2-original.xsl kanjidic2-original.xml > $@

SUFFIXES: .xml .valid

.xml.valid:
	$(RXP) -s -V $<
	touch $@

.PHONY: validate

validate: all.valid kanjidic2.valid
