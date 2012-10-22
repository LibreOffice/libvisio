/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* libvisio
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Fridrich Strba <fridrich.strba@bluewin.ch>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */

#include <string.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlstring.h>
#include <libwpd-stream/libwpd-stream.h>
#include <boost/algorithm/string.hpp>
#include "VSDXMLParserBase.h"
#include "libvisio_utils.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"
#include "VSDZipStream.h"
#include "VSDXMLHelper.h"
#include "VSDXMLTokenMap.h"


libvisio::VSDXMLParserBase::VSDXMLParserBase()
  : m_collector(), m_stencils(), m_currentStencil(0), m_shape(),
    m_isStencilStarted(false), m_currentStencilID(MINUS_ONE),
    m_extractStencils(false), m_isInStyles(false), m_currentLevel(0),
    m_currentShapeLevel(0), m_colours(), m_fieldList(), m_shapeList(),
    m_currentBinaryData(), m_shapeStack(), m_shapeLevelStack(),
    m_isShapeStarted(false), m_isPageStarted(false), m_currentGeometryList(0),
    m_currentGeometryListIndex(MINUS_ONE)
{
}

libvisio::VSDXMLParserBase::~VSDXMLParserBase()
{
  if (m_currentStencil)
    delete m_currentStencil;
}

// Common functions

