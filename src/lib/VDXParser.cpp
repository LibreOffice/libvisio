/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VDXParser.h"

#include <memory>
#include <string.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlstring.h>
#include <librevenge-stream/librevenge-stream.h>
#include "libvisio_utils.h"
#include "libvisio_xml.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"
#include "VSDXMLHelper.h"
#include "VSDXMLTokenMap.h"


libvisio::VDXParser::VDXParser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter)
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

  try
  {
    std::vector<std::map<unsigned, XForm> > groupXFormsSequence;
    std::vector<std::map<unsigned, unsigned> > groupMembershipsSequence;
    std::vector<std::list<unsigned> > documentPageShapeOrders;

    VSDStylesCollector stylesCollector(groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders);
    m_collector = &stylesCollector;
    m_input->seek(0, librevenge::RVNG_SEEK_SET);
    if (!processXmlDocument(m_input))
      return false;

    VSDStyles styles = stylesCollector.getStyleSheets();

    VSDContentCollector contentCollector(m_painter, groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders, styles, m_stencils);
    m_collector = &contentCollector;
    m_input->seek(0, librevenge::RVNG_SEEK_SET);
    if (!processXmlDocument(m_input))
      return false;

    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool libvisio::VDXParser::extractStencils()
{
  m_extractStencils = true;
  return parseMain();
}

bool libvisio::VDXParser::processXmlDocument(librevenge::RVNGInputStream *input)
{
  if (!input)
    return false;

  auto reader = xmlReaderForStream(input);
  if (!reader)
    return false;
  int ret = xmlTextReaderRead(reader.get());
  while (1 == ret)
  {
    processXmlNode(reader.get());

    ret = xmlTextReaderRead(reader.get());
  }

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
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
      handleMasterEnd(reader);
    break;
  case XML_MASTERS:
    if (XML_READER_TYPE_ELEMENT == tokenType && !xmlTextReaderIsEmptyElement(reader))
      handleMastersStart(reader);
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
      handleMastersEnd(reader);
    break;
  case XML_PAGE:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      handlePageStart(reader);
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
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
      if (m_isStencilStarted && m_currentStencil)
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
      int ret = 0;
      do
      {
        ret = xmlTextReaderRead(reader);
#if 0
        // SolutionXML inside VDX file can have invalid namespace URIs
        xmlResetLastError();
#endif
        tokenId = getElementToken(reader);
        tokenType = xmlTextReaderNodeType(reader);
      }
      while ((XML_SOLUTIONXML != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
    }
    break;
  case XML_STYLESHEET:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readStyleSheet(reader);
    break;
  case XML_STYLESHEETS:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      m_isInStyles = true;
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
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
  case XML_MISC:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readMisc(reader);
    break;
  case XML_GEOM:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readGeometry(reader);
    break;
  case XML_LAYER:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readLayerIX(reader);
    break;
  case XML_PARA:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readParaIX(reader);
    break;
  case XML_CHAR:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readCharIX(reader);
    break;
  case XML_TEXT:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readText(reader);
    break;
  case XML_TEXTBLOCK:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readTextBlock(reader);
    break;
  case XML_TEXTXFORM:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readTxtXForm(reader);
    break;
  case XML_XFORM1D:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readXForm1D(reader);
    break;
  case XML_LAYERMEM:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readLayerMem(reader);
    break;
  case XML_TABS:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readTabs(reader);
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
  boost::optional<double> rounding;

  auto level = (unsigned)getElementDepth(reader);
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
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
        ret = readExtendedColourData(colour, reader);
      break;
    case XML_LINEPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(linePattern, reader);
      break;
    case XML_ROUNDING:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(rounding, reader);
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
  while ((XML_LINE != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

  if (m_isInStyles)
    m_collector->collectLineStyle(level, strokeWidth, colour, linePattern, startMarker, endMarker,lineCap, rounding, -1, -1);
  else
    m_shape.m_lineStyle.override(VSDOptionalLineStyle(strokeWidth, colour, linePattern, startMarker, endMarker, lineCap, rounding, -1, -1));
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

  auto level = (unsigned)getElementDepth(reader);
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
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
        ret = readDoubleData(shadowOffsetY, reader);
      break;
    default:
      break;
    }
  }
  while ((XML_FILL != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

  if (m_isInStyles)
    m_collector->collectFillStyle(level, fillColourFG, fillColourBG, fillPattern, fillFGTransparency,
                                  fillBGTransparency, shadowPattern, shadowColourFG, shadowOffsetX, shadowOffsetY, -1, -1, -1);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil fill\n"));
    }
    m_shape.m_fillStyle.override(VSDOptionalFillStyle(fillColourFG, fillColourBG, fillPattern, fillFGTransparency, fillBGTransparency,
                                                      shadowColourFG, shadowPattern, shadowOffsetX, shadowOffsetY, -1, -1, 1));
  }
}

void libvisio::VDXParser::readMisc(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readMisc: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_HIDETEXT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readBoolData(m_shape.m_misc.m_hideText, reader);
      break;
    case XML_BEGTRIGGER:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_xform1d)
          m_shape.m_xform1d = make_unique<XForm1D>();
        readTriggerId(m_shape.m_xform1d->beginId, reader);
      }
      break;
    case XML_ENDTRIGGER:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_xform1d)
          m_shape.m_xform1d = make_unique<XForm1D>();
        readTriggerId(m_shape.m_xform1d->endId, reader);
      }
      break;
    default:
      break;
    }
  }
  while ((XML_MISC != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VDXParser::readXFormData(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
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
    default:
      break;
    }
  }
  while ((XML_XFORM != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VDXParser::readLayerMem(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readLayerMem: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_LAYERMEMBER:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readStringData(m_shape.m_layerMem, reader);
      break;
    default:
      break;
    }
  }
  while ((XML_LAYERMEM != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VDXParser::readTxtXForm(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readTxtXForm: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_TXTPINX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_txtxform)
          m_shape.m_txtxform = make_unique<XForm>();
        ret = readDoubleData(m_shape.m_txtxform->pinX, reader);
      }
      break;
    case XML_TXTPINY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_txtxform)
          m_shape.m_txtxform = make_unique<XForm>();
        ret = readDoubleData(m_shape.m_txtxform->pinY, reader);
      }
      break;
    case XML_TXTWIDTH:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_txtxform)
          m_shape.m_txtxform = make_unique<XForm>();
        ret = readDoubleData(m_shape.m_txtxform->width, reader);
      }
      break;
    case XML_TXTHEIGHT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_txtxform)
          m_shape.m_txtxform = make_unique<XForm>();
        ret = readDoubleData(m_shape.m_txtxform->height, reader);
      }
      break;
    case XML_TXTLOCPINX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_txtxform)
          m_shape.m_txtxform = make_unique<XForm>();
        ret = readDoubleData(m_shape.m_txtxform->pinLocX, reader);
      }
      break;
    case XML_TXTLOCPINY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_txtxform)
          m_shape.m_txtxform = make_unique<XForm>();
        ret = readDoubleData(m_shape.m_txtxform->pinLocY, reader);
      }
      break;
    case XML_TXTANGLE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_txtxform)
          m_shape.m_txtxform = make_unique<XForm>();
        ret = readDoubleData(m_shape.m_txtxform->angle, reader);
      }
      break;
    default:
      break;
    }
  }
  while ((XML_TEXTXFORM != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VDXParser::readXForm1D(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readXForm1D: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_BEGINX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_xform1d)
          m_shape.m_xform1d = make_unique<XForm1D>();
        ret = readDoubleData(m_shape.m_xform1d->beginX, reader);
      }
      break;
    case XML_BEGINY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_xform1d)
          m_shape.m_xform1d = make_unique<XForm1D>();
        ret = readDoubleData(m_shape.m_xform1d->beginY, reader);
      }
      break;
    case XML_ENDX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_xform1d)
          m_shape.m_xform1d = make_unique<XForm1D>();
        ret = readDoubleData(m_shape.m_xform1d->endX, reader);
      }
      break;
    case XML_ENDY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_xform1d)
          m_shape.m_xform1d = make_unique<XForm1D>();
        ret = readDoubleData(m_shape.m_xform1d->endY, reader);
      }
      break;
    default:
      break;
    }
  }
  while ((XML_XFORM1D != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VDXParser::readPageProps(xmlTextReaderPtr reader)
{
  double pageWidth = 0.0;
  double pageHeight = 0.0;
  double shadowOffsetX = 0.0;
  double shadowOffsetY = 0.0;
  double pageScale = 1.0;
  double drawingScale = 1.0;

  auto level = (unsigned)getElementDepth(reader);
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
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
      break;
    case XML_PAGESCALE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(pageScale, reader);
      break;
    case XML_DRAWINGSCALE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(drawingScale, reader);
      break;
    default:
      break;
    }
  }
  while ((XML_PAGEPROPS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

  if (m_isStencilStarted && m_currentStencil)
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
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readFonts: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    if (XML_FACENAME == tokenId)
    {
      std::unique_ptr<xmlChar, decltype(xmlFree)> id(xmlTextReaderGetAttribute(reader, BAD_CAST("ID")), xmlFree);
      std::unique_ptr<xmlChar, decltype(xmlFree)> name(xmlTextReaderGetAttribute(reader, BAD_CAST("Name")), xmlFree);
      if (id && name)
      {
        auto idx = (unsigned)xmlStringToLong(id.get());
        librevenge::RVNGBinaryData textStream(name.get(), xmlStrlen(name.get()));
        m_fonts[idx] = VSDName(textStream, libvisio::VSD_TEXT_UTF8);
      }
    }
  }
  while ((XML_FACENAMES != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VDXParser::readTextBlock(xmlTextReaderPtr reader)
{
  double leftMargin = 0.0;
  double rightMargin = 0.0;
  double topMargin = 0.0;
  double bottomMargin = 0.0;
  unsigned char verticalAlign = 0;
  long bgClrId = 0;
  Colour bgColour;
  double defaultTabStop = 0.0;
  unsigned char textDirection = 0;

  auto level = (unsigned)getElementDepth(reader);
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
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
        ret = readByteData(verticalAlign, reader);
      break;
    case XML_TEXTBKGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        ret = readExtendedColourData(bgColour, bgClrId, reader);
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
      }
      break;
    case XML_DEFAULTTABSTOP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(defaultTabStop, reader);
      break;
    case XML_TEXTDIRECTION:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(textDirection, reader);
      break;
    case XML_TEXTBKGNDTRANS:
    default:
      break;
    }
  }
  while ((XML_TEXTBLOCK != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

  if (m_isInStyles)
    m_collector->collectTextBlockStyle(level, leftMargin, rightMargin, topMargin, bottomMargin,
                                       verticalAlign, !!bgClrId, bgColour, defaultTabStop, textDirection);
  else
    m_shape.m_textBlockStyle.override(VSDOptionalTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin, verticalAlign,
                                                                !!bgClrId, bgColour, defaultTabStop, textDirection));
}

xmlChar *libvisio::VDXParser::readStringData(xmlTextReaderPtr reader)
{
  int ret = xmlTextReaderRead(reader);
  if (1 == ret && XML_READER_TYPE_TEXT == xmlTextReaderNodeType(reader))
  {
    std::unique_ptr<xmlChar, void (*)(void *)> stringValue(xmlTextReaderValue(reader), xmlFree);
    ret = xmlTextReaderRead(reader);
    if (1 == ret && stringValue)
    {
      VSD_DEBUG_MSG(("VDXParser::readStringData stringValue %s\n", (const char *)stringValue.get()));
      return stringValue.release();
    }
  }
  return nullptr;
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
  const int ret = xmlTextReaderRead(reader);
  if (1 == ret && XML_READER_TYPE_TEXT == xmlTextReaderNodeType(reader))
  {
    const xmlChar *data = xmlTextReaderConstValue(reader);
    if (data)
    {
      if (!m_shape.m_foreign)
        m_shape.m_foreign = make_unique<ForeignData>();
      m_shape.m_foreign->data.clear();
      m_shape.m_foreign->data.appendBase64Data(librevenge::RVNGString((const char *)data));
    }
  }
}

void libvisio::VDXParser::readForeignInfo(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
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
          m_shape.m_foreign = make_unique<ForeignData>();
        ret = readDoubleData(m_shape.m_foreign->offsetX, reader);
      }
      break;
    case XML_IMGOFFSETY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_foreign)
          m_shape.m_foreign = make_unique<ForeignData>();
        ret = readDoubleData(m_shape.m_foreign->offsetY, reader);
      }
      break;
    case XML_IMGWIDTH:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_foreign)
          m_shape.m_foreign = make_unique<ForeignData>();
        ret = readDoubleData(m_shape.m_foreign->width, reader);
      }
      break;
    case XML_IMGHEIGHT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_foreign)
          m_shape.m_foreign = make_unique<ForeignData>();
        ret = readDoubleData(m_shape.m_foreign->height, reader);
      }
      break;
    default:
      break;
    }
  }
  while ((XML_FOREIGN != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VDXParser::readTabs(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  unsigned ix = getIX(reader);
  m_currentTabSet = &(m_shape.m_tabSets[ix].m_tabStops);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    m_currentTabSet->clear();
  }
  else
  {
    do
    {
      ret = xmlTextReaderRead(reader);
      tokenId = getElementToken(reader);
      if (XML_TOKEN_INVALID == tokenId)
      {
        VSD_DEBUG_MSG(("VDXParser::readTabs: unknown token %s\n", xmlTextReaderConstName(reader)));
      }
      tokenType = xmlTextReaderNodeType(reader);
      if (XML_TAB == tokenId && XML_READER_TYPE_ELEMENT == tokenType)
        readTab(reader);
    }
    while ((XML_TABS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  }
  m_currentTabSet = nullptr;
}

void libvisio::VDXParser::readTab(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  unsigned ix = getIX(reader);

  if (xmlTextReaderIsEmptyElement(reader))
  {
    m_currentTabSet->erase(ix);
  }
  else
  {
    do
    {
      ret = xmlTextReaderRead(reader);
      tokenId = getElementToken(reader);
      if (XML_TOKEN_INVALID == tokenId)
      {
        VSD_DEBUG_MSG(("VDXParser::readTab: unknown token %s\n", xmlTextReaderConstName(reader)));
      }
      tokenType = xmlTextReaderNodeType(reader);
      switch (tokenId)
      {
      case XML_POSITION:
        if (XML_READER_TYPE_ELEMENT == tokenType)
        {
          ret = readDoubleData((*m_currentTabSet)[ix].m_position, reader);
        }
        break;
      case XML_ALIGNMENT:
        if (XML_READER_TYPE_ELEMENT == tokenType)
        {
          ret = readByteData((*m_currentTabSet)[ix].m_alignment, reader);
        }
        break;
      case XML_LEADER:
        if (XML_READER_TYPE_ELEMENT == tokenType)
        {
          ret = readByteData((*m_currentTabSet)[ix].m_leader, reader);
        }
        break;
      default:
        break;
      }
    }
    while ((XML_TAB != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  }
}


/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
