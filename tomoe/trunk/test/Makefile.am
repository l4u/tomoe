# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

AM_CPPFLAGS = \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\" \
	-DTOMOEDATADIR=\""$(top_srcdir)"/data\"

INCLUDES = \
	-I$(top_srcdir)/lib \
	-I$(top_builddir)/lib

noinst_PROGRAMS = \
	testtomoe

testtomoe_SOURCES = \
	testtomoe.c
testtomoe_LDADD = \
	$(top_builddir)/lib/libtomoe.la \
	-lm
EXTRA_DIST = \
	haru \
	natu \
	aki \
	fuyu
