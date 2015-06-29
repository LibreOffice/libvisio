/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libvisio/libvisio.h>
#include <libxml/xpath.h>
#include <iostream>
#include <cppunit/extensions/HelperMacros.h>
#include "xmldrawinggenerator.h"

namespace
{

/// Allows using CPPUNIT_ASSERT_EQUAL() on librevenge::RVNGString instances.
std::ostream &operator <<(std::ostream &s, const librevenge::RVNGString &string)
{
  return s << string.cstr();
}

/// Caller must call xmlXPathFreeObject.
xmlXPathObjectPtr getXPathNode(xmlDocPtr doc, const librevenge::RVNGString &xpath)
{
  xmlXPathContextPtr xpathContext = xmlXPathNewContext(doc);
  xmlXPathObjectPtr xpathObject = xmlXPathEvalExpression(BAD_CAST(xpath.cstr()), xpathContext);
  xmlXPathFreeContext(xpathContext);
  return xpathObject;
}

/// Same as the assertXPath(), but don't assert: return the string instead.
librevenge::RVNGString getXPath(xmlDocPtr doc, const librevenge::RVNGString &xpath, const librevenge::RVNGString &attribute)
{
  xmlXPathObjectPtr xpathobject = getXPathNode(doc, xpath);
  xmlNodeSetPtr nodeset = xpathobject->nodesetval;

  librevenge::RVNGString message("XPath '");
  message.append(xpath);
  message.append("': number of nodes is incorrect.");
  CPPUNIT_ASSERT_EQUAL_MESSAGE(message.cstr(), 1, xmlXPathNodeSetGetLength(nodeset));
  if (attribute.empty())
    return librevenge::RVNGString();
  xmlNodePtr node = nodeset->nodeTab[0];
  xmlChar *prop = xmlGetProp(node, BAD_CAST(attribute.cstr()));
  librevenge::RVNGString s((const char *)prop);
  xmlFree(prop);
  xmlXPathFreeObject(xpathobject);
  return s;
}

/**
 * Assert that xpath exists, and xpath returns exactly one node.
 * xpath's attribute's value must equal to the expectedValue value.
 */
void assertXPath(xmlDocPtr doc, const librevenge::RVNGString &xpath, const librevenge::RVNGString &attribute, const librevenge::RVNGString &expectedValue)
{
  librevenge::RVNGString actualValue = getXPath(doc, xpath, attribute);
  librevenge::RVNGString message("Attribute '");
  message.append(attribute);
  message.append("' of '");
  message.append(xpath);
  message.append("': incorrect value.");
  CPPUNIT_ASSERT_EQUAL_MESSAGE(message.cstr(), expectedValue, actualValue);
}

/// Assert that xpath exists, and does not contain the given attribute
void assertXPathNoAttribute(xmlDocPtr doc, const librevenge::RVNGString &xpath, const librevenge::RVNGString &attribute)
{
  xmlXPathObjectPtr xpathobject = getXPathNode(doc, xpath);
  xmlNodeSetPtr nodeset = xpathobject->nodesetval;
  librevenge::RVNGString message1("In <");
  message1.append(doc->name);
  message1.append(">, XPath '");
  message1.append(xpath);
  message1.append("' number of nodes is incorrect");
  CPPUNIT_ASSERT_EQUAL_MESSAGE(message1.cstr(), 1, xmlXPathNodeSetGetLength(nodeset));
  xmlNodePtr node = nodeset->nodeTab[0];
  librevenge::RVNGString message2("In <");
  message2.append(doc->name);
  message2.append(">, XPath '");
  message2.append(xpath);
  message2.append("' unexpected '");
  message2.append(attribute);
  message2.append("' attribute");
  CPPUNIT_ASSERT_EQUAL_MESSAGE(message2.cstr(), static_cast<xmlChar*>(0), xmlGetProp(node, BAD_CAST(attribute.cstr())));
  xmlXPathFreeObject(xpathobject);
}


#if 0 // keep for future use
/// Same as the assertXPathContent(), but don't assert: return the string instead.
librevenge::RVNGString getXPathContent(xmlDocPtr doc, const librevenge::RVNGString &xpath)
{
  xmlXPathObjectPtr xpathObject = getXPathNode(doc, xpath);
  xmlNodeSetPtr nodeset = xpathObject->nodesetval;

  librevenge::RVNGString message("XPath '");
  message.append(xpath);
  message.append("': not found.");
  CPPUNIT_ASSERT_MESSAGE(message.cstr(), xmlXPathNodeSetGetLength(nodeset) > 0);

  xmlNodePtr node = nodeset->nodeTab[0];
  librevenge::RVNGString s(reinterpret_cast<char *>((node->children[0]).content));
  xmlXPathFreeObject(xpathObject);
  return s;
}

/// Assert that xpath exists, and its content equals to content.
void assertXPathContent(xmlDocPtr doc, const librevenge::RVNGString &xpath, const librevenge::RVNGString &content)
{
  librevenge::RVNGString message("XPath '");
  message.append(xpath);
  message.append("': contents of child does not match.");
  CPPUNIT_ASSERT_EQUAL_MESSAGE(message.cstr(), content, getXPathContent(doc, xpath));
}
#endif

/// Paints an XML representation of filename into buffer, then returns the parsed buffer content.
xmlDocPtr parse(const char *filename, xmlBufferPtr buffer)
{
  librevenge::RVNGString path(TDOC "/");
  path.append(filename);
  librevenge::RVNGFileStream input(path.cstr());
  CPPUNIT_ASSERT(libvisio::VisioDocument::isSupported(&input));

  xmlTextWriterPtr writer = xmlNewTextWriterMemory(buffer, 0);
  CPPUNIT_ASSERT(writer);
  xmlTextWriterStartDocument(writer, 0, 0, 0);
  libvisio::XmlDrawingGenerator painter(writer);

  CPPUNIT_ASSERT(libvisio::VisioDocument::parse(&input, &painter));

  xmlTextWriterEndDocument(writer);
  xmlFreeTextWriter(writer);

  //std::cerr << "XML is '" << (const char *)xmlBufferContent(buffer) << "'" << std::endl;
  return xmlParseMemory((const char *)xmlBufferContent(buffer), xmlBufferLength(buffer));
}

}

class ImportTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(ImportTest);
  CPPUNIT_TEST(testVsdxMetadataTitle);
  CPPUNIT_TEST(testVsdMetadataTitleMs1252);
  CPPUNIT_TEST(testVsdMetadataTitleUtf8);
  CPPUNIT_TEST(testVsdUserDefinedMetadata);
  CPPUNIT_TEST(testVsdxUserDefinedMetadata);
  CPPUNIT_TEST(testVsdxImportBgColorFromTheme);
  CPPUNIT_TEST(testVsdxCharBgColor);
  CPPUNIT_TEST_SUITE_END();

  void testVsdxMetadataTitle();
  void testVsdMetadataTitleMs1252();
  void testVsdMetadataTitleUtf8();
  void testVsdUserDefinedMetadata();
  void testVsdxUserDefinedMetadata();
  void testVsdxImportBgColorFromTheme();
  void testVsdxCharBgColor();

  xmlBufferPtr m_buffer;
  xmlDocPtr m_doc;

public:
  ImportTest();
  virtual void setUp();
  virtual void tearDown();
};

ImportTest::ImportTest()
  : m_buffer(0),
    m_doc(0)
{
}

void ImportTest::setUp()
{
  CPPUNIT_ASSERT(!m_buffer);
  m_buffer = xmlBufferCreate();
  CPPUNIT_ASSERT(m_buffer);

  CPPUNIT_ASSERT(!m_doc);
}

void ImportTest::tearDown()
{
  xmlFreeDoc(m_doc);
  m_doc = 0;

  xmlBufferFree(m_buffer);
  m_buffer = 0;
}

void ImportTest::testVsdxMetadataTitle()
{
  m_doc = parse("fdo86664.vsdx", m_buffer);
  // The setDocumentMetaData() call was missing, so the node did not exist.
  assertXPath(m_doc, "/document/setDocumentMetaData", "title", "mytitle");
  assertXPath(m_doc, "/document/setDocumentMetaData", "subject", "mysubject");
  assertXPath(m_doc, "/document/setDocumentMetaData", "initial-creator", "vmiklos creator");
  // Test <dcterms:created> and <dcterms:modified>.
  assertXPath(m_doc, "/document/setDocumentMetaData", "creation-date", "2014-11-24T10:35:17Z");
  assertXPath(m_doc, "/document/setDocumentMetaData", "date", "2014-11-24T10:41:22Z");
  assertXPath(m_doc, "/document/setDocumentMetaData", "keyword", "mytag");
  assertXPath(m_doc, "/document/setDocumentMetaData", "description", "mycomment");
  assertXPath(m_doc, "/document/setDocumentMetaData", "creator", "vmiklos modifier");
  assertXPath(m_doc, "/document/setDocumentMetaData", "category", "mycategory");
}

