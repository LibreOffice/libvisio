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
  : m_input(input), m_painter(painter), m_collector(), m_stencils(), m_currentStencil(0), m_stencilShape(),
    m_isStencilStarted(false), m_currentStencilID((unsigned)-1), m_extractStencils(false), m_isInStyles(false),
    m_colours(), m_charList(new VSDCharacterList()), m_charListVector(), m_currentLevel(0), m_currentShapeLevel(0),
    m_fieldList(), m_geomList(new VSDGeometryList()), m_geomListVector(), m_paraList(new VSDParagraphList()),
    m_paraListVector(), m_shapeList()
{
}

libvisio::VDXParser::~VDXParser()
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
  _handleLevelChange((unsigned)xmlTextReaderDepth(reader));
  switch (tokenId)
  {
  case XML_COLORS:
    if (1 == tokenType)
      readColours(reader);
    break;
  case XML_FACENAMES:
    if (1 == tokenType)
      readFonts(reader);
    break;
  case XML_FILL:
    if (1 == tokenType)
      readFillAndShadow(reader);
    break;
  case XML_LINE:
    if (1 == tokenType)
      readLine(reader);
    break;
  case XML_MASTER:
    if (1 == tokenType)
    {
      if (m_extractStencils)
        readPage(reader);
      else
        readStencil(reader);
    }
    else if (tokenType == 15)
    {
      if (m_extractStencils)
      {
        _handleLevelChange(0);
        m_collector->endPage();
      }
      else
      {
        if (m_currentStencil)
        {
          m_stencils.addStencil(m_currentStencilID, *m_currentStencil);
          delete m_currentStencil;
        }
        m_currentStencil = 0;
        m_currentStencilID = (unsigned)-1;
      }
    }
    break;
  case XML_MASTERS:
    if (1 == tokenType)
    {
      if (m_extractStencils)
        m_isStencilStarted = false;
      else
        m_isStencilStarted = true;
    }
    else if (15 == tokenType)
    {
      if (m_extractStencils)
        m_collector->endPages();
      else
        m_isStencilStarted = false;
    }
    break;
  case XML_PAGE:
    if (1 == tokenType)
    {
      if (m_extractStencils)
      {
        // This skips page, because we are spitting out the stencils only
        int ret = 1;
        do
        {
          ret = xmlTextReaderRead(reader);
          tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
          tokenType = xmlTextReaderNodeType(reader);
        }
        while ((XML_PAGE != tokenId || 15 != tokenType) && ret == 1);
      }
      else
        readPage(reader);
    }
    else if (tokenType == 15 && !m_extractStencils)
    {
      _handleLevelChange(0);
      m_collector->endPage();
    }
    break;
  case XML_PAGEPROPS:
    if (1 == tokenType)
      readPageProps(reader);
    break;
  case XML_PAGES:
    if (1 == tokenType)
      m_isStencilStarted = false;
    else if (15 == tokenType && !m_extractStencils)
      m_collector->endPages();
    break;
  case XML_PAGESHEET:
    if (1 == tokenType)
      readPageSheet(reader);
    break;
  case XML_STYLESHEET:
    if (1 == tokenType)
      readStyleSheet(reader);
    break;
  case XML_STYLESHEETS:
    if (1 == tokenType)
      m_isInStyles = true;
    else if (tokenType == 15)
    {
      _handleLevelChange(0);
      m_isInStyles = false;
    }
    break;
  case XML_XFORM:
    if (1 == tokenType)
      readXFormData(reader);
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
  Colour colour;
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
        readExtendedColourData(colour, reader);
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
    m_collector->collectLineStyle(0, level, strokeWidth, colour, (unsigned char)linePattern, (unsigned char)startMarker, (unsigned char)endMarker, (unsigned char)lineCap);
  else if (m_isStencilStarted)
  {
    if (!m_stencilShape.m_lineStyle)
      m_stencilShape.m_lineStyle = new VSDLineStyle(strokeWidth, colour, (unsigned char)linePattern, (unsigned char)startMarker, (unsigned char)endMarker, (unsigned char)lineCap);
  }
  else
    m_collector->collectLine(0, level, strokeWidth, colour, (unsigned char)linePattern, (unsigned char)startMarker, (unsigned char)endMarker, (unsigned char)lineCap);
}

