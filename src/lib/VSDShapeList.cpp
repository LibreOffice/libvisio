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

#include "VSDCollector.h"
#include "VSDShapeList.h"


libvisio::VSDShapeList::VSDShapeList() :
  m_elements(),
  m_elementsOrder(),
  m_shapesOrder()
{
}

libvisio::VSDShapeList::VSDShapeList(const VSDShapeList &shapeList) :
  m_elements(shapeList.m_elements),
  m_elementsOrder(shapeList.m_elementsOrder),
  m_shapesOrder(shapeList.m_shapesOrder)
{
}

libvisio::VSDShapeList &libvisio::VSDShapeList::operator=(const libvisio::VSDShapeList &shapeList)
{
  if (this != &shapeList)
  {
    m_elements = shapeList.m_elements;
    m_elementsOrder = shapeList.m_elementsOrder;
    m_shapesOrder = shapeList.m_shapesOrder;
  }
  return *this;
}

libvisio::VSDShapeList::~VSDShapeList()
{
  clear();
}

void libvisio::VSDShapeList::addShapeId(unsigned id, unsigned shapeId)
{
  m_elements[id] = shapeId;
}

void libvisio::VSDShapeList::addShapeId(unsigned shapeId)
{
  m_elements[shapeId] = shapeId;
  m_elementsOrder.push_back(shapeId);
}

void libvisio::VSDShapeList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder = elementsOrder;
}

const std::vector<unsigned> &libvisio::VSDShapeList::getShapesOrder()
{
  if (empty())
  {
    m_shapesOrder.clear();
    return m_shapesOrder;
  }
  if (!m_shapesOrder.empty())
    return m_shapesOrder;

  std::map<unsigned, unsigned>::const_iterator iter;
  if (!m_elementsOrder.empty())
  {
    for (unsigned i = 0; i < m_elementsOrder.size(); i++)
    {
      iter = m_elements.find(m_elementsOrder[i]);
      if (iter != m_elements.end())
        m_shapesOrder.push_back(iter->second);
    }
  }
  else
  {
    for (iter = m_elements.begin(); iter != m_elements.end(); ++iter)
      m_shapesOrder.push_back(iter->second);
  }
  return m_shapesOrder;
}

void libvisio::VSDShapeList::clear()
{
  m_elements.clear();
  m_elementsOrder.clear();
  m_shapesOrder.clear();
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