void ImportTest::testVsdMetadataTitleMs1252()
{
  m_doc = parse("fdo86729-ms1252.vsd", m_buffer);
  // Test windows-1252 -> UTF-8 conversion, provided by ICU.
  assertXPath(m_doc, "/document/setDocumentMetaData", "title", "mytitle\xC3\xA9\xC3\xA1");

  assertXPath(m_doc, "/document/setDocumentMetaData", "subject", "mysubject");
  assertXPath(m_doc, "/document/setDocumentMetaData", "initial-creator", "vmiklos creator");
  // There is only one author/last-modifier field in the file, so make sure creator is the same as initial-creator.
  assertXPath(m_doc, "/document/setDocumentMetaData", "creator", "vmiklos creator");
  assertXPath(m_doc, "/document/setDocumentMetaData", "keyword", "mytag");
  assertXPath(m_doc, "/document/setDocumentMetaData", "description", "mycomment");
}

void ImportTest::testVsdMetadataTitleUtf8()
{
  m_doc = parse("fdo86729-utf8.vsd", m_buffer);
  // Test the case when the string is UTF-8 encoded already in the file.
  assertXPath(m_doc, "/document/setDocumentMetaData", "title", "mytitle\xC3\xA9\xC3\xA1\xC5\x91\xC5\xB1");
  // Test <dcterms:created> and <dcterms:modified>.
  assertXPath(m_doc, "/document/setDocumentMetaData", "creation-date", "2014-11-26T09:24:56Z");
  assertXPath(m_doc, "/document/setDocumentMetaData", "date", "2014-11-26T09:24:56Z");
}

void ImportTest::testVsdUserDefinedMetadata()
{
  m_doc = parse("dwg.vsd", m_buffer);
  assertXPath(m_doc, "/document/setDocumentMetaData", "category", "Category test");
  assertXPath(m_doc, "/document/setDocumentMetaData", "company", "Company test");
  assertXPath(m_doc, "/document/setDocumentMetaData", "template", "BASICD_M.VSTX");
}

void ImportTest::testVsdxUserDefinedMetadata()
{
  m_doc = parse("dwg.vsdx", m_buffer);
  assertXPath(m_doc, "/document/setDocumentMetaData", "category", "Category test");
  assertXPath(m_doc, "/document/setDocumentMetaData", "company", "Company test");
  assertXPath(m_doc, "/document/setDocumentMetaData", "language", "en-US");
  assertXPath(m_doc, "/document/setDocumentMetaData", "template", "BASICD_M.VSTX");
}

void ImportTest::testVsdxImportBgColorFromTheme()
{
  m_doc = parse("color-boxes.vsdx", m_buffer);
  assertXPath(m_doc, "/document/page/layer[1]//setStyle[2]", "fill-color", "#759fcc");
  assertXPath(m_doc, "/document/page/layer[2]//setStyle[2]", "fill-color", "#70ad47");
  assertXPath(m_doc, "/document/page/layer[3]//setStyle[2]", "fill-color", "#fec000");
  assertXPath(m_doc, "/document/page/layer[4]//setStyle[2]", "fill-color", "#41719c");
  assertXPath(m_doc, "/document/page/layer[5]//setStyle[2]", "fill-color", "#ed7d31");
  assertXPath(m_doc, "/document/page/layer[6]//setStyle[2]", "fill-color", "#bdd0e9");
  assertXPath(m_doc, "/document/page/layer[7]//setStyle[2]", "fill-color", "#5b9bd5");
}

void ImportTest::testVsdxCharBgColor()
{
  m_doc = parse("bgcolor.vsdx", m_buffer);
  assertXPathNoAttribute(m_doc, "/document/page/layer/textObject/paragraph/span", "background-color");
  assertXPath(m_doc, "/document/page/layer/layer[2]/textObject/paragraph/span", "background-color", "#9dbb61");
  assertXPath(m_doc, "/document/page/layer/layer[2]/layer[2]/textObject/paragraph/span", "background-color", "#9dbb61");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ImportTest);

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
