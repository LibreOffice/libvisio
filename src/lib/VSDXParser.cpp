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
#include "VSDXParser.h"
#include "libvisio_utils.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"
#include "VSDZipStream.h"
#include "VSDXMLHelper.h"
#include "VSDXMLTokenMap.h"

namespace
{
static std::string getTargetBaseDirectory(const char *target)
{
  std::string str(target);
  std::string::size_type position = str.find_last_of('/');
  if (position == std::string::npos)
    position = 0;
  str.erase(position ? position+1 : position);
  return str;
}

std::string getRelationshipsForTarget(const char *target)
{
  std::string relStr(target ? target : "");
  std::string::size_type position = relStr.find_last_of('/');
  if (position == std::string::npos)
    position = 0;
  relStr.insert(position ? position+1 : position, "_rels/");
  relStr.append(".rels");
  return relStr;
}

} // anonymous namespace


libvisio::VSDXParser::VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : VSDXMLParserBase(), m_input(input), m_painter(painter), m_currentDepth(0), m_rels(0)
{
  input->seek(0, WPX_SEEK_CUR);
  m_input = new VSDZipStream(input);
  if (!m_input || !m_input->isOLEStream())
  {
    if (m_input)
      delete m_input;
    m_input = 0;
  }
}

libvisio::VSDXParser::~VSDXParser()
{
  if (m_input)
    delete m_input;
}

bool libvisio::VSDXParser::parseMain()
{
  if (!m_input)
    return false;

  WPXInputStream *tmpInput = 0;
  try
  {
    tmpInput = m_input->getDocumentOLEStream("_rels/.rels");
    if (!tmpInput)
      return false;

    libvisio::VSDXRelationships rootRels(tmpInput);
    delete tmpInput;

    // Check whether the relationship points to a Visio document stream
    const libvisio::VSDXRelationship *rel = rootRels.getRelationshipByType("http://schemas.microsoft.com/visio/2010/relationships/document");
    if (!rel)
      return false;

    std::vector<std::map<unsigned, XForm> > groupXFormsSequence;
    std::vector<std::map<unsigned, unsigned> > groupMembershipsSequence;
    std::vector<std::list<unsigned> > documentPageShapeOrders;

    VSDStylesCollector stylesCollector(groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders);
    m_collector = &stylesCollector;
    if (!parseDocument(m_input, rel->getTarget().c_str()))
      return false;

    VSDStyles styles = stylesCollector.getStyleSheets();

    VSDContentCollector contentCollector(m_painter, groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders, styles, m_stencils);
    m_collector = &contentCollector;
    if (!parseDocument(m_input, rel->getTarget().c_str()))
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

bool libvisio::VSDXParser::extractStencils()
{
  m_extractStencils = true;
  return parseMain();
}

bool libvisio::VSDXParser::parseDocument(WPXInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, WPX_SEEK_SET);
  if (!input->isOLEStream())
    return false;
  WPXInputStream *stream = input->getDocumentOLEStream(name);
  input->seek(0, WPX_SEEK_SET);
  if (!stream)
    return false;
  WPXInputStream *relStream = input->getDocumentOLEStream(getRelationshipsForTarget(name).c_str());
  input->seek(0, WPX_SEEK_SET);
  VSDXRelationships rels(relStream);
  if (relStream)
    delete relStream;
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  const VSDXRelationship *rel = rels.getRelationshipByType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme");
  if (rel)
  {
    if (!parseTheme(input, rel->getTarget().c_str()))
    {
      VSD_DEBUG_MSG(("Could not parse theme\n"));
    }
    input->seek(0, WPX_SEEK_SET);
  }

  processXmlDocument(stream, rels);

  rel = rels.getRelationshipByType("http://schemas.microsoft.com/visio/2010/relationships/masters");
  if (rel)
  {
    if (!parseMasters(input, rel->getTarget().c_str()))
    {
      VSD_DEBUG_MSG(("Could not parse masters\n"));
    }
    input->seek(0, WPX_SEEK_SET);
  }

  rel = rels.getRelationshipByType("http://schemas.microsoft.com/visio/2010/relationships/pages");
  if (rel)
  {
    if (!parsePages(input, rel->getTarget().c_str()))
    {
      VSD_DEBUG_MSG(("Could not parse pages\n"));
    }
    input->seek(0, WPX_SEEK_SET);
  }

  if (stream)
    delete stream;
  return true;
}

bool libvisio::VSDXParser::parseMasters(WPXInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, WPX_SEEK_SET);
  if (!input->isOLEStream())
    return false;
  WPXInputStream *stream = input->getDocumentOLEStream(name);
  if (!stream)
    return false;
  WPXInputStream *relStream = input->getDocumentOLEStream(getRelationshipsForTarget(name).c_str());
  input->seek(0, WPX_SEEK_SET);
  VSDXRelationships rels(relStream);
  if (relStream)
    delete relStream;
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  processXmlDocument(stream, rels);

  delete stream;
  return true;
}

