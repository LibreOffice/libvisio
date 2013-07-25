# Process this file with autoconf to create configure.

AC_PREREQ([2.65])

# ====================
# Version informations
# ====================
m4_define([libvisio_version_major],[0])
m4_define([libvisio_version_minor],[0])
m4_define([libvisio_version_micro],[30])
m4_define([libvisio_version],[libvisio_version_major.libvisio_version_minor.libvisio_version_micro])

# =============
# Automake init
# =============
AC_INIT([libvisio],[libvisio_version])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([1.11 foreign dist-xz dist-bzip2])
AM_SILENT_RULES([yes])
AC_LANG([C++])

# ===========================
# Find required base packages
# ===========================
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT([win32-dll disable-static pic-only])
AC_CANONICAL_HOST

PKG_PROG_PKG_CONFIG([0.20])

AC_PROG_EGREP
AC_PROG_SED

dnl ===================================================================
dnl Check for gperf
dnl ===================================================================
AC_ARG_VAR(GPERF, [Setting this variable will override the gperf detection])
AC_PATH_PROG(GPERF, gperf)
if test -z "$GPERF"; then
    AC_MSG_ERROR([gperf not found but needed. Install it.])
fi
AC_MSG_CHECKING([gperf version])
if test "`$GPERF --version | $EGREP ^GNU\ gperf | $AWK '{ print $3 }' | cut -d. -f1`" -ge "3"; then
    AC_MSG_RESULT([OK])
else
    AC_MSG_ERROR([too old, you need at least 3.0.0])
fi
AC_SUBST(GPERF)

# ====================
# Find additional apps
# ====================
PKG_CHECK_MODULES([WPD],[
	libwpd-0.9 >= 0.9.5
	libwpd-stream-0.9 >= 0.9.5
])

PKG_CHECK_MODULES([WPG],[
	libwpg-0.2
])

PKG_CHECK_MODULES([LIBXML],[
	libxml-2.0
])

# ========
# Find icu
# ========
AS_IF([test "${ICU_CFLAGS+set}" = set],[
	ICU_CFLAGS_overriden=yes],[
	ICU_CFLAGS_overriden=no])
PKG_CHECK_MODULES([ICU],[icu-i18n],[
	AS_IF([test "${ICU_CFLAGS_overriden}" = no], [
		ICU_CFLAGS="${ICU_CFLAGS} `${PKG_CONFIG} icu-i18n --variable=CXXFLAGS`"])
	], [
	AC_PATH_PROG([ICU_CONFIG],[icu-config])
	AC_MSG_CHECKING([ICU installation])
	AS_IF([${ICU_CONFIG} --cflags >/dev/null 2>&1],[
		ICU_CFLAGS=`${ICU_CONFIG} --cppflags-searchpath --cxxflags`
		ICU_LIBS=`${ICU_CONFIG} --ldflags`
		AC_MSG_RESULT([found])],[
		AC_MSG_ERROR([libicu config program icu-config not found])])]
)
AC_SUBST(ICU_CFLAGS)
AC_SUBST(ICU_LIBS)

# =========
# Find zlib
# =========
PKG_CHECK_MODULES([ZLIB],[zlib],[],[
    AC_CHECK_HEADER(zlib.h, [ZLIB_CFLAGS=],
        [AC_MSG_ERROR(zlib.h not found. install zlib)], [])
    AC_CHECK_LIB(z, inflate, [ ZLIB_LIBS=-lz ],
        [AC_MSG_ERROR(zlib not found or functional)], [])
])
AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)

# ===========================
# Find required boost headers
# ===========================

