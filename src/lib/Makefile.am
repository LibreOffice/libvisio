if PLATFORM_WIN32
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -avoid-version
else
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
endif

noinst_LTLIBRARIES = libvisio-internal.la
lib_LTLIBRARIES = libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la

AM_CXXFLAGS = \
	-DLIBVISIO_BUILD=1 \
	-I$(top_srcdir)/inc \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-DBOOST_ERROR_CODE_HEADER_ONLY \
	-DBOOST_SPIRIT_USE_PHOENIX_V3 \
	-DBOOST_SYSTEM_NO_DEPRECATED \
	$(LIBVISIO_CXXFLAGS) \
	$(DEBUG_CXXFLAGS)

BUILT_SOURCES = tokens.h tokenhash.h

libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_LIBADD  = $(LIBVISIO_LIBS) libvisio-internal.la @LIBVISIO_WIN32_RESOURCE@
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_DEPENDENCIES = libvisio-internal.la @LIBVISIO_WIN32_RESOURCE@
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_LDFLAGS = $(version_info) -export-dynamic -no-undefined
libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_SOURCES = \
	VisioDocument.cpp

libvisio_internal_la_SOURCES = \
	VDXParser.cpp \
	VDXParser.h \
	VSD5Parser.cpp \
	VSD5Parser.h \
	VSD6Parser.cpp \
	VSD6Parser.h \
	VSDCharacterList.cpp \
	VSDCharacterList.h \
	VSDCollector.h \
	VSDContentCollector.cpp \
	VSDContentCollector.h \
	VSDDocumentStructure.h \
	VSDFieldList.cpp \
	VSDFieldList.h \
	VSDGeometryList.cpp \
	VSDGeometryList.h \
	VSDInternalStream.cpp \
	VSDInternalStream.h \
	VSDLayerList.cpp \
	VSDLayerList.h \
	VSDMetaData.cpp \
	VSDMetaData.h \
	VSDOutputElementList.cpp \
	VSDOutputElementList.h \
	VSDPages.cpp \
	VSDPages.h \
	VSDParagraphList.cpp \
	VSDParagraphList.h \
	VSDParser.cpp \
	VSDParser.h \
	VSDShapeList.cpp \
	VSDShapeList.h \
	VSDStencils.cpp \
	VSDStencils.h \
	VSDStyles.cpp \
	VSDStyles.h \
	VSDStylesCollector.cpp \
	VSDStylesCollector.h \
	VSDTypes.h \
	VSDXMLHelper.cpp \
	VSDXMLHelper.h \
	VSDXMLParserBase.cpp \
	VSDXMLParserBase.h \
	VSDXMLTokenMap.cpp \
	VSDXMLTokenMap.h \
	VSDXMetaData.cpp \
	VSDXMetaData.h \
	VSDXParser.cpp \
	VSDXParser.h \
	VSDXTheme.cpp \
	VSDXTheme.h \
	libvisio_utils.cpp \
	libvisio_utils.h \
	libvisio_xml.cpp \
	libvisio_xml.h

tokens.h : tokens.gperf

tokenhash.h : tokens.gperf
	$(GPERF) --compare-strncmp -C -m 20 tokens.gperf \
		| $(SED) -e 's/(char\*)0/(char\*)0, 0/g' -e 's/register //g' > tokenhash.h

tokens.gperf : $(top_srcdir)/src/lib/tokens.txt $(top_srcdir)/src/lib/gentoken.pl
	$(PERL) $(top_srcdir)/src/lib/gentoken.pl $(top_srcdir)/src/lib/tokens.txt \
		tokens.h tokens.gperf

if OS_WIN32

@LIBVISIO_WIN32_RESOURCE@ : libvisio.rc $(libvisio_@VSD_MAJOR_VERSION@_@VSD_MINOR_VERSION@_la_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource libvisio.rc @LIBVISIO_WIN32_RESOURCE@

endif

MOSTLYCLEANFILES = \
	$(BUILT_SOURCES) \
	tokens.gperf

EXTRA_DIST = \
	$(BUILT_SOURCES) \
	tokens.txt \
	gentoken.pl \
	libvisio.rc \
	libvisio.rc.in
