/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDLAYERLIST_H__
#define __VSDLAYERLIST_H__

#include <map>
#include <vector>
#include "VSDTypes.h"

namespace libvisio
{

struct VSDLayer
{
  VSDLayer();
  VSDLayer(const VSDLayer &layer);
  ~VSDLayer();
  VSDLayer &operator=(const VSDLayer &layer);

  unsigned m_colourId;
  Colour m_colour;
};

class VSDLayerList
{
public:
  VSDLayerList();
  VSDLayerList(const VSDLayerList &layerList);
  ~VSDLayerList();
  VSDLayerList &operator=(const VSDLayerList &layerList);

  void clear();

  void addLayer(unsigned id, const VSDLayer &layer);

  unsigned getColourId(const std::vector<unsigned> &ids);

private:
  std::map<unsigned, VSDLayer> m_elements;
};




} // namespace libvisio

#endif // __VSDLAYERLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
