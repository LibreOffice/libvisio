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

#ifndef __VSDXPAGES_H__
#define __VSDXPAGES_H__

#include "VSDXOutputElementList.h"

namespace libvisio
{

class VSDXPage
{
public:
  VSDXPage();
  VSDXPage(double pageWidth, double pageHeight, unsigned currentPageID, unsigned backgroundPageID, const VSDXOutputElementList &pageElements);
  VSDXPage(const VSDXPage &page);
  ~VSDXPage();
  VSDXPage &operator=(const VSDXPage &page);
  void append(const VSDXOutputElementList &outputElements);
  void draw(libwpg::WPGPaintInterface *painter) const;
  double m_pageWidth, m_pageHeight;
  unsigned m_currentPageID, m_backgroundPageID;
  VSDXOutputElementList m_pageElements;
};

class VSDXPages
{
public:
  VSDXPages();
  ~VSDXPages();
  void addPage(const VSDXPage &page);
  void draw(libwpg::WPGPaintInterface *painter);
private:
  void _drawWithBackground(libwpg::WPGPaintInterface *painter, const VSDXPage &page);
  std::map<unsigned, VSDXPage> m_pages;
};


} // namespace libvisio

#endif // __VSDXPAGES_H__
