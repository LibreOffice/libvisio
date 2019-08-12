/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDXMLHelper.h"

#include <memory>
#include <sstream>
#include <istream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <librevenge-stream/librevenge-stream.h>
#include "libvisio_utils.h"
#include "libvisio_xml.h"

// VSDXRelationship

libvisio::VSDXRelationship::VSDXRelationship(xmlTextReaderPtr reader)
  : m_id(), m_type(), m_target()
{
  if (reader)
    // TODO: check whether we are actually parsing "Relationship" element
  {
    while (xmlTextReaderMoveToNextAttribute(reader))
    {
      const xmlChar *name = xmlTextReaderConstName(reader);
      const xmlChar *value = xmlTextReaderConstValue(reader);
      if (xmlStrEqual(name, BAD_CAST("Id")))
        m_id = (const char *)value;
      else if (xmlStrEqual(name, BAD_CAST("Type")))
        m_type = (const char *)value;
      else if (xmlStrEqual(name, BAD_CAST("Target")))
        m_target = (const char *)value;
    }
    // VSD_DEBUG_MSG(("Relationship : %s type: %s target: %s\n", m_id.c_str(), m_type.c_str(), m_target.c_str()));
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

  for (auto &segment : segments)
  {
    if (segment == "..")
      normalizedSegments.pop_back();
    else if (segment != "." && !segment.empty())
      normalizedSegments.push_back(segment);
  }

  target.clear();
  for (const auto &normalizedSegment : normalizedSegments)
  {
    if (!target.empty())
      target.append("/");
    target.append(normalizedSegment);
  }

  // VSD_DEBUG_MSG(("VSDXRelationship::rebaseTarget %s -> %s\n", m_target.c_str(), target.c_str()));
  m_target = target;
}


// VSDXRelationships

libvisio::VSDXRelationships::VSDXRelationships(librevenge::RVNGInputStream *input)
  : m_relsByType(), m_relsById()
{
  if (input)
  {
    auto reader = xmlReaderForStream(input);
    if (reader)
    {
      bool inRelationships = false;
      int ret = xmlTextReaderRead(reader.get());
      while (ret == 1)
      {
        const xmlChar *name = xmlTextReaderConstName(reader.get());
        if (name)
        {
          if (xmlStrEqual(name, BAD_CAST("Relationships")))
          {
            if (xmlTextReaderNodeType(reader.get()) == 1)
            {
              // VSD_DEBUG_MSG(("Relationships ON\n"));
              inRelationships = true;
            }
            else if (xmlTextReaderNodeType(reader.get()) == 15)
            {
              // VSD_DEBUG_MSG(("Relationships OFF\n"));
              inRelationships = false;
            }
          }
          else if (xmlStrEqual(name, BAD_CAST("Relationship")))
          {
            if (inRelationships)
            {
              VSDXRelationship relationship(reader.get());
              m_relsByType[relationship.getType()] = relationship;
              m_relsById[relationship.getId()] = relationship;
            }
          }
        }
        ret = xmlTextReaderRead(reader.get());
      }
    }
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
  if (!type)
    return nullptr;
  auto iter = m_relsByType.find(type);
  if (iter != m_relsByType.end())
    return &(iter->second);
  return nullptr;
}

const libvisio::VSDXRelationship *libvisio::VSDXRelationships::getRelationshipById(const char *id) const
{
  if (!id)
    return nullptr;
  auto iter = m_relsById.find(id);
  if (iter != m_relsById.end())
    return &(iter->second);
  return nullptr;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
