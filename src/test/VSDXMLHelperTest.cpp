/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <librevenge-stream/librevenge-stream.h>

#include "VSDXMLHelper.h"

namespace test
{

class VSDXMLHelperTest : public CPPUNIT_NS::TestFixture
{
public:
  virtual void setUp();
  virtual void tearDown();

private:
  CPPUNIT_TEST_SUITE(VSDXMLHelperTest);
  CPPUNIT_TEST(testRebaseTargetOverPop);
  CPPUNIT_TEST_SUITE_END();

private:
  void testRebaseTargetOverPop();
};

void VSDXMLHelperTest::setUp()
{
}

void VSDXMLHelperTest::tearDown()
{
}

// A Target whose ".." segments would walk past the base directory must not
// crash. Previously the segment walk called pop_back() on an empty vector,
// which is undefined behaviour and triggered a heap-use-after-free.
void VSDXMLHelperTest::testRebaseTargetOverPop()
{
  static const char rels[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
    "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
    "<Relationship Id=\"rId1\""
    " Type=\"http://schemas.microsoft.com/visio/2010/relationships/page\""
    " Target=\"../../trigger\"/>"
    "</Relationships>";

  librevenge::RVNGStringStream input(reinterpret_cast<const unsigned char *>(rels),
                                     sizeof(rels) - 1);
  libvisio::VSDXRelationships parsed(&input);
  parsed.rebaseTargets("visio");

  const libvisio::VSDXRelationship *r = parsed.getRelationshipById("rId1");
  CPPUNIT_ASSERT(r != nullptr);
  // The over-deep ".." segments are silently dropped; what survives is the
  // tail of the path. The exact result is not the point of the test - the
  // point is that the rebase did not crash.
  CPPUNIT_ASSERT_EQUAL(std::string("trigger"), r->getTarget());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VSDXMLHelperTest);

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
