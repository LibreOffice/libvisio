AC_INIT(src/lib/VisioDocument.cpp)
AC_PREREQ([2.65])

AM_CONFIG_HEADER(config.h)

VSD_MAJOR_VERSION=0
VSD_MINOR_VERSION=0
VSD_MICRO_VERSION=5

LIBWPD_REQUIRED_VERSION="0.9.0"
LIBWPG_REQUIRED_VERSION="0.2.0"

VSD_VERSION="$VSD_MAJOR_VERSION.$VSD_MINOR_VERSION.$VSD_MICRO_VERSION"

AC_SUBST(VSD_MAJOR_VERSION)
AC_SUBST(VSD_MINOR_VERSION)
AC_SUBST(VSD_MICRO_VERSION)
AC_SUBST(VSD_VERSION)

dnl # libtool versioning
dnl LT_RELEASE=$VSD_MAJOR_VERSION.$VSD_MINOR_VERSION
dnl AC_SUBST(LT_RELEASE)

LT_CURRENT=`expr 100 '*' $VSD_MAJOR_VERSION + $VSD_MINOR_VERSION`
LT_REVISION=$VSD_MICRO_VERSION
# For 1.0.0 comment the first line and uncomment the second
LT_AGE=0
# LT_AGE=$VSD_MINOR_VERSION
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

AC_CONFIG_MACRO_DIR([m4])

# initialize automake
#
AM_INIT_AUTOMAKE(libvisio, $VSD_VERSION, foreign dist-bzip2)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_LIBTOOL_WIN32_DLL
AC_ARG_ENABLE([static-tools],
	[AS_HELP_STRING([--enable-static-tools], [Link tools (binaries) statically])]
)
AS_IF([test "x$enable_static_tools" = "xyes"], [
	enable_static="yes"
], [
	AC_DISABLE_STATIC
])
AM_CONDITIONAL(STATIC_TOOLS, [test "x$enable_static_tools" = "xyes"])

AC_ARG_ENABLE([werror],
	[AS_HELP_STRING([--disable-werror], [Treat all warnings as errors, usefull for development])]
)
AS_IF([test x"$enable_werror" != "xno"], [
	CFLAGS="$CFLAGS -Werror"
	CXXFLAGS="$CXXFLAGS -Werror"
])

AC_PROG_LIBTOOL

AC_SUBST(VSD_OBJDIR, [$objdir])

AM_PROG_LIBTOOL
AC_PATH_PROG(PKG_CONFIG, [pkg-config], [no])

dnl ***********************************************************************************

PKG_CHECK_MODULES(LIBVISIO,[
	libwpd-0.9 >= $LIBWPD_REQUIRED_VERSION
	libwpd-stream-0.9 >= $LIBWPD_REQUIRED_VERSION
	libwpg-0.2 >= $LIBWPG_REQUIRED_VERSION
])

AC_HEADER_STDC

AC_MSG_CHECKING([for native Win32])
case "$host" in
	*-*-mingw*)
		native_win32=yes
		LIBVISIO_WIN32_RESOURCE=libvisio-win32res.lo
		VSD2RAW_WIN32_RESOURCE=vsd2raw-win32res.lo
		VSD2XHTML_WIN32_RESOURCE=vsd2xhtml-win32res.lo
	;;
	*)
		native_win32=no
		LIBVISIO_WIN32_RESOURCE=
		VSD2RAW_WIN32_RESOURCE=
		VSD2XHTML_WIN32_RESOURCE=
	;;
esac
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(OS_WIN32, [test "x$native_win32" = "xyes"])
AC_SUBST(LIBVISIO_WIN32_RESOURCE)
AC_SUBST(VSD2RAW_WIN32_RESOURCE)
AC_SUBST(VSD2XHTML_WIN32_RESOURCE)

AC_ARG_WITH(dummy_bitmaps,
	[AS_HELP_STRING([--with-dummy-bitmaps],
		[Output dummy bitmaps of 1x1 pixels instead of the real bitmaps (speeds up the validation test)])]
)
AM_CONDITIONAL([OUTPUT_DUMMY_BITMAPS], [test "x$with_dummy_bitmaps" = "xyes"])

