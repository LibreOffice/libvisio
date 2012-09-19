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
#include "VSDXParser.h"
#include "libvisio_utils.h"

namespace
{

extern "C" {

  int vsdxInputCloseFunc(void * /* context */)
  {
    return 0;
  }

  int vsdxInputReadFunc(void *context, char *buffer, int len)
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

}

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

libvisio::VSDXRelationships::VSDXRelationships(xmlTextReaderPtr reader)
  : m_relsByType(), m_relsById()
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
            inRelationships = true;
          else if (xmlTextReaderNodeType(reader) == 15)
            inRelationships = false;
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

libvisio::VSDXRelationships::VSDXRelationships()
  : m_relsByType(), m_relsById()
{
}

libvisio::VSDXRelationships::~VSDXRelationships()
{
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

void libvisio::parseRelationships(WPXInputStream *input, VSDXRelationships &rels)
{
  if (!input)
    return;
  xmlTextReaderPtr reader = xmlReaderForIO(vsdxInputReadFunc, vsdxInputCloseFunc, (void *)input, NULL, NULL, 0);
  if (!reader)
    return;
  rels = VSDXRelationships(reader);
}


libvisio::VSDXParser::VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : m_input(input), m_painter(painter), m_extractStencils(false)
{}

libvisio::VSDXParser::~VSDXParser()
{
}

bool libvisio::VSDXParser::parseMain()
{
  if (!m_input)
  {
    return false;
  }

  return true;
}

bool libvisio::VSDXParser::extractStencils()
{
  m_extractStencils = true;
  return parseMain();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
