/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
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

#include "VSDXPages.h"
#include "libvisio_utils.h"

libvisio::VSDXPage::VSDXPage()
  : m_pageWidth(0.0), m_pageHeight(0.0),
    m_currentPageID(0), m_backgroundPageID(0xffffffff),
    m_pageElements()
{
}

libvisio::VSDXPage::VSDXPage(double pageWidth, double pageHeight, unsigned currentPageID, unsigned backgroundPageID, const libvisio::VSDXOutputElementList &pageElements)
  : m_pageWidth(pageWidth), m_pageHeight(pageHeight),
    m_currentPageID(currentPageID), m_backgroundPageID(backgroundPageID),
    m_pageElements(pageElements)
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
  m_pageWidth = page.m_pageWidth;
  m_pageHeight = page.m_pageHeight;
  m_currentPageID = page.m_currentPageID;
  m_backgroundPageID = page.m_backgroundPageID;
  m_pageElements = page.m_pageElements;
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
  m_pages[page.getCurrentPageID()] = page;
}

void libvisio::VSDXPages::draw(libwpg::WPGPaintInterface *painter)
{
  if (!painter)
    return;

  for (std::map<unsigned, libvisio::VSDXPage>::iterator iter = m_pages.begin(); iter != m_pages.end(); iter++)
  {
    WPXPropertyList pageProps;
    pageProps.insert("svg:width", iter->second.getPageWidth());
    pageProps.insert("svg:height", iter->second.getPageHeight());
    painter->startGraphics(pageProps);
    _drawWithBackground(painter, iter->second);
    painter->endGraphics();
  }
}

void libvisio::VSDXPages::_drawWithBackground(libwpg::WPGPaintInterface *painter, const libvisio::VSDXPage &page)
{
  if (!painter)
    return;

  if (page.getBackgroundPageID() != 0xffffffff)
  {
     std::map<unsigned, libvisio::VSDXPage>::iterator iter = m_pages.find(page.getBackgroundPageID());
     if (iter != m_pages.end())
       _drawWithBackground(painter, iter->second);
  }
  page.draw(painter);
}


libvisio::VSDXPages::~VSDXPages()
{
}

