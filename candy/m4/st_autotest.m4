# serial 1
AC_DEFUN([AC_ST_AUTOTEST],[

# set up autotest
AM_MISSING_PROG(AUTOM4TE, autom4te)
AC_CONFIG_TESTDIR(tests)

AM_MISSING_PROG(LCOV, lcov)
AM_MISSING_PROG(LCOV_GENHTML, genhtml)
AM_MISSING_PROG(LCOV_GENDESC, gendesc)

])
