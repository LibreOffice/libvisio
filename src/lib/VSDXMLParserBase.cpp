/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDXMLParserBase.h"

#include <string.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlstring.h>
#include <librevenge-stream/librevenge-stream.h>

#include <boost/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>

#include "libvisio_utils.h"
#include "libvisio_xml.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"
#include "VSDXMLHelper.h"
#include "VSDXMLTokenMap.h"

using std::shared_ptr;

libvisio::VSDXMLParserBase::VSDXMLParserBase()
  : m_collector(), m_stencils(), m_currentStencil(), m_shape(),
    m_isStencilStarted(false), m_currentStencilID(MINUS_ONE),
    m_extractStencils(false), m_isInStyles(false), m_currentLevel(0),
    m_currentShapeLevel(0), m_colours(), m_fieldList(), m_shapeList(),
    m_currentBinaryData(), m_shapeStack(), m_shapeLevelStack(),
    m_isShapeStarted(false), m_isPageStarted(false), m_currentGeometryList(nullptr),
    m_currentGeometryListIndex(MINUS_ONE), m_fonts(), m_currentTabSet(nullptr),
    m_watcher(nullptr)
{
  initColours();
}

libvisio::VSDXMLParserBase::~VSDXMLParserBase()
{
}

// Common functions

void libvisio::VSDXMLParserBase::readGeometry(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  m_currentGeometryList = &m_shape.m_geometries[ix];

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
      {
        m_currentGeometryList->clear();
        m_shape.m_geometries.erase(ix);
        m_currentGeometryList = nullptr;
      }
    }
    return;
  }

  boost::optional<bool> noFill;
  boost::optional<bool> noLine;
  boost::optional<bool> noShow;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readGeometry: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_NOFILL:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readBoolData(noFill, reader);
      break;
    case XML_NOLINE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readBoolData(noLine, reader);
      break;
    case XML_NOSHOW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readBoolData(noShow, reader);
      break;
    case XML_MOVETO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readMoveTo(reader);
      break;
    case XML_LINETO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readLineTo(reader);
      break;
    case XML_ARCTO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readArcTo(reader);
      break;
    case XML_NURBSTO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readNURBSTo(reader);
      break;
    case XML_POLYLINETO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readPolylineTo(reader);
      break;
    case XML_INFINITELINE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readInfiniteLine(reader);
      break;
    case XML_ELLIPTICALARCTO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readEllipticalArcTo(reader);
      break;
    case XML_ELLIPSE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readEllipse(reader);
      break;
    case XML_RELCUBBEZTO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readRelCubBezTo(reader);
      break;
    case XML_RELELLIPTICALARCTO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readRelEllipticalArcTo(reader);
      break;
    case XML_RELMOVETO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readRelMoveTo(reader);
      break;
    case XML_RELLINETO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readRelLineTo(reader);
      break;
    case XML_RELQUADBEZTO:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readRelQuadBezTo(reader);
      break;
    case XML_SPLINESTART:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readSplineStart(reader);
      break;
    case XML_SPLINEKNOT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readSplineKnot(reader);
      break;
    default:
      break;
    }
  }
  while (((XML_GEOM != tokenId && XML_SECTION != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addGeometry(0, level+1, noFill, noLine, noShow);
}

void libvisio::VSDXMLParserBase::readMoveTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readMoveTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_MOVETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addMoveTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readLineTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readLineTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_LINETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addLineTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readArcTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readArcTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(a, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_ARCTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addArcTo(ix, level, x, y, a);
}

void libvisio::VSDXMLParserBase::readEllipticalArcTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;
  boost::optional<double> b;
  boost::optional<double> c;
  boost::optional<double> d;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readEllipticalArcTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(a, reader);
      break;
    case XML_B:
      ret = readDoubleData(b, reader);
      break;
    case XML_C:
      ret = readDoubleData(c, reader);
      break;
    case XML_D:
      ret = readDoubleData(d, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_ELLIPTICALARCTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addEllipticalArcTo(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readEllipse(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;
  boost::optional<double> b;
  boost::optional<double> c;
  boost::optional<double> d;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readEllipse: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(a, reader);
      break;
    case XML_B:
      ret = readDoubleData(b, reader);
      break;
    case XML_C:
      ret = readDoubleData(c, reader);
      break;
    case XML_D:
      ret = readDoubleData(d, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_ELLIPSE != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addEllipse(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readNURBSTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> knot; // Second last knot
  boost::optional<double> weight; // Last weight
  boost::optional<double> knotPrev; // First knot
  boost::optional<double> weightPrev ; // First weight
  boost::optional<NURBSData> nurbsData;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readNURBSTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(knot, reader);
      break;
    case XML_B:
      ret = readDoubleData(weight, reader);
      break;
    case XML_C:
      ret = readDoubleData(knotPrev, reader);
      break;
    case XML_D:
      ret = readDoubleData(weightPrev, reader);
      break;
    case XML_E:
      ret = readNURBSData(nurbsData, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_NURBSTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

  if (ret == 1)
    m_currentGeometryList->addNURBSTo(ix, level, x, y, knot, knotPrev, weight, weightPrev, nurbsData);
}

void libvisio::VSDXMLParserBase::readPolylineTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<PolylineData> polyLineData;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readPolylineTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readPolylineData(polyLineData, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_POLYLINETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addPolylineTo(ix, level, x, y, polyLineData);
}

void libvisio::VSDXMLParserBase::readInfiniteLine(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;
  boost::optional<double> b;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readInfiniteLine: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(a, reader);
      break;
    case XML_B:
      ret = readDoubleData(b, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_INFINITELINE != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addInfiniteLine(ix, level, x, y, a, b);
}

void libvisio::VSDXMLParserBase::readRelEllipticalArcTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;
  boost::optional<double> b;
  boost::optional<double> c;
  boost::optional<double> d;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readRelEllipticalArcTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(a, reader);
      break;
    case XML_B:
      ret = readDoubleData(b, reader);
      break;
    case XML_C:
      ret = readDoubleData(c, reader);
      break;
    case XML_D:
      ret = readDoubleData(d, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_RELELLIPTICALARCTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addRelEllipticalArcTo(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readRelCubBezTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;
  boost::optional<double> b;
  boost::optional<double> c;
  boost::optional<double> d;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readRelCubBezTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(a, reader);
      break;
    case XML_B:
      ret = readDoubleData(b, reader);
      break;
    case XML_C:
      ret = readDoubleData(c, reader);
      break;
    case XML_D:
      ret = readDoubleData(d, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_RELCUBBEZTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addRelCubBezTo(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readRelLineTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readRelLineTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_RELLINETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addRelLineTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readRelMoveTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readRelMoveTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_RELMOVETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addRelMoveTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readRelQuadBezTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;
  boost::optional<double> b;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readRelQuadBezTo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(a, reader);
      break;
    case XML_B:
      ret = readDoubleData(b, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_RELQUADBEZTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addRelQuadBezTo(ix, level, x, y, a, b);
}

void libvisio::VSDXMLParserBase::readShape(xmlTextReaderPtr reader)
{
  m_isShapeStarted = true;
  m_currentShapeLevel = getElementDepth(reader);

  const shared_ptr<xmlChar> idString(xmlTextReaderGetAttribute(reader, BAD_CAST("ID")), xmlFree);
  const shared_ptr<xmlChar> masterPageString(xmlTextReaderGetAttribute(reader, BAD_CAST("Master")), xmlFree);
  const shared_ptr<xmlChar> masterShapeString(xmlTextReaderGetAttribute(reader, BAD_CAST("MasterShape")), xmlFree);
  const shared_ptr<xmlChar> lineStyleString(xmlTextReaderGetAttribute(reader, BAD_CAST("LineStyle")), xmlFree);
  const shared_ptr<xmlChar> fillStyleString(xmlTextReaderGetAttribute(reader, BAD_CAST("FillStyle")), xmlFree);
  const shared_ptr<xmlChar> textStyleString(xmlTextReaderGetAttribute(reader, BAD_CAST("TextStyle")), xmlFree);

  unsigned id = idString ? (unsigned)xmlStringToLong(idString) : MINUS_ONE;
  unsigned masterPage = masterPageString ? (unsigned)xmlStringToLong(masterPageString) : MINUS_ONE;
  unsigned masterShape = masterShapeString ? (unsigned)xmlStringToLong(masterShapeString) : MINUS_ONE;
  unsigned lineStyle = lineStyleString ? (unsigned)xmlStringToLong(lineStyleString) : MINUS_ONE;
  unsigned fillStyle =  fillStyleString ? (unsigned)xmlStringToLong(fillStyleString) : MINUS_ONE;
  unsigned textStyle =  textStyleString ? (unsigned)xmlStringToLong(textStyleString) : MINUS_ONE;

  if (masterPage != MINUS_ONE || masterShape != MINUS_ONE)
  {
    if (!m_shapeStack.empty())
      masterPage = m_shapeStack.top().m_masterPage;
  }

  m_shape.clear();
  m_shape.m_textFormat = VSD_TEXT_UTF8;

  if (m_isStencilStarted && m_currentStencil)
    m_currentStencil->setFirstShape(id);

  const VSDStencil *tmpStencil = m_stencils.getStencil(masterPage);
  if (tmpStencil)
  {
    if (MINUS_ONE == masterShape)
      masterShape = tmpStencil->m_firstShapeId;
    const VSDShape *tmpShape = tmpStencil->getStencilShape(masterShape);
    if (tmpShape)
    {
      if (tmpShape->m_foreign)
        m_shape.m_foreign = make_unique<ForeignData>(*(tmpShape->m_foreign));
      m_shape.m_xform = tmpShape->m_xform;
      if (tmpShape->m_txtxform)
        m_shape.m_txtxform = make_unique<XForm>(*(tmpShape->m_txtxform));
      m_shape.m_geometries = tmpShape->m_geometries;
      m_shape.m_charList = tmpShape->m_charList;
      m_shape.m_paraList = tmpShape->m_paraList;
      m_shape.m_tabSets = tmpShape->m_tabSets;
      m_shape.m_text = tmpShape->m_text;
      m_shape.m_textFormat = tmpShape->m_textFormat;
      m_shape.m_misc = tmpShape->m_misc;
    }
  }

  if (!m_shapeStack.empty())
    m_shapeStack.top().m_shapeList.addShapeId(id);
  else
    m_shapeList.addShapeId(id);

  m_shape.m_lineStyleId = lineStyle;
  m_shape.m_fillStyleId = fillStyle;
  m_shape.m_textStyleId = textStyle;

  m_shape.m_parent = m_shapeStack.empty() ? MINUS_ONE : m_shapeStack.top().m_shapeId;
  m_shape.m_masterPage = masterPage;
  m_shape.m_masterShape = masterShape;
  m_shape.m_shapeId = id;
}

void libvisio::VSDXMLParserBase::initColours()
{
  m_colours.clear();
  m_colours[0] = Colour(0x00, 0x00, 0x00, 0);
  m_colours[1] = Colour(0xFF, 0xFF, 0xFF, 0);
  m_colours[2] = Colour(0xFF, 0x00, 0x00, 0);
  m_colours[3] = Colour(0x00, 0xFF, 0x00, 0);
  m_colours[4] = Colour(0x00, 0x00, 0xFF, 0);
  m_colours[5] = Colour(0xFF, 0xFF, 0x00, 0);
  m_colours[6] = Colour(0xFF, 0x00, 0xFF, 0);
  m_colours[7] = Colour(0x00, 0xFF, 0xFF, 0);
  m_colours[8] = Colour(0x80, 0x00, 0x00, 0);
  m_colours[9] = Colour(0x00, 0x80, 0x00, 0);
  m_colours[10] = Colour(0x00, 0x00, 0x80, 0);
  m_colours[11] = Colour(0x80, 0x80, 0x00, 0);
  m_colours[12] = Colour(0x80, 0x00, 0x80, 0);
  m_colours[13] = Colour(0x00, 0x80, 0x80, 0);
  m_colours[14] = Colour(0xC0, 0xC0, 0xC0, 0);
  m_colours[15] = Colour(0xE6, 0xE6, 0xE6, 0);
  m_colours[16] = Colour(0xCD, 0xCD, 0xCD, 0);
  m_colours[17] = Colour(0xB3, 0xB3, 0xB3, 0);
  m_colours[18] = Colour(0x9A, 0x9A, 0x9A, 0);
  m_colours[19] = Colour(0x80, 0x80, 0x80, 0);
  m_colours[20] = Colour(0x66, 0x66, 0x66, 0);
  m_colours[21] = Colour(0x4D, 0x4D, 0x4D, 0);
  m_colours[22] = Colour(0x33, 0x33, 0x33, 0);
  m_colours[23] = Colour(0x1A, 0x1A, 0x1A, 0);
}

void libvisio::VSDXMLParserBase::readColours(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;

  initColours();

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readColours: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    if (XML_COLORENTRY == tokenId)
    {
      unsigned idx = getIX(reader);
      const shared_ptr<xmlChar> rgb(xmlTextReaderGetAttribute(reader, BAD_CAST("RGB")), xmlFree);
      if (MINUS_ONE != idx && rgb)
      {
        Colour rgbColour = xmlStringToColour(rgb);
        m_colours[idx] = rgbColour;
      }
    }
  }
  while ((XML_COLORS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VSDXMLParserBase::readPage(xmlTextReaderPtr reader)
{
  m_shapeList.clear();
  const shared_ptr<xmlChar> id(xmlTextReaderGetAttribute(reader, BAD_CAST("ID")), xmlFree);
  const shared_ptr<xmlChar> bgndPage(xmlTextReaderGetAttribute(reader, BAD_CAST("BackPage")), xmlFree);
  const shared_ptr<xmlChar> background(xmlTextReaderGetAttribute(reader, BAD_CAST("Background")), xmlFree);
  shared_ptr<xmlChar> pageName(xmlTextReaderGetAttribute(reader, BAD_CAST("Name")), xmlFree);
  if (!pageName.get())
    pageName.reset(xmlTextReaderGetAttribute(reader, BAD_CAST("NameU")), xmlFree);
  if (id)
  {
    auto nId = (unsigned)xmlStringToLong(id);
    auto backgroundPageID = (unsigned)(bgndPage ? xmlStringToLong(bgndPage) : -1);
    bool isBackgroundPage = background ? xmlStringToBool(background) : false;
    m_isPageStarted = true;
    m_collector->startPage(nId);
    m_collector->collectPage(nId, (unsigned)getElementDepth(reader), backgroundPageID, isBackgroundPage, pageName ? VSDName(librevenge::RVNGBinaryData(pageName.get(), xmlStrlen(pageName.get())), VSD_TEXT_UTF8) : VSDName());
  }
}

void libvisio::VSDXMLParserBase::readText(xmlTextReaderPtr reader)
{
  if (xmlTextReaderIsEmptyElement(reader))
    return;

  unsigned cp = 0;
  unsigned pp = 0;
  unsigned tp = 0;
  m_shape.m_text.clear();
  m_shape.m_charList.resetCharCount();
  m_shape.m_paraList.resetCharCount();

  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readText: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_CP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        cp = getIX(reader);
      break;
    case XML_PP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        pp = getIX(reader);
      break;
    case XML_TP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        tp = getIX(reader);
      break;
    default:
      if (XML_READER_TYPE_TEXT == tokenType || XML_READER_TYPE_SIGNIFICANT_WHITESPACE == tokenType)
      {
        librevenge::RVNGBinaryData tmpText;
        const unsigned char *tmpBuffer = xmlTextReaderConstValue(reader);
        int tmpLength = xmlStrlen(tmpBuffer);
        for (int i = 0; i < tmpLength && tmpBuffer[i]; ++i)
        {
          if (i < tmpLength-1 && 0xd == tmpBuffer[i] && 0xa == tmpBuffer[i+1])
          {
            tmpText.append((unsigned char)'\n');
            ++i;
          }
          // utf-8 line separator 0xe2 0x80 0xa8 (0x2028) and paragraph separator 0xe2 0x80 0xa9 (0x2029)
          else if (i < tmpLength-2 && 0xe2 == tmpBuffer[i] && 0x80 == tmpBuffer[i+1] && (0xa8 == tmpBuffer[i+2] || 0xa9 == tmpBuffer[i+2]))
          {
            tmpText.append((unsigned char)'\n');
            ++i;
            ++i;
          }
          else
            tmpText.append(tmpBuffer[i]);
        }
        unsigned charCount = m_shape.m_charList.getCharCount(cp);
        if (MINUS_ONE == charCount && !m_shape.m_charList.empty())
          // fill non-existing character style with a legitimate default character style
          m_shape.m_charList.addCharIX(cp, m_shape.m_charList.getLevel(), m_shape.m_charStyle);
        if (!m_shape.m_charList.empty())
        {
          charCount += (unsigned)tmpText.size();
          m_shape.m_charList.setCharCount(cp, charCount);
        }

        charCount = m_shape.m_paraList.getCharCount(pp);
        if (MINUS_ONE == charCount && !m_shape.m_paraList.empty())
          // fill non-existing paragraph style with a legitimate default paragraph style
          m_shape.m_paraList.addParaIX(pp, m_shape.m_paraList.getLevel(), m_shape.m_paraStyle);
        if (!m_shape.m_paraList.empty())
        {
          charCount += (unsigned)tmpText.size();
          m_shape.m_paraList.setCharCount(pp, charCount);
        }

        m_shape.m_tabSets[tp].m_numChars += (unsigned)tmpText.size();

        m_shape.m_text.append(tmpText);
        m_shape.m_textFormat = VSD_TEXT_UTF8;
      }
      break;
    }
  }
  while ((XML_TEXT != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VSDXMLParserBase::readCharIX(xmlTextReaderPtr reader)
{
  if (xmlTextReaderIsEmptyElement(reader))
    return;

  unsigned ix = getIX(reader);

  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned charCount = 0;
  boost::optional<VSDName> font;
  boost::optional<Colour> fontColour;

  boost::optional<bool> bold;
  boost::optional<bool> italic;
  boost::optional<bool> underline;
  boost::optional<bool> doubleunderline;
  boost::optional<bool> strikeout;
  boost::optional<bool> doublestrikeout;
  boost::optional<bool> allcaps;
  boost::optional<bool> initcaps;
  boost::optional<bool> smallcaps;
  boost::optional<bool> superscript;
  boost::optional<bool> subscript;
  boost::optional<double> fontSize;
  boost::optional<double> scaleWidth;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readCharIX: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_FONT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
        if (stringValue && !xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
        {
          try
          {
            auto fontIndex = (unsigned)xmlStringToLong(stringValue);
            std::map<unsigned, VSDName>::const_iterator iter = m_fonts.find(fontIndex);
            if (iter != m_fonts.end())
              font = iter->second;
            else
              font = VSDName(librevenge::RVNGBinaryData(stringValue.get(), xmlStrlen(stringValue.get())), VSD_TEXT_UTF8);
          }
          catch (const XmlParserException &)
          {
            font = VSDName(librevenge::RVNGBinaryData(stringValue.get(), xmlStrlen(stringValue.get())), VSD_TEXT_UTF8);
          }
        }
      }
      break;
    case XML_COLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(fontColour, reader);
      break;
    case XML_STYLE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        long value = 0;
        readLongData(value, reader);
        if (value &0x1)
          bold = true;
        else
          bold = false;
        if (value &0x2)
          italic = true;
        else
          italic = false;
        if (value &0x4)
          underline = true;
        else
          underline = false;
        if (value &0x8)
          smallcaps = true;
        else
          smallcaps = false;
      }
      break;
    case XML_CASE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        long value = 0;
        readLongData(value, reader);
        switch (value)
        {
        case 1:
          allcaps = true;
          initcaps = false;
          break;
        case 2:
          allcaps = false;
          initcaps = true;
          break;
        default:
          allcaps = false;
          initcaps = false;
          break;
        }
      }
      break;
    case XML_POS:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        long value = 0;
        readLongData(value, reader);
        switch (value)
        {
        case 1:
          superscript = true;
          subscript = false;
          break;
        case 2:
          subscript = true;
          superscript = false;
          break;
        default:
          subscript = false;
          superscript = false;
          break;
        }
      }
      break;
    case XML_FONTSCALE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(scaleWidth, reader);
      break;
    case XML_SIZE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(fontSize, reader);
      break;
    case XML_DBLUNDERLINE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readBoolData(doubleunderline, reader);
      break;
    case XML_OVERLINE:
      break;
    case XML_STRIKETHRU:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readBoolData(strikeout, reader);
      break;
    case XML_HIGHLIGHT:
      break;
    case XML_DOUBLESTRIKETHROUGH:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readBoolData(doublestrikeout, reader);
      break;
    default:
      break;
    }

  }
  while (((XML_CHAR != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

  if (m_isInStyles)
    m_collector->collectCharIXStyle(ix, level, charCount, font, fontColour, fontSize, bold, italic,
                                    underline, doubleunderline, strikeout, doublestrikeout, allcaps,
                                    initcaps, smallcaps, superscript, subscript, scaleWidth);
  else
  {
    if (!ix || m_shape.m_charList.empty()) // character style 0 is the default character style
      m_shape.m_charStyle.override(VSDOptionalCharStyle(charCount, font, fontColour, fontSize, bold,
                                                        italic, underline, doubleunderline, strikeout, doublestrikeout,
                                                        allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth));

    m_shape.m_charList.addCharIX(ix, level, charCount, font, fontColour, fontSize, bold, italic,
                                 underline, doubleunderline, strikeout, doublestrikeout, allcaps,
                                 initcaps, smallcaps, superscript, subscript, scaleWidth);
  }
}

void libvisio::VSDXMLParserBase::readLayerIX(xmlTextReaderPtr reader)
{
  if (xmlTextReaderIsEmptyElement(reader))
    return;

  unsigned ix = getIX(reader);

  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  VSDLayer layer;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readLayerIX: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_COLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        Colour colour;
        long idx = -2;
        ret = readExtendedColourData(colour, idx, reader);
        if (idx != -1)
          layer.m_colour = colour;
      }
      break;
    case XML_VISIBLE:
      ret = readBoolData(layer.m_visible, reader);
      break;
    case XML_PRINT:
      ret = readBoolData(layer.m_printable, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_LAYER != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

  m_collector->collectLayer(ix, level, layer);
}

void libvisio::VSDXMLParserBase::readParaIX(xmlTextReaderPtr reader)
{
  if (xmlTextReaderIsEmptyElement(reader))
    return;

  unsigned ix = getIX(reader);

  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned charCount = 0;
  boost::optional<double> indFirst;
  boost::optional<double> indLeft;
  boost::optional<double> indRight;
  boost::optional<double> spLine;
  boost::optional<double> spBefore;
  boost::optional<double> spAfter;
  boost::optional<unsigned char> align;
  boost::optional<unsigned char> bullet;
  boost::optional<VSDName> bulletStr;
  boost::optional<VSDName> bulletFont;
  boost::optional<double> bulletFontSize;
  boost::optional<double> textPosAfterBullet;
  boost::optional<unsigned> flags;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readParaIX: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_INDFIRST:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(indFirst, reader);
      break;
    case XML_INDLEFT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(indLeft, reader);
      break;
    case XML_INDRIGHT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(indRight, reader);
      break;
    case XML_SPLINE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(spLine, reader);
      break;
    case XML_SPBEFORE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(spBefore, reader);
      break;
    case XML_SPAFTER:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(spAfter, reader);
      break;
    case XML_HORZALIGN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(align, reader);
      break;
    case XML_FLAGS:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readUnsignedData(flags, reader);
      break;
    case XML_BULLET:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(bullet, reader);
      break;
    case XML_BULLETSTR:
      if (XML_READER_TYPE_ELEMENT == tokenType && !xmlTextReaderIsEmptyElement(reader))
      {
        const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
        if (stringValue && !xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
        {
          unsigned length = xmlStrlen(stringValue.get());
          const xmlChar *strV = stringValue.get();
          // The character U+E000 is considered as empty string in VDX produced by Visio 2002
          if (3 != length || 0xee != strV[0] || 0x80 != strV[1] || 0x80 != strV[2])
            bulletStr = VSDName(librevenge::RVNGBinaryData(stringValue.get(), xmlStrlen(stringValue.get())), VSD_TEXT_UTF8);
        }
      }
      break;
    case XML_BULLETFONT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
        if (stringValue && !xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
        {
          try
          {
            auto fontIndex = (unsigned)xmlStringToLong(stringValue);
            if (fontIndex)
            {
              std::map<unsigned, VSDName>::const_iterator iter = m_fonts.find(fontIndex);
              if (iter != m_fonts.end())
                bulletFont = iter->second;
              else
                bulletFont = VSDName(librevenge::RVNGBinaryData(stringValue.get(), xmlStrlen(stringValue.get())), VSD_TEXT_UTF8);
            }
          }
          catch (const XmlParserException &)
          {
            bulletFont = VSDName(librevenge::RVNGBinaryData(stringValue.get(), xmlStrlen(stringValue.get())), VSD_TEXT_UTF8);
          }
        }
      }
      break;
    case XML_BULLETFONTSIZE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(bulletFontSize, reader);
      break;
    case XML_TEXTPOSAFTERBULLET:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(textPosAfterBullet, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_PARA != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

  if (m_isInStyles)
    m_collector->collectParaIXStyle(ix, level, charCount, indFirst, indLeft, indRight,
                                    spLine, spBefore, spAfter, align, bullet, bulletStr,
                                    bulletFont, bulletFontSize, textPosAfterBullet, flags);
  else
  {
    if (!ix || m_shape.m_paraList.empty()) // paragraph style 0 is the default paragraph style
      m_shape.m_paraStyle.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight,
                                                        spLine, spBefore, spAfter, align, bullet,
                                                        bulletStr, bulletFont, bulletFontSize,
                                                        textPosAfterBullet, flags));

    m_shape.m_paraList.addParaIX(ix, level, charCount, indFirst, indLeft, indRight,
                                 spLine, spBefore, spAfter, align, bullet, bulletStr,
                                 bulletFont, bulletFontSize, textPosAfterBullet, flags);
  }
}

void libvisio::VSDXMLParserBase::readStyleSheet(xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> id(xmlTextReaderGetAttribute(reader, BAD_CAST("ID")), xmlFree);
  const shared_ptr<xmlChar> lineStyle(xmlTextReaderGetAttribute(reader, BAD_CAST("LineStyle")), xmlFree);
  const shared_ptr<xmlChar> fillStyle(xmlTextReaderGetAttribute(reader, BAD_CAST("FillStyle")), xmlFree);
  const shared_ptr<xmlChar> textStyle(xmlTextReaderGetAttribute(reader, BAD_CAST("TextStyle")), xmlFree);
  if (id)
  {
    auto nId = (unsigned)xmlStringToLong(id);
    auto nLineStyle = (unsigned)(lineStyle ? xmlStringToLong(lineStyle) : -1);
    auto nFillStyle = (unsigned)(fillStyle ? xmlStringToLong(fillStyle) : -1);
    auto nTextStyle = (unsigned)(textStyle ? xmlStringToLong(textStyle) : -1);
    m_collector->collectStyleSheet(nId, (unsigned)getElementDepth(reader), nLineStyle, nFillStyle, nTextStyle);
  }
}

void libvisio::VSDXMLParserBase::readPageSheet(xmlTextReaderPtr reader)
{
  m_currentShapeLevel = (unsigned)getElementDepth(reader);
  m_collector->collectPageSheet(0, m_currentShapeLevel);
}

void libvisio::VSDXMLParserBase::readSplineStart(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;
  boost::optional<double> b;
  boost::optional<double> c;
  boost::optional<unsigned> d;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readSplineStart: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(a, reader);
      break;
    case XML_B:
      ret = readDoubleData(b, reader);
      break;
    case XML_C:
      ret = readDoubleData(c, reader);
      break;
    case XML_D:
      ret = readUnsignedData(d, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_SPLINESTART != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addSplineStart(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readSplineKnot(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    const shared_ptr<xmlChar> delString(xmlTextReaderGetAttribute(reader, BAD_CAST("Del")), xmlFree);
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readSplineKnot: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_X:
      ret = readDoubleData(x, reader);
      break;
    case XML_Y:
      ret = readDoubleData(y, reader);
      break;
    case XML_A:
      ret = readDoubleData(a, reader);
      break;
    default:
      break;
    }
  }
  while (((XML_SPLINEKNOT != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  if (ret == 1)
    m_currentGeometryList->addSplineKnot(ix, level, x, y, a);
}

void libvisio::VSDXMLParserBase::readStencil(xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> id(xmlTextReaderGetAttribute(reader, BAD_CAST("ID")), xmlFree);
  if (id)
  {
    auto nId = (unsigned)xmlStringToLong(id);
    m_currentStencilID = nId;
  }
  else
    m_currentStencilID = MINUS_ONE;
  m_currentStencil.reset(new VSDStencil());
}

void libvisio::VSDXMLParserBase::readForeignData(xmlTextReaderPtr reader)
{
  VSD_DEBUG_MSG(("VSDXMLParser::readForeignData\n"));
  if (!m_shape.m_foreign)
    m_shape.m_foreign = make_unique<ForeignData>();

  const shared_ptr<xmlChar> foreignTypeString(xmlTextReaderGetAttribute(reader, BAD_CAST("ForeignType")), xmlFree);
  if (foreignTypeString)
  {
    if (xmlStrEqual(foreignTypeString.get(), BAD_CAST("Bitmap")))
      m_shape.m_foreign->type = 1;
    else if (xmlStrEqual(foreignTypeString.get(), BAD_CAST("Object")))
      m_shape.m_foreign->type = 2;
    else if (xmlStrEqual(foreignTypeString.get(), BAD_CAST("EnhMetaFile")))
      m_shape.m_foreign->type = 4;
    else if (xmlStrEqual(foreignTypeString.get(), BAD_CAST("MetaFile")))
      m_shape.m_foreign->type = 0;
  }
  const shared_ptr<xmlChar> foreignFormatString(xmlTextReaderGetAttribute(reader, BAD_CAST("CompressionType")), xmlFree);
  if (foreignFormatString)
  {
    if (xmlStrEqual(foreignFormatString.get(), BAD_CAST("JPEG")))
      m_shape.m_foreign->format = 1;
    else if (xmlStrEqual(foreignFormatString.get(), BAD_CAST("GIF")))
      m_shape.m_foreign->format = 2;
    else if (xmlStrEqual(foreignFormatString.get(), BAD_CAST("TIFF")))
      m_shape.m_foreign->format = 3;
    else if (xmlStrEqual(foreignFormatString.get(), BAD_CAST("PNG")))
      m_shape.m_foreign->format = 4;
    else
      m_shape.m_foreign->format = 0;
  }
  else
    m_shape.m_foreign->format = 255;

  getBinaryData(reader);
}

void libvisio::VSDXMLParserBase::_flushShape()
{
  if (!m_isShapeStarted)
    return;

  m_collector->collectShape(m_shape.m_shapeId, m_currentShapeLevel, m_shape.m_parent, m_shape.m_masterPage, m_shape.m_masterShape, m_shape.m_lineStyleId, m_shape.m_fillStyleId, m_shape.m_textStyleId);

  m_collector->collectShapesOrder(0, m_currentShapeLevel+2, m_shape.m_shapeList.getShapesOrder());

  m_collector->collectXFormData(m_currentShapeLevel+2, m_shape.m_xform);

  m_collector->collectLayerMem(m_currentShapeLevel+2, m_shape.m_layerMem);

  m_collector->collectMisc(m_currentShapeLevel+2, m_shape.m_misc);

  if (m_shape.m_txtxform)
    m_collector->collectTxtXForm(m_currentShapeLevel+2, *(m_shape.m_txtxform));

  m_collector->collectLine(m_currentShapeLevel+2, m_shape.m_lineStyle.width, m_shape.m_lineStyle.colour, m_shape.m_lineStyle.pattern,
                           m_shape.m_lineStyle.startMarker, m_shape.m_lineStyle.endMarker, m_shape.m_lineStyle.cap, m_shape.m_lineStyle.rounding,
                           m_shape.m_lineStyle.qsLineColour, m_shape.m_lineStyle.qsLineMatrix);

  m_collector->collectFillAndShadow(m_currentShapeLevel+2, m_shape.m_fillStyle.fgColour, m_shape.m_fillStyle.bgColour, m_shape.m_fillStyle.pattern,
                                    m_shape.m_fillStyle.fgTransparency, m_shape.m_fillStyle.bgTransparency, m_shape.m_fillStyle.shadowPattern,
                                    m_shape.m_fillStyle.shadowFgColour, m_shape.m_fillStyle.shadowOffsetX, m_shape.m_fillStyle.shadowOffsetY,
                                    m_shape.m_fillStyle.qsFillColour, m_shape.m_fillStyle.qsShadowColour, m_shape.m_fillStyle.qsFillMatrix);

  m_collector->collectTextBlock(m_currentShapeLevel+2, m_shape.m_textBlockStyle.leftMargin, m_shape.m_textBlockStyle.rightMargin,
                                m_shape.m_textBlockStyle.topMargin, m_shape.m_textBlockStyle.bottomMargin, m_shape.m_textBlockStyle.verticalAlign,
                                m_shape.m_textBlockStyle.isTextBkgndFilled, m_shape.m_textBlockStyle.textBkgndColour,
                                m_shape.m_textBlockStyle.defaultTabStop, m_shape.m_textBlockStyle.textDirection);

  if (m_shape.m_foreign)
    m_collector->collectForeignDataType(m_currentShapeLevel+2, m_shape.m_foreign->type, m_shape.m_foreign->format,
                                        m_shape.m_foreign->offsetX, m_shape.m_foreign->offsetY, m_shape.m_foreign->width, m_shape.m_foreign->height);

  for (std::map<unsigned, NURBSData>::const_iterator iterNurbs = m_shape.m_nurbsData.begin(); iterNurbs != m_shape.m_nurbsData.end(); ++iterNurbs)
    m_collector->collectShapeData(iterNurbs->first, m_currentShapeLevel+2, iterNurbs->second.xType, iterNurbs->second.yType,
                                  iterNurbs->second.degree, iterNurbs->second.lastKnot, iterNurbs->second.points,
                                  iterNurbs->second.knots, iterNurbs->second.weights);

  for (std::map<unsigned, PolylineData>::const_iterator iterPoly = m_shape.m_polylineData.begin(); iterPoly != m_shape.m_polylineData.end(); ++iterPoly)
    m_collector->collectShapeData(iterPoly->first, m_currentShapeLevel+2, iterPoly->second.xType, iterPoly->second.yType, iterPoly->second.points);

  for (std::map<unsigned, VSDName>::const_iterator iterName = m_shape.m_names.begin(); iterName != m_shape.m_names.end(); ++iterName)
    m_collector->collectName(iterName->first, m_currentShapeLevel+2, iterName->second.m_data, iterName->second.m_format);

  if (!m_shape.m_geometries.empty())
  {
    for (auto &geometry : m_shape.m_geometries)
      geometry.second.resetLevel(m_currentShapeLevel+2);
    std::vector<unsigned> tmpVector;
    for (std::map<unsigned, VSDGeometryList>::const_iterator iterGeom = m_shape.m_geometries.begin(); iterGeom != m_shape.m_geometries.end(); ++iterGeom)
      tmpVector.push_back(iterGeom->first);
    std::sort(tmpVector.begin(), tmpVector.end());

    for (unsigned int i : tmpVector)
    {
      std::map<unsigned, VSDGeometryList>::const_iterator iter = m_shape.m_geometries.find(i);
      if (iter != m_shape.m_geometries.end())
      {
        iter->second.handle(m_collector);
        m_collector->collectUnhandledChunk(0, m_currentShapeLevel+1);
      }
    }
  }

  if (m_shape.m_foreign && m_shape.m_foreign->data.size())
    m_collector->collectForeignData(m_currentShapeLevel+1, m_shape.m_foreign->data);

  m_collector->collectTabsDataList(m_currentShapeLevel+1, m_shape.m_tabSets);

  if (!m_shape.m_fields.empty())
    m_shape.m_fields.handle(m_collector);

  if (m_shape.m_text.size())
    m_collector->collectText(m_currentShapeLevel+1, m_shape.m_text, m_shape.m_textFormat);

  m_collector->collectDefaultCharStyle(m_shape.m_charStyle.charCount, m_shape.m_charStyle.font, m_shape.m_charStyle.colour,
                                       m_shape.m_charStyle.size, m_shape.m_charStyle.bold, m_shape.m_charStyle.italic, m_shape.m_charStyle.underline,
                                       m_shape.m_charStyle.doubleunderline, m_shape.m_charStyle.strikeout, m_shape.m_charStyle.doublestrikeout,
                                       m_shape.m_charStyle.allcaps, m_shape.m_charStyle.initcaps, m_shape.m_charStyle.smallcaps,
                                       m_shape.m_charStyle.superscript, m_shape.m_charStyle.subscript, m_shape.m_charStyle.scaleWidth);

  m_shape.m_charList.handle(m_collector);

  m_collector->collectDefaultParaStyle(m_shape.m_paraStyle.charCount, m_shape.m_paraStyle.indFirst, m_shape.m_paraStyle.indLeft,
                                       m_shape.m_paraStyle.indRight, m_shape.m_paraStyle.spLine, m_shape.m_paraStyle.spBefore,
                                       m_shape.m_paraStyle.spAfter, m_shape.m_paraStyle.align, m_shape.m_paraStyle.bullet,
                                       m_shape.m_paraStyle.bulletStr, m_shape.m_paraStyle.bulletFont, m_shape.m_paraStyle.bulletFontSize,
                                       m_shape.m_paraStyle.textPosAfterBullet, m_shape.m_paraStyle.flags);

  m_shape.m_paraList.handle(m_collector);

  m_collector->collectUnhandledChunk(0, m_currentShapeLevel);
}

void libvisio::VSDXMLParserBase::_handleLevelChange(unsigned level)
{
  m_currentLevel = level;
  m_collector->collectUnhandledChunk(0, m_currentLevel);
}

void libvisio::VSDXMLParserBase::handlePagesStart(xmlTextReaderPtr reader)
{
  m_isShapeStarted = false;
  m_isStencilStarted = false;
  if (m_extractStencils)
    skipPages(reader);
}

void libvisio::VSDXMLParserBase::handlePagesEnd(xmlTextReaderPtr /* reader */)
{
  m_isShapeStarted = false;
  if (!m_extractStencils)
    m_collector->endPages();
}

void libvisio::VSDXMLParserBase::handlePageStart(xmlTextReaderPtr reader)
{
  m_isShapeStarted = false;
  if (!m_extractStencils)
    readPage(reader);
}

void libvisio::VSDXMLParserBase::handlePageEnd(xmlTextReaderPtr /* reader */)
{
  m_isShapeStarted = false;
  if (!m_extractStencils)
  {
    m_collector->collectShapesOrder(0, 2, m_shapeList.getShapesOrder());
    _handleLevelChange(0);
    m_shapeList.clear();
    m_isPageStarted = false;
    m_collector->endPage();
  }
}

void libvisio::VSDXMLParserBase::handleMastersStart(xmlTextReaderPtr reader)
{
  m_isShapeStarted = false;
  if (m_stencils.count())
    skipMasters(reader);
  else
  {
    if (m_extractStencils)
      m_isStencilStarted = false;
    else
      m_isStencilStarted = true;
  }
}

void libvisio::VSDXMLParserBase::handleMastersEnd(xmlTextReaderPtr /* reader */)
{
  m_isShapeStarted = false;
  if (m_extractStencils)
    m_collector->endPages();
  else
    m_isStencilStarted = false;
}

void libvisio::VSDXMLParserBase::handleMasterStart(xmlTextReaderPtr reader)
{
  m_isShapeStarted = false;
  if (m_extractStencils)
    readPage(reader);
  else
    readStencil(reader);
}

void libvisio::VSDXMLParserBase::handleMasterEnd(xmlTextReaderPtr /* reader */)
{
  m_isShapeStarted = false;
  m_isPageStarted = false;
  if (m_extractStencils)
  {
    m_collector->collectShapesOrder(0, 2, m_shapeList.getShapesOrder());
    _handleLevelChange(0);
    m_shapeList.clear();
    m_isPageStarted = false;
    m_collector->endPage();
  }
  else
  {
    if (m_currentStencil)
      m_stencils.addStencil(m_currentStencilID, *m_currentStencil);
    m_currentStencil.reset();
    m_currentStencilID = MINUS_ONE;
  }
}

void libvisio::VSDXMLParserBase::skipMasters(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    tokenType = xmlTextReaderNodeType(reader);
  }
  while ((XML_MASTERS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXMLParserBase::skipPages(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    tokenType = xmlTextReaderNodeType(reader);
  }
  while ((XML_PAGES != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

int libvisio::VSDXMLParserBase::readNURBSData(boost::optional<NURBSData> &data, xmlTextReaderPtr reader)
{
  NURBSData tmpData;

  bool bRes = false;
  const shared_ptr<xmlChar> formula(readStringData(reader), xmlFree);

  if (formula)
  {
    std::pair<double, double> point;

    using namespace boost::spirit::qi;
    namespace phx = boost::phoenix;
    using phx::push_back;
    using phx::ref;

    auto first = reinterpret_cast<const char *>(formula.get());
    const auto last = first + strlen(first);
    bRes = phrase_parse(first, last,
                        //  Begin grammar
                        (
                          lit("NURBS")
                          >> '('
                          >> double_[ref(tmpData.lastKnot) = _1] >> -lit(',')
                          >> int_[ref(tmpData.degree) = _1] >> -lit(',')
                          >> int_[ref(tmpData.xType) = _1] >> -lit(',')
                          >> int_[ref(tmpData.yType) = _1] >> -lit(',')
                          >> // array of points, weights and knots
                          (
                            (
                              (double_[ref(point.first) = _1] >> -lit(',') >>
                               double_[ref(point.second) = _1]
                              )[push_back(phx::ref(tmpData.points), phx::cref(point))]
                              >> -lit(',') >>
                              double_[push_back(phx::ref(tmpData.knots),
                                                _1)] >> -lit(',') >>
                              double_[push_back(phx::ref(tmpData.weights), _1)]
                            )
                            % -lit(',')
                          )
                          >> ')'
                        ),
                        //  End grammar
                        space)
           && first == last;
  }

  if (!bRes)
    return -1;
  data = tmpData;
  return 1;
}

int libvisio::VSDXMLParserBase::readPolylineData(boost::optional<PolylineData> &data, xmlTextReaderPtr reader)
{
  PolylineData tmpData;

  bool bRes = false;
  const shared_ptr<xmlChar> formula(readStringData(reader), xmlFree);

  if (formula)
  {
    std::pair<double, double> point;

    using namespace boost::spirit::qi;
    namespace phx = boost::phoenix;
    using phx::push_back;
    using phx::ref;

    auto first = reinterpret_cast<const char *>(formula.get());
    const auto last = first + strlen(first);
    bRes = phrase_parse(first, last,
                        (
                          lit("POLYLINE")
                          >> '('
                          >> int_[ref(tmpData.xType) = _1] >> -lit(',')
                          >> int_[ref(tmpData.yType) = _1] >> -lit(',')
                          >> // array of points
                          (
                            (
                              double_[ref(point.first) = _1] >> -lit(',')
                              >> double_[ref(point.second) = _1]
                            )[push_back(phx::ref(tmpData.points), phx::cref(point))] % -lit(',')
                          )
                          >> ')'
                        ),
                        space)
           && first == last;
  }

  if (!bRes)
    return -1;
  data = tmpData;
  return 1;
}


int libvisio::VSDXMLParserBase::readDoubleData(double &value, xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXMLParserBase::readDoubleData stringValue %s\n", (const char *)stringValue.get()));
    if (!xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
      value = xmlStringToDouble(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXMLParserBase::readStringData(libvisio::VSDName &text, xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXMLParserBase::readStringData stringValue %s\n", (const char *)stringValue.get()));
    if (!xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
    {
      text.m_data = librevenge::RVNGBinaryData(stringValue.get(), xmlStrlen(stringValue.get()));
      text.m_format = VSD_TEXT_UTF8;
    }
    return 1;
  }
  return -1;
}

int libvisio::VSDXMLParserBase::readDoubleData(boost::optional<double> &value, xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXMLParserBase::readDoubleData stringValue %s\n", (const char *)stringValue.get()));
    if (!xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
      value = xmlStringToDouble(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXMLParserBase::readLongData(long &value, xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXMLParserBase::readLongData stringValue %s\n", (const char *)stringValue.get()));
    if (!xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
      value = xmlStringToLong(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXMLParserBase::readLongData(boost::optional<long> &value, xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXMLParserBase::readLongData stringValue %s\n", (const char *)stringValue.get()));
    if (!xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
      value = xmlStringToLong(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXMLParserBase::readBoolData(bool &value, xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXMLParserBase::readBoolData stringValue %s\n", (const char *)stringValue.get()));
    if (!xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
      value = xmlStringToBool(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXMLParserBase::readBoolData(boost::optional<bool> &value, xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXMLParserBase::readBoolData stringValue %s\n", (const char *)stringValue.get()));
    if (!xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
      value = xmlStringToBool(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXMLParserBase::readUnsignedData(boost::optional<unsigned> &value, xmlTextReaderPtr reader)
{
  boost::optional<long> tmpValue;
  int ret = readLongData(tmpValue, reader);
  if (!!tmpValue)
    value = (unsigned)tmpValue.get();
  return ret;
}

int libvisio::VSDXMLParserBase::readByteData(unsigned char &value, xmlTextReaderPtr reader)
{
  long longValue = 0;
  int ret = readLongData(longValue, reader);
  value = (unsigned char) longValue;
  return ret;
}

int libvisio::VSDXMLParserBase::readByteData(boost::optional<unsigned char> &value, xmlTextReaderPtr reader)
{
  boost::optional<long> tmpValue;
  int ret = readLongData(tmpValue, reader);
  if (!!tmpValue)
    value = (unsigned char) tmpValue.get();
  return ret;
}

int libvisio::VSDXMLParserBase::readExtendedColourData(Colour &value, long &idx, xmlTextReaderPtr reader)
{
  const shared_ptr<xmlChar> stringValue(readStringData(reader), xmlFree);
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXMLParserBase::readColourData stringValue %s\n", (const char *)stringValue.get()));
    if (!xmlStrEqual(stringValue.get(), BAD_CAST("Themed")))
    {
      try
      {
        value = xmlStringToColour(stringValue);
      }
      catch (const XmlParserException &)
      {
        idx = xmlStringToLong(stringValue);
      }
      if (idx >= 0)
      {
        std::map<unsigned, Colour>::const_iterator iter = m_colours.find((unsigned)idx);
        if (iter != m_colours.end())
          value = iter->second;
        else
          idx = -1;
      }
    }
    else
      return -1000;
    return 1;
  }
  return -1;
}

int libvisio::VSDXMLParserBase::readExtendedColourData(boost::optional<Colour> &value, xmlTextReaderPtr reader)
{
  Colour tmpValue;
  int ret = readExtendedColourData(tmpValue, reader);
  if (ret != -1000)
    value = tmpValue;
  else
    ret = 1;
  return ret;
}

int libvisio::VSDXMLParserBase::readExtendedColourData(Colour &value, xmlTextReaderPtr reader)
{
  long idx = -1;
  return readExtendedColourData(value, idx, reader);
}

unsigned libvisio::VSDXMLParserBase::getIX(xmlTextReaderPtr reader)
{
  auto ix = MINUS_ONE;
  const std::shared_ptr<xmlChar> ixString(xmlTextReaderGetAttribute(reader, BAD_CAST("IX")), xmlFree);
  if (ixString)
    ix = (unsigned)xmlStringToLong(ixString.get());
  return ix;
}

void libvisio::VSDXMLParserBase::readTriggerId(unsigned &id, xmlTextReaderPtr reader)
{
  using namespace boost::spirit::qi;

  auto triggerId = MINUS_ONE;
  const std::shared_ptr<xmlChar> triggerString(xmlTextReaderGetAttribute(reader, BAD_CAST("F")), xmlFree);
  if (triggerString)
  {
    auto first = reinterpret_cast<const char *>(triggerString.get());
    const auto last = first + strlen(first);
    if (phrase_parse(first, last,
                     (
                       lit("_XFTRIGGER")
                       >> '(' >> omit[+alnum] >> '.'
                       >> int_ >> '!' >> lit("EventXFMod")
                       >> ')'
                     ),
                     space, triggerId) && first == last)
      id = triggerId;
  }
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
