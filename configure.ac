dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.57)

m4_define([tomoe_major_version], [0])
m4_define([tomoe_minor_version], [6])
m4_define([tomoe_micro_version], [0])
m4_define([tomoe_version],
          [tomoe_major_version.tomoe_minor_version.tomoe_micro_version])

AC_INIT(tomoe, [tomoe_version])
AM_INIT_AUTOMAKE(tomoe, [tomoe_version])
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_HOST

AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])

AC_MSG_CHECKING([for some Win32 platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
    platform_win32=yes
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])

AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(yes)

AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

TOMOE_VERSION=tomoe_version
AC_SUBST(TOMOE_VERSION)

LT_CURRENT=1
LT_REVISION=0
LT_AGE=1
LT_VERSION_INFO="$LT_CURRENT:$LT_REVISION:$LT_AGE"
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)
AC_SUBST(LT_VERSION_INFO)

LT_CURRENT_MINUS_AGE=`expr $LT_CURRENT - $LT_AGE`
AC_SUBST(LT_CURRENT_MINUS_AGE)

LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)


dnl **************************************************************
dnl Checks for programs.
dnl **************************************************************
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_AWK


if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac

  case " $CFLAGS " in
  *[\ \	]-Wmissing-declarations[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wmissing-declarations" ;;
  esac

  case " $CFLAGS " in
  *[\ \	]-Wmissing-prototypes[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wmissing-prototypes" ;;
  esac

  case " $CFLAGS " in
  *[\ \	]-Wpointer-arith[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wpointer-arith" ;;
  esac

  case " $CFLAGS " in
  *[\ \	]-Wcast-align[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wcast-align" ;;
  esac

  if test "x$enable_ansi" = "xyes"; then
    case " $CFLAGS " in
    *[\ \	]-ansi[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -ansi" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-pedantic[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -pedantic" ;;
    esac
  fi
fi

dnl **************************************************************
dnl Checks for pkgconfig
dnl **************************************************************

PKG_PROG_PKG_CONFIG()

dnl **************************************************************
dnl Checks for gtk-doc
dnl **************************************************************

GTK_DOC_CHECK([1.4])

dnl **************************************************************
dnl Check for gettext.
dnl **************************************************************

IT_PROG_INTLTOOL([0.35.0])

AM_GLIB_GNU_GETTEXT
dnl should run after AM_GLIB_GNU_GETTEXT
AC_CONFIG_COMMANDS([modify-itlocaledir], [
sed \
  -i'' \
  -e "s/^itlocaledir = /itlocaledir = \$(datadir)\/locale\n#itlocaledir = /" \
  po/Makefile
])
GETTEXT_PACKAGE=tomoe
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Gettext package])

dnl **************************************************************
dnl Checks for xsltproc
dnl **************************************************************

AC_PATH_PROG(XSLTPROC, xsltproc, none)

dnl **************************************************************
dnl Checks for rxp
dnl **************************************************************

AC_PATH_PROG(RXP, rxp, none)

dnl **************************************************************
dnl Checks for libraries.
dnl **************************************************************
# FIXME: Replace `main' with a function in `-le':
AC_CHECK_LIB([e], [main])
# FIXME: Replace `main' with a function in `-links':
AC_CHECK_LIB([inks], [main])


dnl **************************************************************
dnl Checks for header files.
dnl **************************************************************
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h libintl.h stdlib.h string.h unistd.h])


dnl **************************************************************
dnl Checks for typedefs, structures, and compiler characteristics.
dnl **************************************************************
AC_C_CONST


dnl **************************************************************
dnl Checks for library functions.
dnl **************************************************************
AC_CHECK_FUNCS([setlocale strchr strtol uname memmove strerror])


dnl **************************************************************
dnl Check for MinGW
dnl **************************************************************

AC_ARG_ENABLE([mingw-debug],
              AS_HELP_STRING([--enable-mingw-debug],
	                     [Enable MinGW debug]))
case $host_os in
mingw*)
  if test "x$enable_mingw_debug" != "xno"; then
    LDFLAGS="$LDFLAGS -mwindows"
  fi
  AC_CHECK_TOOL(WINDRES, windres)
  ;;
esac
AM_CONDITIONAL([HAVE_WINDRES], [test x"$WINDRES" != x])


