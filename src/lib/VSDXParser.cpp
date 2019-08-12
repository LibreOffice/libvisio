/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDXParser.h"

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
#include "VSDXMetaData.h"

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


libvisio::VSDXParser::VSDXParser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter)
  : VSDXMLParserBase(),
    m_input(input),
    m_painter(painter),
    m_currentDepth(0),
    m_rels(nullptr),
    m_currentTheme()
{
}

libvisio::VSDXParser::~VSDXParser()
{
}

bool libvisio::VSDXParser::parseMain() try
{
  if (!m_input || !m_input->isStructured())
    return false;

  RVNGInputStreamPtr_t tmpInput;
  tmpInput.reset(m_input->getSubStreamByName("_rels/.rels"));
  if (!tmpInput)
    return false;

  libvisio::VSDXRelationships rootRels(tmpInput.get());

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
  parseMetaData(m_input, rootRels);

  if (!parseDocument(m_input, rel->getTarget().c_str()))
    return false;

  return true;
}
catch (...)
{
  return false;
}

bool libvisio::VSDXParser::extractStencils()
{
  m_extractStencils = true;
  return parseMain();
}

bool libvisio::VSDXParser::parseDocument(librevenge::RVNGInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (!input->isStructured())
    return false;
  const RVNGInputStreamPtr_t stream(input->getSubStreamByName(name));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (!stream)
    return false;
  const RVNGInputStreamPtr_t relStream(input->getSubStreamByName(getRelationshipsForTarget(name).c_str()));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  VSDXRelationships rels(relStream.get());
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  const VSDXRelationship *rel = rels.getRelationshipByType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme");
  if (rel)
  {
    if (!parseTheme(input, rel->getTarget().c_str()))
    {
      VSD_DEBUG_MSG(("Could not parse theme\n"));
      m_collector->collectDocumentTheme(nullptr);
    }
    else
      m_collector->collectDocumentTheme(&m_currentTheme);
    input->seek(0, librevenge::RVNG_SEEK_SET);
  }

  processXmlDocument(stream.get(), rels);

  rel = rels.getRelationshipByType("http://schemas.microsoft.com/visio/2010/relationships/masters");
  if (rel)
  {
    if (!parseMasters(input, rel->getTarget().c_str()))
    {
      VSD_DEBUG_MSG(("Could not parse masters\n"));
    }
    input->seek(0, librevenge::RVNG_SEEK_SET);
  }

  rel = rels.getRelationshipByType("http://schemas.microsoft.com/visio/2010/relationships/pages");
  if (rel)
  {
    if (!parsePages(input, rel->getTarget().c_str()))
    {
      VSD_DEBUG_MSG(("Could not parse pages\n"));
    }
    input->seek(0, librevenge::RVNG_SEEK_SET);
  }

  return true;
}

bool libvisio::VSDXParser::parseMasters(librevenge::RVNGInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (!input->isStructured())
    return false;
  const RVNGInputStreamPtr_t stream(input->getSubStreamByName(name));
  if (!stream)
    return false;
  const RVNGInputStreamPtr_t relStream(input->getSubStreamByName(getRelationshipsForTarget(name).c_str()));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  VSDXRelationships rels(relStream.get());
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  processXmlDocument(stream.get(), rels);

  return true;
}

bool libvisio::VSDXParser::parseMaster(librevenge::RVNGInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (!input->isStructured())
    return false;
  const RVNGInputStreamPtr_t stream(input->getSubStreamByName(name));
  if (!stream)
    return false;
  const RVNGInputStreamPtr_t relStream(input->getSubStreamByName(getRelationshipsForTarget(name).c_str()));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  VSDXRelationships rels(relStream.get());
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  processXmlDocument(stream.get(), rels);

  return true;
}

bool libvisio::VSDXParser::parsePages(librevenge::RVNGInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (!input->isStructured())
    return false;
  const RVNGInputStreamPtr_t stream(input->getSubStreamByName(name));
  if (!stream)
    return false;
  const RVNGInputStreamPtr_t relStream(input->getSubStreamByName(getRelationshipsForTarget(name).c_str()));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  VSDXRelationships rels(relStream.get());
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  processXmlDocument(stream.get(), rels);

  return true;
}

