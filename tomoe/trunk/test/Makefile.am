# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

AM_CPPFLAGS = \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\" \
	-DTOMOEDATADIR=\""$(datadir)"/$(PACKAGE)\"

INCLUDES = \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src 

noinst_PROGRAMS = \
	testtomoe

testtomoe_SOURCES = \
	testtomoe.c
testtomoe_LDADD = \
	$(top_builddir)/src/libtomoe.a \
	-lm
