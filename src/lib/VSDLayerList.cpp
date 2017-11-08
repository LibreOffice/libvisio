/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDLayerList.h"

libvisio::VSDLayer::VSDLayer() : m_colour(), m_visible(1), m_printable(1) {}

libvisio::VSDLayer::VSDLayer(const VSDLayer &layer) :
  m_colour(layer.m_colour), m_visible(layer.m_visible), m_printable(layer.m_printable) {}

libvisio::VSDLayer::~VSDLayer() {}

libvisio::VSDLayer &libvisio::VSDLayer::operator=(const libvisio::VSDLayer &layer)
{
  if (this != &layer)
  {
    m_colour = layer.m_colour;
    m_visible = layer.m_visible;
    m_printable = layer.m_printable;
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
  for (auto iter = layerList.m_elements.begin(); iter != layerList.m_elements.end(); ++iter)
    m_elements[iter->first] = iter->second;
}

libvisio::VSDLayerList &libvisio::VSDLayerList::operator=(const libvisio::VSDLayerList &layerList)
{
  if (this != &layerList)
  {
    clear();
    for (auto iter = layerList.m_elements.begin(); iter != layerList.m_elements.end(); ++iter)
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
  for (unsigned int id : ids)
  {
    std::map<unsigned, libvisio::VSDLayer>::const_iterator iterMap = m_elements.find(id);
    // It is enough that one layer does not override colour and the original colour is used
    if (iterMap == m_elements.end() || !iterMap->second.m_colour)
      return nullptr;
    // This means we are reading the first layer and it overrides colour
    else if (iterColour == m_elements.end())
      iterColour = iterMap;
    // If two layers override colour to two different values, the original colour is used
    else if (!iterColour->second.m_colour || iterColour->second.m_colour.get() != iterMap->second.m_colour.get())
      return nullptr;
  }
  if (iterColour == m_elements.end())
    return nullptr;
  return iterColour->second.m_colour.get_ptr();
}

bool libvisio::VSDLayerList::getVisible(const std::vector<unsigned> &ids)
{
  if (ids.empty())
    return true;

  for (unsigned int id : ids)
  {
    std::map<unsigned, libvisio::VSDLayer>::const_iterator iterMap = m_elements.find(id);
    if (iterMap == m_elements.end())
      return true;
    else if (iterMap->second.m_visible)
      return true;
  }
  return false;
}

bool libvisio::VSDLayerList::getPrintable(const std::vector<unsigned> &ids)
{
  if (ids.empty())
    return true;

  for (unsigned int id : ids)
  {
    std::map<unsigned, libvisio::VSDLayer>::const_iterator iterMap = m_elements.find(id);
    if (iterMap == m_elements.end())
      return true;
    else if (iterMap->second.m_printable)
      return true;
  }
  return false;
}


/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
