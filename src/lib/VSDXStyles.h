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

#ifndef __VSDXSTYLES_H__
#define __VSDXSTYLES_H__

#include <map>
#include <vector>
#include <libwpg/libwpg.h>
#include "VSDXTypes.h"

namespace libvisio {

struct VSDXLineStyle
{
  VSDXLineStyle()
  : width(0.01), colour(), pattern(0), startMarker(0), endMarker(0), cap(0) {}
  VSDXLineStyle(double w, Colour col, unsigned char p, unsigned char sm, 
                unsigned char em, unsigned char c)
  : width(w), colour(col), pattern(p), startMarker(sm), endMarker(em), cap(c) {}
  VSDXLineStyle(const VSDXLineStyle &lineStyle)
  : width(lineStyle.width), colour(lineStyle.colour), pattern(lineStyle.pattern), startMarker(lineStyle.startMarker), endMarker(lineStyle.endMarker), cap(lineStyle.cap) {}
  ~VSDXLineStyle() {}
  double width;
  Colour colour;
  unsigned char pattern;
  unsigned char startMarker;
  unsigned char endMarker;
  unsigned char cap;
};

struct VSDXFillStyle
{
  VSDXFillStyle()
  : fgColourId(1), bgColourId(0), pattern(0), fgTransparency(0), bgTransparency(0), shadowFgColour(), shadowPattern(0), shadowOffsetX(0), shadowOffsetY(0) {}
  VSDXFillStyle(unsigned char fgcId, unsigned char bgcId, unsigned char p, unsigned char fga, unsigned char bga, Colour sfgc, unsigned char shp, double shX, double shY)
  : fgColourId(fgcId), bgColourId(bgcId), pattern(p), fgTransparency(fga), bgTransparency(bga), shadowFgColour(sfgc), shadowPattern(shp), shadowOffsetX(shX), shadowOffsetY(shY) {}
  VSDXFillStyle(const VSDXFillStyle &fillStyle)
  : fgColourId(fillStyle.fgColourId), bgColourId(fillStyle.bgColourId), pattern(fillStyle.pattern),
    fgTransparency(fillStyle.fgTransparency), bgTransparency(fillStyle.bgTransparency), shadowFgColour(fillStyle.shadowFgColour),
    shadowPattern(fillStyle.shadowPattern), shadowOffsetX(fillStyle.shadowOffsetX), shadowOffsetY(fillStyle.shadowOffsetY) {}
  ~VSDXFillStyle() {}
  unsigned char fgColourId;
  //  Colour fgColour;
  unsigned char bgColourId;
  //  Colour bgColour;
  unsigned char pattern;

  unsigned char fgTransparency;
  unsigned char bgTransparency;

  Colour shadowFgColour;
  unsigned char shadowPattern;
  double shadowOffsetX;
  double shadowOffsetY;
};

struct VSDXTextStyle
{
  VSDXTextStyle()
  : characterFormat(), paragraphFormat(), txtBlockFormat() {}
  VSDXTextStyle(const CharFormat &cf, const ParaFormat &pf, const TextBlockFormat &tbf)
  : characterFormat(cf), paragraphFormat(pf), txtBlockFormat(tbf) {}
  VSDXTextStyle(const VSDXTextStyle &textStyle)
  : characterFormat(textStyle.characterFormat), paragraphFormat(textStyle.paragraphFormat), txtBlockFormat(textStyle.txtBlockFormat) {}
  ~VSDXTextStyle() {}
  CharFormat characterFormat;
  ParaFormat paragraphFormat;
  TextBlockFormat txtBlockFormat;
};

class VSDXStyles
{
  public:
    VSDXStyles();
    ~VSDXStyles();
    void addLineStyle(unsigned lineStyleIndex, VSDXLineStyle *lineStyle);
    void addFillStyle(unsigned fillStyleIndex, VSDXFillStyle *fillStyle);
    void addTextStyle(unsigned textStyleIndex, VSDXTextStyle *textStyle);
    
    void addLineStyleMaster(unsigned lineStyleIndex, unsigned lineStyleMaster);
    void addFillStyleMaster(unsigned fillStyleIndex, unsigned fillStyleMaster);
    void addTextStyleMaster(unsigned textStyleIndex, unsigned textStyleMaster);
    
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
