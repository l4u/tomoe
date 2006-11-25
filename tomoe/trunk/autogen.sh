#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

libtoolize --copy --force \
  && gtkdocize \
  && aclocal \
  && autoheader \
  && automake --add-missing --foreign --copy \
  && autoconf
