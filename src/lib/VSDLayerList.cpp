/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDLayerList.h"

libvisio::VSDLayer::VSDLayer() :
  m_colourId(MINUS_ONE), m_colour() {}

libvisio::VSDLayer::VSDLayer(const VSDLayer &layer) :
  m_colourId(layer.m_colourId), m_colour(layer.m_colour) {}

libvisio::VSDLayer::~VSDLayer() {}

libvisio::VSDLayer &libvisio::VSDLayer::operator=(const libvisio::VSDLayer &layer)
{
  if (this != &layer)
  {
    m_colourId = layer.m_colourId;
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

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
