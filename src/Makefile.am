# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

AM_CPPFLAGS = \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\" \
	-DTOMOEDATADIR=\""$(datadir)"/$(PACKAGE)\"

INCLUDES =

bin_PROGRAMS = tomoe

tomoe_SOURCES = \
	match.c

tomoe_LDADD = \
	-lm

EXTRA_DIST = \
	match.rb
