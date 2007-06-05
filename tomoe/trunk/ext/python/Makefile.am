pkgpyexec_LTLIBRARIES = tomoe.la

INCLUDES =			\
	$(PYTHON_CFLAGS)	\
	$(PYGOBJECT_CFLAGS)	\
	$(TOMOE_CFLAGS)

nodist_tomoe_la_SOURCES =	\
	tomoe.c

tomoe_la_SOURCES =	\
	tomoe-module.c

tomoe_la_LDFLAGS =				\
	-module					\
	-avoid-version				\
	$(PYTHON_LDFLAGS)

tomoe_la_LIBADD =			\
	$(TOMOE_LIBS)			\
	$(PYTHON_LIBS)			\
	$(top_builddir)/lib/libtomoe.la

OVERRIDES =					\
	tomoe.override				\
	tomoe-char.override			\
	tomoe-context.override			\
	tomoe-dict.override			\
	tomoe-query.override			\
	tomoe-recognizer.override		\
	tomoe-shelf.override			\
	tomoe-writing.override

tomoe.c: tomoe.defs $(OVERRIDES)

CLEANFILES = tomoe.c
EXTRA_DIST = tomoe.defs $(OVERRIDES)

.defs.c:
	(cd $(srcdir)				\
	 && $(PYGTK_CODEGEN)			\
	    --override $*.override		\
	    --prefix py$* $*.defs) > gen-$*.c	\
	 && cp gen-$*.c $*.c			\
	 && rm -f gen-$*.c
