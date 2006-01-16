## -*- Mode: Makefile; tab-width: 8; indent-tabs-mode: t; -*-

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

MAINTAINERCLEANFILE = Makefile.in html/*
CLEANFILES = *~ *.bak
EXTRA_DIST = doxygen.conf.in

.PHONY: html


if HAVE_DOXYGEN
html: 
	(cd $(top_builddir)/doc; rm -fr html/*; $(DOXYGEN) doxygen.conf)

dist-hook: html
	cp -Rp $(top_builddir)/doc/html $(distdir)
else
html:
	echo "***"
	echo "*** Warning: Documentation not built"
	echo "***"

dist-hook:
	echo "***"
	echo "*** Warning: Documentation not built"
	echo "*** DISTRIBUTION IS INCOMPLETE"
	echo "***"
endif
