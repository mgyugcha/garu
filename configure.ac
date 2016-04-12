#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Garu Music], [1], [mgyugcha@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

GLIB_GSETTINGS

PKG_CHECK_MODULES([GTK], [gtk+-3.0])
PKG_CHECK_MODULES([GSTREAMER], [gstreamer-1.0])
PKG_CHECK_MODULES([GSTREAMERUTILS], [gstreamer-pbutils-1.0])
PKG_CHECK_MODULES([TAGLIB_C], [taglib_c])

# Checks for header files.
AM_GNU_GETTEXT_VERSION([0.19.7])
AM_GNU_GETTEXT([external])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
		 data/Makefile
		 po/Makefile.in
                 src/Makefile])
AC_OUTPUT
