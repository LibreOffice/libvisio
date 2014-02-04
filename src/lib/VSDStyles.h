/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDSTYLES_H__
#define __VSDSTYLES_H__

#include <map>
#include <vector>
#include <boost/optional.hpp>
#include "VSDTypes.h"

namespace libvisio
{

struct VSDOptionalThemeReference
{
  VSDOptionalThemeReference() :
    qsLineColour(), qsFillColour(), qsShadowColour(), qsFontColour() {}
  VSDOptionalThemeReference(const boost::optional<long> &lineColour, const boost::optional<long> &fillColour,
                            const boost::optional<long> &shadowColour, const boost::optional<long> &fontColour) :
    qsLineColour(lineColour), qsFillColour(fillColour), qsShadowColour(shadowColour), qsFontColour(fontColour) {}
  VSDOptionalThemeReference(const VSDOptionalThemeReference &themeRef) :
    qsLineColour(themeRef.qsLineColour), qsFillColour(themeRef.qsFillColour),
    qsShadowColour(themeRef.qsShadowColour), qsFontColour(themeRef.qsFontColour) {}
  ~VSDOptionalThemeReference() {}
  void override(const VSDOptionalThemeReference &themeRef)
  {
    ASSIGN_OPTIONAL(themeRef.qsLineColour, qsLineColour);
    ASSIGN_OPTIONAL(themeRef.qsFillColour, qsFillColour);
    ASSIGN_OPTIONAL(themeRef.qsShadowColour, qsShadowColour);
    ASSIGN_OPTIONAL(themeRef.qsFontColour, qsFontColour);
  }

  boost::optional<long> qsLineColour;
  boost::optional<long> qsFillColour;
  boost::optional<long> qsShadowColour;
  boost::optional<long> qsFontColour;
};

struct VSDThemeReference
{
  VSDThemeReference() :
    qsLineColour(-1), qsFillColour(-1), qsShadowColour(-1), qsFontColour(-1) {}
  VSDThemeReference(long lineColour, long fillColour, long shadowColour, long fontColour) :
    qsLineColour(lineColour), qsFillColour(fillColour), qsShadowColour(shadowColour), qsFontColour(fontColour) {}
  VSDThemeReference(const VSDThemeReference &themeRef) :
    qsLineColour(themeRef.qsLineColour), qsFillColour(themeRef.qsFillColour),
    qsShadowColour(themeRef.qsShadowColour), qsFontColour(themeRef.qsFontColour) {}
  ~VSDThemeReference() {}
  void override(const VSDOptionalThemeReference &themeRef)
  {
    ASSIGN_OPTIONAL(themeRef.qsLineColour, qsLineColour);
    ASSIGN_OPTIONAL(themeRef.qsFillColour, qsFillColour);
    ASSIGN_OPTIONAL(themeRef.qsShadowColour, qsShadowColour);
    ASSIGN_OPTIONAL(themeRef.qsFontColour, qsFontColour);
  }

  long qsLineColour;
  long qsFillColour;
  long qsShadowColour;
  long qsFontColour;
};

struct VSDOptionalLineStyle
{
  VSDOptionalLineStyle() :
    width(), colour(), pattern(), startMarker(), endMarker(), cap() {}
  VSDOptionalLineStyle(const boost::optional<double> &w, const boost::optional<Colour> &col,
                       const boost::optional<unsigned char> &p, const boost::optional<unsigned char> &sm,
                       const boost::optional<unsigned char> &em, const boost::optional<unsigned char> &c) :
    width(w), colour(col), pattern(p), startMarker(sm), endMarker(em), cap(c) {}
  VSDOptionalLineStyle(const VSDOptionalLineStyle &style) :
    width(style.width), colour(style.colour), pattern(style.pattern), startMarker(style.startMarker),
    endMarker(style.endMarker), cap(style.cap) {}
  ~VSDOptionalLineStyle() {}
  void override(const VSDOptionalLineStyle &style)
  {
    ASSIGN_OPTIONAL(style.width, width);
    ASSIGN_OPTIONAL(style.colour, colour);
    ASSIGN_OPTIONAL(style.pattern, pattern);
    ASSIGN_OPTIONAL(style.startMarker, startMarker);
    ASSIGN_OPTIONAL(style.endMarker, endMarker);
    ASSIGN_OPTIONAL(style.cap, cap);
  }

