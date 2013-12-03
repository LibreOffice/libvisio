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

#include "VSDPages.h"
#include "libvisio_utils.h"

libvisio::VSDPage::VSDPage()
  : m_pageWidth(0.0), m_pageHeight(0.0), m_pageName(),
    m_currentPageID(0), m_backgroundPageID(MINUS_ONE),
    m_pageElements()
{
}

libvisio::VSDPage::VSDPage(const libvisio::VSDPage &page)
  : m_pageWidth(page.m_pageWidth), m_pageHeight(page.m_pageHeight), m_pageName(page.m_pageName),
    m_currentPageID(page.m_currentPageID), m_backgroundPageID(page.m_backgroundPageID),
    m_pageElements(page.m_pageElements)
{
}

libvisio::VSDPage::~VSDPage()
{
}

libvisio::VSDPage &libvisio::VSDPage::operator=(const libvisio::VSDPage &page)
{
  if (this != &page)
  {
    m_pageWidth = page.m_pageWidth;
    m_pageHeight = page.m_pageHeight;
    m_pageName = page.m_pageName;
    m_currentPageID = page.m_currentPageID;
    m_backgroundPageID = page.m_backgroundPageID;
    m_pageElements = page.m_pageElements;
  }
  return *this;
}

void libvisio::VSDPage::append(const libvisio::VSDOutputElementList &outputElements)
{
  m_pageElements.append(outputElements);
}

void libvisio::VSDPage::draw(librevenge::RVNGDrawingInterface *painter) const
{
  if (painter)
    m_pageElements.draw(painter);
}

libvisio::VSDPages::VSDPages()
  : m_pages(), m_backgroundPages()
{
}

void libvisio::VSDPages::addPage(const libvisio::VSDPage &page)
{
  m_pages.push_back(page);
}

void libvisio::VSDPages::addBackgroundPage(const libvisio::VSDPage &page)
{
  m_backgroundPages[page.m_currentPageID] = page;
}

void libvisio::VSDPages::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (!painter)
    return;
  if (m_pages.empty())
    return;

  painter->startDocument(librevenge::RVNGPropertyList());

  for (unsigned i = 0; i < m_pages.size(); ++i)
  {
    librevenge::RVNGPropertyList pageProps;
    pageProps.insert("svg:width", m_pages[i].m_pageWidth);
    pageProps.insert("svg:height", m_pages[i].m_pageHeight);
    if (m_pages[i].m_pageName.len())
      pageProps.insert("draw:name", m_pages[i].m_pageName);
    painter->startPage(pageProps);
    _drawWithBackground(painter, m_pages[i]);
    painter->endPage();
  }
  // Visio shows background pages in tabs after the normal pages
  for (std::map<unsigned, libvisio::VSDPage>::const_iterator iter = m_backgroundPages.begin();
       iter != m_backgroundPages.end(); ++iter)
  {
    librevenge::RVNGPropertyList pageProps;
    pageProps.insert("svg:width", iter->second.m_pageWidth);
    pageProps.insert("svg:height", iter->second.m_pageHeight);
    if (iter->second.m_pageName.len())
      pageProps.insert("draw:name", iter->second.m_pageName);
    painter->startPage(pageProps);
    _drawWithBackground(painter, iter->second);
    painter->endPage();
  }

  painter->endDocument();
}

void libvisio::VSDPages::_drawWithBackground(librevenge::RVNGDrawingInterface *painter, const libvisio::VSDPage &page)
{
  if (!painter)
    return;

  if (page.m_backgroundPageID != MINUS_ONE)
  {
    std::map<unsigned, libvisio::VSDPage>::iterator iter = m_backgroundPages.find(page.m_backgroundPageID);
    if (iter != m_backgroundPages.end())
      _drawWithBackground(painter, iter->second);
  }
  page.draw(painter);
}


libvisio::VSDPages::~VSDPages()
{
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
