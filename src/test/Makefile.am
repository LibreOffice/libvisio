check_PROGRAMS = test

AM_CXXFLAGS = \
	      -I$(top_srcdir)/inc \
	      $(LIBVISIO_CXXFLAGS) \
	      $(REVENGE_STREAM_CFLAGS) \
	      $(CPPUNIT_CFLAGS) \
	      $(DEBUG_CXXFLAGS)

test_CPPFLAGS = -DTDOC=\"$(top_srcdir)/src/test/data\"
test_LDADD = \
	     ../lib/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la \
	     $(CPPUNIT_LIBS) \
	     $(LIBVISIO_LIBS) \
	     $(REVENGE_STREAM_LIBS)
test_SOURCES = \
	       xmldrawinggenerator.cpp \
	       xmldrawinggenerator.h \
	       importtest.cpp \
	       test.cpp

EXTRA_DIST = \
	     data/bgcolor.vsdx \
	     data/color-boxes.vsdx \
	     data/fdo86664.vsdx \
	     data/fdo86729-ms1252.vsd \
	     data/fdo86729-utf8.vsd \
	     data/dwg.vsd \
	     data/dwg.vsdx \
	     $(test_SOURCES)

# ImportTest::testVsdMetadataTitleUtf8 checks formatted date string
AM_TESTS_ENVIRONMENT = TZ=UTC; export TZ;

TESTS = test
