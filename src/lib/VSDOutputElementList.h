/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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

#ifndef __VSDOUTPUTELEMENTLIST_H__
#define __VSDOUTPUTELEMENTLIST_H__

#include <map>
#include <list>
#include <vector>
#include <librevenge/librevenge.h>

namespace libvisio
{

class VSDOutputElement;

class VSDOutputElementList
{
public:
  VSDOutputElementList();
  VSDOutputElementList(const VSDOutputElementList &elementList);
  VSDOutputElementList &operator=(const VSDOutputElementList &elementList);
  virtual ~VSDOutputElementList();
  void append(const VSDOutputElementList &elementList);
  void draw(librevenge::RVNGDrawingInterface *painter) const;
  void addStyle(const librevenge::RVNGPropertyList &propList);
  void addPath(const librevenge::RVNGPropertyList &propList);
  void addGraphicObject(const librevenge::RVNGPropertyList &propList);
  void addStartTextObject(const librevenge::RVNGPropertyList &propList);
  void addOpenParagraph(const librevenge::RVNGPropertyList &propList);
  void addOpenSpan(const librevenge::RVNGPropertyList &propList);
  void addInsertText(const librevenge::RVNGString &text);
  void addCloseSpan();
  void addCloseParagraph();
  void addEndTextObject();
  void addStartLayer(const librevenge::RVNGPropertyList &propList);
  void addEndLayer();
  bool empty() const
  {
    return m_elements.empty();
  }
private:
  std::vector<VSDOutputElement *> m_elements;
};


} // namespace libvisio

#endif // __VSDOUTPUTELEMENTLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