bool libvisio::VSDXParser::parsePage(librevenge::RVNGInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (!input->isStructured())
    return false;
  const RVNGInputStreamPtr_t stream(input->getSubStreamByName(name));
  if (!stream)
    return false;
  const RVNGInputStreamPtr_t relStream(input->getSubStreamByName(getRelationshipsForTarget(name).c_str()));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  VSDXRelationships rels(relStream.get());
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  processXmlDocument(stream.get(), rels);

  return true;
}

bool libvisio::VSDXParser::parseTheme(librevenge::RVNGInputStream *input, const char *name)
{
  if (!input)
    return false;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (!input->isStructured())
    return false;
  const RVNGInputStreamPtr_t stream(input->getSubStreamByName(name));
  if (!stream)
    return false;

  m_currentTheme.parse(stream.get());

  return true;
}

void libvisio::VSDXParser::parseMetaData(librevenge::RVNGInputStream *input, libvisio::VSDXRelationships &rels) try
{
  if (!input)
    return;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (!input->isStructured())
    return;

  VSDXMetaData metaData;
  const libvisio::VSDXRelationship *coreProp = rels.getRelationshipByType("http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties");
  if (coreProp)
  {
    const RVNGInputStreamPtr_t stream(input->getSubStreamByName(coreProp->getTarget().c_str()));
    if (stream)
    {
      metaData.parse(stream.get());
    }
  }

  const libvisio::VSDXRelationship *extendedProp = rels.getRelationshipByType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties");
  if (extendedProp)
  {
    const RVNGInputStreamPtr_t stream(input->getSubStreamByName(extendedProp->getTarget().c_str()));
    if (stream)
    {
      metaData.parse(stream.get());
    }
  }
  m_collector->collectMetaData(metaData.getMetaData());
}
catch (...)
{
  // Ignore any exceptions in metadata. They are not important enough to stop parsing.
}

void libvisio::VSDXParser::processXmlDocument(librevenge::RVNGInputStream *input, VSDXRelationships &rels)
{
  if (!input)
    return;

  m_rels = &rels;

  XMLErrorWatcher watcher;

  auto reader = xmlReaderForStream(input, &watcher, false);
  if (!reader)
    return;

  XMLErrorWatcher *oldWatcher = m_watcher;
  try
  {
    m_watcher = &watcher;

    int ret = xmlTextReaderRead(reader.get());
    while (1 == ret && !watcher.isError())
    {
      int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader.get()));
      int tokenType = xmlTextReaderNodeType(reader.get());

      switch (tokenId)
      {
      case XML_REL:
        if (XML_READER_TYPE_ELEMENT == tokenType)
        {
          std::shared_ptr<xmlChar> id(xmlTextReaderGetAttribute(reader.get(), BAD_CAST("r:id")), xmlFree);
          if (id)
          {
            const VSDXRelationship *rel = rels.getRelationshipById((char *)id.get());
            if (rel)
            {
              std::string type = rel->getType();
              if (type == "http://schemas.microsoft.com/visio/2010/relationships/master")
              {
                m_currentDepth += xmlTextReaderDepth(reader.get());
                parseMaster(m_input, rel->getTarget().c_str());
                m_currentDepth -= xmlTextReaderDepth(reader.get());
              }
              else if (type == "http://schemas.microsoft.com/visio/2010/relationships/page")
              {
                m_currentDepth += xmlTextReaderDepth(reader.get());
                parsePage(m_input, rel->getTarget().c_str());
                m_currentDepth -= xmlTextReaderDepth(reader.get());
              }
              else if (type == "http://schemas.openxmlformats.org/officeDocument/2006/relationships/image")
              {
                extractBinaryData(m_input, rel->getTarget().c_str());
              }
              else
                processXmlNode(reader.get());
            }
          }
        }
        break;
      default:
        processXmlNode(reader.get());
        break;
      }
      ret = xmlTextReaderRead(reader.get());
    }

    m_watcher = oldWatcher;
  }
  catch (...)
  {
    m_watcher = oldWatcher;
    throw;
  }
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
  case XML_FACENAMES:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readFonts(reader);
    break;
  case XML_MASTER:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      handleMasterStart(reader);
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
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
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
      handlePageEnd(reader);
    break;
  case XML_PAGES:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      handlePagesStart(reader);
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
      handlePagesEnd(reader);
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
    else if (XML_READER_TYPE_END_ELEMENT == tokenType)
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
        if (m_isStencilStarted && m_currentStencil)
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
      if (m_isStencilStarted && m_currentStencil)
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
      fprintf(stderr, " %s=\"%s\"", name1, value1);
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

