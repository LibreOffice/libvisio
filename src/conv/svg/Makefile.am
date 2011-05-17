bin_PROGRAMS = vsd2svg

AM_CXXFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib $(LIBVISIO_CXXFLAGS) $(DEBUG_CXXFLAGS)

vsd2svg_DEPENDENCIES = @WPG2SVG_WIN32_RESOURCE@

if STATIC_TOOLS
vsd2svg_LDADD = ../../lib/@WPG_OBJDIR@/libvisio-@WPG_MAJOR_VERSION@.@WPG_MINOR_VERSION@.a $(LIBVISIO_LIBS) @WPG2SVG_WIN32_RESOURCE@
vsd2svg_LDFLAGS = -all-static 
else
vsd2svg_LDADD = ../../lib/libvisio-@WPG_MAJOR_VERSION@.@WPG_MINOR_VERSION@.la $(LIBVISIO_LIBS) @WPG2SVG_WIN32_RESOURCE@ 
endif

vsd2svg_SOURCES = \
	vsd2svg.cpp

if OS_WIN32

@WPG2SVG_WIN32_RESOURCE@ : vsd2svg.rc $(vsd2svg_OBJECTS)
	chmod +x $(top_srcdir)/build/win32/*compile-resource
	WINDRES=@WINDRES@ $(top_srcdir)/build/win32/lt-compile-resource vsd2svg.rc @WPG2SVG_WIN32_RESOURCE@
endif

EXTRA_DIST = \
	$(vsd2svg_SOURCES) \
	vsd2svg.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	vsd2svg.rc
