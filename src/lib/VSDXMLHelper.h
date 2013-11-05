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

#ifndef __VSDXMLHELPER_H__
#define __VSDXMLHELPER_H__

#include <map>
#include <string>
#include <librevenge-stream/librevenge-stream.h>
#include <libxml/xmlreader.h>
#include "VSDTypes.h"

namespace libvisio
{

// create an xmlTextReader pointer from a librevenge::RVNGInputStream pointer
// needs to be freed using xmlTextReaderFree function.

xmlTextReaderPtr xmlReaderForStream(librevenge::RVNGInputStream *input,
                                    const char *URL,
                                    const char *encoding,
                                    int options);

Colour xmlStringToColour(const xmlChar *s);

long xmlStringToLong(const xmlChar *s);

double xmlStringToDouble(const xmlChar *s);

bool xmlStringToBool(const xmlChar *s);


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
