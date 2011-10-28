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
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
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

#include "VSDXCollector.h"
#include "VSDXShapeList.h"

namespace libvisio
{

class VSDXShapeListElement
{
public:
  VSDXShapeListElement() {}
  virtual ~VSDXShapeListElement() {}
  virtual void handle(VSDXCollector *collector) = 0;
};

class VSDXShapeId : public VSDXShapeListElement
{
public:
  VSDXShapeId(unsigned id, unsigned level, unsigned shapeId) :
    m_id(id), m_level(level), m_shapeId(shapeId) {}
  ~VSDXShapeId() {}
  void handle(VSDXCollector *collector);
private:
  unsigned m_id;
  unsigned m_level;
  unsigned m_shapeId;
};

} // namespace libvisio


void libvisio::VSDXShapeId::handle(VSDXCollector *collector)
{
  collector->collectShapeId(m_id, m_level, m_shapeId);
}

libvisio::VSDXShapeList::VSDXShapeList() :
  m_elements(),
  m_elementsOrder()
{
}

libvisio::VSDXShapeList::~VSDXShapeList()
{
  clear();
}

void libvisio::VSDXShapeList::addShapeId(unsigned id, unsigned level, unsigned shapeId)
{
  m_elements[id] = new VSDXShapeId(id, level, shapeId);
}

void libvisio::VSDXShapeList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDXShapeList::handle(VSDXCollector *collector)
{
  if (empty())
    return;
  std::map<unsigned, VSDXShapeListElement *>::iterator iter;
  if (m_elementsOrder.size())
  {
    for (unsigned i = 0; i < m_elementsOrder.size(); i++)
    {
      iter = m_elements.find(m_elementsOrder[i]);
      if (iter != m_elements.end())
        iter->second->handle(collector);
    }
  }
  else
  {
    for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
      iter->second->handle(collector);
  }
}

void libvisio::VSDXShapeList::clear()
{
  for (std::map<unsigned, VSDXShapeListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
