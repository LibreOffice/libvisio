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
  : m_input(0), m_painter(painter), m_collector(), m_stencils(), m_extractStencils(false),
    m_currentDepth(0), m_currentBinaryData()
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
    const xmlChar *nodeName = xmlTextReaderConstName(reader);
    if (!nodeName)
    {
      ret = xmlTextReaderRead(reader);
      continue;
    }

    if (xmlStrEqual(nodeName, BAD_CAST("Rel")) && xmlTextReaderNodeType(reader) == 1)
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
    else
      processXmlNode(reader);

    ret = xmlTextReaderRead(reader);
  }

  xmlFreeTextReader(reader);
}

void libvisio::VSDXParser::processXmlNode(xmlTextReaderPtr reader)
{
  if (!reader)
    return;
#ifdef DEBUG
  const xmlChar *name = xmlTextReaderConstName(reader);
  const xmlChar *value = xmlTextReaderConstValue(reader);
  int type = xmlTextReaderNodeType(reader);
  int isEmptyElement = xmlTextReaderIsEmptyElement(reader);

  for (int i=0; i<xmlTextReaderDepth(reader)+m_currentDepth; ++i)
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

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
