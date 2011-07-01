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

#ifndef __VSDXSHAPELIST_H__
#define __VSDXSHAPELIST_H__

#include <vector>
#include <map>

namespace libvisio {

class VSDXShapeListElement;

class VSDXShapeList
{
public:
  VSDXShapeList();
  ~VSDXShapeList();
  void addShapeId(unsigned id, unsigned level, unsigned shapeId);
  void setElementsOrder(const std::vector<unsigned> &elementsOrder);
  void handle(VSDXCollector *collector);
  void clear();
  bool empty() const { return (!m_elements.size()); }
private:
  std::map<unsigned, VSDXShapeListElement *> m_elements;
  std::vector<unsigned> m_elementsOrder;
};

} // namespace libvisio

#endif // __VSDXSHAPELIST_H__
