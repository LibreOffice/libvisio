noinst_PROGRAMS = vsdfuzzer vsdxfuzzer vdxfuzzer

AM_CXXFLAGS = -I$(top_srcdir)/inc \
	$(REVENGE_GENERATORS_CFLAGS) \
	$(REVENGE_CFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(DEBUG_CXXFLAGS)

commonfuzzer_ldadd = \
	$(top_builddir)/src/lib/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la \
	$(REVENGE_GENERATORS_LIBS) \
	$(REVENGE_LIBS) \
	$(REVENGE_STREAM_LIBS) \
	-lFuzzingEngine

vsdfuzzer_LDADD = $(commonfuzzer_ldadd)

vsdfuzzer_SOURCES = \
	vsdfuzzer.cpp

vsdxfuzzer_LDADD = $(commonfuzzer_ldadd)

vsdxfuzzer_SOURCES = \
	vsdxfuzzer.cpp

vdxfuzzer_LDADD = $(commonfuzzer_ldadd)

vdxfuzzer_SOURCES = \
	vdxfuzzer.cpp

EXTRA_DIST = \
	vdx.dict \
	vsdx.dict
