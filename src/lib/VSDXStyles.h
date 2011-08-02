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

#ifndef __VSDXSTYLES_H__
#define __VSDXSTYLES_H__

#include <map>
#include <libwpg/libwpg.h>

namespace libvisio {

class VSDXLineStyle
{
  public:
    VSDXLineStyle() {};
    ~VSDXLineStyle() {};
    WPXPropertyList properties;
};

class VSDXFillStyle
{
  public:
    VSDXFillStyle() {};
    ~VSDXFillStyle() {};
    WPXPropertyList properties;
};

class VSDXTextStyle
{
  public:
    VSDXTextStyle() {};
    ~VSDXTextStyle() {};
    WPXPropertyList properties;
};

class VSDXStyles
{
  public:
    VSDXStyles();
    ~VSDXStyles();
	void addLineStyle(unsigned lineStyleIndex, VSDXLineStyle *lineStyle, unsigned lineStyleMaster);
	void addFillStyle(unsigned fillStyleIndex, VSDXFillStyle *fillStyle, unsigned fillStyleMaster);
	void addTextStyle(unsigned textStyleIndex, VSDXTextStyle *textStyle, unsigned textStyleMaster);
	
	const VSDXLineStyle getLineStyle(unsigned lineStyleIndex) const;
	const VSDXFillStyle getFillStyle(unsigned fillStyleIndex) const;
	const VSDXTextStyle getTextStyle(unsigned textStyleIndex) const;
  private:
    std::map<unsigned, VSDXLineStyle> m_lineStyles;
    std::map<unsigned, VSDXFillStyle> m_fillStyles;
    std::map<unsigned, VSDXTextStyle> m_textStyles;
    std::map<unsigned, unsigned> m_lineStyleMasters;
    std::map<unsigned, unsigned> m_fillStyleMasters;
    std::map<unsigned, unsigned> m_textStyleMasters;
};


} // namespace libvisio

#endif // __VSDXSTYLES_H__
