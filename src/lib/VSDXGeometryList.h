/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02111-1301 USA
 */

#ifndef __VSDXGEOMETRYLIST_H__
#define __VSDXGEOMETRYLIST_H__

#include <vector>
#include <map>
#include "VSDXCollector.h"

namespace libvisio {

class VSDXGeometryListElement;

class VSDXGeometryList
{
public:
  VSDXGeometryList();
  ~VSDXGeometryList();
  void addGeomList(unsigned id, unsigned level, const std::vector<unsigned> &geometryOrder);
  void addGeometry(unsigned id, unsigned level, unsigned geomFlags);
  void addMoveTo(unsigned id, unsigned level, double x, double y);
  void addLineTo(unsigned id, unsigned level, double x, double y);
  void addArcTo(unsigned id, unsigned level, double x2, double y2, double bow);
  void addEllipse(unsigned id, unsigned level, double cx, double cy, double aa, double dd);
  void addEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void setElementsOrder(const std::vector<unsigned> &m_elementsOrder);
  void handle(VSDXCollector *collector);
  void clear();
private:
  std::map<unsigned, VSDXGeometryListElement *> m_elements;
  std::vector<unsigned> m_elementsOrder;
};

} // namespace libvisio

#endif // __VSDXGEOMETRYLIST_H__
