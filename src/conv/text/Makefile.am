if BUILD_TOOLS

bin_PROGRAMS = vsd2text vss2text

AM_CXXFLAGS = \
	-I$(top_srcdir)/inc \
	$(LIBVISIO_CXXFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(DEBUG_CXXFLAGS)

vsd2text_DEPENDENCIES = @VSD2TEXT_WIN32_RESOURCE@

vss2text_DEPENDENCIES = @VSS2TEXT_WIN32_RESOURCE@

vsd2text_LDADD = \
	../../lib/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la \
	$(REVENGE_GENERATORS_LIBS) \
	$(LIBVISIO_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	@VSD2TEXT_WIN32_RESOURCE@

vss2text_LDADD = \
	../../lib/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la \
	$(REVENGE_GENERATORS_LIBS) \
	$(LIBVISIO_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	@VSS2TEXT_WIN32_RESOURCE@

vsd2text_SOURCES = \
	vsd2text.cpp

vss2text_SOURCES = \
	vss2text.cpp

if OS_WIN32

@VSD2TEXT_WIN32_RESOURCE@ : vsd2text.rc $(vsd2text_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource vsd2text.rc @VSD2TEXT_WIN32_RESOURCE@

@VSS2TEXT_WIN32_RESOURCE@ : vss2text.rc $(vss2text_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource vss2text.rc @VSS2TEXT_WIN32_RESOURCE@
endif

EXTRA_DIST = \
	$(vsd2text_SOURCES) \
	vsd2text.rc.in \
	$(vss2text_SOURCES) \
	vss2text.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	vsd2text.rc	\
	vss2text.rc	

endif
