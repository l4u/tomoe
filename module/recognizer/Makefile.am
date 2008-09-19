# -*- Mode: Makefile.am; tab-width: 8; indent-tabs-mode: t; -*-

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

AM_CPPFLAGS =
INCLUDES = $(TOMOE_CFLAGS)

if OS_WIN32
export_symbols = -export-symbols $(top_srcdir)/lib/libtomoe-module.def
else
export_symbols = $(LIBTOOL_EXPORT_OPTIONS)
endif

LDFLAGS =							\
  -rpath $(recognizer_moduledir) -avoid-version -module		\
  -no-undefined $(export_symbols)

recognizer_module_LTLIBRARIES = simple.la
if USE_ZINNIA
recognizer_module_LTLIBRARIES += zinnia.la
endif

simple_la_CPPFLAGS = -DG_LOG_DOMAIN=\"Tomoe/Recognizer:Simple\" $(AM_CPPFLAGS)
simple_la_SOURCES = \
	tomoe-recognizer-simple.c				\
	tomoe-recognizer-simple-logic.c				\
	tomoe-recognizer-simple-logic.h

simple_la_LIBADD = $(TOMOE_LIBS) $(top_builddir)/lib/libtomoe.la


zinnia_la_CPPFLAGS = 					\
	-DG_LOG_DOMAIN=\"Tomoe/Recognizer:Zinnia\"	\
	$(AM_CPPFLAGS)

zinnia_la_CFLAGS =		\
	$(ZINNIA_CFLAGS)

zinnia_la_SOURCES = 			\
	tomoe-recognizer-zinnia.c

zinnia_la_LIBADD = $(TOMOE_LIBS) $(top_builddir)/lib/libtomoe.la $(ZINNIA_LIB)
