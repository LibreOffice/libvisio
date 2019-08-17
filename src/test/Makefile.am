tests = importtest unittest

check_PROGRAMS = $(tests)
check_LTLIBRARIES = libtest_driver.la

libtest_driver_la_CPPFLAGS = \
	$(CPPUNIT_CFLAGS) \
	$(DEBUG_CXXFLAGS)

libtest_driver_la_SOURCES = \
	test.cpp

importtest_CPPFLAGS = \
	-DTDOC=\"$(top_srcdir)/src/test/data\" \
	-I$(top_srcdir)/inc \
	$(LIBVISIO_CXXFLAGS) \
	$(REVENGE_STREAM_CFLAGS) \
	$(CPPUNIT_CFLAGS) \
	$(DEBUG_CXXFLAGS)

importtest_LDADD = \
	../lib/libvisio-@VSD_MAJOR_VERSION@.@VSD_MINOR_VERSION@.la \
	libtest_driver.la \
	$(CPPUNIT_LIBS) \
	$(LIBVISIO_LIBS) \
	$(REVENGE_STREAM_LIBS)

importtest_SOURCES = \
	xmldrawinggenerator.cpp \
	xmldrawinggenerator.h \
	importtest.cpp

unittest_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	$(LIBVISIO_CXXFLAGS) \
	$(CPPUNIT_CFLAGS) \
	$(DEBUG_CXXFLAGS)

unittest_LDFLAGS = -L$(top_srcdir)/src/lib
unittest_LDADD = \
	$(top_builddir)/src/lib/libvisio-internal.la \
	libtest_driver.la \
	$(LIBVISIO_LIBS) \
	$(CPPUNIT_LIBS)

unittest_SOURCES = \
	VSDInternalStreamTest.cpp

EXTRA_DIST = \
	data/Visio11FormatLine.vsd \
	data/Visio11TextFieldsWithCurrency.vsd \
	data/Visio11TextFieldsWithUnits.vsd \
	data/Visio5TextFieldsWithUnits.vsd \
	data/Visio6TextFieldsWithUnits.vsd \
	data/bgcolor.vsdx \
	data/bitmaps.vsd \
	data/bitmaps2.vsd \
	data/color-boxes.vsdx \
	data/dwg.vsd \
	data/dwg.vsdx \
	data/fdo86664.vsdx \
	data/fdo86729-ms1252.vsd \
	data/fdo86729-utf8.vsd \
	data/no-bgcolor.vsd \
	data/tdf76829-datetime-format.vsd \
	data/tdf76829-numeric-format.vsd

# ImportTest::testVsdMetadataTitleUtf8 checks formatted date string
AM_TESTS_ENVIRONMENT = TZ=UTC; export TZ;

TESTS = $(tests)
