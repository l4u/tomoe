# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

AM_CPPFLAGS = \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\" \
	-DTOMOEDATADIR=\""$(datadir)"/$(PACKAGE)\"

INCLUDES =

#lib_LIBRARIES = libtomoe.a
#
#libtomoe_a_SOURCES = \
#	array.h array.c \
#	tomoe.h tomoe.c
#
#libtomoe_a_LD_ADD = \
#	-lm

lib_LTLIBRARIES = libtomoe.la
libtomoe_la_SOURCES = \
	array.h array.c \
	tomoe.h tomoe.c

libtomoe_la_LDFLAGS = -version-info 1:0:1

libtomoe_la_LD_ADD = \
	-lm
