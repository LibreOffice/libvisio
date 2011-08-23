if PLATFORM_WIN32
no_undefined = -no-undefined
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -avoid-version
else
no_undefined =
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
endif

if OS_WIN32
install-libtool-import-lib:
	$(INSTALL) @VSD_OBJDIR@/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.dll.a $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.dll.a

else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

lib_LTLIBRARIES = libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_includedir = $(includedir)/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@/libvisio
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_include_HEADERS = \
	libvisio.h \
	VisioDocument.h

AM_CXXFLAGS = $(LIBVISIO_CXXFLAGS) $(DEBUG_CXXFLAGS)

libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_LIBADD  = $(LIBVISIO_LIBS) @LIBVISIO_WIN32_RESOURCE@
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_DEPENDENCIES = @LIBVISIO_WIN32_RESOURCE@
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_LDFLAGS = $(version_info) -export-dynamic $(no_undefined)
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_SOURCES = \
	libvisio_utils.cpp \
	VisioDocument.cpp \
	VSD11Parser.cpp \
	VSD6Parser.cpp \
	VSDInternalStream.cpp \
	VSDSVGGenerator.cpp \
	VSDXCharacterList.cpp \
	VSDXCollector.cpp \
	VSDXContentCollector.cpp \
	VSDXGeometryList.cpp \
	VSDXOutputElementList.cpp \
	VSDXPages.cpp \
	VSDXParser.cpp \
	VSDXShapeList.cpp \
	VSDXStencils.cpp \
	VSDXStyles.cpp \
	VSDXStylesCollector.cpp \
	libvisio_utils.h \
	VSD11Parser.h \
	VSD6Parser.h \
	VSDInternalStream.h \
	VSDSVGGenerator.h \
	VSDXCharacterList.h \
	VSDXCollector.h \
	VSDXContentCollector.h \
	VSDXDocumentStructure.h \
	VSDXGeometryList.h \
	VSDXOutputElementList.h \
	VSDXPages.h \
	VSDXParser.h \
	VSDXShapeList.h \
	VSDXStencils.h \
	VSDXStyles.h \
	VSDXStylesCollector.h \
	VSDXTypes.h

if OS_WIN32

@LIBVISIO_WIN32_RESOURCE@ : libvisio.rc $(libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource libvisio.rc @LIBVISIO_WIN32_RESOURCE@

endif


install-data-local: install-libtool-import-lib

uninstall-local: uninstall-libtool-import-lib

EXTRA_DIST = \
	libvisio.rc.in \
	makefile.mk

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	libvisio.rc