void libvisio::VDXParser::readFillAndShadow(xmlTextReaderPtr reader)
{
  Colour fillColourFG;
  double fillFGTransparency = 0.0;
  Colour fillColourBG;
  double fillBGTransparency = 0.0;
  long fillPattern = 0;
  Colour shadowColourFG;
  Colour shadowColourBG;
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
        ret = readExtendedColourData(fillColourFG, reader);
      break;
    case XML_FILLBKGND:
      if (1 == tokenType)
        ret = readExtendedColourData(fillColourBG, reader);
      break;
    case XML_FILLPATTERN:
      if (1 == tokenType)
        ret = readLongData(fillPattern, reader);
      break;
    case XML_SHDWFOREGND:
      if (1 == tokenType)
        ret = readExtendedColourData(shadowColourFG, reader);
      break;
    case XML_SHDWBKGND:
      if (1 == tokenType)
        ret = readExtendedColourData(shadowColourBG, reader);
      break;
    case XML_SHDWPATTERN:
      if (1 == tokenType)
        ret = readLongData(shadowPattern, reader);
      break;
    case XML_FILLFOREGNDTRANS:
      if (1 == tokenType)
        ret = readDoubleData(fillFGTransparency, reader);
      break;
    case XML_FILLBKGNDTRANS:
      if (1 == tokenType)
        ret = readDoubleData(fillBGTransparency, reader);
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
    m_collector->collectFillStyle(0, level, fillColourFG, fillColourBG, (unsigned char)fillPattern, fillFGTransparency,
                                  fillBGTransparency, (unsigned char)shadowPattern, shadowColourFG, shadowOffsetX, shadowOffsetY);
  else if (m_isStencilStarted)
  {
    VSD_DEBUG_MSG(("Found stencil fill\n"));
    if (!m_stencilShape.m_fillStyle)
      m_stencilShape.m_fillStyle = new VSDFillStyle(fillColourFG, fillColourBG, (unsigned char)fillPattern,
          fillFGTransparency, fillBGTransparency, shadowColourFG, (unsigned char)shadowPattern,
          shadowOffsetX, shadowOffsetY);
  }
  else
    m_collector->collectFillAndShadow(0, level, fillColourFG, fillColourBG, (unsigned char)fillPattern, fillFGTransparency, fillBGTransparency,
                                      (unsigned char)shadowPattern, shadowColourFG, shadowOffsetX, shadowOffsetY);
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

void libvisio::VDXParser::readXFormData(xmlTextReaderPtr reader)
{
  XForm xform;

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
    case XML_PINX:
      if (1 == tokenType)
        ret = readDoubleData(xform.pinX, reader);
      break;
    case XML_PINY:
      if (1 == tokenType)
        ret = readDoubleData(xform.pinY, reader);
      break;
    case XML_WIDTH:
      if (1 == tokenType)
        ret = readDoubleData(xform.height, reader);
      break;
    case XML_HEIGHT:
      if (1 == tokenType)
        ret = readDoubleData(xform.width, reader);
      break;
    case XML_LOCPINX:
      if (1 == tokenType)
        ret = readDoubleData(xform.pinLocX, reader);
      break;
    case XML_LOCPINY:
      if (1 == tokenType)
        ret = readDoubleData(xform.pinLocY, reader);
      break;
    case XML_ANGLE:
      if (1 == tokenType)
        ret = readDoubleData(xform.angle, reader);
      break;
    case XML_FLIPX:
      if (1 == tokenType)
        ret = readBoolData(xform.flipX, reader);
      break;
    case XML_FLIPY:
      if (1 == tokenType)
        ret = readBoolData(xform.flipY, reader);
      break;
    case XML_RESIZEMODE:
    default:
      break;
    }
  }
  while ((XML_XFORM != tokenId || 15 != tokenType) && ret == 1);

  m_collector->collectXFormData(0, level, xform);
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

void libvisio::VDXParser::readPageProps(xmlTextReaderPtr reader)
{
  double pageWidth = 0.0;
  double pageHeight = 0.0;
  double shadowOffsetX = 0.0;
  double shadowOffsetY = 0.0;
  double pageScale = 1.0;
  double drawingScale = 1.0;

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
    case XML_PAGEWIDTH:
      if (1 == tokenType)
        ret =readDoubleData(pageWidth, reader);
      break;
    case XML_PAGEHEIGHT:
      if (1 == tokenType)
        ret = readDoubleData(pageHeight, reader);
      break;
    case XML_SHDWOFFSETX:
      if (1 == tokenType)
        ret = readDoubleData(shadowOffsetX, reader);
      break;
    case XML_SHDWOFFSETY:
      if (1 == tokenType)
        ret = readDoubleData(shadowOffsetY, reader);
      break;
    case XML_PAGESCALE:
      if (1 == tokenType)
        ret = readDoubleData(pageScale, reader);
      break;
    case XML_DRAWINGSCALE:
      if (1 == tokenType)
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
  while ((XML_PAGEPROPS != tokenId || 15 != tokenType) && ret == 1);

  if (m_isStencilStarted)
  {
    m_currentStencil->m_shadowOffsetX = shadowOffsetX;
    m_currentStencil->m_shadowOffsetY = shadowOffsetY;
  }
  else
  {
    m_collector->collectPageProps(0, level, pageWidth, pageHeight, shadowOffsetX, shadowOffsetY, pageScale/drawingScale);
  }
}

