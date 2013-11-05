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

#ifndef __VSDPAGES_H__
#define __VSDPAGES_H__

#include "VSDOutputElementList.h"
#include "VSDTypes.h"

namespace libvisio
{

class VSDPage
{
public:
  VSDPage();
  VSDPage(const VSDPage &page);
  ~VSDPage();
  VSDPage &operator=(const VSDPage &page);
  void append(const VSDOutputElementList &outputElements);
  void draw(librevenge::RVNGDrawingInterface *painter) const;
  double m_pageWidth, m_pageHeight;
  librevenge::RVNGString m_pageName;
  unsigned m_currentPageID, m_backgroundPageID;
  VSDOutputElementList m_pageElements;
};

class VSDPages
{
public:
  VSDPages();
  ~VSDPages();
  void addPage(const VSDPage &page);
  void addBackgroundPage(const VSDPage &page);
  void draw(librevenge::RVNGDrawingInterface *painter);
private:
  void _drawWithBackground(librevenge::RVNGDrawingInterface *painter, const VSDPage &page);
  std::vector<VSDPage> m_pages;
  std::map<unsigned, VSDPage> m_backgroundPages;
};


} // namespace libvisio

#endif // __VSDPAGES_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
