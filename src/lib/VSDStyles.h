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
#include <optional>
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
  VSDOptionalLineStyle(const std::optional<double> &w, const std::optional<Colour> &col,
                       const std::optional<unsigned char> &p, const std::optional<unsigned char> &sm,
                       const std::optional<unsigned char> &em, const std::optional<unsigned char> &c,
                       const std::optional<double> &r, const std::optional<long> &qlc,
                       const std::optional<long> &qlm) :
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

  std::optional<double> width;
  std::optional<Colour> colour;
  std::optional<unsigned char> pattern;
  std::optional<unsigned char> startMarker;
  std::optional<unsigned char> endMarker;
  std::optional<unsigned char> cap;
  std::optional<double> rounding;
  std::optional<long> qsLineColour;
  std::optional<long> qsLineMatrix;
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
      if (!!style.qsLineColour && style.qsLineColour.value() >= 0)
        ASSIGN_OPTIONAL(theme->getThemeColour(style.qsLineColour.value()), colour);
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
    qsFillMatrix(), variationColorIndex(), variationStyleIndex() {}
  VSDOptionalFillStyle(const std::optional<Colour> &fgc, const std::optional<Colour> &bgc,
                       const std::optional<unsigned char> &p, const std::optional<double> &fga,
                       const std::optional<double> &bga, const std::optional<Colour> &sfgc,
                       const std::optional<unsigned char> &shp, const std::optional<double> &shX,
                       const std::optional<double> &shY, const std::optional<long> &qsFc,
                       const std::optional<long> &qsSc, const std::optional<long> &qsFm,
                       const std::optional<unsigned> &vCIn, const std::optional<unsigned> &vSIn) :
    fgColour(fgc), bgColour(bgc), pattern(p), fgTransparency(fga), bgTransparency(bga),
    shadowFgColour(sfgc), shadowPattern(shp), shadowOffsetX(shX), shadowOffsetY(shY),
    qsFillColour(qsFc), qsShadowColour(qsSc), qsFillMatrix(qsFm), variationColorIndex(vCIn),
    variationStyleIndex(vSIn) {}
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
    ASSIGN_OPTIONAL(style.variationColorIndex, variationColorIndex);
    ASSIGN_OPTIONAL(style.variationStyleIndex, variationStyleIndex);
  }

  std::optional<Colour> fgColour;
  std::optional<Colour> bgColour;
  std::optional<unsigned char> pattern;
  std::optional<double> fgTransparency;
  std::optional<double> bgTransparency;
  std::optional<Colour> shadowFgColour;
  std::optional<unsigned char> shadowPattern;
  std::optional<double> shadowOffsetX;
  std::optional<double> shadowOffsetY;
  std::optional<long> qsFillColour;
  std::optional<long> qsShadowColour;
  std::optional<long> qsFillMatrix;
  std::optional<unsigned> variationColorIndex;
  std::optional<unsigned> variationStyleIndex;
};