void libvisio::VDXParser::readShape(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readColours(xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  m_colours.clear();
  while (ret == 1 && ((XML_COLORS != tokenId || 15 != xmlTextReaderNodeType(reader))))
  {
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
        unsigned idx = (unsigned)xmlStringToLong(id);
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

void libvisio::VDXParser::readPage(xmlTextReaderPtr reader)
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
    m_collector->collectPage(nId, (unsigned)xmlTextReaderDepth(reader), backgroundPageID, isBackgroundPage);
  }
  if (id)
    xmlFree(id);
  if (bgndPage)
    xmlFree(bgndPage);
  if (background)
    xmlFree(background);
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
    case XML_LEFTMARGIN:
      if (1 == tokenType)
        ret = readDoubleData(leftMargin, reader);
      break;
    case XML_RIGHTMARGIN:
      if (1 == tokenType)
        ret = readDoubleData(rightMargin, reader);
      break;
    case XML_TOPMARGIN:
      if (1 == tokenType)
        ret = readDoubleData(topMargin, reader);
      break;
    case XML_BOTTOMMARGIN:
      if (1 == tokenType)
        ret = readDoubleData(bottomMargin, reader);
      break;
    case XML_VERTICALALIGN:
      if (1 == tokenType)
        ret = readLongData(verticalAlign, reader);
      break;
    case XML_TEXTBKGND:
      if (1 == tokenType)
        ret = readExtendedColourData(bgColour, bgClrId, reader);
      break;
    case XML_DEFAULTTABSTOP:
      if (1 == tokenType)
        ret = readDoubleData(defaultTabStop, reader);
      break;
    case XML_TEXTDIRECTION:
      if (1 == tokenType)
        ret = readLongData(textDirection, reader);
      break;
    case XML_TEXTBKGNDTRANS:
    default:
      break;
    }
  }
  while ((XML_TEXTBLOCK != tokenId || 15 != tokenType) && ret == 1);

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
    m_collector->collectTextBlockStyle(0, level, leftMargin, rightMargin, topMargin, bottomMargin,
                                       (unsigned char)verticalAlign, !!bgClrId, bgColour, defaultTabStop, (unsigned char)textDirection);
  else if (m_isStencilStarted)
  {
    if (!m_stencilShape.m_textBlockStyle)
      m_stencilShape.m_textBlockStyle = new VSDTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin,
          (unsigned char)verticalAlign, !!bgClrId, bgColour, defaultTabStop, (unsigned char)textDirection);
  }
  else
    m_collector->collectTextBlock(0, level, leftMargin, rightMargin, topMargin, bottomMargin,
                                  (unsigned char)verticalAlign, !!bgClrId, bgColour, defaultTabStop, (unsigned char)textDirection);
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
    unsigned nId = (unsigned)xmlStringToLong(id);
    unsigned nLineStyle =  (unsigned)(lineStyle ? xmlStringToLong(lineStyle) : -1);
    unsigned nFillStyle = (unsigned)(fillStyle ? xmlStringToLong(fillStyle) : -1);
    unsigned nTextStyle = (unsigned)(textStyle ? xmlStringToLong(textStyle) : -1);
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

void libvisio::VDXParser::readPageSheet(xmlTextReaderPtr reader)
{
  m_currentShapeLevel = (unsigned)xmlTextReaderDepth(reader);
  m_collector->collectPageSheet(0, m_currentShapeLevel);
}

void libvisio::VDXParser::readSplineStart(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readSplineKnot(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VDXParser::readStencil(xmlTextReaderPtr reader)
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
  if (3 == xmlTextReaderNodeType(reader))
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
  if (3 == xmlTextReaderNodeType(reader))
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
  if (3 == xmlTextReaderNodeType(reader))
  {
    const xmlChar *stringValue = xmlTextReaderConstValue(reader);
    if (stringValue)
    {
      VSD_DEBUG_MSG(("VDXParser::readBoolData stringValue %s\n", (const char *)stringValue));
      value = xmlStringToColour(stringValue);
    }
    ret = xmlTextReaderRead(reader);
  }
  return ret;
}

int libvisio::VDXParser::readExtendedColourData(Colour &value, long &idx, xmlTextReaderPtr reader)
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

int libvisio::VDXParser::readExtendedColourData(Colour &value, xmlTextReaderPtr reader)
{
  long idx = -1;
  return readExtendedColourData(value, idx, reader);
}

void libvisio::VDXParser::_handleLevelChange(unsigned level)
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