bool libvisio::VSDXParser::parseMaster(WPXInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, WPX_SEEK_SET);
  if (!input->isOLEStream())
    return false;
  WPXInputStream *stream = input->getDocumentOLEStream(name);
  if (!stream)
    return false;
  WPXInputStream *relStream = input->getDocumentOLEStream(getRelationshipsForTarget(name).c_str());
  input->seek(0, WPX_SEEK_SET);
  VSDXRelationships rels(relStream);
  if (relStream)
    delete relStream;
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  processXmlDocument(stream, rels);

  delete stream;
  return true;
}

bool libvisio::VSDXParser::parsePages(WPXInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, WPX_SEEK_SET);
  if (!input->isOLEStream())
    return false;
  WPXInputStream *stream = input->getDocumentOLEStream(name);
  if (!stream)
    return false;
  WPXInputStream *relStream = input->getDocumentOLEStream(getRelationshipsForTarget(name).c_str());
  input->seek(0, WPX_SEEK_SET);
  VSDXRelationships rels(relStream);
  if (relStream)
    delete relStream;
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  processXmlDocument(stream, rels);

  delete stream;
  return true;
}

bool libvisio::VSDXParser::parsePage(WPXInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, WPX_SEEK_SET);
  if (!input->isOLEStream())
    return false;
  WPXInputStream *stream = input->getDocumentOLEStream(name);
  if (!stream)
    return false;
  WPXInputStream *relStream = input->getDocumentOLEStream(getRelationshipsForTarget(name).c_str());
  input->seek(0, WPX_SEEK_SET);
  VSDXRelationships rels(relStream);
  if (relStream)
    delete relStream;
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  processXmlDocument(stream, rels);

  delete stream;
  return true;
}

bool libvisio::VSDXParser::parseTheme(WPXInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, WPX_SEEK_SET);
  if (!input->isOLEStream())
    return false;
  WPXInputStream *stream = input->getDocumentOLEStream(name);
  if (!stream)
    return false;
  WPXInputStream *relStream = input->getDocumentOLEStream(getRelationshipsForTarget(name).c_str());
  input->seek(0, WPX_SEEK_SET);
  VSDXRelationships rels(relStream);
  if (relStream)
    delete relStream;
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  processXmlDocument(stream, rels);

  delete stream;
  return true;
}

void libvisio::VSDXParser::processXmlDocument(WPXInputStream *input, VSDXRelationships &rels)
{
  if (!input)
    return;

  m_rels = &rels;

  xmlTextReaderPtr reader = xmlReaderForStream(input, 0, 0, XML_PARSE_NOENT|XML_PARSE_NOBLANKS|XML_PARSE_NONET);
  if (!reader)
    return;
  int ret = xmlTextReaderRead(reader);
  while (1 == ret)
  {
    int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
    int tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_REL:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("r:id"));
        if (id)
        {
          const VSDXRelationship *rel = rels.getRelationshipById((char *)id);
          if (rel)
          {
            std::string type = rel->getType();
            if (type == "http://schemas.microsoft.com/visio/2010/relationships/master")
            {
              m_currentDepth += xmlTextReaderDepth(reader);
              parseMaster(m_input, rel->getTarget().c_str());
              m_currentDepth -= xmlTextReaderDepth(reader);
            }
            else if (type == "http://schemas.microsoft.com/visio/2010/relationships/page")
            {
              m_currentDepth += xmlTextReaderDepth(reader);
              parsePage(m_input, rel->getTarget().c_str());
              m_currentDepth -= xmlTextReaderDepth(reader);
            }
            else if (type == "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image")
            {
              extractBinaryData(m_input, rel->getTarget().c_str());
            }
            else
              processXmlNode(reader);
          }
          xmlFree(id);
        }
      }
      break;
    default:
      processXmlNode(reader);
      break;
    }
    ret = xmlTextReaderRead(reader);
  }
  xmlFreeTextReader(reader);
}

