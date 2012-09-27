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

#ifndef __VSDSTYLES_H__
#define __VSDSTYLES_H__

#include <map>
#include <vector>
#include <libwpg/libwpg.h>
#include "VSDTypes.h"

namespace libvisio
{

struct VSDLineStyle
{
  VSDLineStyle()
    : width(0.01), colourId(0), pattern(0), startMarker(0), endMarker(0), cap(0) {}
  VSDLineStyle(double w, unsigned char col, unsigned char p, unsigned char sm,
               unsigned char em, unsigned char c)
    : width(w), colourId(col), pattern(p), startMarker(sm), endMarker(em), cap(c) {}
  ~VSDLineStyle() {}
  double width;
  unsigned char colourId;
  unsigned char pattern;
  unsigned char startMarker;
  unsigned char endMarker;
  unsigned char cap;
};

struct VSDFillStyle
{
  VSDFillStyle()
    : fgColourId(1), bgColourId(0), pattern(0), fgTransparency(0), bgTransparency(0), shadowPattern(0),
      shadowFgColourId(), shadowBgColourId(), shadowOffsetX(0), shadowOffsetY(0) {}
  VSDFillStyle(unsigned char fgcId, unsigned char bgcId, unsigned char p, unsigned char fga, unsigned char bga,
               unsigned char shp, unsigned char sfgcId, unsigned char sbgcId, double shX, double shY)
    : fgColourId(fgcId), bgColourId(bgcId), pattern(p), fgTransparency(fga), bgTransparency(bga),
      shadowPattern(shp), shadowFgColourId(sfgcId), shadowBgColourId(sbgcId), shadowOffsetX(shX), shadowOffsetY(shY) {}
  ~VSDFillStyle() {}
  unsigned char fgColourId;
  //  Colour fgColour;
  unsigned char bgColourId;
  //  Colour bgColour;
  unsigned char pattern;

  unsigned char fgTransparency;
  unsigned char bgTransparency;

  unsigned char shadowPattern;
  unsigned char shadowFgColourId;
  unsigned char shadowBgColourId;
  double shadowOffsetX;
  double shadowOffsetY;
};

struct VSDCharStyle
{
  VSDCharStyle() :
    charCount(0),
    faceID(0),
    colour(),
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
    face() {}
  VSDCharStyle(unsigned cc, unsigned short id, Colour c, double s, bool b, bool i, bool u, bool du, bool so, bool dso, bool ac, bool ic, bool sc, bool super, bool sub, VSDFont f) :
    charCount(cc),
    faceID(id),
    colour(c),
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
  ~VSDCharStyle() {}
  unsigned charCount;
  unsigned short faceID;
  Colour colour;
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
  VSDFont face;
};

struct VSDParaStyle
{
  VSDParaStyle() :
    charCount(0),
    indFirst(0.0),
    indLeft(0.0),
    indRight(0.0),
    spLine(-1.2),
    spBefore(0.0),
    spAfter(0.0),
    align(1),
    flags(0) {}
  VSDParaStyle(unsigned cc, double ifst, double il, double ir, double sl, double sb, double sa, unsigned char a, unsigned f) :
    charCount(cc),
    indFirst(ifst),
    indLeft(il),
    indRight(ir),
    spLine(sl),
    spBefore(sb),
    spAfter(sa),
    align(a),
    flags(f) {}
  ~VSDParaStyle() {}
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

struct VSDTextBlockStyle
{
  VSDTextBlockStyle() :
    leftMargin(0.0),
    rightMargin(0.0),
    topMargin(0.0),
    bottomMargin(0.0),
    verticalAlign(0),
    textBkgndColourId(0),
    textBkgndColour(0xff,0xff,0xff,0),
    defaultTabStop(0.5),
    textDirection(0) {}
  VSDTextBlockStyle(double lm, double rm, double tm, double bm, unsigned char va, unsigned char bgClrId, Colour bgClr, double defTab, unsigned char td) :
    leftMargin(lm),
    rightMargin(rm),
    topMargin(tm),
    bottomMargin(bm),
    verticalAlign(va),
    textBkgndColourId(bgClrId),
    textBkgndColour(bgClr),
    defaultTabStop(defTab),
    textDirection(td) {}
  ~VSDTextBlockStyle() {}
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

class VSDStyles
{
public:
  VSDStyles();
  VSDStyles(const VSDStyles &styles);
  ~VSDStyles();
  VSDStyles &operator=(const VSDStyles &styles);
  void addLineStyle(unsigned lineStyleIndex, VSDLineStyle *lineStyle);
  void addFillStyle(unsigned fillStyleIndex, VSDFillStyle *fillStyle);
  void addTextBlockStyle(unsigned textStyleIndex, VSDTextBlockStyle *textBlockStyle);
  void addCharStyle(unsigned textStyleIndex, VSDCharStyle *charStyle);
  void addParaStyle(unsigned textStyleIndex, VSDParaStyle *paraStyle);

  void addLineStyleMaster(unsigned lineStyleIndex, unsigned lineStyleMaster);
  void addFillStyleMaster(unsigned fillStyleIndex, unsigned fillStyleMaster);
  void addTextStyleMaster(unsigned textStyleIndex, unsigned textStyleMaster);

  const VSDLineStyle *getLineStyle(unsigned lineStyleIndex) const;
  const VSDFillStyle *getFillStyle(unsigned fillStyleIndex) const;
  const VSDTextBlockStyle *getTextBlockStyle(unsigned textStyleIndex) const;
  const VSDCharStyle *getCharStyle(unsigned textStyleIndex) const;
  const VSDParaStyle *getParaStyle(unsigned textStyleIndex) const;

private:
  std::map<unsigned, VSDLineStyle *> m_lineStyles;
  std::map<unsigned, VSDFillStyle *> m_fillStyles;
  std::map<unsigned, VSDTextBlockStyle *> m_textBlockStyles;
  std::map<unsigned, VSDCharStyle *> m_charStyles;
  std::map<unsigned, VSDParaStyle *> m_paraStyles;
  std::map<unsigned, unsigned> m_lineStyleMasters;
  std::map<unsigned, unsigned> m_fillStyleMasters;
  std::map<unsigned, unsigned> m_textStyleMasters;
};


} // namespace libvisio

#endif // __VSDSTYLES_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
