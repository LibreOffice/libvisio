# Process this file with autoconf to create configure.

AC_PREREQ([2.65])

# ====================
# Version informations
# ====================
m4_define([libvisio_version_major],[0])
m4_define([libvisio_version_minor],[0])
m4_define([libvisio_version_micro],[8])
m4_define([libvisio_version],[libvisio_version_major.libvisio_version_minor.libvisio_version_micro])

# =============
# Automake init
# =============
AC_INIT([libvisio],[libvisio_version])
AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([1.11 foreign dist-xz dist-bzip2])
AM_SILENT_RULES([yes])
AC_LANG([C++])

# ===========================
# Find required base packages
# ===========================
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_LIBTOOL_WIN32_DLL
PKG_PROG_PKG_CONFIG([0.20])

# ====================
# Find additional apps
# ====================
PKG_CHECK_MODULES([LIBVISIO],[
	libwpd-0.9
	libwpd-stream-0.9
	libwpg-0.2
])

# =================================
# Libtool/Version Makefile settings
# =================================
AC_SUBST(VSD_MAJOR_VERSION, [libvisio_version_major])
AC_SUBST(VSD_MINOR_VERSION, [libvisio_version_minor])
AC_SUBST(VSD_MICRO_VERSION, [libvisio_version_micro])
AC_SUBST(VSD_VERSION, [libvisio_version])
# AC_SUBST(LT_RELEASE, [libvisio_version_major.libvisio_version_minor])
LT_CURRENT=`expr 100 '*' libvisio_version_major + libvisio_version_minor`
# For 1.0.0 comment the first line and uncomment the second
LT_AGE=0
# LT_AGE=libvisio_version_minor
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION, [libvisio_version_micro])
AC_SUBST(LT_AGE)

# ==========================
# Platform check for windows
# ==========================
AC_MSG_CHECKING([for native Win32])
AS_CASE([$host],
	[*-*-mingw*], [
		native_win32=yes
		LIBVISIO_WIN32_RESOURCE=libvisio-win32res.lo
		VSD2RAW_WIN32_RESOURCE=vsd2raw-win32res.lo
		VSD2XHTML_WIN32_RESOURCE=vsd2xhtml-win32res.lo
	], [
		native_win32=no
		LIBVISIO_WIN32_RESOURCE=
		VSD2RAW_WIN32_RESOURCE=
		VSD2XHTML_WIN32_RESOURCE=
	]
)
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(OS_WIN32, [test "x$native_win32" = "xyes"])
AC_SUBST(LIBVISIO_WIN32_RESOURCE)
AC_SUBST(VSD2RAW_WIN32_RESOURCE)
AC_SUBST(VSD2XHTML_WIN32_RESOURCE)
AC_MSG_CHECKING([for Win32 platform in general])
AS_CASE([$host],
	[*-*-mingw*|*-*-cygwin*], [platform_win32=yes],
	[platform_win32=no]
)
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL([PLATFORM_WIN32], [test "x$platform_win32" = "xyes"])

# ================
# Check for cflags
# ================
AC_ARG_ENABLE([werror],
	[AS_HELP_STRING([--disable-werror], [Treat all warnings as errors, usefull for development])],
	[enable_werror="$enableval"],
	[enable_werror=yes]
)
AS_IF([test x"$enable_werror" != "xno"], [
	CFLAGS="$CFLAGS -Werror"
	CXXFLAGS="$CXXFLAGS -Werror"
])
# Courtesy of Glib: Ensure MSVC-compatible struct packing convention
# is used when compiling for Win32 with gcc.
AS_IF([test "x$native_win32" = "xyes"], [
	AC_CHECK_TOOL(WINDRES, windres)
	AS_IF([test x"$GCC" = xyes], [
		AC_MSG_CHECKING([how to get MSVC-compatible struct packing])
		AS_IF([test -z "$ac_cv_prog_CC"], [
			our_gcc="$CC"
		], [
			our_gcc="$ac_cv_prog_CC"
		])
		AS_IF([$our_gcc -v --help 2>/dev/null | grep ms-bitfields >/dev/null], [
			msnative_struct="-mms-bitfields"
			CFLAGS="$CFLAGS $msnative_struct"
			CXXFLAGS="$CXXFLAGS $msnative_struct"
			AC_MSG_RESULT([${msnative_struct}])
		], [
			AC_MSG_RESULT([no way])
			AC_MSG_WARN([produced libraries might be incompatible with MSVC-compiled code])
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

# ============
# Debug switch
# ============
AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug], [Turn on debugging])],
	[enable_debug="$enableval"],
	[enable_debug=no]
)
AS_IF([test "x$enable_debug" = "xyes"], [
	DEBUG_CXXFLAGS="-DDEBUG -g"
], [
	DEBUG_CXXFLAGS="-DNDEBUG"
])
AC_SUBST(DEBUG_CXXFLAGS)

# ============
# Static tools
# ============
AC_ARG_ENABLE([static-tools],
	[AS_HELP_STRING([--enable-static-tools], [Link tools (binaries) statically])],
	[enable_static_tools="$enableval"],
	[enable_static_tools=no]
)
AS_IF([test "x$enable_static_tools" = "xyes"], [
	enable_static="yes"
], [
	AC_DISABLE_STATIC
])
AM_CONDITIONAL(STATIC_TOOLS, [test "x$enable_static_tools" = "xyes"])

# =============
# Dummy bitmaps
# =============
AC_ARG_WITH(dummy_bitmaps,
	[AS_HELP_STRING([--with-dummy-bitmaps],
		[Output dummy bitmaps of 1x1 pixels instead of the real bitmaps (speeds up the validation test)])],
	[with_dummy_bitmaps="$withval"],
	[with_dummy_bitmaps=no]
)
AM_CONDITIONAL([OUTPUT_DUMMY_BITMAPS], [test "x$with_dummy_bitmaps" = "xyes"])

# =============
# Documentation
# =============
AC_ARG_WITH(docs,
	[AS_HELP_STRING([--without-docs], [Do not build documentation])],
	[with_docs="$withval"],
	[AS_IF([test "x$native_win32" = "xyes"], [with_docs=no], [with_docs=yes])]
)
AS_IF([test "x$with_docs" != "xno"], [
	AC_PATH_PROG(DOXYGEN, [doxygen])
	AS_IF([test -z "$DOXYGEN"], [
		AC_MSG_WARN([*** Could not find doxygen in your PATH.])
		AC_MSG_WARN([*** The documentation will not be built.])
		build_docs=no
	], [build_docs=yes])
], [build_docs=no])
AM_CONDITIONAL([WITH_LIBVISIO_DOCS], [test "x$build_docs" != "xno"])

# =====================
# Prepare all .in files
# =====================
AC_CONFIG_FILES([
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
AC_OUTPUT

# ==============================================
# Display final informations about configuration
# ==============================================
AC_MSG_NOTICE([
==============================================================================
Build configuration:
	debug:           ${enable_debug}
	docs:            ${build_docs}
	dummy-bitmaps:   ${with_dummy_bitmaps}
	static-tools:    ${enable_static_tools}
	werror:          ${enable_werror}
==============================================================================
])
