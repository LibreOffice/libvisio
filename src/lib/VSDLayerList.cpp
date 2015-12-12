/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDLayerList.h"

libvisio::VSDLayer::VSDLayer() : m_colour() {}

libvisio::VSDLayer::VSDLayer(const VSDLayer &layer) : m_colour(layer.m_colour) {}

libvisio::VSDLayer::~VSDLayer() {}

libvisio::VSDLayer &libvisio::VSDLayer::operator=(const libvisio::VSDLayer &layer)
{
  if (this != &layer)
  {
    m_colour = layer.m_colour;
  }
  return *this;
}

libvisio::VSDLayerList::VSDLayerList() :
  m_elements()
{
}

libvisio::VSDLayerList::VSDLayerList(const libvisio::VSDLayerList &layerList) :
  m_elements()
{
  std::map<unsigned, VSDLayer>::const_iterator iter = layerList.m_elements.begin();
  for (; iter != layerList.m_elements.end(); ++iter)
    m_elements[iter->first] = iter->second;
}

libvisio::VSDLayerList &libvisio::VSDLayerList::operator=(const libvisio::VSDLayerList &layerList)
{
  if (this != &layerList)
  {
    clear();
    std::map<unsigned, VSDLayer>::const_iterator iter = layerList.m_elements.begin();
    for (; iter != layerList.m_elements.end(); ++iter)
      m_elements[iter->first] = iter->second;
  }
  return *this;
}

libvisio::VSDLayerList::~VSDLayerList()
{
  clear();
}

void libvisio::VSDLayerList::clear()
{
  m_elements.clear();
}

void libvisio::VSDLayerList::addLayer(unsigned id, const libvisio::VSDLayer &layer)
{
  m_elements[id] = layer;
}

const libvisio::Colour *libvisio::VSDLayerList::getColour(const std::vector<unsigned> &ids)
{
  std::map<unsigned, libvisio::VSDLayer>::const_iterator iterColour = m_elements.end();
  for (std::vector<unsigned>::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
  {
    std::map<unsigned, libvisio::VSDLayer>::const_iterator iterMap = m_elements.find(*iter);
    // It is enough that one layer does not override colour and the original colour is used
    if (!iterMap->second.m_colour)
      return 0;
    // This means we are reading the first layer and it overrides colour
    else if (iterColour == m_elements.end())
      iterColour = iterMap;
    // If two layers override colour to two different values, the original colour is used
    else if (!iterColour->second.m_colour || iterColour->second.m_colour.get() != iterMap->second.m_colour.get())
      return 0;
  }
  if (iterColour == m_elements.end())
    return 0;
  return iterColour->second.m_colour.get_ptr();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