AC_ARG_WITH(docs,
	[AS_HELP_STRING([--without-docs], [Do not build documentation])],
)
AS_IF([test "x$native_win32" = "xyes"], [with_docs=false])
AS_IF([test "x$with_docs" != "xno"], [
	AC_PATH_PROG(DOXYGEN, [doxygen])
	AS_IF([test -z "$DOXYGEN"], [
		AC_MSG_WARN([*** Could not find doxygen in your PATH.])
		AC_MSG_WARN([*** The documentation will not be built.])
		enable_docs=false
	], [enable_docs=true])
], [enable_docs=false])
AM_CONDITIONAL([WITH_LIBVISIO_DOCS], [test "x$enable_docs" != "xfalse"])

AC_MSG_CHECKING([for Win32 platform in general])
case "$host" in
	*-*-mingw*|*-*-cygwin*)
		platform_win32=yes
	;;
	*)
		platform_win32=no
	;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL([PLATFORM_WIN32], [test "x$platform_win32" = "xyes"])

AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug], [Turn on debugging])]
)
AS_IF([test "x$enable_debug" = "xyes"], [
	DEBUG_CXXFLAGS="-DDEBUG -g"
], [
	DEBUG_CXXFLAGS="-DNDEBUG"
])

# Courtesy of Glib: Ensure MSVC-compatible struct packing convention
# is used when compiling for Win32 with gcc.
# What flag to depends on gcc version: gcc3 uses "-mms-bitfields", while
# gcc2 uses "-fnative-struct".
AS_IF([test "x$native_win32" = "xyes"], [
	AC_CHECK_TOOL(WINDRES, windres)
	AS_IF([x"$GCC" = xyes], [
		AC_MSG_CHECKING([how to get MSVC-compatible struct packing])
		AS_IF([test -z "$ac_cv_prog_CC"], [
			our_gcc="$CC"
		], [
			our_gcc="$ac_cv_prog_CC"
		])
		case `$our_gcc --version | sed -e 's,\..*,.,' -e q` in
		2.)
			AS_IF([$our_gcc -v --help 2>/dev/null | grep fnative-struct >/dev/null], [
				msnative_struct='-fnative-struct'
			])
		;;
		*)
			AS_IF([$our_gcc -v --help 2>/dev/null | grep ms-bitfields >/dev/null], [
				msnative_struct='-mms-bitfields'
			])
		;;
		esac
		AS_IF([test -z "$msnative_struct"], [
			AC_MSG_RESULT([no way])
			AC_MSG_WARN([produced libraries might be incompatible with MSVC-compiled code])
		], [
			CFLAGS="$CFLAGS $msnative_struct"
			CXXFLAGS="$CXXFLAGS $msnative_struct"
			AC_MSG_RESULT([${msnative_struct}])
		])
	])
	CFLAGS="$CFLAGS -Wall -pedantic"
	CXXFLAGS="$CXXFLAGS -Wall -pedantic"
], [
	AS_IF([test x"$GCC" = xyes], [
		# Be tough with warnings and produce less careless code
		CFLAGS="$CFLAGS -Wall -pedantic"
		CXXFLAGS="$CXXFLAGS -Wall -pedantic"
	])
])

LIBVISIO_CXXFLAGS="$LIBVISIO_CFLAGS"

AC_SUBST(LIBVISIO_CXXFLAGS)
AC_SUBST(DEBUG_CXXFLAGS)

AC_OUTPUT([
Makefile
src/Makefile
src/conv/Makefile
src/conv/raw/Makefile
src/conv/raw/vsd2raw.rc
src/conv/svg/Makefile
src/conv/svg/vsd2xhtml.rc
src/lib/Makefile
src/lib/libvisio.rc
build/Makefile
build/win32/Makefile
docs/Makefile
docs/doxygen/Makefile
libvisio-0.0.pc
libvisio.spec
libvisio-zip
])

AS_IF([test "x$enable_debug" = "xyes"], [
	AC_MSG_RESULT([Configured with debugging symbols.])
])
