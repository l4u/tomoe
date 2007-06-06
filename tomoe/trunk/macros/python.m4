# -*- autoconf -*-
AC_DEFUN([PYTHON_CHECK],
[
python_available="no"
AC_ARG_WITH([python],
            AS_HELP_STRING([--with-python=PATH],
                           [Python interpreter path (default: auto-detect)]),
            [PYTHON="$withval"])

if test "$PYTHON" = "no"; then
  : # ignore
elif test "$PYTHON" = ""; then
  AM_PATH_PYTHON
  if test "$PYTHON" != "none"; then
    python_available="yes"
  fi
else
  AC_CHECK_FILE([$PYTHON],
                [python_available="yes"],
                [AC_MSG_WARN([$PYTHON is not found. Disable PYTHON binding.])])
fi

if test "$python_available" = "yes"; then
  AC_PATH_PROG(PYTHON_CONFIG, python-config, no)

  PYTHON_CFLAGS=`$PYTHON_CONFIG --cflags`
  PYTHON_LIBS=`$PYTHON_CONFIG --libs`
  PYTHON_LDFLAGS=`$PYTHON_CONFIG --ldflags`

  PKG_CHECK_MODULES(PYGOBJECT, pygobject-2.0)

  AC_SUBST(PYTHON_CFLAGS)
  AC_SUBST(PYTHON_LIBS)
  AC_SUBST(PYTHON_LDFLAGS)

  AC_SUBST(PYGOBJECT_CFLAGS)

  python_undef_package_macros="
#undef PACKAGE_NAME
#undef PACKAGE_TARNAME
#undef PACKAGE_STRING
#undef PACKAGE_VERSION
"
  python_disable_message="Disable Python binding."
  CFLAGS="$CFLAGS $PYTHON_CFLAGS"
  AC_CHECK_HEADERS(Python.h, [],
                   [python_available="no"
                    AC_MSG_WARN([$python_disable_message])],
                   [$python_undef_package_macros])
  CFLAGS=$_SAVE_CFLAGS

  AC_PATH_PROG(PYGTK_CODEGEN, pygtk-codegen-2.0, none)
fi

AM_CONDITIONAL([WITH_PYTHON], [test "$python_available" = "yes"])
])