  boost::optional<double> width;
  boost::optional<Colour> colour;
  boost::optional<unsigned char> pattern;
  boost::optional<unsigned char> startMarker;
  boost::optional<unsigned char> endMarker;
  boost::optional<unsigned char> cap;
};

struct VSDLineStyle
{
  VSDLineStyle() :
    width(0.01), colour(), pattern(1), startMarker(0), endMarker(0), cap(0) {}
  VSDLineStyle(double w, Colour col, unsigned char p, unsigned char sm,
               unsigned char em, unsigned char c) :
    width(w), colour(col), pattern(p), startMarker(sm), endMarker(em), cap(c) {}
  VSDLineStyle(const VSDLineStyle &style) :
    width(style.width), colour(style.colour), pattern(style.pattern), startMarker(style.startMarker),
    endMarker(style.endMarker), cap(style.cap) {}
  ~VSDLineStyle() {}
  void override(const VSDOptionalLineStyle &style)
  {
    ASSIGN_OPTIONAL(style.width, width);
    ASSIGN_OPTIONAL(style.colour, colour);
    ASSIGN_OPTIONAL(style.pattern, pattern);
    ASSIGN_OPTIONAL(style.startMarker, startMarker);
    ASSIGN_OPTIONAL(style.endMarker, endMarker);
    ASSIGN_OPTIONAL(style.cap, cap);
  }

  double width;
  Colour colour;
  unsigned char pattern;
  unsigned char startMarker;
  unsigned char endMarker;
  unsigned char cap;
};

struct VSDOptionalFillStyle
{
  VSDOptionalFillStyle() :
    fgColour(), bgColour(), pattern(), fgTransparency(), bgTransparency(), shadowFgColour(),
    shadowPattern(), shadowOffsetX(), shadowOffsetY() {}
  VSDOptionalFillStyle(const boost::optional<Colour> &fgc, const boost::optional<Colour> &bgc,
                       const boost::optional<unsigned char> &p, const boost::optional<double> &fga,
                       const boost::optional<double> &bga, const boost::optional<Colour> &sfgc,
                       const boost::optional<unsigned char> &shp, const boost::optional<double> &shX,
                       const boost::optional<double> &shY) :
    fgColour(fgc), bgColour(bgc), pattern(p), fgTransparency(fga), bgTransparency(bga),
    shadowFgColour(sfgc), shadowPattern(shp), shadowOffsetX(shX), shadowOffsetY(shY) {}
  VSDOptionalFillStyle(const VSDOptionalFillStyle &style) :
    fgColour(style.fgColour), bgColour(style.bgColour), pattern(style.pattern), fgTransparency(style.fgTransparency),
    bgTransparency(style.bgTransparency), shadowFgColour(style.shadowFgColour), shadowPattern(style.shadowPattern),
    shadowOffsetX(style.shadowOffsetX), shadowOffsetY(style.shadowOffsetY) {}
  ~VSDOptionalFillStyle() {}
  void override(const VSDOptionalFillStyle &style)
  {
    ASSIGN_OPTIONAL(style.fgColour, fgColour);
    ASSIGN_OPTIONAL(style.bgColour, bgColour);
    ASSIGN_OPTIONAL(style.pattern, pattern);
    ASSIGN_OPTIONAL(style.fgTransparency, fgTransparency);
    ASSIGN_OPTIONAL(style.bgTransparency, bgTransparency);
    ASSIGN_OPTIONAL(style.shadowFgColour, shadowFgColour);
    ASSIGN_OPTIONAL(style.shadowPattern, shadowPattern);
    ASSIGN_OPTIONAL(style.shadowOffsetX, shadowOffsetX);
    ASSIGN_OPTIONAL(style.shadowOffsetY, shadowOffsetY);
  }

