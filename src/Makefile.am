# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

AM_CPPFLAGS = \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\" \
	-DTOMOEDATADIR=\""$(datadir)"/$(PACKAGE)\"

INCLUDES = \
	-I$(top_srcdir)/lib \
	-I$(top_builddir)/lib

bin_PROGRAMS = \
	tomoe1 tomoe

tomoe1_SOURCES = \
	match.c

tomoe1_LDADD = \
	-lm

tomoe_SOURCES = \
	main.c

tomoe_LDADD = \
	$(top_builddir)/lib/libtomoe.la \
	-lm

EXTRA_DIST = \
	match.rb
