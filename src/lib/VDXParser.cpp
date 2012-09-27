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
#include "VDXParser.h"
#include "libvisio_utils.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"
#include "VSDZipStream.h"
#include "VSDXMLHelper.h"
#include "VSDXMLTokenMap.h"


libvisio::VDXParser::VDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : m_input(input), m_painter(painter), m_collector(), m_stencils(), m_stencilShape(),
    m_isStencilStarted(false), m_extractStencils(false), m_isInStyles(false)
{
}

libvisio::VDXParser::~VDXParser()
{
}

bool libvisio::VDXParser::parseMain()
{
  if (!m_input)
    return false;

  WPXInputStream *tmpInput = 0;
  try
  {
    std::vector<std::map<unsigned, XForm> > groupXFormsSequence;
    std::vector<std::map<unsigned, unsigned> > groupMembershipsSequence;
    std::vector<std::list<unsigned> > documentPageShapeOrders;

    VSDStylesCollector stylesCollector(groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders);
    m_collector = &stylesCollector;
    m_input->seek(0, WPX_SEEK_SET);
    if (!processXmlDocument(m_input))
      return false;

    VSDStyles styles = stylesCollector.getStyleSheets();

    VSDContentCollector contentCollector(m_painter, groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders, styles, m_stencils);
    m_collector = &contentCollector;
    m_input->seek(0, WPX_SEEK_SET);
    if (!processXmlDocument(m_input))
      return false;

    return true;
  }
  catch (...)
  {
    if (tmpInput)
      delete tmpInput;
    return false;
  }
}

bool libvisio::VDXParser::extractStencils()
{
  m_extractStencils = true;
  return parseMain();
}

bool libvisio::VDXParser::processXmlDocument(WPXInputStream *input)
{
  if (!input)
    return false;

  xmlTextReaderPtr reader = xmlReaderForStream(input, 0, 0, XML_PARSE_NOENT|XML_PARSE_NOBLANKS|XML_PARSE_NONET);
  if (!reader)
    return false;
  int ret = xmlTextReaderRead(reader);
  while (ret == 1)
  {
    processXmlNode(reader);

    ret = xmlTextReaderRead(reader);
  }
  xmlFreeTextReader(reader);

  if (ret)
  {
    VSD_DEBUG_MSG(("Failed to parse DiagramML document\n"));
    return false;
  }

  return true;
}

void libvisio::VDXParser::processXmlNode(xmlTextReaderPtr reader)
{
  if (!reader)
    return;
  int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  int tokenType = xmlTextReaderNodeType(reader);
  switch (tokenId)
  {
  case XML_COLORS:
    if (tokenType == 1)
      readColours(reader);
    break;
  case XML_FACENAMES:
    if (tokenType == 1)
      readFonts(reader);
    break;
  case XML_STYLESHEETS:
    if (tokenType == 1)
      m_isInStyles = true;
    else if (tokenType == 15)
      m_isInStyles = false;
    break;
  case XML_STYLESHEET:
    if (tokenType == 1)
      readStyleSheet(reader);
    break;
  case XML_LINE:
    if (tokenType == 1)
      readLine(reader);
    break;
  case XML_FILL:
    if (tokenType == 1)
      readFillAndShadow(reader);
    break;
  case XML_PAGES:
    if (tokenType == 1 && m_extractStencils)
    {
      int ret = 1;
      do
      {
        ret = xmlTextReaderRead(reader);
        tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
        tokenType = xmlTextReaderNodeType(reader);
      }
      while ((XML_PAGES != tokenId || 15 != tokenType) && ret == 1);
    }
    break;
  case XML_PAGE:
    readPage(reader);
    break;
  case XML_MASTERS:
    if (tokenType == 1 && !m_extractStencils)
      m_isStencilStarted = true;
    else
      m_isStencilStarted = false;
    break;
  case XML_MASTER:
    if (m_extractStencils)
      readPage(reader);
    else
      readStencil(reader);
    break;
  default:
    break;
  }

#ifdef DEBUG
  const xmlChar *name = xmlTextReaderConstName(reader);
  const xmlChar *value = xmlTextReaderConstValue(reader);
  int isEmptyElement = xmlTextReaderIsEmptyElement(reader);

  for (int i=0; i<xmlTextReaderDepth(reader); ++i)
  {
    VSD_DEBUG_MSG((" "));
  }
  VSD_DEBUG_MSG(("%i %i %s", isEmptyElement, tokenType, name ? (const char *)name : ""));
  if (xmlTextReaderNodeType(reader) == 1)
  {
    while (xmlTextReaderMoveToNextAttribute(reader))
    {
      const xmlChar *name1 = xmlTextReaderConstName(reader);
      const xmlChar *value1 = xmlTextReaderConstValue(reader);
      printf(" %s=\"%s\"", name1, value1);
    }
  }

  if (!value)
    VSD_DEBUG_MSG(("\n"));
  else
  {
    VSD_DEBUG_MSG((" %s\n", value));
  }
#endif
}