  boost::optional<Colour> fgColour;
  boost::optional<Colour> bgColour;
  boost::optional<unsigned char> pattern;
  boost::optional<double> fgTransparency;
  boost::optional<double> bgTransparency;
  boost::optional<Colour> shadowFgColour;
  boost::optional<unsigned char> shadowPattern;
  boost::optional<double> shadowOffsetX;
  boost::optional<double> shadowOffsetY;
};

struct VSDFillStyle
{
  VSDFillStyle()
    : fgColour(), bgColour(0xff, 0xff, 0xff, 0), pattern(0), fgTransparency(0), bgTransparency(0), shadowFgColour(),
      shadowPattern(0), shadowOffsetX(0), shadowOffsetY(0) {}
  VSDFillStyle(const Colour &fgc, const Colour &bgc, unsigned char p, double fga, double bga, const Colour &sfgc,
               unsigned char shp, double shX, double shY)
    : fgColour(fgc), bgColour(bgc), pattern(p), fgTransparency(fga), bgTransparency(bga),
      shadowFgColour(sfgc), shadowPattern(shp), shadowOffsetX(shX), shadowOffsetY(shY) {}
  VSDFillStyle(const VSDFillStyle &style) :
    fgColour(style.fgColour), bgColour(style.bgColour), pattern(style.pattern), fgTransparency(style.fgTransparency),
    bgTransparency(style.bgTransparency), shadowFgColour(style.shadowFgColour), shadowPattern(style.shadowPattern),
    shadowOffsetX(style.shadowOffsetX), shadowOffsetY(style.shadowOffsetY) {}
  ~VSDFillStyle() {}
  void override(const VSDOptionalFillStyle &style)
  {
    ASSIGN_OPTIONAL(style.fgColour, fgColour);
    ASSIGN_OPTIONAL(style.bgColour, bgColour);
    ASSIGN_OPTIONAL(style.pattern, pattern);
    ASSIGN_OPTIONAL(style.fgTransparency, fgTransparency);
    ASSIGN_OPTIONAL(style.bgTransparency, bgTransparency);
    ASSIGN_OPTIONAL(style.shadowFgColour, shadowFgColour);
    ASSIGN_OPTIONAL(style.shadowPattern, shadowPattern);
    ASSIGN_OPTIONAL(style.shadowOffsetX, shadowOffsetX);
    ASSIGN_OPTIONAL(style.shadowOffsetY, shadowOffsetY);
  }

