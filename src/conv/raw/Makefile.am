bin_PROGRAMS = vsd2raw

AM_CXXFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib $(LIBVISIO_CXXFLAGS) $(DEBUG_CXXFLAGS)

vsd2raw_DEPENDENCIES = @VSD2RAW_WIN32_RESOURCE@

if STATIC_TOOLS
vsd2raw_LDADD = ../../lib/@VSD_OBJDIR@/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.a $(LIBVISIO_LIBS) @VSD2RAW_WIN32_RESOURCE@
vsd2raw_LDFLAGS = -all-static 
else
vsd2raw_LDADD = ../../lib/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la $(LIBVISIO_LIBS) @VSD2RAW_WIN32_RESOURCE@ 
endif

vsd2raw_SOURCES = \
	vsd2raw.cpp

if OS_WIN32

@VSD2RAW_WIN32_RESOURCE@ : vsd2raw.rc $(vsd2raw_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource vsd2raw.rc @VSD2RAW_WIN32_RESOURCE@
endif

EXTRA_DIST = \
	$(vsd2raw_SOURCES) \
	vsd2raw.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	vsd2raw.rc	 
