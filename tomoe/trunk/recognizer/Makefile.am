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
	-DG_LOG_DOMAIN=\"TOMOE-RECOGNIZER\"			\
	-DSYSCONFDIR=\""$(sysconfdir)"\"			\
	-DDATADIR=\""$(datadir)"\"				\
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\"	\
	-DTOMOEDATADIR=\""$(datadir)"/$(PACKAGE)\"		\
	-I$(top_srcdir)/lib

INCLUDES = $(GLIB_CFLAGS)

recognizer_LTLIBRARIES = libsimple.la

libsimple_la_SOURCES = \
	tomoe-recognizer-simple.c tomoe-recognizer-simple-logic.c

libsimple_la_LDFLAGS =						\
  -version-info $(LT_VERSION_INFO)				\
  -export-dynamic $(no_undefined) $(LIBTOOL_EXPORT_OPTIONS)

libsimple_la_LD_ADD = \
	-lm

libsimple_la_LIBADD = $(GLIB_LIBS)