  Colour fgColour;
  Colour bgColour;
  unsigned char pattern;
  double fgTransparency;
  double bgTransparency;
  Colour shadowFgColour;
  unsigned char shadowPattern;
  double shadowOffsetX;
  double shadowOffsetY;
};

struct VSDOptionalCharStyle
{
  VSDOptionalCharStyle()
    : charCount(0), font(), colour(), size(), bold(), italic(), underline(), doubleunderline(), strikeout(),
      doublestrikeout(), allcaps(), initcaps(), smallcaps(), superscript(), subscript() {}
  VSDOptionalCharStyle(unsigned cc, const boost::optional<VSDName> &ft,
                       const boost::optional<Colour> &c, const boost::optional<double> &s, const boost::optional<bool> &b,
                       const boost::optional<bool> &i, const boost::optional<bool> &u, const boost::optional<bool> &du,
                       const boost::optional<bool> &so, const boost::optional<bool> &dso, const boost::optional<bool> &ac,
                       const boost::optional<bool> &ic, const boost::optional<bool> &sc, const boost::optional<bool> &super,
                       const boost::optional<bool> &sub) :
    charCount(cc), font(ft), colour(c), size(s), bold(b), italic(i), underline(u), doubleunderline(du),
    strikeout(so), doublestrikeout(dso), allcaps(ac), initcaps(ic), smallcaps(sc), superscript(super),
    subscript(sub) {}
  VSDOptionalCharStyle(const VSDOptionalCharStyle &style) :
    charCount(style.charCount), font(style.font), colour(style.colour), size(style.size), bold(style.bold),
    italic(style.italic), underline(style.underline), doubleunderline(style.doubleunderline), strikeout(style.strikeout),
    doublestrikeout(style.doublestrikeout), allcaps(style.allcaps), initcaps(style.initcaps), smallcaps(style.smallcaps),
    superscript(style.superscript), subscript(style.subscript) {}
  ~VSDOptionalCharStyle() {}
  void override(const VSDOptionalCharStyle &style)
  {
    ASSIGN_OPTIONAL(style.font, font);
    ASSIGN_OPTIONAL(style.colour, colour);
    ASSIGN_OPTIONAL(style.size, size);
    ASSIGN_OPTIONAL(style.bold, bold);
    ASSIGN_OPTIONAL(style.italic, italic);
    ASSIGN_OPTIONAL(style.underline, underline);
    ASSIGN_OPTIONAL(style.doubleunderline, doubleunderline);
    ASSIGN_OPTIONAL(style.strikeout, strikeout);
    ASSIGN_OPTIONAL(style.doublestrikeout, doublestrikeout);
    ASSIGN_OPTIONAL(style.allcaps, allcaps);
    ASSIGN_OPTIONAL(style.initcaps, initcaps);
    ASSIGN_OPTIONAL(style.smallcaps, smallcaps);
    ASSIGN_OPTIONAL(style.superscript, superscript);
    ASSIGN_OPTIONAL(style.subscript, subscript);
  }

  unsigned charCount;
  boost::optional<VSDName> font;
  boost::optional<Colour> colour;
  boost::optional<double> size;
  boost::optional<bool> bold;
  boost::optional<bool> italic;
  boost::optional<bool> underline;
  boost::optional<bool> doubleunderline;
  boost::optional<bool> strikeout;
  boost::optional<bool> doublestrikeout;
  boost::optional<bool> allcaps;
  boost::optional<bool> initcaps;
  boost::optional<bool> smallcaps;
  boost::optional<bool> superscript;
  boost::optional<bool> subscript;
};

struct VSDCharStyle
{
  VSDCharStyle()
    : charCount(0), font(), colour(), size(12.0/72.0), bold(false), italic(false), underline(false),
      doubleunderline(false), strikeout(false), doublestrikeout(false), allcaps(false), initcaps(false),
      smallcaps(false), superscript(false), subscript(false) {}
  VSDCharStyle(unsigned cc, const VSDName &ft, const Colour &c, double s, bool b, bool i, bool u, bool du,
               bool so, bool dso, bool ac, bool ic, bool sc, bool super, bool sub) :
    charCount(cc), font(ft), colour(c), size(s), bold(b), italic(i), underline(u), doubleunderline(du),
    strikeout(so), doublestrikeout(dso), allcaps(ac), initcaps(ic), smallcaps(sc), superscript(super),
    subscript(sub) {}
  VSDCharStyle(const VSDCharStyle &style) :
    charCount(style.charCount), font(style.font), colour(style.colour), size(style.size), bold(style.bold),
    italic(style.italic), underline(style.underline), doubleunderline(style.doubleunderline), strikeout(style.strikeout),
    doublestrikeout(style.doublestrikeout), allcaps(style.allcaps), initcaps(style.initcaps), smallcaps(style.smallcaps),
    superscript(style.superscript), subscript(style.subscript) {}
  ~VSDCharStyle() {}
  void override(const VSDOptionalCharStyle &style)
  {
    ASSIGN_OPTIONAL(style.font, font);
    ASSIGN_OPTIONAL(style.colour, colour);
    ASSIGN_OPTIONAL(style.size, size);
    ASSIGN_OPTIONAL(style.bold, bold);
    ASSIGN_OPTIONAL(style.italic, italic);
    ASSIGN_OPTIONAL(style.underline, underline);
    ASSIGN_OPTIONAL(style.doubleunderline, doubleunderline);
    ASSIGN_OPTIONAL(style.strikeout, strikeout);
    ASSIGN_OPTIONAL(style.doublestrikeout, doublestrikeout);
    ASSIGN_OPTIONAL(style.allcaps, allcaps);
    ASSIGN_OPTIONAL(style.initcaps, initcaps);
    ASSIGN_OPTIONAL(style.smallcaps, smallcaps);
    ASSIGN_OPTIONAL(style.superscript, superscript);
    ASSIGN_OPTIONAL(style.subscript, subscript);
  }

