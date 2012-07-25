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

#include "VSDXPages.h"
#include "libvisio_utils.h"

libvisio::VSDXPage::VSDXPage()
  : m_pageWidth(0.0), m_pageHeight(0.0),
    m_currentPageID(0), m_backgroundPageID(0xffffffff),
    m_pageElements()
{
}

libvisio::VSDXPage::VSDXPage(const libvisio::VSDXPage &page)
  : m_pageWidth(page.m_pageWidth), m_pageHeight(page.m_pageHeight),
    m_currentPageID(page.m_currentPageID), m_backgroundPageID(page.m_backgroundPageID),
    m_pageElements(page.m_pageElements)
{
}

libvisio::VSDXPage::~VSDXPage()
{
}

libvisio::VSDXPage &libvisio::VSDXPage::operator=(const libvisio::VSDXPage &page)
{
  if (this != &page)
  {
    m_pageWidth = page.m_pageWidth;
    m_pageHeight = page.m_pageHeight;
    m_currentPageID = page.m_currentPageID;
    m_backgroundPageID = page.m_backgroundPageID;
    m_pageElements = page.m_pageElements;
  }
  return *this;
}

void libvisio::VSDXPage::append(const libvisio::VSDXOutputElementList &outputElements)
{
  m_pageElements.append(outputElements);
}

void libvisio::VSDXPage::draw(libwpg::WPGPaintInterface *painter) const
{
  if (painter)
    m_pageElements.draw(painter);
}

libvisio::VSDXPages::VSDXPages()
  : m_pages()
{
}

void libvisio::VSDXPages::addPage(const libvisio::VSDXPage &page)
{
  m_pages[page.m_currentPageID] = page;
}

void libvisio::VSDXPages::draw(libwpg::WPGPaintInterface *painter)
{
  if (!painter)
    return;

  for (std::map<unsigned, libvisio::VSDXPage>::iterator iter = m_pages.begin(); iter != m_pages.end(); ++iter)
  {
    WPXPropertyList pageProps;
    pageProps.insert("svg:width", iter->second.m_pageWidth);
    pageProps.insert("svg:height", iter->second.m_pageHeight);
    painter->startGraphics(pageProps);
    _drawWithBackground(painter, iter->second);
    painter->endGraphics();
  }
}

void libvisio::VSDXPages::_drawWithBackground(libwpg::WPGPaintInterface *painter, const libvisio::VSDXPage &page)
{
  if (!painter)
    return;

  if (page.m_backgroundPageID != 0xffffffff)
  {
    std::map<unsigned, libvisio::VSDXPage>::iterator iter = m_pages.find(page.m_backgroundPageID);
    if (iter != m_pages.end())
      _drawWithBackground(painter, iter->second);
  }
  page.draw(painter);
}


libvisio::VSDXPages::~VSDXPages()
{
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
