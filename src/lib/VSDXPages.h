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

#ifndef __VSDXPAGES_H__
#define __VSDXPAGES_H__

#include "VSDXOutputElementList.h"

namespace libvisio {

class VSDXPage
{
  public:
    VSDXPage();
    VSDXPage(double pageWidth, double pageHeight, unsigned currentPageID, unsigned backgroundPageID, const VSDXOutputElementList &pageElements);
    VSDXPage(const VSDXPage &page);
    ~VSDXPage();
    VSDXPage &operator=(const VSDXPage &page);
    void draw(libwpg::WPGPaintInterface *painter) const;
    double getPageWidth() const { return m_pageWidth; }
    double getPageHeight() const { return m_pageHeight; }
    unsigned getCurrentPageID() const { return m_currentPageID; }
    unsigned getBackgroundPageID() const { return m_backgroundPageID; }
  private:
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
