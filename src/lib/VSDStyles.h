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
#include "VSDXTheme.h"
#include "libvisio_utils.h"

namespace libvisio
{

struct VSDOptionalLineStyle
{
  VSDOptionalLineStyle() :
    width(), colour(), pattern(), startMarker(), endMarker(), cap(), rounding(),
    qsLineColour(), qsLineMatrix() {}
  VSDOptionalLineStyle(const boost::optional<double> &w, const boost::optional<Colour> &col,
                       const boost::optional<unsigned char> &p, const boost::optional<unsigned char> &sm,
                       const boost::optional<unsigned char> &em, const boost::optional<unsigned char> &c,
                       const boost::optional<double> &r, const boost::optional<long> &qlc,
                       const boost::optional<long> &qlm) :
    width(w), colour(col), pattern(p), startMarker(sm), endMarker(em), cap(c), rounding(r),
    qsLineColour(qlc), qsLineMatrix(qlm) {}
  VSDOptionalLineStyle(const VSDOptionalLineStyle &style) = default;
  ~VSDOptionalLineStyle() {}
  VSDOptionalLineStyle &operator=(const VSDOptionalLineStyle &style) = default;
  void override(const VSDOptionalLineStyle &style)
  {
    ASSIGN_OPTIONAL(style.width, width);
    ASSIGN_OPTIONAL(style.colour, colour);
    ASSIGN_OPTIONAL(style.pattern, pattern);
    ASSIGN_OPTIONAL(style.startMarker, startMarker);
    ASSIGN_OPTIONAL(style.endMarker, endMarker);
    ASSIGN_OPTIONAL(style.cap, cap);
    ASSIGN_OPTIONAL(style.rounding, rounding);
    ASSIGN_OPTIONAL(style.qsLineColour, qsLineColour);
    ASSIGN_OPTIONAL(style.qsLineMatrix, qsLineMatrix);
  }

  boost::optional<double> width;
  boost::optional<Colour> colour;
  boost::optional<unsigned char> pattern;
  boost::optional<unsigned char> startMarker;
  boost::optional<unsigned char> endMarker;
  boost::optional<unsigned char> cap;
  boost::optional<double> rounding;
  boost::optional<long> qsLineColour;
  boost::optional<long> qsLineMatrix;
};

struct VSDLineStyle
{
  VSDLineStyle() :
    width(0.01), colour(), pattern(1), startMarker(0), endMarker(0), cap(0),
    rounding(0.0), qsLineColour(-1), qsLineMatrix(-1) {}
  VSDLineStyle(double w, Colour col, unsigned char p, unsigned char sm,
               unsigned char em, unsigned char c, double r, long qlc, long qlm) :
    width(w), colour(col), pattern(p), startMarker(sm), endMarker(em), cap(c),
    rounding(r), qsLineColour(qlc), qsLineMatrix(qlm) {}
  VSDLineStyle(const VSDLineStyle &style) = default;
  ~VSDLineStyle() {}
  VSDLineStyle &operator=(const VSDLineStyle &style) = default;
  void override(const VSDOptionalLineStyle &style, const VSDXTheme *theme)
  {
    ASSIGN_OPTIONAL(style.width, width);
    ASSIGN_OPTIONAL(style.pattern, pattern);
    ASSIGN_OPTIONAL(style.startMarker, startMarker);
    ASSIGN_OPTIONAL(style.endMarker, endMarker);
    ASSIGN_OPTIONAL(style.cap, cap);
    ASSIGN_OPTIONAL(style.rounding, rounding);
    ASSIGN_OPTIONAL(style.qsLineColour, qsLineColour);
    ASSIGN_OPTIONAL(style.qsLineMatrix, qsLineMatrix);
    if (theme)
    {
      if (!!style.qsLineColour && style.qsLineColour.get() >= 0)
        ASSIGN_OPTIONAL(theme->getThemeColour(style.qsLineColour.get()), colour);
    }
    ASSIGN_OPTIONAL(style.colour, colour);
  }

