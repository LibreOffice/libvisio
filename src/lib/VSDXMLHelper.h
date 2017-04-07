/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDXMLHELPER_H__
#define __VSDXMLHELPER_H__

#include <map>
#include <memory>
#include <string>

#include <librevenge-stream/librevenge-stream.h>
#include <libxml/xmlreader.h>
#include "VSDTypes.h"

namespace libvisio
{

class VSDCollector;

// Helper classes to properly handle OPC relationships

class VSDXRelationship
{
public:
  VSDXRelationship(xmlTextReaderPtr reader);
  VSDXRelationship();
  ~VSDXRelationship();

  void rebaseTarget(const char *baseDir);

  const std::string getId() const
  {
    return m_id;
  }
  const std::string getType() const
  {
    return m_type;
  }
  const std::string getTarget() const
  {
    return m_target;
  }

private:
  std::string m_id;
  std::string m_type;
  std::string m_target;
};

class VSDXRelationships
{
public:
  VSDXRelationships(librevenge::RVNGInputStream *input);
  ~VSDXRelationships();

  void rebaseTargets(const char *baseDir);

  const VSDXRelationship *getRelationshipByType(const char *type) const;
  const VSDXRelationship *getRelationshipById(const char *id) const;

  bool empty() const
  {
    return m_relsByType.empty() && m_relsById.empty();
  }

private:
  std::map<std::string, VSDXRelationship> m_relsByType;
  std::map<std::string, VSDXRelationship> m_relsById;
};

} // namespace libvisio

#endif // __VSDXMLHELPER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