  unsigned charCount;
  VSDName font;
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
};

struct VSDOptionalParaStyle
{
  VSDOptionalParaStyle() :
    charCount(0), indFirst(), indLeft(), indRight(), spLine(), spBefore(), spAfter(), align(), flags() {}
  VSDOptionalParaStyle(unsigned cc, const boost::optional<double> &ifst, const boost::optional<double> &il,
                       const boost::optional<double> &ir, const boost::optional<double> &sl, const boost::optional<double> &sb,
                       const boost::optional<double> &sa, const boost::optional<unsigned char> &a, const boost::optional<unsigned> &f) :
    charCount(cc), indFirst(ifst), indLeft(il), indRight(ir), spLine(sl), spBefore(sb), spAfter(sa), align(a), flags(f) {}
  VSDOptionalParaStyle(const VSDOptionalParaStyle &style) :
    charCount(style.charCount), indFirst(style.indFirst), indLeft(style.indLeft), indRight(style.indRight), spLine(style.spLine),
    spBefore(style.spBefore), spAfter(style.spAfter), align(style.align), flags(style.flags) {}
  ~VSDOptionalParaStyle() {}
  void override(const VSDOptionalParaStyle &style)
  {
    ASSIGN_OPTIONAL(style.indFirst, indFirst);
    ASSIGN_OPTIONAL(style.indLeft, indLeft);
    ASSIGN_OPTIONAL(style.indRight,indRight);
    ASSIGN_OPTIONAL(style.spLine, spLine);
    ASSIGN_OPTIONAL(style.spBefore, spBefore);
    ASSIGN_OPTIONAL(style.spAfter, spAfter);
    ASSIGN_OPTIONAL(style.align, align);
    ASSIGN_OPTIONAL(style.flags, flags);
  }