void libvisio::VSDXParser::processXmlNode(xmlTextReaderPtr reader)
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
  case XML_MASTER:
    m_isShapeStarted = false;
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      if (m_extractStencils)
        readPage(reader);
      else
        readStencil(reader);
    }
    else if (tokenType == XML_READER_TYPE_END_ELEMENT)
    {
      if (m_extractStencils)
      {
        _handleLevelChange(0);
        m_isPageStarted = false;
        m_isShapeStarted = false;
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
        m_currentStencilID = MINUS_ONE;
      }
    }
    break;
  case XML_MASTERS:
    m_isShapeStarted = false;
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      if (m_stencils.count())
      {
        // This skips page, because we are spitting out the stencils only
        int ret = 1;
        do
        {
          ret = xmlTextReaderRead(reader);
          tokenId = getElementToken(reader);
          tokenType = xmlTextReaderNodeType(reader);
        }
        while ((XML_MASTERS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
      }
      else
      {
        if (m_extractStencils)
          m_isStencilStarted = false;
        else
          m_isStencilStarted = true;
      }
    }
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
    {
      if (m_extractStencils)
        m_collector->endPages();
      else
        m_isStencilStarted = false;
    }
    break;
  case XML_PAGE:
    m_isShapeStarted = false;
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      if (m_extractStencils)
      {
        // This skips page, because we are spitting out the stencils only
        int ret = 1;
        do
        {
          ret = xmlTextReaderRead(reader);
          tokenId = getElementToken(reader);
          tokenType = xmlTextReaderNodeType(reader);
        }
        while ((XML_PAGE != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
      }
      else
        readPage(reader);
    }
    else if (tokenType == XML_READER_TYPE_END_ELEMENT && !m_extractStencils)
    {
      _handleLevelChange(0);
      m_collector->collectShapesOrder(0, 2, m_shapeList.getShapesOrder());
      m_shapeList.clear();
      m_isPageStarted = false;
      m_collector->endPage();
    }
    break;
  case XML_PAGES:
    m_isShapeStarted = false;
    if (XML_READER_TYPE_ELEMENT == tokenType)
      m_isStencilStarted = false;
    else if (XML_READER_TYPE_END_ELEMENT == tokenType && !m_extractStencils)
      m_collector->endPages();
    break;
  case XML_PAGESHEET:
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      readPageSheet(reader);
      readPageSheetProperties(reader);
    }
    break;
  case XML_STYLESHEET:
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      readStyleSheet(reader);
      readStyleProperties(reader);
    }
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
  case XML_SHAPE:
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      readShape(reader);
      if (!xmlTextReaderIsEmptyElement(reader))
        readShapeProperties(reader);
      else
      {
        if (m_isStencilStarted)
          m_currentStencil->addStencilShape(m_shape.m_shapeId, m_shape);
        else
          _flushShape();
        m_shape.clear();
        if (m_shapeStack.empty())
          m_isShapeStarted = false;
      }
    }
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
    {
      if (m_isStencilStarted)
        m_currentStencil->addStencilShape(m_shape.m_shapeId, m_shape);
      else
      {
        _flushShape();
        if (m_shapeStack.empty())
          m_isShapeStarted = false;
      }
      m_shape.clear();
    }
    break;
  case XML_SHAPES:
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      if (m_isShapeStarted)
      {
        m_shapeStack.push(m_shape);
        m_shapeLevelStack.push(m_currentShapeLevel);
        _handleLevelChange(0);
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
  default:
    break;
  }