dnl **************************************************************
dnl Check for GLib.
dnl **************************************************************
GLIB_REQUIRED=2.8.0
AC_SUBST(GLIB_REQUIRED)
AM_PATH_GLIB_2_0($GLIB_REQUIRED, [], [], [gmodule gobject])

dnl **************************************************************
dnl Check for Hyper Estraier.
dnl **************************************************************

use_est=no
PKG_CHECK_MODULES(EST, hyperestraier,
                  [use_est=yes], [use_est=no])
AM_CONDITIONAL([USE_EST], [test x"$use_est" = "xyes"])

dnl **************************************************************
dnl Configure for configuration.
dnl **************************************************************

confdir="\${sysconfdir}/\$(PACKAGE)"
AC_SUBST(confdir)

dnl **************************************************************
dnl Configure for data.
dnl **************************************************************

dict_datadir="\${pkgdatadir}/dict"
AC_SUBST(dict_datadir)

recognizer_datadir="\${pkgdatadir}/recognizer"
AC_SUBST(recognizer_datadir)

dnl **************************************************************
dnl Bindings: Check for Ruby.
dnl **************************************************************

RUBY_CHECK([rbgobject.h], [$GLIB_CFLAGS])

dnl **************************************************************
dnl Bindings: Check for Python.
dnl **************************************************************
PYTHON_CHECK

dnl **************************************************************
dnl Configure for modules.
dnl **************************************************************

moduledir="\${libdir}/\$(PACKAGE)/module"
AC_SUBST(moduledir)

dnl **************************************************************
dnl Configure for recognizer modules.
dnl **************************************************************

recognizer_moduledir="\${moduledir}/recognizer"
AC_SUBST(recognizer_moduledir)

dnl **************************************************************
dnl Configure for Zinnia recognizer modules.
dnl **************************************************************