AC_CHECK_HEADERS(
	boost/algorithm/string.hpp \
	boost/archive/iterators/binary_from_base64.hpp \
	boost/archive/iterators/remove_whitespace.hpp \
	boost/archive/iterators/transform_width.hpp \
	boost/optional.hpp \
	boost/range/iterator_range.hpp \
	boost/spirit/include/classic.hpp,
	[],
	[AC_MSG_ERROR(Required boost headers not found. install boost >= 1.36)],
	[]
)

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
		VSS2RAW_WIN32_RESOURCE=vss2raw-win32res.lo
		VSD2TEXT_WIN32_RESOURCE=vsd2text-win32res.lo
		VSS2TEXT_WIN32_RESOURCE=vss2text-win32res.lo
		VSD2XHTML_WIN32_RESOURCE=vsd2xhtml-win32res.lo
		VSS2XHTML_WIN32_RESOURCE=vss2xhtml-win32res.lo
	], [
		native_win32=no
		LIBVISIO_WIN32_RESOURCE=
		VSD2RAW_WIN32_RESOURCE=
		VSS2RAW_WIN32_RESOURCE=
		VSD2TEXT_WIN32_RESOURCE=
		VSS2TEXT_WIN32_RESOURCE=
		VSD2XHTML_WIN32_RESOURCE=
		VSD2XHTML_WIN32_RESOURCE=
	]
)
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(OS_WIN32, [test "x$native_win32" = "xyes"])
AC_SUBST(LIBVISIO_WIN32_RESOURCE)
AC_SUBST(VSD2RAW_WIN32_RESOURCE)
AC_SUBST(VSS2RAW_WIN32_RESOURCE)
AC_SUBST(VSD2TEXT_WIN32_RESOURCE)
AC_SUBST(VSS2TEXT_WIN32_RESOURCE)
AC_SUBST(VSD2XHTML_WIN32_RESOURCE)
AC_SUBST(VSS2XHTML_WIN32_RESOURCE)

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
	[AS_HELP_STRING([--disable-werror], [Treat all warnings as errors, useful for development])],
	[enable_werror="$enableval"],
	[enable_werror=yes]
)
AS_IF([test x"$enable_werror" != "xno"], [
	CFLAGS="$CFLAGS -Werror"
	CXXFLAGS="$CXXFLAGS -Werror"
])
AC_ARG_ENABLE([weffc],
	[AS_HELP_STRING([--disable-weffc], [ Disable -Weffc++ warnings, usefull when using an old version of gcc or of boost])],
	[enable_weffc="$enableval"],
	[enable_weffc=yes]
)
AC_ARG_ENABLE([wparanoic],
	[AS_HELP_STRING([--enable-wparanoic], [Enable a lot of warnings...])],
	[enable_wparanoic="$enableval"],
	[enable_wparanoic=no]
)
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
	CFLAGS="$CFLAGS -Wall -Wextra -pedantic"
		CXXFLAGS="$CXXFLAGS -Wall -Wextra -pedantic -Wshadow -Wunused-variable"
		AS_IF([test x"$enable_weffc" != "xno"], [
			CXXFLAGS="$CXXFLAGS -Weffc++"
		])
		AS_IF([test x"$enable_wparanoic" != "xno"], [
			CXXFLAGS="$CXXFLAGS -Wcast-align -Wcast-qual -Wchar-subscripts"
			CXXFLAGS="$CXXFLAGS -Wcomment -Wconversion -Wdisabled-optimization"
			CXXFLAGS="$CXXFLAGS -Wfloat-equal -Wformat -Wformat=2"
			CXXFLAGS="$CXXFLAGS -Wformat-nonliteral -Wformat-security"
			CXXFLAGS="$CXXFLAGS -Wformat-y2k -Wimport -Winit-self -Winvalid-pch"
			CXXFLAGS="$CXXFLAGS -Wmissing-braces -Wmissing-field-initializers"
			CXXFLAGS="$CXXFLAGS -Wmissing-format-attribute -Wmissing-include-dirs"
			CXXFLAGS="$CXXFLAGS -Wmissing-noreturn -Wpacked -Wparentheses"
			CXXFLAGS="$CXXFLAGS -Wpointer-arith -Wredundant-decls -Wreturn-type"
			CXXFLAGS="$CXXFLAGS -Wsequence-point -Wsign-compare -Wstrict-aliasing"
			CXXFLAGS="$CXXFLAGS -Wstrict-aliasing=2 -Wswitch -Wswitch-default"
			CXXFLAGS="$CXXFLAGS -Wswitch-enum -Wtrigraphs -Wunknown-pragmas -Wunused"
			CXXFLAGS="$CXXFLAGS -Wunused-function -Wunused-label -Wunused-parameter"
			CXXFLAGS="$CXXFLAGS -Wunused-value -Wvariadic-macros"
			CXXFLAGS="$CXXFLAGS -Wvolatile-register-var -Wwrite-strings"
		])
], [
	AS_IF([test x"$GCC" = xyes], [
		# Be tough with warnings and produce less careless code
		CFLAGS="$CFLAGS -Wall -Wextra -pedantic"
		CXXFLAGS="$CXXFLAGS -Wall -Wextra -Wshadow -pedantic"
	])
])
LIBVISIO_CXXFLAGS="${WPD_CFLAGS} ${WPG_CFLAGS} ${LIBXML_CFLAGS} ${ZLIB_CFLAGS} ${ICU_CFLAGS}"
LIBVISIO_LIBS="${WPD_LIBS} ${WPG_LIBS} ${LIBXML_LIBS} ${ZLIB_LIBS} ${ICU_LIBS}"
AC_SUBST(LIBVISIO_CXXFLAGS)
AC_SUBST(LIBVISIO_LIBS)

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
	CXXFLAGS="$CXXFLAGS -O0"
	CFLAGS="$CFLAGS -O0"
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
src/conv/raw/vss2raw.rc
src/conv/svg/Makefile
src/conv/svg/vsd2xhtml.rc
src/conv/svg/vss2xhtml.rc
src/conv/text/Makefile
src/conv/text/vsd2text.rc
src/conv/text/vss2text.rc
src/lib/Makefile
src/lib/libvisio.rc
inc/Makefile
inc/libvisio/Makefile
build/Makefile
build/win32/Makefile
docs/Makefile
docs/doxygen/Makefile
libvisio-$VSD_MAJOR_VERSION.$VSD_MINOR_VERSION.pc:libvisio.pc.in
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
	static-tools:    ${enable_static_tools}
	werror:          ${enable_werror}
==============================================================================
])
