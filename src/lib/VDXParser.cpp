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
  : VSDXMLParserBase(), m_input(input), m_painter(painter)
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
  while (1 == ret)
  {
    processXmlNode(reader);

    ret = xmlTextReaderRead(reader);
  }
  xmlFreeTextReader(reader);

  return true;
}

void libvisio::VDXParser::processXmlNode(xmlTextReaderPtr reader)
{
  if (!reader)
    return;
  int tokenId = getElementToken(reader);
  int tokenType = xmlTextReaderNodeType(reader);
  _handleLevelChange((unsigned)getElementDepth(reader));
  switch (tokenId)
  {
  case XML_COLORS:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readColours(reader);
    break;
  case XML_FACENAMES:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readFonts(reader);
    break;
  case XML_FILL:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readFillAndShadow(reader);
    break;
  case XML_LINE:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readLine(reader);
    break;
  case XML_MASTER:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      handleMasterStart(reader);
    else if (tokenType == XML_READER_TYPE_END_ELEMENT)
      handleMasterEnd(reader);
    break;
  case XML_MASTERS:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      handleMastersStart(reader);
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
      handleMastersEnd(reader);
    break;
  case XML_PAGE:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      handlePageStart(reader);
    else if (tokenType == XML_READER_TYPE_END_ELEMENT)
      handlePageEnd(reader);
    break;
  case XML_PAGEPROPS:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readPageProps(reader);
    break;
  case XML_PAGES:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      handlePagesStart(reader);
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
      handlePagesEnd(reader);
    break;
  case XML_PAGESHEET:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readPageSheet(reader);
    break;
  case XML_SHAPE:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readShape(reader);
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
    {
      if (m_isStencilStarted)
        m_currentStencil->addStencilShape(m_shape.m_shapeId, m_shape);
      else
        _flushShape();
      m_shape.clear();
      if (m_shapeStack.empty())
        m_isShapeStarted = false;
    }
    break;
  case XML_SHAPES:
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      if (m_isShapeStarted)
      {
        m_shapeStack.push(m_shape);
        m_shapeLevelStack.push(m_currentShapeLevel);
        m_currentShapeLevel = 0;
      }
    }
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
    {
      if (!m_shapeStack.empty() && !m_shapeLevelStack.empty())
      {
        m_shape = m_shapeStack.top();
        m_shapeStack.pop();
        m_currentShapeLevel = m_shapeLevelStack.top();
        m_shapeLevelStack.pop();
      }
      else
      {
        m_isShapeStarted = false;
        while (!m_shapeLevelStack.empty())
          m_shapeLevelStack.pop();
        while (!m_shapeStack.empty())
          m_shapeStack.pop();
      }
    }
    break;
  case XML_SOLUTIONXML:
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      do
      {
        xmlTextReaderRead(reader);
        // SolutionXML inside VDX file can have invalid namespace URIs
        xmlResetLastError();
        tokenId = getElementToken(reader);
        tokenType = xmlTextReaderNodeType(reader);
      }
      while (XML_SOLUTIONXML != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType);
    }
    break;
  case XML_STYLESHEET:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readStyleSheet(reader);
    break;
  case XML_STYLESHEETS:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      m_isInStyles = true;
    else if (tokenType == XML_READER_TYPE_END_ELEMENT)
    {
      _handleLevelChange(0);
      m_isInStyles = false;
    }
    break;
  case XML_FOREIGN:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readForeignInfo(reader);
    break;
  case XML_FOREIGNDATA:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readForeignData(reader);
    break;
  case XML_XFORM:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readXFormData(reader);
    break;
  case XML_GEOM:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readGeometry(reader);
    break;
  default:
    break;
  }

#ifdef DEBUG
  const xmlChar *name = xmlTextReaderConstName(reader);
  const xmlChar *value = xmlTextReaderConstValue(reader);
  int isEmptyElement = xmlTextReaderIsEmptyElement(reader);

  for (int i=0; i<getElementDepth(reader); ++i)
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