#ifdef DEBUG
  const xmlChar *name = xmlTextReaderConstName(reader);
  const xmlChar *value = xmlTextReaderConstValue(reader);
  int type = xmlTextReaderNodeType(reader);
  int isEmptyElement = xmlTextReaderIsEmptyElement(reader);

  for (int i=0; i<getElementDepth(reader); ++i)
  {
    VSD_DEBUG_MSG((" "));
  }
  VSD_DEBUG_MSG(("%i %i %s", isEmptyElement, type, name ? (const char *)name : ""));
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

#define VSDX_DATA_READ_SIZE 4096UL

void libvisio::VSDXParser::extractBinaryData(WPXInputStream *input, const char *name)
{
  m_currentBinaryData.clear();
  if (!input || !input->isOLEStream())
    return;
  input->seek(0, WPX_SEEK_SET);
  WPXInputStream *stream = input->getDocumentOLEStream(name);
  if (!stream)
    return;
  while (true)
  {
    unsigned long numBytesRead;
    const unsigned char *buffer = stream->read(VSDX_DATA_READ_SIZE, numBytesRead);
    if (numBytesRead)
      m_currentBinaryData.append(buffer, numBytesRead);
    if (stream->atEOS())
      break;
  }
  delete stream;
  VSD_DEBUG_MSG(("%s\n", m_currentBinaryData.getBase64Data().cstr()));
}

int libvisio::VSDXParser::readLongData(long &value, xmlTextReaderPtr reader)
{
  xmlChar *stringValue = xmlTextReaderGetAttribute(reader, BAD_CAST("V"));
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXParser::readLongData stringValue %s\n", (const char *)stringValue));
    try
    {
      value = xmlStringToLong(stringValue);
    }
    catch (const XmlParserException &e)
    {
      xmlFree(stringValue);
      throw e;
    }
    xmlFree(stringValue);
    return 1;
  }
  xmlFree(stringValue);
  return -1;
}