// Functions reading the DiagramML document content

void libvisio::VDXParser::readEllipticalArcTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readForeignData(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readEllipse(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readLine(xmlTextReaderPtr reader)
{
  double strokeWidth = 0;
  long colourId = 0;
  long linePattern = 0;
  long startMarker = 0;
  long endMarker = 0;
  long lineCap = 0;

  unsigned level = (unsigned)xmlTextReaderDepth(reader);
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_LINEWEIGHT:
      if (1 == tokenType)
        ret = readDoubleData(strokeWidth, reader);
      break;
    case XML_LINECOLOR:
      if (1 == tokenType)
        ret = readLongData(colourId, reader);
      break;
    case XML_LINEPATTERN:
      if (1 == tokenType)
        ret = readLongData(linePattern, reader);
      break;
    case XML_BEGINARROW:
      if (1 == tokenType)
        ret = readLongData(startMarker, reader);
      break;
    case XML_ENDARROW:
      if (1 == tokenType)
        ret = readLongData(endMarker, reader);
      break;
    case XML_LINECAP:
      if (1 == tokenType)
        ret = readLongData(lineCap, reader);
      break;
    default:
      break;
    }
  }
  while ((XML_LINE != tokenId || 15 != tokenType) && ret == 1);

  if (m_isInStyles)
    m_collector->collectLineStyle(0, level, strokeWidth, (unsigned char)colourId, (unsigned char)linePattern, (unsigned char)startMarker, (unsigned char)endMarker, (unsigned char)lineCap);
  else if (m_isStencilStarted)
  {
    if (!m_stencilShape.m_lineStyle)
      m_stencilShape.m_lineStyle = new VSDLineStyle(strokeWidth, (unsigned char)colourId, (unsigned char)linePattern, (unsigned char)startMarker, (unsigned char)endMarker, (unsigned char)lineCap);
  }
  else
    m_collector->collectLine(0, level, strokeWidth, (unsigned char)colourId, (unsigned char)linePattern, (unsigned char)startMarker, (unsigned char)endMarker, (unsigned char)lineCap);

}

void libvisio::VDXParser::readFillAndShadow(xmlTextReaderPtr reader)
{
  long fillColourIndexFG = 0;
  long fillFGTransparency = 0;
  long fillColourIndexBG = 0;
  long fillBGTransparency = 0;
  long fillPattern = 0;
  long shadowColourIndexFG = 0;
  long shadowColourIndexBG = 0;
  long shadowPattern = 0;
  double shadowOffsetX = 0;
  double shadowOffsetY = 0;

  unsigned level = (unsigned)xmlTextReaderDepth(reader);
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_FILLFOREGND:
      if (1 == tokenType)
        ret = readLongData(fillColourIndexFG, reader);
      break;
    case XML_FILLBKGND:
      if (1 == tokenType)
        ret = readLongData(fillColourIndexBG, reader);
      break;
    case XML_FILLPATTERN:
      if (1 == tokenType)
        ret = readLongData(fillPattern, reader);
      break;
    case XML_SHDWFOREGND:
      if (1 == tokenType)
        ret = readLongData(shadowColourIndexFG, reader);
      break;
    case XML_SHDWBKGND:
      if (1 == tokenType)
        ret = readLongData(shadowColourIndexBG, reader);
      break;
    case XML_SHDWPATTERN:
      if (1 == tokenType)
        ret = readLongData(shadowPattern, reader);
      break;
    case XML_FILLFOREGNDTRANS:
      if (1 == tokenType)
        ret = readLongData(fillFGTransparency, reader);
      break;
    case XML_FILLBKGNDTRANS:
      if (1 == tokenType)
        ret = readLongData(fillBGTransparency, reader);
      break;
    case XML_SHAPESHDWOFFSETX:
      if (1 == tokenType)
        ret = readDoubleData(shadowOffsetX, reader);
      break;
    case XML_SHAPESHDWOFFSETY:
      if (1 == tokenType)
        ret = readDoubleData(shadowOffsetY, reader);
      break;
    case XML_SHDWFOREGNDTRANS:
    case XML_SHDWBKGNDTRANS:
    case XML_SHAPESHDWTYPE:
    case XML_SHAPESHDWOBLIQUEANGLE:
	case XML_SHAPESHDWSCALEFACTOR:
    default:
      break;
    }
  }
  while ((XML_FILL != tokenId || 15 != tokenType) && ret == 1);

  if (m_isInStyles)
    m_collector->collectFillStyle(0, level, (unsigned char)fillColourIndexFG, (unsigned char)fillColourIndexBG, (unsigned char)fillPattern,
                                  (unsigned char)fillFGTransparency, (unsigned char)fillBGTransparency, (unsigned char)shadowPattern,
                                  (unsigned char)shadowColourIndexFG, (unsigned char)shadowColourIndexBG, shadowOffsetX, shadowOffsetY);
  else if (m_isStencilStarted)
  {
    VSD_DEBUG_MSG(("Found stencil fill\n"));
    if (!m_stencilShape.m_fillStyle)
      m_stencilShape.m_fillStyle = new VSDFillStyle((unsigned char)fillColourIndexFG, (unsigned char)fillColourIndexBG, (unsigned char)fillPattern,
          (unsigned char)fillFGTransparency, (unsigned char)fillBGTransparency, (unsigned char)shadowPattern,
          (unsigned char)shadowColourIndexFG, (unsigned char)shadowColourIndexBG, shadowOffsetX, shadowOffsetY);
  }
  else
    m_collector->collectFillAndShadow(0, level, (unsigned char)fillColourIndexFG, (unsigned char)fillColourIndexBG, (unsigned char)fillPattern,
                                      (unsigned char)fillFGTransparency, (unsigned char)fillBGTransparency, (unsigned char)shadowPattern,
                                      (unsigned char)shadowColourIndexFG, (unsigned char)shadowColourIndexBG, shadowOffsetX, shadowOffsetY);

}

