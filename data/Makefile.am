# -*- Mode: Makefile.am; tab-width: 8; indent-tabs-mode: t; -*-

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

INCLUDES = $(TOMOE_CFLAGS)
noinst_PROGRAMS = xml2binary
xml2binary_LDADD = $(TOMOE_LIBS) $(top_builddir)/lib/libtomoe.la
xml2binary_SOURCES = xml2binary.c

pkgdata_DATA = dict.dtd xml2est.rb kanjidic2xml.rb
dict_data_DATA =
recognizer_data_DATA = handwriting-zh_CN.xml handwriting-ja.xml
if USE_ZINNIA
noinst_PROGRAMS += xml2zinnia
recognizer_data_DATA += handwriting-zh_CN.model handwriting-ja.model
endif
conf_DATA = config

EXTRA_DIST =						\
	kanjidic2-original.xml kanjidic2-original.xsl	\
	kanjidic.html kanjidic2.html			\
	kanjidic-licence.html				\
	kanjidic2.xml					\
	$(pkgdata_DATA)					\
	$(dict_data_DATA)				\
	$(recognizer_data_DATA)				\
	$(conf_DATA)

xml2zinnia_SOURCES = xml2zinnia.c
xml2zinnia_CFLAGS = $(ZINNIA_CFLAGS)
xml2zinnia_LDADD =			\
	$(TOMOE_LIBS)			\
	$(top_builddir)/lib/libtomoe.la	\
	$(ZINNIA_LIBS)

if USE_KANJIDIC
kanjidic2.xml: $(KANJIDIC)
	$(RUBY) $(srcdir)/kanjidic2xml.rb $(KANJIDIC) > $@
else
kanjidic2.xml: kanjidic2-original.xsl kanjidic2-original.xml
	$(XSLTPROC) kanjidic2-original.xsl kanjidic2-original.xml > $@
endif

SUFFIXES: .xml .valid .model

.xml.valid:
	$(RXP) -s -V $<
	touch $@

.xml.model:
	TOMOE_DICT_MODULE_DIR=$(top_builddir)/module/dict/.libs	\
	  ./xml2zinnia $< $@

.PHONY: validate

validate: handwriting-zh_CN.valid handwriting-ja.valid kanjidic2.valid

# temporary added for hardcoded test code.
# should be removed
BUILT_SOURCES = handwriting.xml
handwriting.xml:
	ln -sf handwriting-ja.xml handwriting.xml