  unsigned charCount;
  boost::optional<double> indFirst;
  boost::optional<double> indLeft;
  boost::optional<double> indRight;
  boost::optional<double> spLine;
  boost::optional<double> spBefore;
  boost::optional<double> spAfter;
  boost::optional<unsigned char> align;
  boost::optional<unsigned> flags;
};

struct VSDParaStyle
{
  VSDParaStyle() :
    charCount(0), indFirst(0.0), indLeft(0.0), indRight(0.0), spLine(-1.2), spBefore(0.0), spAfter(0.0), align(1), flags(0) {}
  VSDParaStyle(unsigned cc, double ifst, double il, double ir, double sl, double sb,
               double sa, unsigned char a, unsigned f) :
    charCount(cc), indFirst(ifst), indLeft(il), indRight(ir), spLine(sl), spBefore(sb), spAfter(sa), align(a), flags(f) {}
  VSDParaStyle(const VSDParaStyle &style) :
    charCount(style.charCount), indFirst(style.indFirst), indLeft(style.indLeft), indRight(style.indRight), spLine(style.spLine),
    spBefore(style.spBefore), spAfter(style.spAfter), align(style.align), flags(style.flags) {}
  ~VSDParaStyle() {}
  void override(const VSDOptionalParaStyle &style)
  {
    ASSIGN_OPTIONAL(style.indFirst, indFirst);
    ASSIGN_OPTIONAL(style.indLeft, indLeft);
    ASSIGN_OPTIONAL(style.indRight,indRight);
    ASSIGN_OPTIONAL(style.spLine, spLine);
    ASSIGN_OPTIONAL(style.spBefore, spBefore);
    ASSIGN_OPTIONAL(style.spAfter, spAfter);
    ASSIGN_OPTIONAL(style.align, align);
    ASSIGN_OPTIONAL(style.flags, flags);
  }

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

struct VSDOptionalTextBlockStyle
{
  VSDOptionalTextBlockStyle() :
    leftMargin(), rightMargin(), topMargin(), bottomMargin(), verticalAlign(), isTextBkgndFilled(),
    textBkgndColour(), defaultTabStop(), textDirection() {}
  VSDOptionalTextBlockStyle(const boost::optional<double> &lm, const boost::optional<double> &rm,
                            const boost::optional<double> &tm, const boost::optional<double> &bm,
                            const boost::optional<unsigned char> &va, const boost::optional<bool> &isBgFilled,
                            const boost::optional<Colour> &bgClr, const boost::optional<double> &defTab,
                            const boost::optional<unsigned char> &td) :
    leftMargin(lm), rightMargin(rm), topMargin(tm), bottomMargin(bm), verticalAlign(va),
    isTextBkgndFilled(isBgFilled), textBkgndColour(bgClr), defaultTabStop(defTab), textDirection(td) {}
  VSDOptionalTextBlockStyle(const VSDOptionalTextBlockStyle &style) :
    leftMargin(style.leftMargin), rightMargin(style.rightMargin), topMargin(style.topMargin),
    bottomMargin(style.bottomMargin), verticalAlign(style.verticalAlign), isTextBkgndFilled(style.isTextBkgndFilled),
    textBkgndColour(style.textBkgndColour), defaultTabStop(style.defaultTabStop), textDirection(style.textDirection) {}
  ~VSDOptionalTextBlockStyle() {}
  void override(const VSDOptionalTextBlockStyle &style)
  {
    ASSIGN_OPTIONAL(style.leftMargin, leftMargin);
    ASSIGN_OPTIONAL(style.rightMargin, rightMargin);
    ASSIGN_OPTIONAL(style.topMargin, topMargin);
    ASSIGN_OPTIONAL(style.bottomMargin, bottomMargin);
    ASSIGN_OPTIONAL(style.verticalAlign, verticalAlign);
    ASSIGN_OPTIONAL(style.isTextBkgndFilled, isTextBkgndFilled);
    ASSIGN_OPTIONAL(style.textBkgndColour, textBkgndColour);
    ASSIGN_OPTIONAL(style.defaultTabStop, defaultTabStop);
    ASSIGN_OPTIONAL(style.textDirection, textDirection);
  }