use_zinnia=no
ZINNIA_CFLAGS=
ZINNIA_LIBS=
AC_SUBST(ZINNIA_CFLAGS)
AC_SUBST(ZINNIA_LIBS)
PKG_CHECK_MODULES(ZINNIA, zinnia,
                  [use_zinnia=yes],
                  [use_zinnia=no
                   AC_MSG_NOTICE([Zinnia doesn't support pkg-config. :(])])
if test x"$use_zinnia" != x"yes"; then
  AC_CHECK_HEADERS([zinnia.h], [use_zinnia=yes], [use_zinnia=no])
  if test x"$use_zinnia" = x"yes"; then
    AC_CHECK_LIB([zinnia], [zinnia_version],
                 [ZINNIA_LIBS="-lzinnia"],
                 [use_zinnia=no])
  fi
fi
AM_CONDITIONAL(USE_ZINNIA, [test x"$use_zinnia" = x"yes"])

dnl **************************************************************
dnl Configure for dictionary modules.
dnl **************************************************************

dict_moduledir="\${moduledir}/dict"
AC_SUBST(dict_moduledir)

dnl **************************************************************
dnl Configure for Unihan dictionary.
dnl **************************************************************

AC_ARG_ENABLE(unihan,
              AS_HELP_STRING([--enable-unihan],
                             [Enable Unihan detabase (default: yes)]),
              [use_unihan=$enableval],
              [use_unihan=yes])
AM_CONDITIONAL([USE_UNIHAN], [test x"$use_unihan" = "xyes"])

AC_PATH_PROG(UNZIP, unzip, unzip-not-found)

dnl **************************************************************
dnl Configure for Subverion dictionary.
dnl **************************************************************

SVN_CHECK()

dnl **************************************************************
dnl Configure for MySQL dictionary.
dnl **************************************************************

use_mysql=no

AC_PATH_PROG(MYSQL_CONFIG, mysql_config, mysql_config-not-found)

if test "x$MYSQL_CONFIG" != "xmysql_config-not-found"; then
  use_mysql=yes
  MYSQL_CFLAGS=`$MYSQL_CONFIG --cflags`
  MYSQL_LIBS=`$MYSQL_CONFIG --libs`
fi

AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_LIBS)

AM_CONDITIONAL([USE_MYSQL], [test "x$use_mysql" = "xyes"])

dnl **************************************************************
dnl Configure for Ruby dictionary.
dnl **************************************************************
AC_ARG_ENABLE(dict_ruby,
              AS_HELP_STRING([--enable-dict-ruby],
                             [Enable Ruby based dictionary module (default: no)]),
              [use_dict_ruby=$enableval],
              [use_dict_ruby=no])
if test x"$ruby_available" != "xyes"; then
  use_dict_ruby=no
fi
AM_CONDITIONAL(USE_DICT_RUBY, [test x"$use_dict_ruby" = "xyes"])

dnl **************************************************************
dnl Configure for KanjiDic.
dnl **************************************************************

kanjidic_result="bundled"
AC_ARG_WITH([kanjidic],
            AS_HELP_STRING([--with-kanjidic=PATH],
                           [Use specified kanjidic instead of including kanjidic2]),
            [use_kanjidic=yes; KANJIDIC="$withval"])

AC_SUBST(KANJIDIC)

if test x"$ruby_available" != "xyes"; then
  use_kanjidic=no
fi
if test x"$use_kanjidic" = "xyes"; then
  kanjidic_result=$KANJIDIC
fi

AM_CONDITIONAL(USE_KANJIDIC, [test x"$use_kanjidic" = "xyes"])


dnl **************************************************************
dnl Configure for Tomoe.
dnl **************************************************************

TOMOE_CFLAGS="$TOMOE_CFLAGS \$(GLIB_CFLAGS)"
TOMOE_CFLAGS="$TOMOE_CFLAGS -DCONFDIR=\\\"\"\$(confdir)\"\\\""
TOMOE_CFLAGS="$TOMOE_CFLAGS -DDATADIR=\\\"\"\$(pkgdatadir)\"\\\""
TOMOE_CFLAGS="$TOMOE_CFLAGS -DDICT_DATADIR=\\\"\"\$(dict_datadir)\"\\\""
TOMOE_CFLAGS="$TOMOE_CFLAGS -DRECOGNIZER_DATADIR=\\\"\"\$(recognizer_datadir)\"\\\""
TOMOE_CFLAGS="$TOMOE_CFLAGS -DDICT_MODULEDIR=\\\"\"\$(dict_moduledir)\"\\\""
TOMOE_CFLAGS="$TOMOE_CFLAGS -DRECOGNIZER_MODULEDIR=\\\"\"\$(recognizer_moduledir)\"\\\""
TOMOE_CFLAGS="$TOMOE_CFLAGS -I\$(top_srcdir)/lib"
AC_SUBST(TOMOE_CFLAGS)

TOMOE_LIBS='$(GLIB_LIBS)'
AC_SUBST(TOMOE_LIBS)

AC_CONFIG_FILES([
Makefile
po/Makefile.in
lib/Makefile
module/Makefile
module/dict/Makefile
module/recognizer/Makefile
data/Makefile
bindings/Makefile
bindings/ruby/Makefile
bindings/python/Makefile
bindings/python/pytomoe.pc
db/Makefile
db/lib/Makefile
db/migrate/Makefile
test/Makefile
test/data/Makefile
test/python/Makefile
benchmark/Makefile
doc/Makefile
doc/reference/Makefile
doc/reference/version.xml
tomoe.pc
spec/Makefile
spec/turbo-linux/Makefile
spec/turbo-linux/tomoe.spec
spec/miracle-linux/Makefile
spec/miracle-linux/tomoe.spec
])

AC_OUTPUT

dnl **************************************************************
dnl Show configure result
dnl **************************************************************

echo
echo "Configure Result:"
echo
echo "  * Language Bindings"
echo
echo "      Ruby            : ${ruby_available}"
echo "      Python          : ${python_available}"
echo
echo "  * Dictionary Backends"
echo
echo "      Unihan Database : ${use_unihan}"
echo "      Hyper Estraier  : ${use_est}"
echo "      Subversion      : ${use_svn}"
echo "      MySQL           : ${use_mysql}"
echo "      Ruby            : ${use_dict_ruby}"
echo
echo "  * Recognizer Backends"
echo
echo "      Simple          : yes"
echo "      Zinnia          : ${use_zinnia}"
echo
echo "  * Other Features"
echo
echo "      gtk-doc         : ${enable_gtk_doc}"
echo "      KanjiDic        : ${kanjidic_result}"
echo
