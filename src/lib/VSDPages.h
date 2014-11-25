/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
  void setMetaData(const librevenge::RVNGPropertyList &metaData);
private:
  void _drawWithBackground(librevenge::RVNGDrawingInterface *painter, const VSDPage &page);
  std::vector<VSDPage> m_pages;
  std::map<unsigned, VSDPage> m_backgroundPages;
  librevenge::RVNGPropertyList m_metaData;
};


} // namespace libvisio

#endif // __VSDPAGES_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
