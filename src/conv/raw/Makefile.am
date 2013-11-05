if BUILD_TOOLS

bin_PROGRAMS = vsd2raw vss2raw

AM_CXXFLAGS = \
	-I$(top_srcdir)/inc \
	$(LIBVISIO_CXXFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(DEBUG_CXXFLAGS)

vsd2raw_DEPENDENCIES = @VSD2RAW_WIN32_RESOURCE@

vss2raw_DEPENDENCIES = @VSS2RAW_WIN32_RESOURCE@

vsd2raw_LDADD = \
	../../lib/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la \
	$(REVENGE_GENERATORS_LIBS) \
	$(LIBVISIO_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	@VSD2RAW_WIN32_RESOURCE@

vss2raw_LDADD = \
	../../lib/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la \
	$(REVENGE_GENERATORS_LIBS) \
	$(LIBVISIO_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	@VSS2RAW_WIN32_RESOURCE@

vsd2raw_SOURCES = \
	vsd2raw.cpp

vss2raw_SOURCES = \
	vss2raw.cpp

if OS_WIN32

@VSD2RAW_WIN32_RESOURCE@ : vsd2raw.rc $(vsd2raw_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource vsd2raw.rc @VSD2RAW_WIN32_RESOURCE@

@VSS2RAW_WIN32_RESOURCE@ : vss2raw.rc $(vss2raw_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource vss2raw.rc @VSS2RAW_WIN32_RESOURCE@
endif

EXTRA_DIST = \
	$(vsd2raw_SOURCES) \
	vsd2raw.rc.in \
	$(vss2raw_SOURCES) \
	vss2raw.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	vsd2raw.rc \
	vss2raw.rc	

endif