void libvisio::VSDXParser::extractBinaryData(librevenge::RVNGInputStream *input, const char *name)
{
  m_currentBinaryData.clear();
  if (!input || !input->isStructured())
    return;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  const RVNGInputStreamPtr_t stream(input->getSubStreamByName(name));
  if (!stream)
    return;
  while (true)
  {
    unsigned long numBytesRead;
    const unsigned char *buffer = stream->read(VSDX_DATA_READ_SIZE, numBytesRead);
    if (numBytesRead)
      m_currentBinaryData.append(buffer, numBytesRead);
    if (stream->isEnd())
      break;
  }
  VSD_DEBUG_MSG(("%s\n", m_currentBinaryData.getBase64Data().cstr()));
}

xmlChar *libvisio::VSDXParser::readStringData(xmlTextReaderPtr reader)
{
  std::unique_ptr<xmlChar, void (*)(void *)> stringValue(xmlTextReaderGetAttribute(reader, BAD_CAST("V")), xmlFree);
  if (stringValue)
  {
    VSD_DEBUG_MSG(("VSDXParser::readStringData stringValue %s\n", (const char *)stringValue.get()));
    return stringValue.release();
  }
  return nullptr;
}

int libvisio::VSDXParser::getElementToken(xmlTextReaderPtr reader)
{
  int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  if (XML_READER_TYPE_END_ELEMENT == xmlTextReaderNodeType(reader))
    return tokenId;

  std::unique_ptr<xmlChar, decltype(xmlFree)> stringValue(nullptr, xmlFree);

  switch (tokenId)
  {
  case XML_CELL:
    stringValue.reset(xmlTextReaderGetAttribute(reader, BAD_CAST("N")));
    if (stringValue)
    {
      tokenId = VSDXMLTokenMap::getTokenId(stringValue.get());
      if (tokenId == XML_TOKEN_INVALID)
      {
        if (*stringValue.get() == 'P' && !strncmp((char *)stringValue.get(), "Position", 8))
          tokenId = XML_POSITION;
        else if (*stringValue.get() == 'A' && !strncmp((char *)stringValue.get(), "Alignment", 9))
          tokenId = XML_ALIGNMENT;
      }
    }
    break;
  case XML_ROW:
    stringValue.reset(xmlTextReaderGetAttribute(reader, BAD_CAST("N")));
    if (!stringValue)
      stringValue.reset(xmlTextReaderGetAttribute(reader, BAD_CAST("T")));
    if (stringValue)
      tokenId = VSDXMLTokenMap::getTokenId(stringValue.get());
    break;
  case XML_SECTION:
    stringValue.reset(xmlTextReaderGetAttribute(reader, BAD_CAST("N")));
    if (stringValue)
      tokenId = VSDXMLTokenMap::getTokenId(stringValue.get());
    break;
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
      break;
    case XML_PAGESCALE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(pageScale, reader);
      break;
    case XML_DRAWINGSCALE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(drawingScale, reader);
      break;
    case XML_LAYER:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readLayer(reader);
      break;
    default:
      break;
    }
  }
  while ((XML_PAGESHEET != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

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

void libvisio::VSDXParser::readFonts(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  unsigned idx = 0;

  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXParser::readFonts: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    if (XML_FACENAME == tokenId && XML_READER_TYPE_ELEMENT == tokenType)
    {
      std::unique_ptr<xmlChar, decltype(xmlFree)> name(xmlTextReaderGetAttribute(reader, BAD_CAST("NameU")), xmlFree);
      if (name)
      {
        librevenge::RVNGBinaryData textStream(name.get(), xmlStrlen(name.get()));
        m_fonts[idx] = VSDName(textStream, libvisio::VSD_TEXT_UTF8);
      }
      ++idx;
    }
  }
  while ((XML_FACENAMES != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VSDXParser::readStyleProperties(xmlTextReaderPtr reader)
{
  // Line properties
  boost::optional<double> strokeWidth;
  boost::optional<Colour> strokeColour;
  boost::optional<unsigned char> linePattern;
  boost::optional<double> rounding;
  boost::optional<unsigned char> startMarker;
  boost::optional<unsigned char> endMarker;
  boost::optional<unsigned char> lineCap;
  boost::optional<long> qsLineColour;
  boost::optional<long> qsLineMatrix;

  // Fill and shadow properties
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
  boost::optional<long> qsFillColour;
  boost::optional<long> qsShadowColour;
  boost::optional<long> qsFillMatrix;

  // Text block properties
  boost::optional<double> leftMargin;
  boost::optional<double> rightMargin;
  boost::optional<double> topMargin;
  boost::optional<double> bottomMargin;
  boost::optional<unsigned char> verticalAlign;
  boost::optional<bool> bgClrId;
  boost::optional<Colour> bgColour;
  boost::optional<double> defaultTabStop;
  boost::optional<unsigned char> textDirection;

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
      VSD_DEBUG_MSG(("VSDXParser::readLine: unknown token %s\n", xmlTextReaderConstName(reader)));
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
        ret = readExtendedColourData(strokeColour, reader);
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
#if 0
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(bgColour, bgClrId, reader);
#endif
      break;
    case XML_DEFAULTTABSTOP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(defaultTabStop, reader);
      break;
    case XML_TEXTDIRECTION:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(textDirection, reader);
      break;
    case XML_PARAGRAPH:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readParagraph(reader);
      break;
    case XML_CHARACTER:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readCharacter(reader);
      break;
    case XML_QUICKSTYLELINECOLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(qsLineColour, reader);
      break;
    case XML_QUICKSTYLELINEMATRIX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(qsLineMatrix, reader);
      break;
    case XML_QUICKSTYLEFILLCOLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(qsFillColour, reader);
      break;
    case XML_QUICKSTYLESHADOWCOLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(qsShadowColour, reader);
      break;
    case XML_QUICKSTYLEFILLMATRIX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(qsFillMatrix, reader);
      break;
    default:
      break;
    }
  }
  while ((XML_STYLESHEET != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

#if 0
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
#endif

  if (m_isInStyles)
  {
    m_collector->collectLineStyle(level, strokeWidth, strokeColour, linePattern, startMarker, endMarker, lineCap,
                                  rounding, qsLineColour, qsLineMatrix);
    m_collector->collectFillStyle(level, fillColourFG, fillColourBG, fillPattern, fillFGTransparency,
                                  fillBGTransparency, shadowPattern, shadowColourFG, shadowOffsetX,
                                  shadowOffsetY, qsFillColour, qsShadowColour, qsFillMatrix);
    m_collector->collectTextBlockStyle(level, leftMargin, rightMargin, topMargin, bottomMargin,
                                       verticalAlign, bgClrId, bgColour, defaultTabStop, textDirection);
  }
  else
  {
    m_shape.m_lineStyle.override(VSDOptionalLineStyle(strokeWidth, strokeColour, linePattern, startMarker, endMarker, lineCap, rounding,
                                                      qsLineColour, qsLineMatrix));
    m_shape.m_fillStyle.override(VSDOptionalFillStyle(fillColourFG, fillColourBG, fillPattern, fillFGTransparency, fillBGTransparency,
                                                      shadowColourFG, shadowPattern, shadowOffsetX, shadowOffsetY,
                                                      qsFillColour, qsShadowColour, qsFillMatrix));
    m_shape.m_textBlockStyle.override(VSDOptionalTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin, verticalAlign, !!bgClrId, bgColour,
                                                                defaultTabStop, textDirection));
  }
}

int libvisio::VSDXParser::getElementDepth(xmlTextReaderPtr reader)
{
  return xmlTextReaderDepth(reader)+m_currentDepth;
}

void libvisio::VSDXParser::readShapeProperties(xmlTextReaderPtr reader)
{
  // Text block properties
  long bgClrId = -1;

  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    int tokenClass = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXParser::readShapeProperties: unknown token %s\n", xmlTextReaderConstName(reader)));
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
    case XML_FOREIGNDATA:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readForeignData(reader);
      break;
    case XML_LINEWEIGHT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_lineStyle.width, reader);
      break;
    case XML_LINECOLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(m_shape.m_lineStyle.colour, reader);
      break;
    case XML_LINEPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(m_shape.m_lineStyle.pattern, reader);
      break;
    case XML_BEGINARROW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(m_shape.m_lineStyle.startMarker, reader);
      break;
    case XML_ENDARROW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(m_shape.m_lineStyle.endMarker, reader);
      break;
    case XML_LINECAP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(m_shape.m_lineStyle.cap, reader);
      break;
    case XML_FILLFOREGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(m_shape.m_fillStyle.fgColour, reader);
      break;
    case XML_FILLBKGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(m_shape.m_fillStyle.bgColour, reader);
      break;
    case XML_FILLPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(m_shape.m_fillStyle.pattern, reader);
      break;
    case XML_SHDWFOREGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readExtendedColourData(m_shape.m_fillStyle.shadowFgColour, reader);
      break;
    case XML_SHDWBKGND: /* unsupported */
      break;
    case XML_SHDWPATTERN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(m_shape.m_fillStyle.shadowPattern, reader);
      break;
    case XML_FILLFOREGNDTRANS:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_fillStyle.fgTransparency, reader);
      break;
    case XML_FILLBKGNDTRANS:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_fillStyle.bgTransparency, reader);
      break;
    case XML_SHAPESHDWOFFSETX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_fillStyle.shadowOffsetX, reader);
      break;
    case XML_SHAPESHDWOFFSETY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_fillStyle.shadowOffsetY, reader);
      break;
    case XML_LEFTMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_textBlockStyle.leftMargin, reader);
      break;
    case XML_RIGHTMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_textBlockStyle.rightMargin, reader);
      break;
    case XML_TOPMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_textBlockStyle.topMargin, reader);
      break;
    case XML_BOTTOMMARGIN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_textBlockStyle.bottomMargin, reader);
      break;
    case XML_VERTICALALIGN:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(m_shape.m_textBlockStyle.verticalAlign, reader);
      break;
    case XML_TEXTBKGND:
      if (XML_READER_TYPE_ELEMENT == tokenType)
      {
        Colour textBkgndColour(0xff, 0xff, 0xff, 0);
        ret = readExtendedColourData(textBkgndColour, bgClrId, reader);
        if (bgClrId < 0) bgClrId = 0;
        if (bgClrId)
        {
          std::map<unsigned, Colour>::const_iterator iter = m_colours.find(bgClrId-1);
          if (iter != m_colours.end())
            textBkgndColour = iter->second;
          else
            textBkgndColour = Colour(0xff, 0xff, 0xff, 0);
        }
        m_shape.m_textBlockStyle.textBkgndColour = textBkgndColour;
        m_shape.m_textBlockStyle.isTextBkgndFilled = true;
      }
      break;
    case XML_DEFAULTTABSTOP:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readDoubleData(m_shape.m_textBlockStyle.defaultTabStop, reader);
      break;
    case XML_TEXTDIRECTION:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readByteData(m_shape.m_textBlockStyle.textDirection, reader);
      break;
    case XML_PARAGRAPH:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readParagraph(reader);
      break;
    case XML_CHARACTER:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readCharacter(reader);
      break;
    case XML_GEOM:
    case XML_GEOMETRY:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readGeometry(reader);
      break;
    case XML_TEXT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readText(reader);
      break;
    case XML_HIDETEXT:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readBoolData(m_shape.m_misc.m_hideText, reader);
      break;
    case XML_QUICKSTYLELINECOLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(m_shape.m_lineStyle.qsLineColour, reader);
      break;
    case XML_QUICKSTYLELINEMATRIX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(m_shape.m_lineStyle.qsLineMatrix, reader);
      break;
    case XML_QUICKSTYLEFILLCOLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(m_shape.m_fillStyle.qsFillColour, reader);
      break;
    case XML_QUICKSTYLESHADOWCOLOR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(m_shape.m_fillStyle.qsShadowColour, reader);
      break;
    case XML_QUICKSTYLEFILLMATRIX:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readLongData(m_shape.m_fillStyle.qsFillMatrix, reader);
      break;
    case XML_LAYERMEMBER:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        ret = readStringData(m_shape.m_layerMem, reader);
      break;
    case XML_TABS:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readTabs(reader);
      break;
    default:
      if (XML_SECTION == tokenClass && XML_READER_TYPE_ELEMENT == tokenType)
        ret = skipSection(reader);
      break;
    }
  }
  while ((XML_SHAPES != tokenId) && (XML_SHAPE != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));

  if (1 == ret)
    processXmlNode(reader);
}