void libvisio::VDXParser::readGeomList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readGeometry(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readMoveTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readLineTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readArcTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readNURBSTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readPolylineTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readInfiniteLine(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readShapeData(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readXFormData(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readTxtXForm(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readShapeId(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readShapeList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readForeignDataType(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readPageProps(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readShape(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readColours(xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  std::map<unsigned, Colour> colours;
  while (ret == 1 && ((XML_COLORS != tokenId || 15 != xmlTextReaderNodeType(reader))))
  {
    if (XML_COLORENTRY == tokenId)
    {
      xmlChar *ix = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
      xmlChar *rgb = xmlTextReaderGetAttribute(reader, BAD_CAST("RGB"));
      if (ix && rgb)
      {
        unsigned idx = (unsigned)xmlStringToInt(ix);
        Colour rgbColour = xmlStringToColour(rgb);
        if (idx)
          colours[idx] = rgbColour;
      }
      xmlFree(rgb);
      xmlFree(ix);

    }
    ret = xmlTextReaderRead(reader);
    tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  }

}

void libvisio::VDXParser::readFonts(xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  while (ret == 1 && ((XML_FACENAMES != tokenId || 15 != xmlTextReaderNodeType(reader))))
  {
    if (XML_FACENAME == tokenId)
    {
      xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
      xmlChar *name = xmlTextReaderGetAttribute(reader, BAD_CAST("Name"));
      if (id && name)
      {
        unsigned idx = (unsigned)xmlStringToInt(id);
        WPXBinaryData textStream(name, xmlStrlen(name));
        m_collector->collectFont(idx, textStream, libvisio::VSD_TEXT_UTF8);
      }
      xmlFree(name);
      xmlFree(id);

    }
    ret = xmlTextReaderRead(reader);
    tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  }
}

void libvisio::VDXParser::readCharList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readParaList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readPage(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readText(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readCharIX(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readParaIX(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readTextBlock(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readNameList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readName(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readFieldList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readTextField(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readStyleSheet(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  xmlChar *lineStyle = xmlTextReaderGetAttribute(reader, BAD_CAST("LineStyle"));
  xmlChar *fillStyle = xmlTextReaderGetAttribute(reader, BAD_CAST("FillStyle"));
  xmlChar *textStyle = xmlTextReaderGetAttribute(reader, BAD_CAST("TextStyle"));
  if (id)
  {
    unsigned nId = (unsigned)xmlStringToInt(id);
    unsigned nLineStyle =  (unsigned)(lineStyle ? xmlStringToInt(lineStyle) : -1);
    unsigned nFillStyle = (unsigned)(fillStyle ? xmlStringToInt(fillStyle) : -1);
    unsigned nTextStyle = (unsigned)(textStyle ? xmlStringToInt(textStyle) : -1);
    m_collector->collectStyleSheet(nId, (unsigned)xmlTextReaderDepth(reader), nLineStyle, nFillStyle, nTextStyle);
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

void libvisio::VDXParser::readPageSheet(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readSplineStart(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readSplineKnot(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readStencil(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readStencilShape(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readOLEList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readOLEData(xmlTextReaderPtr /* reader */)
{
}

int libvisio::VDXParser::readLongData(long &value, xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  if (3 == xmlTextReaderNodeType(reader))
  {
    const xmlChar *stringValue = xmlTextReaderConstValue(reader);
    if (stringValue)
      value = xmlStringToInt(stringValue);
    ret = xmlTextReaderRead(reader);
  }
  return ret;
}

int libvisio::VDXParser::readDoubleData(double &value, xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  if (3 == xmlTextReaderNodeType(reader))
  {
    const xmlChar *stringValue = xmlTextReaderConstValue(reader);
    if (stringValue)
      value = (unsigned char)xmlStringToDouble(stringValue);
    ret = xmlTextReaderRead(reader);
  }
  return ret;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
