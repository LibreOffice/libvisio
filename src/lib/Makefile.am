if PLATFORM_WIN32
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -avoid-version
else
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
endif

lib_LTLIBRARIES = libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la
noinst_LTLIBRARIES = libtokenmap.la
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_includedir = $(includedir)/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@/libvisio
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_include_HEADERS = \
	$(top_srcdir)/inc/libvisio/libvisio.h \
	$(top_srcdir)/inc/libvisio/VSDStringVector.h \
	$(top_srcdir)/inc/libvisio/VisioDocument.h

AM_CXXFLAGS = -I$(top_srcdir)/inc -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib $(LIBVISIO_CXXFLAGS) $(DEBUG_CXXFLAGS)

generated_files = \
	$(top_builddir)/src/lib/tokens.h \
	$(top_builddir)/src/lib/tokenhash.h

libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_LIBADD  = libtokenmap.la $(LIBVISIO_LIBS) @LIBVISIO_WIN32_RESOURCE@
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_DEPENDENCIES = @LIBVISIO_WIN32_RESOURCE@ libtokenmap.la
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_LDFLAGS = $(version_info) -export-dynamic -no-undefined
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_SOURCES = \
	libvisio_utils.cpp \
	VisioDocument.cpp \
	VSD5Parser.cpp \
	VSD6Parser.cpp \
	VSDInternalStream.cpp \
	VSDSVGGenerator.cpp \
	VSDCharacterList.cpp \
	VSDContentCollector.cpp \
	VSDFieldList.cpp \
	VSDGeometryList.cpp \
	VSDOutputElementList.cpp \
	VSDPages.cpp \
	VSDParagraphList.cpp \
	VSDParser.cpp \
	VSDShapeList.cpp \
	VSDStencils.cpp \
	VSDStringVector.cpp \
	VSDStyles.cpp \
	VSDStylesCollector.cpp \
	VSDXMLHelper.cpp \
	VSDZipStream.cpp \
	libvisio_utils.h \
	VSD5Parser.h \
	VSD6Parser.h \
	VSDInternalStream.h \
	VSDSVGGenerator.h \
	VSDCharacterList.h \
	VSDCollector.h \
	VSDContentCollector.h \
	VSDDocumentStructure.h \
	VSDFieldList.h \
	VSDGeometryList.h \
	VSDOutputElementList.h \
	VSDPages.h \
	VSDParagraphList.h \
	VSDParser.h \
	VSDShapeList.h \
	VSDStencils.h \
	VSDStyles.h \
	VSDStylesCollector.h \
	VSDTypes.h \
	VSDXMLHelper.h \
	VSDZipStream.h

libtokenmap_la_SOURCES = \
	VDXParser.cpp \
	VDXParser.h \
	VSDXMLParserBase.cpp \
	VSDXMLParserBase.h \
	VSDXMLTokenMap.cpp \
	VSDXMLTokenMap.h \
	VSDXParser.cpp \
	VSDXParser.h \
	VSDXTheme.cpp \
	VSDXTheme.h \
	$(generated_files)
	

$(libtokenmap_la_OBJECTS) : $(generated_files)

$(top_builddir)/src/lib/tokens.h : $(top_builddir)/src/lib/tokens.gperf

$(top_builddir)/src/lib/tokenhash.h : $(top_builddir)/src/lib/tokens.gperf
	$(GPERF) --compare-strncmp -C -m 20 $(top_builddir)/src/lib/tokens.gperf \
		| $(SED) -e "s/(char\*)0/(char\*)0, 0/g" > $(top_builddir)/src/lib/tokenhash.h

$(top_builddir)/src/lib/tokens.gperf : $(top_srcdir)/src/lib/tokens.txt gentoken.pl
	perl $(top_srcdir)/src/lib/gentoken.pl $(top_srcdir)/src/lib/tokens.txt \
		$(top_builddir)/src/lib/tokens.h $(top_builddir)/src/lib/tokens.gperf

if OS_WIN32

@LIBVISIO_WIN32_RESOURCE@ : libvisio.rc $(libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource libvisio.rc @LIBVISIO_WIN32_RESOURCE@

endif

distclean-local :
	rm -f $(generated_files) $(top_builddir)/src/lib/*.gperf

EXTRA_DIST = \
	tokens.txt \
    gentoken.pl \
	libvisio.rc.in \
	makefile.mk

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	libvisio.rc
