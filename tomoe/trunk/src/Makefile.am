# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

AM_CPPFLAGS = \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\" \
	-DTOMOEDATADIR=\""$(datadir)"/$(PACKAGE)\"

INCLUDES =

EXTRA_DIST = \
	match.rb

lib_LIBRARIES = libtomoe.a

libtomoe_a_SOURCES = \
	tomoe.h tomoe.c

libtomoe_a_LD_ADD = \
	-lm
