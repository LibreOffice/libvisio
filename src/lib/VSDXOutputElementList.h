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
  VSDXOutputElementList();
  VSDXOutputElementList(const VSDXOutputElementList &elementList);
  VSDXOutputElementList &operator=(const VSDXOutputElementList &elementList);
  virtual ~VSDXOutputElementList();
  void append(const VSDXOutputElementList &elementList);
  void draw(libwpg::WPGPaintInterface *painter) const;
  void addStyle(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec);
  void addEllipse(const WPXPropertyList &propList);
  void addPath(const WPXPropertyListVector &propListVec);
  void addGraphicObject(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData);
  void addStartLayer(const WPXPropertyList &propList);
  void addEndLayer();
  void addStartTextObject(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec);
  void addStartTextSpan(const WPXPropertyList &propList);
  void addInsertText(const WPXString &text);
  void addEndTextSpan();
  void addEndTextObject();
  bool empty() const  { return !m_elements.size(); }
  void clear();
private:
  std::vector<VSDXOutputElement *> m_elements;
};


} // namespace libvisio

#endif // __VSDXOUTPUTELEMENTLIST_H__
