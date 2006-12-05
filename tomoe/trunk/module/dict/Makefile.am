# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2006 Kouhei Sutou <kou@cozmixng.org>
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

CLEANFILES = *~ *.bak

AM_CPPFLAGS =							\
	-DG_LOG_DOMAIN=\"TOMOE-DICT\"				\
	-DRUBY_EXTDIR=\""$(ruby_extdir)"\"			\
	-DRUBY_LIBDIR=\""$(ruby_libdir)"\"

INCLUDES =				\
	-I$(top_srcdir)/ext/ruby	\
	$(TOMOE_CFLAGS)			\
	$(EST_CFLAGS)			\
	$(RUBY_CFLAGS)

LIBADD = $(TOMOE_LIBS)
LDFLAGS =							\
  -version-info $(LT_VERSION_INFO)				\
  -export-dynamic $(no_undefined) $(LIBTOOL_EXPORT_OPTIONS)

dict_module_LTLIBRARIES = libxml.la

libxml_la_SOURCES = tomoe-dict-xml.c

if WITH_EST
dict_module_LTLIBRARIES += libest.la

libest_la_SOURCES = tomoe-dict-est.c
libest_la_LIBADD = $(EST_LIBS)
endif

if ENABLE_RUBY_DICT
dict_module_LTLIBRARIES += libruby.la

libruby_la_SOURCES = tomoe-dict-ruby.c
libruby_la_LIBADD = $(RUBY_LIBS)
endif

if WITH_UNIHAN
dict_module_LTLIBRARIES += libunihan.la

unihan_built_sources = tomoe-unihan-data.h

CLEANFILES += Unihan.txt Unihan.txt.cache
BUILT_SOURCES = $(unihan_built_sources) Unihan.zip

libunihan_la_SOURCES =		\
	tomoe-dict-unihan.c	\
	tomoe-unihan.c		\
	tomoe-unihan.h		\
	$(unihan_built_sources)

tomoe-unihan-data.h: unihan-compiler.rb Unihan.txt
	$(RUBY) unihan-compiler.rb Unihan.txt > $@

Unihan.txt: Unihan.zip
	$(UNZIP) Unihan.zip
	touch $@

Unihan_URL = ftp://ftp.unicode.org/Public/UNIDATA/Unihan.zip
Unihan.zip:
	$(RUBY) -r open-uri -e "print open('$(Unihan_URL)').read" > $@
endif
