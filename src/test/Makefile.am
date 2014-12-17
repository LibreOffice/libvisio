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
	     data/fdo86664.vsdx \
	     data/fdo86729-ms1252.vsd \
	     data/fdo86729-utf8.vsd \
	     $(test_SOURCES)

TESTS = test
