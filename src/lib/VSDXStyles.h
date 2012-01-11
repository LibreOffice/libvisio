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

#ifndef __VSDXSTYLES_H__
#define __VSDXSTYLES_H__

#include <map>
#include <vector>
#include <libwpg/libwpg.h>
#include "VSDXTypes.h"

namespace libvisio
{

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

struct VSDXCharStyle
{
  VSDXCharStyle() :
    charCount(0),
    faceID(0),
    colour(),
    langID(0),
    size(12.0/72.0),
    bold(false),
    italic(false),
    underline(false),
    doubleunderline(false),
    strikeout(false),
    doublestrikeout(false),
    allcaps(false),
    initcaps(false),
    smallcaps(false),
    superscript(false),
    subscript(false),
    face("Arial") {}
  VSDXCharStyle(unsigned cc, unsigned short id, Colour c, unsigned lang, double s, bool b, bool i, bool u, bool du, bool so, bool dso, bool ac, bool ic, bool sc, bool super, bool sub, WPXString f) :
    charCount(cc),
    faceID(id),
    colour(c),
    langID(lang),
    size(s),
    bold(b),
    italic(i),
    underline(u),
    doubleunderline(du),
    strikeout(so),
    doublestrikeout(dso),
    allcaps(ac),
    initcaps(ic),
    smallcaps(sc),
    superscript(super),
    subscript(sub),
    face(f) {}
  ~VSDXCharStyle() {}
  unsigned charCount;
  unsigned short faceID;
  Colour colour;
  unsigned langID;
  double size;
  bool bold;
  bool italic;
  bool underline;
  bool doubleunderline;
  bool strikeout;
  bool doublestrikeout;
  bool allcaps;
  bool initcaps;
  bool smallcaps;
  bool superscript;
  bool subscript;
  WPXString face;
};

struct VSDXParaStyle
{
  VSDXParaStyle() :
    charCount(0),
    indFirst(0.0),
    indLeft(0.0),
    indRight(0.0),
    spLine(-1.2),
    spBefore(0.0),
    spAfter(0.0),
    align(1),
    flags(0) {}
  VSDXParaStyle(unsigned cc, double ifst, double il, double ir, double sl, double sb, double sa, unsigned char a, unsigned f) :
    charCount(cc),
    indFirst(ifst),
    indLeft(il),
    indRight(ir),
    spLine(sl),
    spBefore(sb),
    spAfter(sa),
    align(a),
    flags(f) {}
  ~VSDXParaStyle() {}
  unsigned charCount;
  double indFirst;
  double indLeft;
  double indRight;
  double spLine;
  double spBefore;
  double spAfter;
  unsigned char align;
  unsigned flags;
};

struct VSDXTextBlockStyle
{
  VSDXTextBlockStyle() :
    leftMargin(0.0),
    rightMargin(0.0),
    topMargin(0.0),
    bottomMargin(0.0),
    verticalAlign(0),
    textBkgndColourId(0),
    textBkgndColour(0xff,0xff,0xff,0),
    defaultTabStop(0.5),
    textDirection(0) {}
  VSDXTextBlockStyle(double lm, double rm, double tm, double bm, unsigned char va, unsigned char bgClrId, Colour bgClr, double defTab, unsigned char td) :
    leftMargin(lm),
    rightMargin(rm),
    topMargin(tm),
    bottomMargin(bm),
    verticalAlign(va),
    textBkgndColourId(bgClrId),
    textBkgndColour(bgClr),
    defaultTabStop(defTab),
    textDirection(td) {}
  ~VSDXTextBlockStyle() {}
  double leftMargin;
  double rightMargin;
  double topMargin;
  double bottomMargin;
  unsigned char verticalAlign;
  unsigned char textBkgndColourId;
  Colour textBkgndColour;
  double defaultTabStop;
  unsigned char textDirection;
};

class VSDXStyles
{
public:
  VSDXStyles();
  VSDXStyles(const VSDXStyles &styles);
  ~VSDXStyles();
  VSDXStyles &operator=(const VSDXStyles &styles);
  void addLineStyle(unsigned lineStyleIndex, VSDXLineStyle *lineStyle);
  void addFillStyle(unsigned fillStyleIndex, VSDXFillStyle *fillStyle);
  void addTextBlockStyle(unsigned textStyleIndex, VSDXTextBlockStyle *textBlockStyle);
  void addCharStyle(unsigned textStyleIndex, VSDXCharStyle *charStyle);
  void addParaStyle(unsigned textStyleIndex, VSDXParaStyle *paraStyle);

  void addLineStyleMaster(unsigned lineStyleIndex, unsigned lineStyleMaster);
  void addFillStyleMaster(unsigned fillStyleIndex, unsigned fillStyleMaster);
  void addTextStyleMaster(unsigned textStyleIndex, unsigned textStyleMaster);

  const VSDXLineStyle *getLineStyle(unsigned lineStyleIndex) const;
  const VSDXFillStyle *getFillStyle(unsigned fillStyleIndex) const;
  const VSDXTextBlockStyle *getTextBlockStyle(unsigned textStyleIndex) const;
  const VSDXCharStyle *getCharStyle(unsigned textStyleIndex) const;
  const VSDXParaStyle *getParaStyle(unsigned textStyleIndex) const;

private:
  std::map<unsigned, VSDXLineStyle *> m_lineStyles;
  std::map<unsigned, VSDXFillStyle *> m_fillStyles;
  std::map<unsigned, VSDXTextBlockStyle *> m_textBlockStyles;
  std::map<unsigned, VSDXCharStyle *> m_charStyles;
  std::map<unsigned, VSDXParaStyle *> m_paraStyles;
  std::map<unsigned, unsigned> m_lineStyleMasters;
  std::map<unsigned, unsigned> m_fillStyleMasters;
  std::map<unsigned, unsigned> m_textStyleMasters;
};


} // namespace libvisio

#endif // __VSDXSTYLES_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