  boost::optional<double> leftMargin;
  boost::optional<double> rightMargin;
  boost::optional<double> topMargin;
  boost::optional<double> bottomMargin;
  boost::optional<unsigned char> verticalAlign;
  boost::optional<bool> isTextBkgndFilled;
  boost::optional<Colour> textBkgndColour;
  boost::optional<double> defaultTabStop;
  boost::optional<unsigned char> textDirection;
};

struct VSDTextBlockStyle
{
  VSDTextBlockStyle() :
    leftMargin(0.0), rightMargin(0.0), topMargin(0.0), bottomMargin(0.0), verticalAlign(1),
    isTextBkgndFilled(true), textBkgndColour(0xff,0xff,0xff,0), defaultTabStop(0.5), textDirection(0) {}
  VSDTextBlockStyle(double lm, double rm, double tm, double bm, unsigned char va,
                    bool isBgFilled, Colour bgClr, double defTab, unsigned char td) :
    leftMargin(lm), rightMargin(rm), topMargin(tm), bottomMargin(bm), verticalAlign(va),
    isTextBkgndFilled(isBgFilled), textBkgndColour(bgClr), defaultTabStop(defTab), textDirection(td) {}
  VSDTextBlockStyle(const VSDTextBlockStyle &style) :
    leftMargin(style.leftMargin), rightMargin(style.rightMargin), topMargin(style.topMargin),
    bottomMargin(style.bottomMargin), verticalAlign(style.verticalAlign), isTextBkgndFilled(style.isTextBkgndFilled),
    textBkgndColour(style.textBkgndColour), defaultTabStop(style.defaultTabStop), textDirection(style.textDirection) {}
  ~VSDTextBlockStyle() {}
  void override(const VSDOptionalTextBlockStyle &style)
  {
    ASSIGN_OPTIONAL(style.leftMargin, leftMargin);
    ASSIGN_OPTIONAL(style.rightMargin, rightMargin);
    ASSIGN_OPTIONAL(style.topMargin, topMargin);
    ASSIGN_OPTIONAL(style.bottomMargin, bottomMargin);
    ASSIGN_OPTIONAL(style.verticalAlign, verticalAlign);
    ASSIGN_OPTIONAL(style.isTextBkgndFilled, isTextBkgndFilled);
    ASSIGN_OPTIONAL(style.textBkgndColour, textBkgndColour);
    ASSIGN_OPTIONAL(style.defaultTabStop, defaultTabStop);
    ASSIGN_OPTIONAL(style.textDirection, textDirection);
  }

  double leftMargin;
  double rightMargin;
  double topMargin;
  double bottomMargin;
  unsigned char verticalAlign;
  bool isTextBkgndFilled;
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
  void addLineStyle(unsigned lineStyleIndex, const VSDOptionalLineStyle &lineStyle);
  void addFillStyle(unsigned fillStyleIndex, const VSDOptionalFillStyle &fillStyle);
  void addTextBlockStyle(unsigned textStyleIndex, const VSDOptionalTextBlockStyle &textBlockStyle);
  void addCharStyle(unsigned textStyleIndex, const VSDOptionalCharStyle &charStyle);
  void addParaStyle(unsigned textStyleIndex, const VSDOptionalParaStyle &paraStyle);
  void addStyleThemeReference(unsigned styleIndex, const VSDOptionalThemeReference &themeRef);

  void addLineStyleMaster(unsigned lineStyleIndex, unsigned lineStyleMaster);
  void addFillStyleMaster(unsigned fillStyleIndex, unsigned fillStyleMaster);
  void addTextStyleMaster(unsigned textStyleIndex, unsigned textStyleMaster);

  VSDOptionalLineStyle getOptionalLineStyle(unsigned lineStyleIndex) const;
  VSDFillStyle getFillStyle(unsigned fillStyleIndex) const;
  VSDOptionalFillStyle getOptionalFillStyle(unsigned fillStyleIndex) const;
  VSDOptionalTextBlockStyle getOptionalTextBlockStyle(unsigned textStyleIndex) const;
  VSDOptionalCharStyle getOptionalCharStyle(unsigned textStyleIndex) const;
  VSDOptionalParaStyle getOptionalParaStyle(unsigned textStyleIndex) const;
  VSDOptionalThemeReference getOptionalThemeReference(unsigned styleIndex) const;

private:
  std::map<unsigned, VSDOptionalLineStyle> m_lineStyles;
  std::map<unsigned, VSDOptionalFillStyle> m_fillStyles;
  std::map<unsigned, VSDOptionalTextBlockStyle> m_textBlockStyles;
  std::map<unsigned, VSDOptionalCharStyle> m_charStyles;
  std::map<unsigned, VSDOptionalParaStyle> m_paraStyles;
  std::map<unsigned, VSDOptionalThemeReference> m_themeRefs;
  std::map<unsigned, unsigned> m_lineStyleMasters;
  std::map<unsigned, unsigned> m_fillStyleMasters;
  std::map<unsigned, unsigned> m_textStyleMasters;
};


} // namespace libvisio

#endif // __VSDSTYLES_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