void libvisio::VSDXParser::readLayer(xmlTextReaderPtr reader)
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
      VSD_DEBUG_MSG(("VSDXParser::readLayer: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    if (XML_ROW == tokenId && XML_READER_TYPE_ELEMENT == tokenType)
      readLayerIX(reader);
  }
  while ((XML_SECTION != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VSDXParser::readParagraph(xmlTextReaderPtr reader)
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
      VSD_DEBUG_MSG(("VSDXParser::readParagraph: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    if (XML_ROW == tokenId && XML_READER_TYPE_ELEMENT == tokenType)
      readParaIX(reader);
  }
  while ((XML_SECTION != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VSDXParser::readTabs(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;

  if (xmlTextReaderIsEmptyElement(reader))
  {
    m_shape.m_tabSets.clear();
  }
  else
  {
    do
    {
      ret = xmlTextReaderRead(reader);
      tokenId = getElementToken(reader);
      if (XML_TOKEN_INVALID == tokenId)
      {
        VSD_DEBUG_MSG(("VSDXParser::readTabs: unknown token %s\n", xmlTextReaderConstName(reader)));
      }
      tokenType = xmlTextReaderNodeType(reader);
      if (XML_ROW == tokenId && XML_READER_TYPE_ELEMENT == tokenType)
        readTabRow(reader);
    }
    while ((XML_SECTION != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  }
}

void libvisio::VSDXParser::readTabRow(xmlTextReaderPtr reader)
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
        VSD_DEBUG_MSG(("VSDXParser::readTabs: unknown token %s\n", xmlTextReaderConstName(reader)));
      }
      tokenType = xmlTextReaderNodeType(reader);
      switch (tokenId)
      {
      case XML_POSITION:
        if (XML_READER_TYPE_ELEMENT == tokenType)
        {
          const std::shared_ptr<xmlChar> stringValue(xmlTextReaderGetAttribute(reader, BAD_CAST("N")), xmlFree);
          if (stringValue)
          {
            unsigned idx = xmlStringToLong(stringValue.get()+8);
            ret = readDoubleData((*m_currentTabSet)[idx].m_position, reader);
          }
        }
        break;
      case XML_ALIGNMENT:
        if (XML_READER_TYPE_ELEMENT == tokenType)
        {
          const std::shared_ptr<xmlChar> stringValue(xmlTextReaderGetAttribute(reader, BAD_CAST("N")), xmlFree);
          if (stringValue)
          {
            unsigned idx = xmlStringToLong(stringValue.get()+9);
            ret = readByteData((*m_currentTabSet)[idx].m_alignment, reader);
          }
        }
        break;
      default:
        break;
      }
    }
    while ((XML_ROW != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
  }
  m_currentTabSet = nullptr;
}

void libvisio::VSDXParser::readCharacter(xmlTextReaderPtr reader)
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
      VSD_DEBUG_MSG(("VSDXParser::readCharacter: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    if (XML_ROW == tokenId && XML_READER_TYPE_ELEMENT == tokenType)
      readCharIX(reader);
  }
  while ((XML_SECTION != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret && (!m_watcher || !m_watcher->isError()));
}

void libvisio::VSDXParser::getBinaryData(xmlTextReaderPtr reader)
{
  const int ret = xmlTextReaderRead(reader);
  int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
  int tokenType = xmlTextReaderNodeType(reader);

  m_currentBinaryData.clear();
  if (1 == ret && XML_REL == tokenId && XML_READER_TYPE_ELEMENT == tokenType)
  {
    std::unique_ptr<xmlChar, decltype(xmlFree)> id(xmlTextReaderGetAttribute(reader, BAD_CAST("r:id")), xmlFree);
    if (id)
    {
      const VSDXRelationship *rel = m_rels->getRelationshipById((char *)id.get());
      if (rel)
      {
        if ("http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" == rel->getType()
            || "http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject" == rel->getType())
          extractBinaryData(m_input, rel->getTarget().c_str());
      }
    }
  }
  if (!m_shape.m_foreign)
    m_shape.m_foreign = make_unique<ForeignData>();
  m_shape.m_foreign->data = m_currentBinaryData;
}

int libvisio::VSDXParser::skipSection(xmlTextReaderPtr reader)
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
  while ((XML_SECTION != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  return ret;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
