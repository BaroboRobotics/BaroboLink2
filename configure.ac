#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([RoboMancer], [0.1], [david@barobo.com])
AC_CONFIG_SRCDIR([buttons.x.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([RoboMancer], [0.1])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lmobot':
AC_CHECK_LIB([mobot], [Mobot_init])
LDFLAGS="$LDFLAGS -Llibstkcomms -lstkcomms"

AC_CANONICAL_HOST
echo $host_os 

case $host_os in
  linux* )
    AC_CHECK_LIB([scintilla], [scintilla_new], [], [], [-pthread -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lm -lcairo -lpango-1.0 -lfreetype -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0])
PKG_CHECK_MODULES([DEPS], [
                   gtk+-2.0 >= 2.20.0
                   gmodule-export-2.0 >= 2.20.0])
    ;;
  darwin* )
    AC_CHECK_LIB([scintilla], [scintilla_new], [], [], [-lgtk-quartz-2.0 -lgdk-quartz-2.0 -latk-1.0 -lgio-2.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lpango-1.0 -lcairo -lgmodule-2.0 -lgthread-2.0 -lgobject-2.0 -lglib-2.0 -lintl -lstdc++])
PKG_CHECK_MODULES([DEPS], [
                   gtk+-2.0 >= 2.20.0
                   gmodule-export-2.0 >= 2.20.0
                   gtk-mac-integration])
    ;;
esac

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
#AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset socket strdup strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