void libvisio::VSDXMLParserBase::readGeometry(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  m_currentGeometryList = &m_shape.m_geometries[ix];

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
      {
        m_currentGeometryList->clear();
        m_shape.m_geometries.erase(ix);
      }
      xmlFree(delString);
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
    if (-1 == tokenId)
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
    default:
      break;
    }
  }
  while (((XML_GEOM != tokenId && XML_SECTION != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addGeometry(0, level+1, noFill, noLine, noShow);
}

void libvisio::VSDXMLParserBase::readMoveTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
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
  while (((XML_MOVETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addMoveTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readLineTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
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
  while (((XML_LINETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addLineTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readArcTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
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
    if (-1 == tokenId)
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
  while (((XML_ARCTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addArcTo(ix, level, x, y, a);
}

void libvisio::VSDXMLParserBase::readEllipticalArcTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
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
    if (-1 == tokenId)
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
  while (((XML_ELLIPTICALARCTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addEllipticalArcTo(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readEllipse(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
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
    if (-1 == tokenId)
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
  while (((XML_ELLIPSE != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addEllipse(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readNURBSTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
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
    default:
      break;
    }
  }
  while (((XML_NURBSTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addMoveTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readPolylineTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
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
    default:
      break;
    }
  }
  while (((XML_POLYLINETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addMoveTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readInfiniteLine(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
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
    if (-1 == tokenId)
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
  while (((XML_INFINITELINE != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addInfiniteLine(ix, level, x, y, a, b);
}

void libvisio::VSDXMLParserBase::readRelEllipticalArcTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
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
    if (-1 == tokenId)
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
  while (((XML_RELELLIPTICALARCTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addRelEllipticalArcTo(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readRelCubBezTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
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
    if (-1 == tokenId)
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
  while (((XML_RELCUBBEZTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addRelCubBezTo(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readRelLineTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
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
  while (((XML_RELLINETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addRelLineTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readRelMoveTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
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
  while (((XML_RELMOVETO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addRelMoveTo(ix, level, x, y);
}

void libvisio::VSDXMLParserBase::readRelQuadBezTo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
    }
    return;
  }

  boost::optional<double> x;
  boost::optional<double> y;
  boost::optional<double> a;
  boost::optional<double> b;
  boost::optional<double> d;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
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
  while (((XML_RELQUADBEZTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addRelQuadBezTo(ix, level, x, y, a, b);
}

void libvisio::VSDXMLParserBase::readShape(xmlTextReaderPtr reader)
{
  m_isShapeStarted = true;
  m_currentShapeLevel = getElementDepth(reader);

  xmlChar *idString = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  xmlChar *masterPageString = xmlTextReaderGetAttribute(reader, BAD_CAST("MasterPage"));
  if (!masterPageString)
    masterPageString = xmlTextReaderGetAttribute(reader, BAD_CAST("Master"));
  xmlChar *masterShapeString = xmlTextReaderGetAttribute(reader, BAD_CAST("MasterShape"));
  xmlChar *lineStyleString = xmlTextReaderGetAttribute(reader, BAD_CAST("LineStyle"));
  xmlChar *fillStyleString = xmlTextReaderGetAttribute(reader, BAD_CAST("FillStyle"));
  xmlChar *textStyleString = xmlTextReaderGetAttribute(reader, BAD_CAST("TextStyle"));

  unsigned id = (unsigned)(idString ? xmlStringToLong(idString) : -1);
  unsigned masterPage = MINUS_ONE;
  if (!m_shapeStack.empty())
    masterPage = m_shapeStack.top().m_masterPage;
  if (masterPageString)
    masterPage = xmlStringToLong(masterPageString);
  unsigned masterShape =  (unsigned)(masterShapeString ? xmlStringToLong(masterShapeString) : -1);
  unsigned lineStyle =  (unsigned)(lineStyleString ? xmlStringToLong(lineStyleString) : -1);
  unsigned fillStyle =  (unsigned)(fillStyleString ? xmlStringToLong(fillStyleString) : -1);
  unsigned textStyle =  (unsigned)(textStyleString ? xmlStringToLong(textStyleString) : -1);
  if (idString)
    xmlFree(idString);
  if (masterPageString)
    xmlFree(masterPageString);
  if (masterShapeString)
    xmlFree(masterShapeString);
  if (lineStyleString)
    xmlFree(lineStyleString);
  if (fillStyleString)
    xmlFree(fillStyleString);
  if (textStyleString)
    xmlFree(textStyleString);

  m_shape.clear();

  if (m_isStencilStarted)
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
        m_shape.m_foreign = new ForeignData(*(tmpShape->m_foreign));
      m_shape.m_text = tmpShape->m_text;
      m_shape.m_textFormat = tmpShape->m_textFormat;
      m_shape.m_lineStyle = tmpShape->m_lineStyle;
      m_shape.m_fillStyle = tmpShape->m_fillStyle;
      m_shape.m_xform = tmpShape->m_xform;
      if (tmpShape->m_txtxform)
        m_shape.m_txtxform = new XForm(*(tmpShape->m_txtxform));
      m_shape.m_geometries = tmpShape->m_geometries;
      m_shape.m_lineStyleId = tmpShape->m_lineStyleId;
      m_shape.m_fillStyleId = tmpShape->m_fillStyleId;
      m_shape.m_textStyleId = tmpShape->m_textStyleId;
    }
  }

  if (!m_shapeStack.empty())
    m_shapeStack.top().m_shapeList.addShapeId(id);
  else
    m_shapeList.addShapeId(id);

  m_shape.m_lineStyleId = lineStyle != MINUS_ONE ? lineStyle : m_shape.m_lineStyleId;
  m_shape.m_fillStyleId = fillStyle != MINUS_ONE ? fillStyle : m_shape.m_fillStyleId;
  m_shape.m_textStyleId = textStyle != MINUS_ONE ? textStyle : m_shape.m_textStyleId;;
  m_shape.m_parent = m_shapeStack.empty() ? MINUS_ONE : m_shapeStack.top().m_shapeId;
  m_shape.m_masterPage = masterPage;
  m_shape.m_masterShape = masterShape;
  m_shape.m_shapeId = id;
}

void libvisio::VSDXMLParserBase::readColours(xmlTextReaderPtr reader)
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

  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readColours: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    if (XML_COLORENTRY == tokenId)
    {
      xmlChar *ix = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
      xmlChar *rgb = xmlTextReaderGetAttribute(reader, BAD_CAST("RGB"));
      if (ix && rgb)
      {
        unsigned idx = (unsigned)xmlStringToLong(ix);
        Colour rgbColour = xmlStringToColour(rgb);
        if (idx)
          m_colours[idx] = rgbColour;
      }
      if (rgb)
        xmlFree(rgb);
      if (ix)
        xmlFree(ix);

    }
  }
  while ((XML_COLORS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXMLParserBase::readCharList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readParaList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readPage(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  xmlChar *bgndPage = xmlTextReaderGetAttribute(reader, BAD_CAST("BackPage"));
  xmlChar *background = xmlTextReaderGetAttribute(reader, BAD_CAST("Background"));
  if (id)
  {
    unsigned nId = (unsigned)xmlStringToLong(id);
    unsigned backgroundPageID =  (unsigned)(bgndPage ? xmlStringToLong(bgndPage) : -1);
    bool isBackgroundPage = background ? xmlStringToBool(background) : false;
    m_isPageStarted = true;
    m_collector->startPage(nId);
    m_collector->collectPage(nId, (unsigned)getElementDepth(reader), backgroundPageID, isBackgroundPage);
  }
  if (id)
    xmlFree(id);
  if (bgndPage)
    xmlFree(bgndPage);
  if (background)
    xmlFree(background);
}

void libvisio::VSDXMLParserBase::readText(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readCharIX(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readParaIX(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readNameList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readName(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readFieldList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readTextField(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readStyleSheet(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  xmlChar *lineStyle = xmlTextReaderGetAttribute(reader, BAD_CAST("LineStyle"));
  xmlChar *fillStyle = xmlTextReaderGetAttribute(reader, BAD_CAST("FillStyle"));
  xmlChar *textStyle = xmlTextReaderGetAttribute(reader, BAD_CAST("TextStyle"));
  if (id)
  {
    unsigned nId = (unsigned)xmlStringToLong(id);
    unsigned nLineStyle =  (unsigned)(lineStyle ? xmlStringToLong(lineStyle) : -1);
    unsigned nFillStyle = (unsigned)(fillStyle ? xmlStringToLong(fillStyle) : -1);
    unsigned nTextStyle = (unsigned)(textStyle ? xmlStringToLong(textStyle) : -1);
    m_collector->collectStyleSheet(nId, (unsigned)getElementDepth(reader), nLineStyle, nFillStyle, nTextStyle);
  }
  if (id)
    xmlFree(id);
  if (lineStyle)
    xmlFree(lineStyle);
  if (fillStyle)
    xmlFree(fillStyle);
  if (textStyle)
    xmlFree(textStyle);
}

void libvisio::VSDXMLParserBase::readPageSheet(xmlTextReaderPtr reader)
{
  m_currentShapeLevel = (unsigned)getElementDepth(reader);
  m_collector->collectPageSheet(0, m_currentShapeLevel);
}

void libvisio::VSDXMLParserBase::readSplineStart(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
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
    if (-1 == tokenId)
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
  while (((XML_ELLIPTICALARCTO != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addSplineStart(ix, level, x, y, a, b, c, d);
}

void libvisio::VSDXMLParserBase::readSplineKnot(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  int level = getElementDepth(reader);

  unsigned ix = MINUS_ONE;
  xmlChar *ixString = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
  if (ixString)
  {
    ix = xmlStringToLong(ixString);
    xmlFree(ixString);
  }

  if (xmlTextReaderIsEmptyElement(reader))
  {
    xmlChar *delString = xmlTextReaderGetAttribute(reader, BAD_CAST("del"));
    if (delString)
    {
      if (xmlStringToBool(delString))
        m_currentGeometryList->addEmpty(ix, level);
      xmlFree(delString);
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
    if (-1 == tokenId)
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
  while (((XML_SPLINEKNOT != tokenId && XML_ROW != tokenId) || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (ret == 1)
    m_currentGeometryList->addSplineKnot(ix, level, x, y, a);
}

void libvisio::VSDXMLParserBase::readStencil(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  if (id)
  {
    unsigned nId = (unsigned)xmlStringToLong(id);
    m_currentStencilID = nId;
    xmlFree(id);
  }
  else
    m_currentStencilID = MINUS_ONE;
  if (m_currentStencil)
    delete m_currentStencil;
  m_currentStencil = new VSDStencil();
}

void libvisio::VSDXMLParserBase::readStencilShape(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readOLEList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readOLEData(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readForeignData(xmlTextReaderPtr reader)
{
  VSD_DEBUG_MSG(("VSDXParser::readForeignData\n"));
  if (!m_shape.m_foreign)
    m_shape.m_foreign = new ForeignData();

  xmlChar *foreignTypeString = xmlTextReaderGetAttribute(reader, BAD_CAST("ForeignType"));
  if (foreignTypeString)
  {
    if (xmlStrEqual(foreignTypeString, BAD_CAST("Bitmap")))
      m_shape.m_foreign->type = 1;
    else if (xmlStrEqual(foreignTypeString, BAD_CAST("Object")))
      m_shape.m_foreign->type = 2;
    else if (xmlStrEqual(foreignTypeString, BAD_CAST("EnhMetaFile")))
      m_shape.m_foreign->type = 4;
    xmlFree(foreignTypeString);
  }
  xmlChar *foreignFormatString = xmlTextReaderGetAttribute(reader, BAD_CAST("CompressionType"));
  if (foreignFormatString)
  {
    if (xmlStrEqual(foreignFormatString, BAD_CAST("JPEG")))
      m_shape.m_foreign->format = 1;
    else if (xmlStrEqual(foreignFormatString, BAD_CAST("GIF")))
      m_shape.m_foreign->format = 2;
    else if (xmlStrEqual(foreignFormatString, BAD_CAST("TIFF")))
      m_shape.m_foreign->format = 3;
    else if (xmlStrEqual(foreignFormatString, BAD_CAST("PNG")))
      m_shape.m_foreign->format = 4;
    else
      m_shape.m_foreign->format = 0;
    xmlFree(foreignFormatString);
  }
  else
    m_shape.m_foreign->format = 255;

  getBinaryData(reader);
}

int libvisio::VSDXMLParserBase::readLongData(boost::optional<long> &value, xmlTextReaderPtr reader)
{
  long tmpValue;
  int ret = readLongData(tmpValue, reader);
  value = tmpValue;
  return ret;
}

int libvisio::VSDXMLParserBase::readExtendedColourData(boost::optional<Colour> &value, xmlTextReaderPtr reader)
{
  Colour tmpValue;
  int ret = readExtendedColourData(tmpValue, reader);
  value = tmpValue;
  return ret;
}

int libvisio::VSDXMLParserBase::readExtendedColourData(Colour &value, xmlTextReaderPtr reader)
{
  long idx = -1;
  return readExtendedColourData(value, idx, reader);
}

void libvisio::VSDXMLParserBase::_flushShape()
{
  if (!m_isShapeStarted)
    return;

  m_collector->collectShape(m_shape.m_shapeId, m_currentShapeLevel, m_shape.m_parent, m_shape.m_masterPage, m_shape.m_masterShape, m_shape.m_lineStyleId, m_shape.m_fillStyleId, m_shape.m_textStyleId);

  m_collector->collectShapesOrder(0, m_currentShapeLevel+2, m_shape.m_shapeList.getShapesOrder());

  m_collector->collectXFormData(m_currentShapeLevel+2, m_shape.m_xform);

  if (m_shape.m_txtxform)
    m_collector->collectTxtXForm(m_currentShapeLevel+2, *(m_shape.m_txtxform));

  m_collector->collectLine(m_currentShapeLevel+2, m_shape.m_lineStyle.width, m_shape.m_lineStyle.colour, m_shape.m_lineStyle.pattern,
                           m_shape.m_lineStyle.startMarker, m_shape.m_lineStyle.endMarker, m_shape.m_lineStyle.cap);

  m_collector->collectFillAndShadow(m_currentShapeLevel+2, m_shape.m_fillStyle.fgColour, m_shape.m_fillStyle.bgColour, m_shape.m_fillStyle.pattern,
                                    m_shape.m_fillStyle.fgTransparency, m_shape.m_fillStyle.bgTransparency, m_shape.m_fillStyle.shadowPattern,
                                    m_shape.m_fillStyle.shadowFgColour, m_shape.m_fillStyle.shadowOffsetX, m_shape.m_fillStyle.shadowOffsetY);

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
    for (std::map<unsigned, VSDGeometryList>::iterator iter = m_shape.m_geometries.begin(); iter != m_shape.m_geometries.end(); ++iter)
      iter->second.resetLevel(m_currentShapeLevel+2);
    std::vector<unsigned> tmpVector;
    for (std::map<unsigned, VSDGeometryList>::const_iterator iterGeom = m_shape.m_geometries.begin(); iterGeom != m_shape.m_geometries.end(); ++iterGeom)
      tmpVector.push_back(iterGeom->first);
    std::sort(tmpVector.begin(), tmpVector.end());

    for (unsigned i = 0; i < tmpVector.size(); i++)
    {
      std::map<unsigned, VSDGeometryList>::const_iterator iter = m_shape.m_geometries.find(tmpVector[i]);
      if (iter != m_shape.m_geometries.end())
      {
        iter->second.handle(m_collector);
        m_collector->collectUnhandledChunk(0, m_currentShapeLevel+1);
      }
    }
  }

  if (m_shape.m_foreign)
    m_collector->collectForeignData(m_currentShapeLevel+1, m_shape.m_foreign->data);

  if (!m_shape.m_fields.empty())
    m_shape.m_fields.handle(m_collector);

  if (m_shape.m_text.size())
    m_collector->collectText(m_currentShapeLevel+1, m_shape.m_text, m_shape.m_textFormat);


  for (std::vector<VSDCharacterList>::const_iterator iterChar = m_shape.m_charListVector.begin(); iterChar != m_shape.m_charListVector.end(); ++iterChar)
    iterChar->handle(m_collector);

  for (std::vector<VSDParagraphList>::const_iterator iterPara = m_shape.m_paraListVector.begin(); iterPara != m_shape.m_paraListVector.end(); ++iterPara)
    iterPara->handle(m_collector);

  m_collector->collectUnhandledChunk(0, m_currentShapeLevel);
}

void libvisio::VSDXMLParserBase::_handleLevelChange(unsigned level)
{
  m_currentLevel = level;
  m_collector->collectUnhandledChunk(0, m_currentLevel);
}

int libvisio::VSDXMLParserBase::readDoubleData(boost::optional<double> &value, xmlTextReaderPtr reader)
{
  double tmpValue = 0.0;
  int ret = 1;
  try
  {
    ret = readDoubleData(tmpValue, reader);
    value = tmpValue;
  }
  catch (const XmlParserException &)
  {
    return -1;
  }
  return ret;
}

int libvisio::VSDXMLParserBase::readBoolData(boost::optional<bool> &value, xmlTextReaderPtr reader)
{
  bool tmpValue = false;
  int ret = 1;
  try
  {
    ret = readBoolData(tmpValue, reader);
    value = tmpValue;
  }
  catch (const XmlParserException &)
  {
    return -1;
  }
  return ret;
}

int libvisio::VSDXMLParserBase::readUnsignedData(boost::optional<unsigned> &value, xmlTextReaderPtr reader)
{
  long tmpValue = 0;
  int ret = 1;
  try
  {
    ret = readLongData(tmpValue, reader);
    value = (unsigned)tmpValue;
  }
  catch (const XmlParserException &)
  {
    return -1;
  }
  return ret;
}

int libvisio::VSDXMLParserBase::readByteData(boost::optional<unsigned char> &value, xmlTextReaderPtr reader)
{
  unsigned char tmpValue = 0;
  int ret = readByteData(tmpValue, reader);
  value = tmpValue;
  return ret;
}

int libvisio::VSDXMLParserBase::readByteData(unsigned char &value, xmlTextReaderPtr reader)
{
  long longValue = 0;
  int ret = readLongData(longValue, reader);
  value = (unsigned char) longValue;
  return ret;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