void libvisio::VDXParser::readLine(xmlTextReaderPtr reader)
{
  boost::optional<double> strokeWidth;
  boost::optional<Colour> colour;
  boost::optional<unsigned char> linePattern;
  boost::optional<unsigned char> startMarker;
  boost::optional<unsigned char> endMarker;
  boost::optional<unsigned char> lineCap;

  unsigned level = (unsigned)getElementDepth(reader);
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readLine: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_LINEWEIGHT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(strokeWidth, reader);
      break;
    case XML_LINECOLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readExtendedColourData(colour, reader);
      break;
    case XML_LINEPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(linePattern, reader);
      break;
    case XML_BEGINARROW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(startMarker, reader);
      break;
    case XML_ENDARROW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(endMarker, reader);
      break;
    case XML_LINECAP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(lineCap, reader);
      break;
    default:
      break;
    }
  }
  while ((XML_LINE != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);

  if (m_isInStyles)
    m_collector->collectLineStyle(level, strokeWidth, colour, linePattern, startMarker, endMarker, lineCap);
  else
    m_shape.m_lineStyle.override(VSDOptionalLineStyle(strokeWidth, colour, linePattern, startMarker, endMarker, lineCap));
}

void libvisio::VDXParser::readFillAndShadow(xmlTextReaderPtr reader)
{
  boost::optional<Colour> fillColourFG;
  boost::optional<double> fillFGTransparency;
  boost::optional<Colour> fillColourBG;
  boost::optional<double> fillBGTransparency;
  boost::optional<unsigned char> fillPattern;
  boost::optional<Colour> shadowColourFG;
  boost::optional<Colour> shadowColourBG;
  boost::optional<unsigned char> shadowPattern;
  boost::optional<double> shadowOffsetX;
  boost::optional<double> shadowOffsetY;

  unsigned level = (unsigned)getElementDepth(reader);
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readFillAndShadow: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_FILLFOREGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(fillColourFG, reader);
      break;
    case XML_FILLBKGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(fillColourBG, reader);
      break;
    case XML_FILLPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(fillPattern, reader);
      break;
    case XML_SHDWFOREGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(shadowColourFG, reader);
      break;
    case XML_SHDWBKGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(shadowColourBG, reader);
      break;
    case XML_SHDWPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(shadowPattern, reader);
      break;
    case XML_FILLFOREGNDTRANS:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(fillFGTransparency, reader);
      break;
    case XML_FILLBKGNDTRANS:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(fillBGTransparency, reader);
      break;
    case XML_SHAPESHDWOFFSETX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(shadowOffsetX, reader);
      break;
    case XML_SHAPESHDWOFFSETY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        double tmpDoubleData = 0.0;
        ret = readDoubleData(tmpDoubleData, reader);
        shadowOffsetY = - tmpDoubleData;
      }
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
  while ((XML_FILL != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);

  if (m_isInStyles)
    m_collector->collectFillStyle(level, fillColourFG, fillColourBG, fillPattern, fillFGTransparency,
                                  fillBGTransparency, shadowPattern, shadowColourFG, shadowOffsetX, shadowOffsetY);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil fill\n"));
    }
    m_shape.m_fillStyle.override(VSDOptionalFillStyle(fillColourFG, fillColourBG, fillPattern, fillFGTransparency, fillBGTransparency,
                                 shadowColourFG, shadowPattern, shadowOffsetX, shadowOffsetY));
  }
}

