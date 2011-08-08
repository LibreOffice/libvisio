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

#ifndef __VSDXCHARACTERLIST_H__
#define __VSDXCHARACTERLIST_H__

#include <vector>
#include <map>

namespace libvisio {

class VSDXCharacterListElement;
class VSDXCollector;

class VSDXCharacterList
{
public:
  VSDXCharacterList();
  VSDXCharacterList(const VSDXCharacterList &charList);
  ~VSDXCharacterList();
  VSDXCharacterList &operator=(const VSDXCharacterList &charList);
  void addCharIX(unsigned id, unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, unsigned langId, double fontSize, bool bold, bool italic, bool underline, WPXString fontFace);
  void setElementsOrder(const std::vector<unsigned> &m_elementsOrder);
  void handle(VSDXCollector *collector);
  void clear();
  bool empty() const { return (!m_elements.size()); }
  VSDXCharacterListElement *getElement(unsigned index);
private:
  std::map<unsigned, VSDXCharacterListElement *> m_elements;
  std::vector<unsigned> m_elementsOrder;
};

} // namespace libvisio

#endif // __VSDXCHARACTERLIST_H__
