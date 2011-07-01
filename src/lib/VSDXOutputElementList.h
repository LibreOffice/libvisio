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

#ifndef __VSDXOUTPUTELEMENTLIST_H__
#define __VSDXOUTPUTELEMENTLIST_H__

#include <map>
#include <list>
#include <vector>
#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>

namespace libvisio {

class VSDXOutputElement;

class VSDXOutputElementList
{
public:
  VSDXOutputElementList() : m_elements() {}
  virtual ~VSDXOutputElementList();
  void draw(libwpg::WPGPaintInterface *painter);
  void addStyle(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec);
  void addEllipse(const WPXPropertyList &propList);
  void addPath(const WPXPropertyListVector &propListVec);
  void addGraphicObject(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData);
  bool empty() const  { return !m_elements.size(); }
  void clear();
private:
  std::vector<VSDXOutputElement *> m_elements;
};


} // namespace libvisio

#endif // __VSDXOUTPUTELEMENTLIST_H__
