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
	-DSYSCONFDIR=\""$(sysconfdir)"\"			\
	-DDATADIR=\""$(datadir)"\"				\
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\"	\
	-DTOMOEDATADIR=\""$(datadir)"/$(PACKAGE)\"		\
	-DRUBY_EXTDIR=\""$(RUBY_EXTDIR)"\"			\
	-DRUBY_LIBDIR=\""$(RUBY_LIBDIR)"\"

INCLUDES = -I$(top_srcdir)/lib $(GLIB_CFLAGS) $(EST_CFLAGS)
LIBADD = $(GLIB_LIBS)
LDFLAGS =							\
  -version-info $(LT_VERSION_INFO)				\
  -export-dynamic $(no_undefined) $(LIBTOOL_EXPORT_OPTIONS)

dict_LTLIBRARIES = libxml.la

libxml_la_SOURCES = tomoe-dict-xml.c

if WITH_EST
dict_LTLIBRARIES += libest.la

libest_la_SOURCES = tomoe-dict-est.c
libest_la_LIBADD = $(EST_LIBS)
endif

if WITH_UNIHAN
dict_LTLIBRARIES += libunihan.la

unihan_built_sources =		\
	tomoe-unihan.c		\
	tomoe-unihan.h		\
	tomoe-unihan0.c		\
	tomoe-unihan1.c		\
	tomoe-unihan2.c		\
	tomoe-unihan3.c		\
	tomoe-unihan4.c		\
	tomoe-unihan5.c		\
	tomoe-unihan6.c		\
	tomoe-unihan7.c		\
	tomoe-unihan8.c		\
	tomoe-unihan9.c

CLEANFILES += $(unihan_built_sources) Unihan.zip Unihan.txt Unihan.txt.cache
BUILT_SOUCES = $(unihan_built_sources)

libunihan_la_SOURCES =		\
	$(unihan_built_sources)	\
	tomoe-dict-unihan.c

tomoe-unihan.c: unihan-compiler.rb Unihan.txt
	$(RUBY) unihan-compiler.rb Unihan.txt tomoe-unihan 10

Unihan.txt: Unihan.zip
	unzip Unihan.zip
	touch Unihan.txt

Unihan_URL = ftp://ftp.unicode.org/Public/UNIDATA/Unihan.zip
Unihan.zip:
	$(RUBY) -r open-uri -e "print open('$(Unihan_URL)').read" > $@
endif