int libvisio::VSDXParser::readDoubleData(double &value, xmlTextReaderPtr reader)
{
  xmlChar *stringValue = xmlTextReaderGetAttribute(reader, BAD_CAST("V"));
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXParser::readDoubleData stringValue %s\n", (const char *)stringValue));
    value = xmlStringToDouble(stringValue);
    xmlFree(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXParser::readBoolData(bool &value, xmlTextReaderPtr reader)
{
  xmlChar *stringValue = xmlTextReaderGetAttribute(reader, BAD_CAST("V"));
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXParser::readBoolData stringValue %s\n", (const char *)stringValue));
    value = xmlStringToBool(stringValue);
    xmlFree(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXParser::readColourData(Colour &value, xmlTextReaderPtr reader)
{
  xmlChar *stringValue = xmlTextReaderGetAttribute(reader, BAD_CAST("V"));
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXParser::readColourData stringValue %s\n", (const char *)stringValue));
    try
    {
      Colour tmpColour = xmlStringToColour(stringValue);
      value = tmpColour;
    }
    catch (const XmlParserException &)
    {
      xmlFree(stringValue);
      throw XmlParserException();
    }
    xmlFree(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXParser::readExtendedColourData(Colour &value, long &idx, xmlTextReaderPtr reader)
{
  idx = -1;
  xmlChar *stringValue = xmlTextReaderGetAttribute(reader, BAD_CAST("V"));
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
      try
      {
        idx = xmlStringToLong(stringValue);
      }
      catch (const XmlParserException &)
      {
        xmlFree(stringValue);
        return -1;
      }
    }
    xmlFree(stringValue);
    if (idx >= 0)
    {
      std::map<unsigned, Colour>::const_iterator iter = m_colours.find((unsigned)idx);
      if (iter != m_colours.end())
        value = iter->second;
      else
        idx = -1;
    }
    return 1;
  }
  return -1;
}

int libvisio::VSDXParser::getElementToken(xmlTextReaderPtr reader)
{
  int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  if (XML_READER_TYPE_END_ELEMENT == xmlTextReaderNodeType(reader))
    return tokenId;

  xmlChar *stringValue = 0;

  switch (tokenId)
  {
  case XML_CELL:
    stringValue = xmlTextReaderGetAttribute(reader, BAD_CAST("N"));
    if (stringValue)
    {
      tokenId = VSDXMLTokenMap::getTokenId(stringValue);
      xmlFree(stringValue);
    }
    return tokenId;
  case XML_ROW:
    stringValue = xmlTextReaderGetAttribute(reader, BAD_CAST("N"));
    if (!stringValue)
      stringValue = xmlTextReaderGetAttribute(reader, BAD_CAST("T"));
    if (stringValue)
    {
      tokenId = VSDXMLTokenMap::getTokenId(stringValue);
      xmlFree(stringValue);
    }
    return tokenId;
  case XML_SECTION:
    stringValue = xmlTextReaderGetAttribute(reader, BAD_CAST("N"));
    if (stringValue)
    {
      tokenId = VSDXMLTokenMap::getTokenId(stringValue);
      xmlFree(stringValue);
    }
    return tokenId;
  default:
    break;
  }
  return tokenId;
}

void libvisio::VSDXParser::readPageSheetProperties(xmlTextReaderPtr reader)
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
      VSD_DEBUG_MSG(("VSDXParser::readPageSheetProperties: unknown token %s\n", xmlTextReaderConstName(reader)));
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
  while ((XML_PAGESHEET != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);

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

void libvisio::VSDXParser::readStyleProperties(xmlTextReaderPtr reader)
{
  // Line properties
  double strokeWidth = 0;
  Colour strokeColour;
  long linePattern = 0;
  long startMarker = 0;
  long endMarker = 0;
  long lineCap = 0;

  // Fill and shadow properties
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

  // Text block properties
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
        readExtendedColourData(strokeColour, reader);
      break;
    case XML_LINEPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(linePattern, reader);
      break;
    case XML_BEGINARROW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(startMarker, reader);
      break;
    case XML_ENDARROW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(endMarker, reader);
      break;
    case XML_LINECAP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(lineCap, reader);
      break;
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
        ret = readLongData(fillPattern, reader);
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
        ret = readLongData(shadowPattern, reader);
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
      shadowOffsetY *= -1.0;
      break;
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
    case XML_SHDWFOREGNDTRANS:
    case XML_SHDWBKGNDTRANS:
    case XML_SHAPESHDWTYPE:
    case XML_SHAPESHDWOBLIQUEANGLE:
    case XML_SHAPESHDWSCALEFACTOR:
    case XML_TEXTBKGNDTRANS:
    default:
      break;
    }
  }
  while ((XML_STYLESHEET != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);

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
  {
    m_collector->collectLineStyle(level, strokeWidth, strokeColour, (unsigned char)linePattern, (unsigned char)startMarker, (unsigned char)endMarker, (unsigned char)lineCap);
    m_collector->collectFillStyle(level, fillColourFG, fillColourBG, (unsigned char)fillPattern, fillFGTransparency,
                                  fillBGTransparency, (unsigned char)shadowPattern, shadowColourFG, shadowOffsetX, shadowOffsetY);
    m_collector->collectTextBlockStyle(level, leftMargin, rightMargin, topMargin, bottomMargin,
                                       (unsigned char)verticalAlign, !!bgClrId, bgColour, defaultTabStop, (unsigned char)textDirection);
  }
  else
  {
    if (!m_shape.m_lineStyle)
      m_shape.m_lineStyle = new VSDLineStyle();
    *(m_shape.m_lineStyle) = VSDLineStyle(strokeWidth, strokeColour, (unsigned char)linePattern, (unsigned char)startMarker, (unsigned char)endMarker, (unsigned char)lineCap);
    if (!m_shape.m_fillStyle)
      m_shape.m_fillStyle = new VSDFillStyle();
    *(m_shape.m_fillStyle) = VSDFillStyle(fillColourFG, fillColourBG, (unsigned char)fillPattern, fillFGTransparency, fillBGTransparency, shadowColourFG,
                                          (unsigned char)shadowPattern, shadowOffsetX, shadowOffsetY);
    if (!m_shape.m_textBlockStyle)
      m_shape.m_textBlockStyle = new VSDTextBlockStyle();
    *(m_shape.m_textBlockStyle) = VSDTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin, (unsigned char)verticalAlign, !!bgClrId, bgColour,
                                  defaultTabStop, (unsigned char)textDirection);
  }
}

int libvisio::VSDXParser::getElementDepth(xmlTextReaderPtr reader)
{
  return xmlTextReaderDepth(reader)+m_currentDepth;
}

void libvisio::VSDXParser::readShapeProperties(xmlTextReaderPtr reader)
{
  // Text block properties
  long bgClrId = 0;

  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
    {
      VSD_DEBUG_MSG(("VDXParser::readShapeProperties: unknown token %s\n", xmlTextReaderConstName(reader)));
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
    case XML_FOREIGNDATA:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readForeignData(reader);
      break;
    case XML_LINEWEIGHT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_lineStyle)
          m_shape.m_lineStyle = new VSDLineStyle();
        ret = readDoubleData(m_shape.m_lineStyle->width, reader);
      }
      break;
    case XML_LINECOLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_lineStyle)
          m_shape.m_lineStyle = new VSDLineStyle();
        ret = readExtendedColourData(m_shape.m_lineStyle->colour, reader);
      }
      break;
    case XML_LINEPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_lineStyle)
          m_shape.m_lineStyle = new VSDLineStyle();
        ret = readByteData(m_shape.m_lineStyle->pattern, reader);
      }
      break;
    case XML_BEGINARROW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_lineStyle)
          m_shape.m_lineStyle = new VSDLineStyle();
        ret = readByteData(m_shape.m_lineStyle->startMarker, reader);
      }
      break;
    case XML_ENDARROW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_lineStyle)
          m_shape.m_lineStyle = new VSDLineStyle();
        ret = readByteData(m_shape.m_lineStyle->endMarker, reader);
      }
      break;
    case XML_LINECAP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_lineStyle)
          m_shape.m_lineStyle = new VSDLineStyle();
        ret = readByteData(m_shape.m_lineStyle->cap, reader);
      }
      break;
    case XML_FILLFOREGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_fillStyle)
          m_shape.m_fillStyle = new VSDFillStyle();
        ret = readExtendedColourData(m_shape.m_fillStyle->fgColour, reader);
      }
      break;
    case XML_FILLBKGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_fillStyle)
          m_shape.m_fillStyle = new VSDFillStyle();
        ret = readExtendedColourData(m_shape.m_fillStyle->bgColour, reader);
      }
      break;
    case XML_FILLPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_fillStyle)
          m_shape.m_fillStyle = new VSDFillStyle();
        ret = readByteData(m_shape.m_fillStyle->pattern, reader);
      }
      break;
    case XML_SHDWFOREGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_fillStyle)
          m_shape.m_fillStyle = new VSDFillStyle();
        ret = readExtendedColourData(m_shape.m_fillStyle->shadowFgColour, reader);
      }
      break;
    case XML_SHDWBKGND: /* unsupported */
      break;
    case XML_SHDWPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_fillStyle)
          m_shape.m_fillStyle = new VSDFillStyle();
        ret = readByteData(m_shape.m_fillStyle->shadowPattern, reader);
      }
      break;
    case XML_FILLFOREGNDTRANS:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_fillStyle)
          m_shape.m_fillStyle = new VSDFillStyle();
        ret = readDoubleData(m_shape.m_fillStyle->fgTransparency, reader);
      }
      break;
    case XML_FILLBKGNDTRANS:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_fillStyle)
          m_shape.m_fillStyle = new VSDFillStyle();
        ret = readDoubleData(m_shape.m_fillStyle->bgTransparency, reader);
      }
      break;
    case XML_SHAPESHDWOFFSETX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_fillStyle)
          m_shape.m_fillStyle = new VSDFillStyle();
        ret = readDoubleData(m_shape.m_fillStyle->shadowOffsetX, reader);
      }
      break;
    case XML_SHAPESHDWOFFSETY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_fillStyle)
          m_shape.m_fillStyle = new VSDFillStyle();
        ret = readDoubleData(m_shape.m_fillStyle->shadowOffsetY, reader);
        m_shape.m_fillStyle->shadowOffsetY *= -1.0;
      }
      break;
    case XML_LEFTMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_textBlockStyle)
          m_shape.m_textBlockStyle = new VSDTextBlockStyle();
        ret = readDoubleData(m_shape.m_textBlockStyle->leftMargin, reader);
      }
      break;
    case XML_RIGHTMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_textBlockStyle)
          m_shape.m_textBlockStyle = new VSDTextBlockStyle();
        ret = readDoubleData(m_shape.m_textBlockStyle->rightMargin, reader);
      }
      break;
    case XML_TOPMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_textBlockStyle)
          m_shape.m_textBlockStyle = new VSDTextBlockStyle();
        ret = readDoubleData(m_shape.m_textBlockStyle->topMargin, reader);
      }
      break;
    case XML_BOTTOMMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_textBlockStyle)
          m_shape.m_textBlockStyle = new VSDTextBlockStyle();
        ret = readDoubleData(m_shape.m_textBlockStyle->bottomMargin, reader);
      }
      break;
    case XML_VERTICALALIGN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_textBlockStyle)
          m_shape.m_textBlockStyle = new VSDTextBlockStyle();
        ret = readByteData(m_shape.m_textBlockStyle->verticalAlign, reader);
      }
      break;
    case XML_TEXTBKGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_textBlockStyle)
          m_shape.m_textBlockStyle = new VSDTextBlockStyle();
        ret = readExtendedColourData(m_shape.m_textBlockStyle->textBkgndColour, bgClrId, reader);
        if (bgClrId < 0) bgClrId = 0;
        if (bgClrId)
        {
          std::map<unsigned, Colour>::const_iterator iter = m_colours.find(bgClrId-1);
          if (iter != m_colours.end())
            m_shape.m_textBlockStyle->textBkgndColour = iter->second;
          else
            m_shape.m_textBlockStyle->textBkgndColour = Colour();
        }
      }
      break;
    case XML_DEFAULTTABSTOP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_textBlockStyle)
          m_shape.m_textBlockStyle = new VSDTextBlockStyle();
        ret = readDoubleData(m_shape.m_textBlockStyle->defaultTabStop, reader);
      }
      break;
    case XML_TEXTDIRECTION:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        if (!m_shape.m_textBlockStyle)
          m_shape.m_textBlockStyle = new VSDTextBlockStyle();
        ret = readByteData(m_shape.m_textBlockStyle->textDirection, reader);
      }
      break;
    case XML_GEOM:
    case XML_GEOMETRY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readGeometry(reader);
      break;
    case XML_RESIZEMODE:
    default:
      break;
    }
  }
  while ((XML_SHAPES != tokenId) && (XML_SHAPE != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);

  if (XML_SHAPE == tokenId && XML_READER_TYPE_END_ELEMENT == tokenType)
  {
    if (m_isStencilStarted)
      m_currentStencil->addStencilShape(m_shape.m_shapeId, m_shape);
    else
      _flushShape();
    m_shape.clear();
  }
  else if (XML_SHAPES == tokenId)
    processXmlNode(reader);
}

void libvisio::VSDXParser::getBinaryData(xmlTextReaderPtr reader)
{
  xmlTextReaderRead(reader);
  int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  int tokenType = xmlTextReaderNodeType(reader);

  m_currentBinaryData.clear();
  if (XML_REL == tokenId && XML_READER_TYPE_ELEMENT == tokenType)
  {
    xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("r:id"));
    if (id)
    {
      const VSDXRelationship *rel = m_rels->getRelationshipById((char *)id);
      if (rel)
      {
        if ("http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" == rel->getType()
            || "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject" == rel->getType())
          extractBinaryData(m_input, rel->getTarget().c_str());
      }
      xmlFree(id);
    }
  }
  m_shape.m_foreign->data = m_currentBinaryData;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
