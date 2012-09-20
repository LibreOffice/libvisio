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

namespace
{

extern "C" {

  static int vsdxInputCloseFunc(void * /* context */)
  {
    return 0;
  }

  static int vsdxInputReadFunc(void *context, char *buffer, int len)
  {
    WPXInputStream *input = (WPXInputStream *)context;

    if ((!input) || (!buffer) || (len < 0))
      return -1;

    if (input->atEOS())
      return 0;

    unsigned long tmpNumBytesRead = 0;
    const unsigned char *tmpBuffer = input->read(len, tmpNumBytesRead);

    if (tmpBuffer && tmpNumBytesRead)
      memcpy(buffer, tmpBuffer, tmpNumBytesRead);
    return tmpNumBytesRead;
  }

}

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


// VSDXRelationship

libvisio::VSDXRelationship::VSDXRelationship(xmlTextReaderPtr reader)
  : m_id(), m_type(), m_target()
{
  if (reader)
    // TODO: check whether we are actually parsing "Relationship" element
  {
    while (xmlTextReaderMoveToNextAttribute(reader))
    {
      xmlChar *name = xmlTextReaderName(reader);
      xmlChar *value = xmlTextReaderValue(reader);
      if (xmlStrEqual(name, BAD_CAST("Id")))
        m_id = (char *)value;
      else if (xmlStrEqual(name, BAD_CAST("Type")))
        m_type = (char *)value;
      else if (xmlStrEqual(name, BAD_CAST("Target")))
        m_target = (char *)value;
      xmlFree(name);
      xmlFree(value);
    }
    VSD_DEBUG_MSG(("Relationship : %s type: %s target: %s\n", m_id.c_str(), m_type.c_str(), m_target.c_str()));
  }
}

libvisio::VSDXRelationship::VSDXRelationship()
  : m_id(), m_type(), m_target()
{
}

libvisio::VSDXRelationship::~VSDXRelationship()
{
}

void libvisio::VSDXRelationship::rebaseTarget(const char *baseDir)
{
  std::string target(baseDir ? baseDir : "");
  if (!target.empty())
    target += "/";
  target += m_target;

  // normalize path by resolving any ".." or "." segments
  // and eliminating duplicate path separators
  std::vector<std::string> segments;
  boost::split(segments, target, boost::is_any_of("/\\"));
  std::vector<std::string> normalizedSegments;

  for (unsigned i = 0; i < segments.size(); ++i)
  {
    if (segments[i] == "..")
      normalizedSegments.pop_back();
    else if (segments[i] != "." && !segments[i].empty())
      normalizedSegments.push_back(segments[i]);
  }

  target.clear();
  for(unsigned j = 0; j < normalizedSegments.size(); ++j)
  {
    if (!target.empty())
      target.append("/");
    target.append(normalizedSegments[j]);
  }

  VSD_DEBUG_MSG(("VSDXRelationship::rebaseTarget %s -> %s\n", m_target.c_str(), target.c_str()));
  m_target = target;
}


// VSDXRelationships

libvisio::VSDXRelationships::VSDXRelationships(xmlTextReaderPtr reader)
  : m_relsByType(), m_relsById()
{
  parseRelationships(reader);
}

libvisio::VSDXRelationships::VSDXRelationships(WPXInputStream *input)
  : m_relsByType(), m_relsById()
{
  if (input)
  {
    xmlTextReaderPtr reader = xmlReaderForIO(vsdxInputReadFunc, vsdxInputCloseFunc, (void *)input, NULL, NULL, 0);
    parseRelationships(reader);
  }
}

libvisio::VSDXRelationships::~VSDXRelationships()
{
}

void libvisio::VSDXRelationships::rebaseTargets(const char *baseDir)
{
  std::map<std::string, libvisio::VSDXRelationship>::iterator iter;
  for (iter = m_relsByType.begin(); iter != m_relsByType.end(); ++iter)
    iter->second.rebaseTarget(baseDir);
  for (iter = m_relsById.begin(); iter != m_relsById.end(); ++iter)
    iter->second.rebaseTarget(baseDir);
}

const libvisio::VSDXRelationship *libvisio::VSDXRelationships::getRelationshipByType(const char *type) const
{
  std::map<std::string, libvisio::VSDXRelationship>::const_iterator iter = m_relsByType.find(type);
  if (iter != m_relsByType.end())
    return &(iter->second);
  return 0;
}

const libvisio::VSDXRelationship *libvisio::VSDXRelationships::getRelationshipById(const char *type) const
{
  std::map<std::string, libvisio::VSDXRelationship>::const_iterator iter = m_relsById.find(type);
  if (iter != m_relsById.end())
    return &(iter->second);
  return 0;
}

void libvisio::VSDXRelationships::parseRelationships(xmlTextReaderPtr reader)
{
  if (reader)
  {
    bool inRelationships = false;
    int ret = xmlTextReaderRead(reader);
    while (ret == 1)
    {
      xmlChar *name = xmlTextReaderName(reader);
      if (name)
      {
        if (xmlStrEqual(name, BAD_CAST("Relationships")))
        {
          if (xmlTextReaderNodeType(reader) == 1)
          {
            VSD_DEBUG_MSG(("Relationships ON\n"));
            inRelationships = true;
          }
          else if (xmlTextReaderNodeType(reader) == 15)
          {
            VSD_DEBUG_MSG(("Relationships OFF\n"));
            inRelationships = false;
          }
        }
        else if (xmlStrEqual(name, BAD_CAST("Relationship")))
        {
          if (inRelationships)
          {
            VSDXRelationship relationship(reader);
            m_relsByType[relationship.getType()] = relationship;
            m_relsById[relationship.getId()] = relationship;
          }
        }
      }
      xmlFree(name);
      ret = xmlTextReaderRead(reader);
    }
  }
}

libvisio::VSDXParser::VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : m_input(0), m_painter(painter), m_collector(), m_stencils(), m_extractStencils(false)
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

  // TODO: put here the real document.xml parsing instructions

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

  // TODO: put here the real masters.xml parsing instructions

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
  WPXInputStream *relStream = input->getDocumentOLEStream(getRelationshipsForTarget(name).c_str());
  input->seek(0, WPX_SEEK_SET);
  VSDXRelationships rels(relStream);
  if (relStream)
    delete relStream;
  rels.rebaseTargets(getTargetBaseDirectory(name).c_str());

  // TODO: put here the real masterN.xml parsing instructions

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

  // TODO: put here the real pages.xml parsing instructions

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

  // TODO: put here the real pageN.xml parsing instructions

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

  // TODO: put here the real themeN.xml parsing instructions

  delete stream;
  return true;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
