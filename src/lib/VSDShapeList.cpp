/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDShapeList.h"

#include "VSDCollector.h"

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
    for (unsigned int i : m_elementsOrder)
    {
      iter = m_elements.find(i);
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
