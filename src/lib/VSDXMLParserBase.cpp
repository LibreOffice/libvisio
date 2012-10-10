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
    m_isStencilStarted(false), m_currentStencilID((unsigned)-1), m_extractStencils(false), m_isInStyles(false),
    m_currentLevel(0), m_currentShapeLevel(0), m_colours(), m_charList(new VSDCharacterList()), m_charListVector(),
    m_fieldList(), m_geomList(new VSDGeometryList()), m_geomListVector(), m_paraList(new VSDParagraphList()),
    m_paraListVector(), m_shapeList(), m_currentBinaryData()
{
}

libvisio::VSDXMLParserBase::~VSDXMLParserBase()
{
  if (m_geomList)
  {
    m_geomList->clear();
    delete m_geomList;
  }
  if (m_charList)
  {
    m_charList->clear();
    delete m_charList;
  }
  if (m_paraList)
  {
    m_paraList->clear();
    delete m_paraList;
  }
  if (m_currentStencil)
    delete m_currentStencil;
}

// Common functions

void libvisio::VSDXMLParserBase::readEllipticalArcTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readForeignData(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readEllipse(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readGeomList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readGeometry(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readMoveTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readLineTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readArcTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readNURBSTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readPolylineTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readInfiniteLine(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readShapeData(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readShapeId(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readShapeList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readForeignDataType(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readShape(xmlTextReaderPtr reader)
{
  m_currentShapeLevel = getElementDepth(reader);

  xmlChar *idString = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  xmlChar *masterPageString = xmlTextReaderGetAttribute(reader, BAD_CAST("MasterPage"));
  xmlChar *masterShapeString = xmlTextReaderGetAttribute(reader, BAD_CAST("MasterShape"));
  xmlChar *lineStyleString = xmlTextReaderGetAttribute(reader, BAD_CAST("LineStyle"));
  xmlChar *fillStyleString = xmlTextReaderGetAttribute(reader, BAD_CAST("FillStyle"));
  xmlChar *textStyleString = xmlTextReaderGetAttribute(reader, BAD_CAST("TextStyle"));

  unsigned id = (unsigned)(idString ? xmlStringToLong(idString) : -1);
  unsigned masterPage =  (unsigned)(masterPageString ? xmlStringToLong(masterPageString) : -1);
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

  if (m_isStencilStarted)
  {
    m_shape = VSDShape();

    m_shape.m_lineStyleId = lineStyle;
    m_shape.m_fillStyleId = fillStyle;
    m_shape.m_textStyleId = textStyle;
  }
  else
    m_collector->collectShape(id, m_currentShapeLevel, 0, masterPage, masterShape, lineStyle, fillStyle, textStyle);
}

void libvisio::VSDXMLParserBase::readColours(xmlTextReaderPtr reader)
{
  m_colours.clear();

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
      xmlFree(rgb);
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

void libvisio::VSDXMLParserBase::readSplineStart(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readSplineKnot(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readStencil(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  if (id)
  {
    unsigned nId = (unsigned)xmlStringToLong(id);
    m_currentStencilID = nId;
  }
  else
    m_currentStencilID = (unsigned)-1;
  if (m_currentStencil)
    delete m_currentStencil;
  m_currentStencil = new VSDStencil();
  if (id)
    xmlFree(id);
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

int libvisio::VSDXMLParserBase::readExtendedColourData(Colour &value, long &idx, xmlTextReaderPtr reader)
{
  int ret = 1;
  idx = -1;
  try
  {
    ret = readLongData(idx, reader);
  }
  catch (const XmlParserException &)
  {
    idx = -1;
    ret = readColourData(value, reader);
  }
  if (idx > 0)
  {
    std::map<unsigned, Colour>::const_iterator iter = m_colours.find((unsigned)idx);
    if (iter != m_colours.end())
      value = iter->second;
    else
      idx = -1;
  }
  return ret;
}

int libvisio::VSDXMLParserBase::readExtendedColourData(Colour &value, xmlTextReaderPtr reader)
{
  long idx = -1;
  return readExtendedColourData(value, idx, reader);
}

void libvisio::VSDXMLParserBase::_handleLevelChange(unsigned level)
{
  if (level == m_currentLevel)
    return;
  if (level <= m_currentShapeLevel+1)
  {
    m_geomListVector.push_back(m_geomList);
    m_charListVector.push_back(m_charList);
    m_paraListVector.push_back(m_paraList);
    // reinitialize, but don't clear, because we want those pointers to be valid until we handle the whole vector
    m_geomList = new VSDGeometryList();
    m_charList = new VSDCharacterList();
    m_paraList = new VSDParagraphList();
    m_shapeList.handle(m_collector);
    m_shapeList.clear();
  }
  if (level <= m_currentShapeLevel)
  {
    for (std::vector<VSDGeometryList *>::iterator iter = m_geomListVector.begin(); iter != m_geomListVector.end(); ++iter)
    {
      (*iter)->handle(m_collector);
      (*iter)->clear();
      delete *iter;
    }
    m_geomListVector.clear();
    for (std::vector<VSDCharacterList *>::iterator iter2 = m_charListVector.begin(); iter2 != m_charListVector.end(); ++iter2)
    {
      (*iter2)->handle(m_collector);
      (*iter2)->clear();
      delete *iter2;
    }
    m_charListVector.clear();
    for (std::vector<VSDParagraphList *>::iterator iter3 = m_paraListVector.begin(); iter3 != m_paraListVector.end(); ++iter3)
    {
      (*iter3)->handle(m_collector);
      (*iter3)->clear();
      delete *iter3;
    }
    m_paraListVector.clear();
    if (!m_fieldList.empty())
    {
      m_fieldList.handle(m_collector);
      m_fieldList.clear();
    }
  }
  m_currentLevel = level;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
