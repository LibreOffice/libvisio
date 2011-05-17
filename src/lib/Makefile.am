if PLATFORM_WIN32
no_undefined = -no-undefined
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -avoid-version
else
no_undefined =
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
endif

if OS_WIN32
install-libtool-import-lib:
	$(INSTALL) @WPG_OBJDIR@/libwpg-@WPG_MAJOR_VERSION@.@WPG_MINOR_VERSION@.dll.a $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libwpg-@WPG_MAJOR_VERSION@.@WPG_MINOR_VERSION@.dll.a

else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

lib_LTLIBRARIES = libwpg-@WPG_MAJOR_VERSION@.@WPG_MINOR_VERSION@.la
libwpg_@WPG_MAJOR_VERSION@_@WPG_MINOR_VERSION@_includedir = $(includedir)/libwpg-@WPG_MAJOR_VERSION@.@WPG_MINOR_VERSION@/libwpg
libwpg_@WPG_MAJOR_VERSION@_@WPG_MINOR_VERSION@_include_HEADERS = \
	libvisio.h \
	VisioDocument.h

AM_CXXFLAGS = $(LIBVISIO_CXXFLAGS) $(DEBUG_CXXFLAGS)

libwpg_@WPG_MAJOR_VERSION@_@WPG_MINOR_VERSION@_la_LIBADD  = $(LIBVISIO_LIBS) @LIBVISIO_WIN32_RESOURCE@
libwpg_@WPG_MAJOR_VERSION@_@WPG_MINOR_VERSION@_la_DEPENDENCIES = @LIBVISIO_WIN32_RESOURCE@
libwpg_@WPG_MAJOR_VERSION@_@WPG_MINOR_VERSION@_la_LDFLAGS = $(version_info) -export-dynamic $(no_undefined)
libwpg_@WPG_MAJOR_VERSION@_@WPG_MINOR_VERSION@_la_SOURCES = \
	VisioDocument.cpp \
	VSDSVGGenerator.cpp \
	libvisio.h \
	VSDSVGGenerator.h

if OS_WIN32

@LIBVISIO_WIN32_RESOURCE@ : libwpg.rc $(libwpg_@WPG_MAJOR_VERSION@_@WPG_MINOR_VERSION@_la_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource libwpg.rc @LIBVISIO_WIN32_RESOURCE@

endif


install-data-local: install-libtool-import-lib

uninstall-local: uninstall-libtool-import-lib

EXTRA_DIST = \
	libwpg.h.in \
	libwpg.rc.in \
	makefile.mk

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	libwpg.rc
