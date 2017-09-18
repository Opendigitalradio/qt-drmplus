# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([libdrmplus],[1.0.1],[Sergiy <piratfm@gmail.com>])
#AM_INIT_AUTOMAKE([dist-bzip2])
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])
# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
# Checks for libraries.
AC_SEARCH_LIBS([sqrt],[m])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h memory.h stdint.h stdlib.h string.h],[],[
    AC_MSG_ERROR([You need standard header files.])
])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([floor memmove memset pow sqrt],[],[
    AC_MSG_ERROR([You basic math functions.])
])

dnl-----------------------------------------------------------------------------
dnl link the fftw3 library if requested
dnl-----------------------------------------------------------------------------
PKG_CHECK_MODULES([FFTW3], [fftw3 >= 3.3.0])
AC_SUBST(FFTW3_INCFLAGS)
AC_SUBST(FFTW3_CFLAGS)
AC_SUBST(FFTW3_LIB)
AC_SUBST(FFTW3_LDFLAGS)

DRMPLUS_VERSION="$VERSION"
DRMPLUS_CPPFLAGS="-I$drmplus_includedir $FFTW3_CPPFLAGS"
DRMPLUS_CFLAGS+="$FFTW3_CFLAGS"
DRMPLUS_LIBS="-ldrmplus -lm ${FFTW3_LIB} ${FFTW3_LDFLAGS}"

AC_SUBST(DRMPLUS_LIBS)
AC_SUBST(DRMPLUS_REQUIRES)
AC_SUBST(DRMPLUS_CPPFLAGS)
AC_SUBST(DRMPLUS_CFLAGS)

dnl-----------------------------------------------------------------------------
dnl link the fftw3 library if requested
dnl-----------------------------------------------------------------------------
#PKG_CHECK_MODULES([FAAD2_DRM], [faad2_drm >= 2.0.0])
#AC_SUBST(FAAD2_DRM_INCFLAGS)
#AC_SUBST(FAAD2_DRM_CFLAGS)
#AC_SUBST(FAAD2_DRM_LIB)
#AC_SUBST(FAAD2_DRM_LDFLAGS)

dnl Make substitutions 

AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(OPT)
AC_SUBST(LIBS)
AC_SUBST(DEBUG)
AC_SUBST(CFLAGS)
AC_SUBST(PROFILE)

AC_CONFIG_FILES([ drmplus.spec drmplus.pc ])
AC_OUTPUT([ Makefile src/Makefile include/Makefile frontend/Makefile ])
