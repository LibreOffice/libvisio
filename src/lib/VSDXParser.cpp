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
  : m_input(0), m_painter(painter), m_collector(), m_stencils(), m_currentStencil(0), m_stencilShape(),
    m_isStencilStarted(false), m_currentStencilID((unsigned)-1), m_extractStencils(false),
    m_isInStyles(false), m_currentDepth(0), m_currentLevel(0), m_currentShapeLevel(0), m_currentBinaryData(),
    m_colours(), m_charList(new VSDCharacterList()), m_charListVector(), m_fieldList(),
    m_geomList(new VSDGeometryList()), m_geomListVector(), m_paraList(new VSDParagraphList()),
    m_paraListVector(), m_shapeList()
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

  xmlTextReaderPtr reader = xmlReaderForStream(input, 0, 0, XML_PARSE_NOENT|XML_PARSE_NOBLANKS|XML_PARSE_NONET);
  if (!reader)
    return;
  int ret = xmlTextReaderRead(reader);
  while (ret == 1)
  {
    int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
    int tokenType = xmlTextReaderNodeType(reader);

    switch (tokenId)
    {
    case XML_REL:
      if (1 == tokenType)
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
    if (1 == tokenType)
      readColours(reader);
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
          tokenId = getElementToken(reader);
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
    value = xmlStringToLong(stringValue);
    xmlFree(stringValue);
    return 1;
  }
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
    VSD_DEBUG_MSG(("VSDXParser::readBoolData stringValue %s\n", (const char *)stringValue));
    value = xmlStringToColour(stringValue);
    xmlFree(stringValue);
    return 1;
  }
  return -1;
}

int libvisio::VSDXParser::readExtendedColourData(Colour &value, long &idx, xmlTextReaderPtr reader)
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

int libvisio::VSDXParser::readExtendedColourData(Colour &value, xmlTextReaderPtr reader)
{
  long idx = -1;
  return readExtendedColourData(value, idx, reader);
}

void libvisio::VSDXParser::_handleLevelChange(unsigned level)
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

int libvisio::VSDXParser::getElementToken(xmlTextReaderPtr reader)
{
  int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
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

void libvisio::VSDXParser::readLine(xmlTextReaderPtr reader)
{
  double strokeWidth = 0;
  Colour colour;
  long linePattern = 0;
  long startMarker = 0;
  long endMarker = 0;
  long lineCap = 0;

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
      VSD_DEBUG_MSG(("VSDXParser::readLine: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
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

void libvisio::VSDXParser::readFillAndShadow(xmlTextReaderPtr reader)
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
      VSD_DEBUG_MSG(("VSDXParser::readFillAndShadow: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
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

void libvisio::VSDXParser::readPageProps(xmlTextReaderPtr reader)
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
      VSD_DEBUG_MSG(("VSDXParser::readPageProps: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
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
  while ((XML_PAGESHEET != tokenId || 15 != tokenType) && 1 == ret);

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

void libvisio::VSDXParser::readShape(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXParser::readColours(xmlTextReaderPtr reader)
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
      VSD_DEBUG_MSG(("VSDXParser::readColours: unknown token %s\n", xmlTextReaderConstName(reader)));
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
  while ((XML_COLORS != tokenId || 15 != tokenType) && 1 == ret);
}

void libvisio::VSDXParser::readPage(xmlTextReaderPtr reader)
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

void libvisio::VSDXParser::readTextBlock(xmlTextReaderPtr reader)
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

void libvisio::VSDXParser::readStyleSheet(xmlTextReaderPtr reader)
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

void libvisio::VSDXParser::readStencil(xmlTextReaderPtr reader)
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

void libvisio::VSDXParser::readPageSheet(xmlTextReaderPtr reader)
{
  m_currentShapeLevel = (unsigned)getElementDepth(reader);
  m_collector->collectPageSheet(0, m_currentShapeLevel);
  readPageProps(reader);
}

int libvisio::VSDXParser::getElementDepth(xmlTextReaderPtr reader)
{
  return xmlTextReaderDepth(reader)+m_currentDepth;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