void libvisio::VDXParser::readXFormData(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readXFormData: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_PINX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_xform.pinX, reader);
      break;
    case XML_PINY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_xform.pinY, reader);
      break;
    case XML_WIDTH:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_xform.width, reader);
      break;
    case XML_HEIGHT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_xform.height, reader);
      break;
    case XML_LOCPINX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_xform.pinLocX, reader);
      break;
    case XML_LOCPINY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_xform.pinLocY, reader);
      break;
    case XML_ANGLE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_xform.angle, reader);
      break;
    case XML_FLIPX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readBoolData(m_shape.m_xform.flipX, reader);
      break;
    case XML_FLIPY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readBoolData(m_shape.m_xform.flipY, reader);
      break;
    case XML_RESIZEMODE:
    default:
      break;
    }
  }
  while ((XML_XFORM != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VDXParser::readTxtXForm(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readPageProps(xmlTextReaderPtr reader)
{
  double pageWidth = 0.0;
  double pageHeight = 0.0;
  double shadowOffsetX = 0.0;
  double shadowOffsetY = 0.0;
  double pageScale = 1.0;
  double drawingScale = 1.0;

  unsigned level = (unsigned)getElementDepth(reader);
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readPageProps: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_PAGEWIDTH:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret =readDoubleData(pageWidth, reader);
      break;
    case XML_PAGEHEIGHT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(pageHeight, reader);
      break;
    case XML_SHDWOFFSETX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(shadowOffsetX, reader);
      break;
    case XML_SHDWOFFSETY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(shadowOffsetY, reader);
      shadowOffsetY *= -1.0;
      break;
    case XML_PAGESCALE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(pageScale, reader);
      break;
    case XML_DRAWINGSCALE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(drawingScale, reader);
      break;
    case XML_DRAWINGSIZETYPE:
    case XML_DRAWINGSCALETYPE:
    case XML_INHIBITSNAP:
    case XML_UIVISIBILITY:
    case XML_SHDWTYPE:
    case XML_SHDWOBLIQUEANGLE:
    case XML_SHDWSCALEFACTOR:
    default:
      break;
    }
  }
  while ((XML_PAGEPROPS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);

  if (m_isStencilStarted)
  {
    m_currentStencil->m_shadowOffsetX = shadowOffsetX;
    m_currentStencil->m_shadowOffsetY = shadowOffsetY;
  }
  else if (m_isPageStarted)
  {
    double scale = drawingScale > 0 || drawingScale < 0 ? pageScale/drawingScale : 1.0;
    m_collector->collectPageProps(0, level, pageWidth, pageHeight, shadowOffsetX, shadowOffsetY, scale);
  }
}

void libvisio::VDXParser::readFonts(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readFonts: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    if (XML_FACENAME == tokenId)
    {
      xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
      xmlChar *name = xmlTextReaderGetAttribute(reader, BAD_CAST("Name"));
      if (id && name)
      {
        unsigned idx = (unsigned)xmlStringToLong(id);
        WPXBinaryData textStream(name, xmlStrlen(name));
        m_collector->collectFont(idx, textStream, libvisio::VSD_TEXT_UTF8);
      }
      xmlFree(name);
      xmlFree(id);
    }
  }
  while ((XML_FACENAMES != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VDXParser::readTextBlock(xmlTextReaderPtr reader)
{
  double leftMargin = 0.0;
  double rightMargin = 0.0;
  double topMargin = 0.0;
  double bottomMargin = 0.0;
  long verticalAlign = 0;
  long bgClrId = 0;
  Colour bgColour;
  double defaultTabStop = 0.0;
  long textDirection = 0.0;

  unsigned level = (unsigned)getElementDepth(reader);
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_LEFTMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(leftMargin, reader);
      break;
    case XML_RIGHTMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(rightMargin, reader);
      break;
    case XML_TOPMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(topMargin, reader);
      break;
    case XML_BOTTOMMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(bottomMargin, reader);
      break;
    case XML_VERTICALALIGN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(verticalAlign, reader);
      break;
    case XML_TEXTBKGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(bgColour, bgClrId, reader);
      break;
    case XML_DEFAULTTABSTOP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(defaultTabStop, reader);
      break;
    case XML_TEXTDIRECTION:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(textDirection, reader);
      break;
    case XML_TEXTBKGNDTRANS:
    default:
      break;
    }
  }
  while ((XML_TEXTBLOCK != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);

  if (bgClrId < 0)
    bgClrId = 0;
  if (bgClrId)
  {
    std::map<unsigned, Colour>::const_iterator iter = m_colours.find(bgClrId-1);
    if (iter != m_colours.end())
      bgColour = iter->second;
    else
      bgColour = Colour();
  }

  if (m_isInStyles)
    m_collector->collectTextBlockStyle(level, leftMargin, rightMargin, topMargin, bottomMargin,
                                       (unsigned char)verticalAlign, !!bgClrId, bgColour, defaultTabStop, (unsigned char)textDirection);
  else
    m_shape.m_textBlockStyle.override(VSDOptionalTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin,
                                      (unsigned char)verticalAlign, !!bgClrId, bgColour, defaultTabStop, (unsigned char)textDirection));
}

int libvisio::VDXParser::readLongData(long &value, xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  if (XML_READER_TYPE_TEXT == xmlTextReaderNodeType(reader))
  {
    const xmlChar *stringValue = xmlTextReaderConstValue(reader);
    if (stringValue)
    {
      VSD_DEBUG_MSG(("VDXParser::readLongData stringValue %s\n", (const char *)stringValue));
      value = xmlStringToLong(stringValue);
    }
    ret = xmlTextReaderRead(reader);
  }
  return ret;
}

int libvisio::VDXParser::readDoubleData(double &value, xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  if (XML_READER_TYPE_TEXT == xmlTextReaderNodeType(reader))
  {
    const xmlChar *stringValue = xmlTextReaderConstValue(reader);
    if (stringValue)
    {
      VSD_DEBUG_MSG(("VDXParser::readDoubleData stringValue %s\n", (const char *)stringValue));
      value = xmlStringToDouble(stringValue);
    }
    ret = xmlTextReaderRead(reader);
  }
  return ret;
}

int libvisio::VDXParser::readBoolData(bool &value, xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  if (XML_READER_TYPE_TEXT == xmlTextReaderNodeType(reader))
  {
    const xmlChar *stringValue = xmlTextReaderConstValue(reader);
    if (stringValue)
    {
      VSD_DEBUG_MSG(("VDXParser::readBoolData stringValue %s\n", (const char *)stringValue));
      value = xmlStringToBool(stringValue);
    }
    ret = xmlTextReaderRead(reader);
  }
  return ret;
}

int libvisio::VDXParser::readColourData(Colour &value, xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  if (XML_READER_TYPE_TEXT == xmlTextReaderNodeType(reader))
  {
    const xmlChar *stringValue = xmlTextReaderConstValue(reader);
    if (stringValue)
    {
      VSD_DEBUG_MSG(("VDXParser::readColourData stringValue %s\n", (const char *)stringValue));
      Colour tmpValue = xmlStringToColour(stringValue);
      value = tmpValue;
    }
    ret = xmlTextReaderRead(reader);
  }
  return ret;
}

int libvisio::VDXParser::readExtendedColourData(Colour &value, long &idx, xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  if (XML_READER_TYPE_TEXT == xmlTextReaderNodeType(reader))
  {
    const xmlChar *stringValue = xmlTextReaderConstValue(reader);
    if (stringValue)
    {
      VSD_DEBUG_MSG(("VSDXParser::readExtendedColourData stringValue %s\n", (const char *)stringValue));
      try
      {
        Colour tmpColour = xmlStringToColour(stringValue);
        value = tmpColour;
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
      ret = xmlTextReaderRead(reader);
    }
  }
  return ret;
}

int libvisio::VDXParser::getElementToken(xmlTextReaderPtr reader)
{
  return VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
}

int libvisio::VDXParser::getElementDepth(xmlTextReaderPtr reader)
{
  return xmlTextReaderDepth(reader);
}

void libvisio::VDXParser::getBinaryData(xmlTextReaderPtr reader)
{
  xmlTextReaderRead(reader);
  if (XML_READER_TYPE_TEXT == xmlTextReaderNodeType(reader))
  {
    const xmlChar *data = xmlTextReaderConstValue(reader);
    if (data)
    {
      if (!m_shape.m_foreign)
        m_shape.m_foreign = new ForeignData();
      m_shape.m_foreign->data.clear();
      appendFromBase64(m_shape.m_foreign->data, data, xmlStrlen(data));
    }
  }
}

void libvisio::VDXParser::readForeignInfo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readForeignInfo: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_IMGOFFSETX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_foreign)
          m_shape.m_foreign = new ForeignData();
        ret = readDoubleData(m_shape.m_foreign->offsetX, reader);
      }
      break;
    case XML_IMGOFFSETY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_foreign)
          m_shape.m_foreign = new ForeignData();
        ret = readDoubleData(m_shape.m_foreign->offsetY, reader);
      }
      break;
    case XML_IMGWIDTH:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_foreign)
          m_shape.m_foreign = new ForeignData();
        ret = readDoubleData(m_shape.m_foreign->width, reader);
      }
      break;
    case XML_IMGHEIGHT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_foreign)
          m_shape.m_foreign = new ForeignData();
        ret = readDoubleData(m_shape.m_foreign->height, reader);
      }
      break;
    default:
      break;
    }
  }
  while ((XML_FOREIGN != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}



/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
