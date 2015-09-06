dnl
dnl configure.ac - autoconf script for termutils
dnl
dnl Copyright (c) 2015, Matthew Madison
dnl Distributed under license, see LICENSE for more information.
dnl

AC_INIT([termutils], [0.1.1])
AC_DEFINE([TERMUTILS_VERSION_MAJOR], [0], [Major version])
AC_DEFINE([TERMUTILS_VERSION_MINOR], [1], [Minor version])
AC_DEFINE([TERMUTILS_VERSION_MAINT], [1], [Maintenance level])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AM_SILENT_RULES([yes])
AC_COPYRIGHT([Copyright (c) 2015, Matthew Madison])
AC_CONFIG_MACRO_DIR([m4])

AC_PREREQ([2.69])
dnl LT_PREREQ([2.4])
dnl LT_INIT([])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