  double width;
  Colour colour;
  unsigned char pattern;
  unsigned char startMarker;
  unsigned char endMarker;
  unsigned char cap;
  double rounding;
  long qsLineColour;
  long qsLineMatrix;
};

struct VSDOptionalFillStyle
{
  VSDOptionalFillStyle() :
    fgColour(), bgColour(), pattern(), fgTransparency(), bgTransparency(), shadowFgColour(),
    shadowPattern(), shadowOffsetX(), shadowOffsetY(), qsFillColour(), qsShadowColour(),
    qsFillMatrix() {}
  VSDOptionalFillStyle(const boost::optional<Colour> &fgc, const boost::optional<Colour> &bgc,
                       const boost::optional<unsigned char> &p, const boost::optional<double> &fga,
                       const boost::optional<double> &bga, const boost::optional<Colour> &sfgc,
                       const boost::optional<unsigned char> &shp, const boost::optional<double> &shX,
                       const boost::optional<double> &shY, const boost::optional<long> &qsFc,
                       const boost::optional<long> &qsSc, const boost::optional<long> &qsFm) :
    fgColour(fgc), bgColour(bgc), pattern(p), fgTransparency(fga), bgTransparency(bga),
    shadowFgColour(sfgc), shadowPattern(shp), shadowOffsetX(shX), shadowOffsetY(shY),
    qsFillColour(qsFc), qsShadowColour(qsSc), qsFillMatrix(qsFm) {}
  VSDOptionalFillStyle(const VSDOptionalFillStyle &style) = default;
  ~VSDOptionalFillStyle() {}
  VSDOptionalFillStyle &operator=(const VSDOptionalFillStyle &style) = default;
  void override(const VSDOptionalFillStyle &style)
  {
    ASSIGN_OPTIONAL(style.pattern, pattern);
    ASSIGN_OPTIONAL(style.fgTransparency, fgTransparency);
    ASSIGN_OPTIONAL(style.bgTransparency, bgTransparency);
    ASSIGN_OPTIONAL(style.shadowPattern, shadowPattern);
    ASSIGN_OPTIONAL(style.shadowOffsetX, shadowOffsetX);
    ASSIGN_OPTIONAL(style.shadowOffsetY, shadowOffsetY);
    ASSIGN_OPTIONAL(style.qsFillColour, qsFillColour);
    ASSIGN_OPTIONAL(style.qsShadowColour, qsShadowColour);
    ASSIGN_OPTIONAL(style.qsFillMatrix, qsFillMatrix);
    ASSIGN_OPTIONAL(style.fgColour, fgColour);
    ASSIGN_OPTIONAL(style.bgColour, bgColour);
    ASSIGN_OPTIONAL(style.shadowFgColour, shadowFgColour);
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
  boost::optional<long> qsFillColour;
  boost::optional<long> qsShadowColour;
  boost::optional<long> qsFillMatrix;
};

struct VSDFillStyle
{
  VSDFillStyle()
    : fgColour(), bgColour(0xff, 0xff, 0xff, 0), pattern(0), fgTransparency(0),
      bgTransparency(0), shadowFgColour(),  shadowPattern(0), shadowOffsetX(0),
      shadowOffsetY(0), qsFillColour(-1), qsShadowColour(-1), qsFillMatrix(-1) {}
  VSDFillStyle(const Colour &fgc, const Colour &bgc, unsigned char p,
               double fga, double bga, const Colour &sfgc, unsigned char shp,
               double shX, double shY, long qsFc, long qsSc, long qsFm)
    : fgColour(fgc), bgColour(bgc), pattern(p), fgTransparency(fga), bgTransparency(bga),
      shadowFgColour(sfgc), shadowPattern(shp), shadowOffsetX(shX), shadowOffsetY(shY),
      qsFillColour(qsFc), qsShadowColour(qsSc), qsFillMatrix(qsFm) {}
  VSDFillStyle(const VSDFillStyle &style) = default;
  ~VSDFillStyle() {}
  VSDFillStyle &operator=(const VSDFillStyle &style) = default;
  void override(const VSDOptionalFillStyle &style, const VSDXTheme *theme)
  {
    ASSIGN_OPTIONAL(style.pattern, pattern);
    ASSIGN_OPTIONAL(style.fgTransparency, fgTransparency);
    ASSIGN_OPTIONAL(style.bgTransparency, bgTransparency);
    ASSIGN_OPTIONAL(style.shadowPattern, shadowPattern);
    ASSIGN_OPTIONAL(style.shadowOffsetX, shadowOffsetX);
    ASSIGN_OPTIONAL(style.shadowOffsetY, shadowOffsetY);
    ASSIGN_OPTIONAL(style.shadowOffsetY, shadowOffsetY);
    ASSIGN_OPTIONAL(style.qsFillColour, qsFillColour);
    ASSIGN_OPTIONAL(style.qsShadowColour, qsShadowColour);
    ASSIGN_OPTIONAL(style.qsFillMatrix, qsFillMatrix);
    if (theme)
    {
      if (!!style.qsFillColour && style.qsFillColour.get() >= 0)
        ASSIGN_OPTIONAL(theme->getThemeColour(style.qsFillColour.get()), fgColour);

      if (!!style.qsFillColour && style.qsFillColour.get() >= 0)
        ASSIGN_OPTIONAL(theme->getThemeColour(style.qsFillColour.get()), bgColour);

      if (!!style.qsShadowColour && style.qsShadowColour.get() >= 0)
        ASSIGN_OPTIONAL(theme->getThemeColour(style.qsShadowColour.get()), shadowFgColour);
    }
    ASSIGN_OPTIONAL(style.fgColour, fgColour);
    ASSIGN_OPTIONAL(style.bgColour, bgColour);
    ASSIGN_OPTIONAL(style.shadowFgColour, shadowFgColour);
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
  long qsFillColour;
  long qsShadowColour;
  long qsFillMatrix;
};

struct VSDOptionalCharStyle
{
  VSDOptionalCharStyle()
    : charCount(0), font(), colour(), size(), bold(), italic(),
      underline(), doubleunderline(), strikeout(), doublestrikeout(),
      allcaps(), initcaps(), smallcaps(), superscript(), subscript(),
      scaleWidth() {}
  VSDOptionalCharStyle(unsigned cc, const boost::optional<VSDName> &ft,
                       const boost::optional<Colour> &c, const boost::optional<double> &s,
                       const boost::optional<bool> &b, const boost::optional<bool> &i,
                       const boost::optional<bool> &u, const boost::optional<bool> &du,
                       const boost::optional<bool> &so, const boost::optional<bool> &dso,
                       const boost::optional<bool> &ac, const boost::optional<bool> &ic,
                       const boost::optional<bool> &sc, const boost::optional<bool> &super,
                       const boost::optional<bool> &sub, const boost::optional<double> &sw) :
    charCount(cc), font(ft), colour(c), size(s), bold(b), italic(i),
    underline(u), doubleunderline(du), strikeout(so), doublestrikeout(dso),
    allcaps(ac), initcaps(ic), smallcaps(sc), superscript(super),
    subscript(sub), scaleWidth(sw) {}
  VSDOptionalCharStyle(const VSDOptionalCharStyle &style) = default;
  ~VSDOptionalCharStyle() {}
  VSDOptionalCharStyle &operator=(const VSDOptionalCharStyle &style) = default;
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
    ASSIGN_OPTIONAL(style.scaleWidth, scaleWidth);
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
  boost::optional<double> scaleWidth;
};

struct VSDCharStyle
{
  VSDCharStyle()
    : charCount(0), font(), colour(), size(12.0/72.0), bold(false),
      italic(false), underline(false), doubleunderline(false),
      strikeout(false), doublestrikeout(false), allcaps(false),
      initcaps(false), smallcaps(false), superscript(false),
      subscript(false), scaleWidth(1.0) {}
  VSDCharStyle(unsigned cc, const VSDName &ft, const Colour &c, double s,
               bool b, bool i, bool u, bool du, bool so, bool dso, bool ac,
               bool ic, bool sc, bool super, bool sub, double sw) :
    charCount(cc), font(ft), colour(c), size(s), bold(b), italic(i),
    underline(u), doubleunderline(du), strikeout(so), doublestrikeout(dso),
    allcaps(ac), initcaps(ic), smallcaps(sc), superscript(super),
    subscript(sub), scaleWidth(sw) {}
  VSDCharStyle(const VSDCharStyle &style) = default;
  ~VSDCharStyle() {}
  VSDCharStyle &operator=(const VSDCharStyle &style) = default;
  void override(const VSDOptionalCharStyle &style, const VSDXTheme * /* theme */)
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
    ASSIGN_OPTIONAL(style.scaleWidth, scaleWidth);
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
  double scaleWidth;
};

struct VSDOptionalParaStyle
{
  VSDOptionalParaStyle() :
    charCount(0), indFirst(), indLeft(), indRight(), spLine(), spBefore(), spAfter(), align(), bullet(),
    bulletStr(), bulletFont(), bulletFontSize(0.0), textPosAfterBullet(0.0), flags() {}
  VSDOptionalParaStyle(unsigned cc, const boost::optional<double> &ifst, const boost::optional<double> &il,
                       const boost::optional<double> &ir, const boost::optional<double> &sl,
                       const boost::optional<double> &sb, const boost::optional<double> &sa,
                       const boost::optional<unsigned char> &a, const boost::optional<unsigned char> &b,
                       const boost::optional<VSDName> &bs, const boost::optional<VSDName> &bf,
                       const boost::optional<double> bfs, const boost::optional<double> &tpab,
                       const boost::optional<unsigned> &f) :
    charCount(cc), indFirst(ifst), indLeft(il), indRight(ir), spLine(sl), spBefore(sb), spAfter(sa),
    align(a), bullet(b), bulletStr(bs), bulletFont(bf), bulletFontSize(bfs),
    textPosAfterBullet(tpab), flags(f) {}
  VSDOptionalParaStyle(const VSDOptionalParaStyle &style) = default;
  ~VSDOptionalParaStyle() {}
  VSDOptionalParaStyle &operator=(const VSDOptionalParaStyle &style) = default;
  void override(const VSDOptionalParaStyle &style)
  {
    ASSIGN_OPTIONAL(style.indFirst, indFirst);
    ASSIGN_OPTIONAL(style.indLeft, indLeft);
    ASSIGN_OPTIONAL(style.indRight,indRight);
    ASSIGN_OPTIONAL(style.spLine, spLine);
    ASSIGN_OPTIONAL(style.spBefore, spBefore);
    ASSIGN_OPTIONAL(style.spAfter, spAfter);
    ASSIGN_OPTIONAL(style.align, align);
    ASSIGN_OPTIONAL(style.bullet, bullet);
    ASSIGN_OPTIONAL(style.bulletStr, bulletStr);
    ASSIGN_OPTIONAL(style.bulletFont, bulletFont);
    ASSIGN_OPTIONAL(style.bulletFontSize, bulletFontSize);
    ASSIGN_OPTIONAL(style.textPosAfterBullet, textPosAfterBullet);
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
  boost::optional<unsigned char> bullet;
  boost::optional<VSDName> bulletStr;
  boost::optional<VSDName> bulletFont;
  boost::optional<double> bulletFontSize;
  boost::optional<double> textPosAfterBullet;
  boost::optional<unsigned> flags;
};

struct VSDParaStyle
{
  VSDParaStyle() :
    charCount(0), indFirst(0.0), indLeft(0.0), indRight(0.0), spLine(-1.2), spBefore(0.0), spAfter(0.0), align(1),
    bullet(0), bulletStr(), bulletFont(), bulletFontSize(0.0), textPosAfterBullet(0.0), flags(0) {}
  VSDParaStyle(unsigned cc, double ifst, double il, double ir, double sl, double sb,
               double sa, unsigned char a, unsigned b, const VSDName &bs,
               const VSDName &bf, double bfs, double tpab, unsigned f) :
    charCount(cc), indFirst(ifst), indLeft(il), indRight(ir), spLine(sl), spBefore(sb), spAfter(sa), align(a),
    bullet(b), bulletStr(bs), bulletFont(bf), bulletFontSize(bfs), textPosAfterBullet(tpab), flags(f) {}
  VSDParaStyle(const VSDParaStyle &style) = default;
  ~VSDParaStyle() {}
  VSDParaStyle &operator=(const VSDParaStyle &style) = default;
  void override(const VSDOptionalParaStyle &style, const VSDXTheme * /* theme */)
  {
    ASSIGN_OPTIONAL(style.indFirst, indFirst);
    ASSIGN_OPTIONAL(style.indLeft, indLeft);
    ASSIGN_OPTIONAL(style.indRight,indRight);
    ASSIGN_OPTIONAL(style.spLine, spLine);
    ASSIGN_OPTIONAL(style.spBefore, spBefore);
    ASSIGN_OPTIONAL(style.spAfter, spAfter);
    ASSIGN_OPTIONAL(style.align, align);
    ASSIGN_OPTIONAL(style.bullet, bullet);
    ASSIGN_OPTIONAL(style.bulletStr, bulletStr);
    ASSIGN_OPTIONAL(style.bulletFont, bulletFont);
    ASSIGN_OPTIONAL(style.bulletFontSize, bulletFontSize);
    ASSIGN_OPTIONAL(style.textPosAfterBullet, textPosAfterBullet);
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
  unsigned char bullet;
  VSDName bulletStr;
  VSDName bulletFont;
  double bulletFontSize;
  double textPosAfterBullet;
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
  VSDOptionalTextBlockStyle(const VSDOptionalTextBlockStyle &style) = default;
  ~VSDOptionalTextBlockStyle() {}
  VSDOptionalTextBlockStyle &operator=(const VSDOptionalTextBlockStyle &style) = default;
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
    isTextBkgndFilled(false), textBkgndColour(0xff,0xff,0xff,0), defaultTabStop(0.5), textDirection(0) {}
  VSDTextBlockStyle(double lm, double rm, double tm, double bm, unsigned char va,
                    bool isBgFilled, Colour bgClr, double defTab, unsigned char td) :
    leftMargin(lm), rightMargin(rm), topMargin(tm), bottomMargin(bm), verticalAlign(va),
    isTextBkgndFilled(isBgFilled), textBkgndColour(bgClr), defaultTabStop(defTab), textDirection(td) {}
  VSDTextBlockStyle(const VSDTextBlockStyle &style) = default;
  ~VSDTextBlockStyle() {}
  VSDTextBlockStyle &operator=(const VSDTextBlockStyle &style) = default;
  void override(const VSDOptionalTextBlockStyle &style, const VSDXTheme * /* theme */)
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
  VSDStyles(const VSDStyles &styles) = default;
  ~VSDStyles();
  VSDStyles &operator=(const VSDStyles &styles) = default;
  void addLineStyle(unsigned lineStyleIndex, const VSDOptionalLineStyle &lineStyle);
  void addFillStyle(unsigned fillStyleIndex, const VSDOptionalFillStyle &fillStyle);
  void addTextBlockStyle(unsigned textStyleIndex, const VSDOptionalTextBlockStyle &textBlockStyle);
  void addCharStyle(unsigned textStyleIndex, const VSDOptionalCharStyle &charStyle);
  void addParaStyle(unsigned textStyleIndex, const VSDOptionalParaStyle &paraStyle);

  void addLineStyleMaster(unsigned lineStyleIndex, unsigned lineStyleMaster);
  void addFillStyleMaster(unsigned fillStyleIndex, unsigned fillStyleMaster);
  void addTextStyleMaster(unsigned textStyleIndex, unsigned textStyleMaster);

  VSDOptionalLineStyle getOptionalLineStyle(unsigned lineStyleIndex) const;
  VSDFillStyle getFillStyle(unsigned fillStyleIndex, const VSDXTheme *theme) const;
  VSDOptionalFillStyle getOptionalFillStyle(unsigned fillStyleIndex) const;
  VSDOptionalTextBlockStyle getOptionalTextBlockStyle(unsigned textStyleIndex) const;
  VSDOptionalCharStyle getOptionalCharStyle(unsigned textStyleIndex) const;
  VSDOptionalParaStyle getOptionalParaStyle(unsigned textStyleIndex) const;

private:
  std::map<unsigned, VSDOptionalLineStyle> m_lineStyles;
  std::map<unsigned, VSDOptionalFillStyle> m_fillStyles;
  std::map<unsigned, VSDOptionalTextBlockStyle> m_textBlockStyles;
  std::map<unsigned, VSDOptionalCharStyle> m_charStyles;
  std::map<unsigned, VSDOptionalParaStyle> m_paraStyles;
  std::map<unsigned, unsigned> m_lineStyleMasters;
  std::map<unsigned, unsigned> m_fillStyleMasters;
  std::map<unsigned, unsigned> m_textStyleMasters;
};


} // namespace libvisio

#endif // __VSDSTYLES_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
