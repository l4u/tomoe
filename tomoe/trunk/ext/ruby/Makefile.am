# -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

rubylibdir = $(RUBY_LIBDIR)
rubylib_DATA = tomoe.rb

EXTRA_DIST = \
	$(rubylib_DATA)

rubyarchdir = $(RUBY_EXTDIR)
rubyarch_HEADERS = rbtomoe.h
rubyarch_LTLIBRARIES = tomoe.la

AM_CPPFLAGS = \
	-DTOMOESYSCONFDIR=\""$(sysconfdir)/$(PACKAGE)"\"	\
	-DTOMOEDATADIR=\""$(datadir)"/$(PACKAGE)\"		\
	-I$(top_srcdir)/lib

INCLUDES = \
	$(RUBY_CFLAGS) \
	$(GLIB_CFLAGS) \
	-I$(top_srcdir)/lib

tomoe_la_SOURCES =		\
	tomoe-rb.c tomoe-rb.h	\
	tomoe-rb-candidate.c	\
	tomoe-rb-char.c		\
	tomoe-rb-config.c	\
	tomoe-rb-context.c	\
	tomoe-rb-dict.c		\
	tomoe-rb-query.c	\
	tomoe-rb-reading.c	\
	tomoe-rb-recognizer.c	\
	tomoe-rb-writing.c

tomoe_la_LDFLAGS = \
	-module \
	-version-info $(LT_VERSION_INFO) \
	$(RUBY_LDFLAGS) \
	$(RUBY_EXT_LDFLAGS)

tomoe_la_LIBADD = \
	$(RUBY_LIBS) \
	$(GLIB_LIBS) \
	$(top_builddir)/lib/libtomoe.la
