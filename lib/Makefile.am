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

CLEANFILES = *~ *.bak

AM_CPPFLAGS =							\
	-DSYSCONFDIR=\""$(sysconfdir)"\"			\
	-DDATADIR=\""$(datadir)"\"				\
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\"	\
	-DTOMOEDATADIR=\""$(datadir)"/$(PACKAGE)\"

INCLUDES = $(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS)

pkginclude_HEADERS = \
	tomoe.h \
	tomoe-array.h \
	tomoe-char.h \
	tomoe-dict.h \
	tomoe-handwrite.h \
	tomoe-data-types.h \
	tomoe-debug.h \
	tomoe-db.h \
	tomoe-config.h \
	tomoe-object.h

lib_LTLIBRARIES = libtomoe.la
libtomoe_la_SOURCES = \
	tomoe.c \
	tomoe-array.c \
	tomoe-char.c \
	tomoe-dict.c \
	tomoe-handwrite.c \
	tomoe-debug.c \
	tomoe-db.c \
	tomoe-config.c \
	tomoe-object.c

libtomoe_la_LDFLAGS =						\
  -version-info $(LT_VERSION_INFO)				\
  -export-dynamic $(no_undefined) $(LIBTOOL_EXPORT_OPTIONS)

libtomoe_la_LD_ADD = \
	-lm

libtomoe_la_LIBADD = $(LIBXML_LIBS) $(LIBXSLT_LIBS)