struct VSDFillStyle
{
  VSDFillStyle()
    : fgColour(), bgColour(0xff, 0xff, 0xff, 0), pattern(0), fgTransparency(0),
      bgTransparency(0), shadowFgColour(),  shadowPattern(0), shadowOffsetX(0),
      shadowOffsetY(0), qsFillColour(100), qsShadowColour(100), qsFillMatrix(-1),
      variationColorIndex(0), variationStyleIndex(0) {}
  VSDFillStyle(const Colour &fgc, const Colour &bgc, unsigned char p,
               double fga, double bga, const Colour &sfgc, unsigned char shp,
               double shX, double shY, long qsFc, long qsSc, long qsFm, unsigned vCIn, unsigned vSIn)
    : fgColour(fgc), bgColour(bgc), pattern(p), fgTransparency(fga), bgTransparency(bga),
      shadowFgColour(sfgc), shadowPattern(shp), shadowOffsetX(shX), shadowOffsetY(shY),
      qsFillColour(qsFc), qsShadowColour(qsSc), qsFillMatrix(qsFm), variationColorIndex(vCIn),
      variationStyleIndex(vSIn) {}
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
    ASSIGN_OPTIONAL(style.variationColorIndex, variationColorIndex);
    ASSIGN_OPTIONAL(style.variationStyleIndex, variationStyleIndex);
    if (theme)
    {
      // Quick Style Colour 100 is special. It is the default,
      // and it is not saved explicitely in the VSDX file.
      ASSIGN_OPTIONAL(theme->getThemeColour(qsFillColour, variationColorIndex), fgColour);
      ASSIGN_OPTIONAL(theme->getThemeColour(qsFillColour, variationColorIndex), bgColour);
      ASSIGN_OPTIONAL(theme->getThemeColour(qsShadowColour, variationColorIndex), shadowFgColour);
      if (!!style.qsFillMatrix && style.qsFillMatrix.value() >= 0)
        ASSIGN_OPTIONAL(theme->getFillStyleColour(style.qsFillMatrix.value()), fgColour);
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
  unsigned variationColorIndex;
  unsigned variationStyleIndex;
};

struct VSDOptionalCharStyle
{
  VSDOptionalCharStyle()
    : charCount(0), font(), colour(), size(), bold(), italic(),
      underline(), doubleunderline(), strikeout(), doublestrikeout(),
      allcaps(), initcaps(), smallcaps(), superscript(), subscript(),
      scaleWidth() {}
  VSDOptionalCharStyle(unsigned cc, const std::optional<VSDName> &ft,
                       const std::optional<Colour> &c, const std::optional<double> &s,
                       const std::optional<bool> &b, const std::optional<bool> &i,
                       const std::optional<bool> &u, const std::optional<bool> &du,
                       const std::optional<bool> &so, const std::optional<bool> &dso,
                       const std::optional<bool> &ac, const std::optional<bool> &ic,
                       const std::optional<bool> &sc, const std::optional<bool> &super,
                       const std::optional<bool> &sub, const std::optional<double> &sw) :
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
  std::optional<VSDName> font;
  std::optional<Colour> colour;
  std::optional<double> size;
  std::optional<bool> bold;
  std::optional<bool> italic;
  std::optional<bool> underline;
  std::optional<bool> doubleunderline;
  std::optional<bool> strikeout;
  std::optional<bool> doublestrikeout;
  std::optional<bool> allcaps;
  std::optional<bool> initcaps;
  std::optional<bool> smallcaps;
  std::optional<bool> superscript;
  std::optional<bool> subscript;
  std::optional<double> scaleWidth;
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
  VSDOptionalParaStyle(unsigned cc, const std::optional<double> &ifst, const std::optional<double> &il,
                       const std::optional<double> &ir, const std::optional<double> &sl,
                       const std::optional<double> &sb, const std::optional<double> &sa,
                       const std::optional<unsigned char> &a, const std::optional<unsigned char> &b,
                       const std::optional<VSDName> &bs, const std::optional<VSDName> &bf,
                       const std::optional<double> bfs, const std::optional<double> &tpab,
                       const std::optional<unsigned> &f) :
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
  std::optional<double> indFirst;
  std::optional<double> indLeft;
  std::optional<double> indRight;
  std::optional<double> spLine;
  std::optional<double> spBefore;
  std::optional<double> spAfter;
  std::optional<unsigned char> align;
  std::optional<unsigned char> bullet;
  std::optional<VSDName> bulletStr;
  std::optional<VSDName> bulletFont;
  std::optional<double> bulletFontSize;
  std::optional<double> textPosAfterBullet;
  std::optional<unsigned> flags;
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
  VSDOptionalTextBlockStyle(const std::optional<double> &lm, const std::optional<double> &rm,
                            const std::optional<double> &tm, const std::optional<double> &bm,
                            const std::optional<unsigned char> &va, const std::optional<bool> &isBgFilled,
                            const std::optional<Colour> &bgClr, const std::optional<double> &defTab,
                            const std::optional<unsigned char> &td) :
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

  std::optional<double> leftMargin;
  std::optional<double> rightMargin;
  std::optional<double> topMargin;
  std::optional<double> bottomMargin;
  std::optional<unsigned char> verticalAlign;
  std::optional<bool> isTextBkgndFilled;
  std::optional<Colour> textBkgndColour;
  std::optional<double> defaultTabStop;
  std::optional<unsigned char> textDirection;
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
