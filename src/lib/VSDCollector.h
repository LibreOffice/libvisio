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

#ifndef VSDCOLLECTOR_H
#define VSDCOLLECTOR_H

#include <vector>
#include "VSDParser.h"

namespace libvisio
{

class VSDCollector
{
public:
  VSDCollector() {};
  virtual ~VSDCollector() {}

  virtual void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) = 0;
  virtual void collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData) = 0;
  virtual void collectOLEList(unsigned id, unsigned level) = 0;
  virtual void collectOLEData(unsigned id, unsigned level, const WPXBinaryData &oleData) = 0;
  virtual void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop) = 0;
  virtual void collectLine(unsigned id, unsigned level, double strokeWidth, const Colour &c, unsigned linePattern, unsigned char startMarker, unsigned char endMarker, unsigned lineCap) = 0;
  virtual void collectFillAndShadow(unsigned id, unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned fillPattern, double fillFGTransparency,
                                    double fillBGTransparency, unsigned shadowPattern, const Colour &shfgc, double shadowOffsetX, double shadowOffsetY) = 0;
  virtual void collectFillAndShadow(unsigned id, unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned fillPattern, double fillFGTransparency,
                                    double fillBGTransparency, unsigned shadowPattern, const Colour &shfgc) = 0;
  virtual void collectGeometry(unsigned id, unsigned level, unsigned char geomFlags) = 0;
  virtual void collectMoveTo(unsigned id, unsigned level, double x, double y) = 0;
  virtual void collectLineTo(unsigned id, unsigned level, double x, double y) = 0;
  virtual void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow) = 0;
  virtual void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
                              std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) = 0;
  virtual void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID) = 0;
  virtual void collectPolylineTo(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > &points) = 0;
  virtual void collectPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID) = 0;
  virtual void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, unsigned degree, double lastKnot,
                                std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) = 0;
  virtual void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points) = 0;
  virtual void collectXFormData(unsigned id, unsigned level, const XForm &xform) = 0;
  virtual void collectTxtXForm(unsigned id, unsigned level, const XForm &txtxform) = 0;
  virtual void collectShapeId(unsigned id, unsigned level, unsigned shapeId) = 0;
  virtual void collectForeignDataType(unsigned id, unsigned level, unsigned foreignType, unsigned foreignFormat, double offsetX, double offsetY, double width, double height) = 0;
  virtual void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale) = 0;
  virtual void collectPage(unsigned id, unsigned level, unsigned backgroundPageID) = 0;
  virtual void collectShape(unsigned id, unsigned level, unsigned masterPage, unsigned masterShape, unsigned lineStyle, unsigned fillStyle, unsigned textStyle) = 0;
  virtual void collectSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree) = 0;
  virtual void collectSplineKnot(unsigned id, unsigned level, double x, double y, double knot) = 0;
  virtual void collectSplineEnd() = 0;
  virtual void collectInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2) = 0;

  virtual void collectUnhandledChunk(unsigned id, unsigned level) = 0;

  virtual void collectFont(unsigned short fontID, const ::WPXBinaryData &textStream, TextFormat format) = 0;
  virtual void collectText(unsigned id, unsigned level, const ::WPXBinaryData &textStream, TextFormat format) = 0;
  virtual void collectVSDCharStyle(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, double fontSize,
                                   bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                                   bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, VSDFont fontFace) = 0;
  virtual void collectVSDParaStyle(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                                   double spLine, double spBefore, double spAfter, unsigned char align, unsigned flags) = 0;
  virtual void collectTextBlock(unsigned id, unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin,
                                unsigned char verticalAlign, bool isBgFilled, const Colour &bgColour, double defaultTabStop, unsigned char textDirection) = 0;
  virtual void collectNameList(unsigned id, unsigned level) = 0;
  virtual void collectName(unsigned id, unsigned level,  const ::WPXBinaryData &name, TextFormat format) = 0;
  virtual void collectPageSheet(unsigned id, unsigned level) = 0;

  // Style collectors
  virtual void collectStyleSheet(unsigned id, unsigned level, unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle) = 0;
  virtual void collectLineStyle(unsigned id, unsigned level, double strokeWidth, const Colour &c, unsigned char linePattern,
                                unsigned char startMarker, unsigned char endMarker, unsigned char lineCap) = 0;
  virtual void collectFillStyle(unsigned id, unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned char fillPattern,
                                double fillFGTransparency, double fillBGTransparency, unsigned char shadowPattern, const Colour &shfgc,
                                double shadowOffsetX, double shadowOffsetY) = 0;
  virtual void collectFillStyle(unsigned id, unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned char fillPattern,
                                double fillFGTransparency, double fillBGTransparency, unsigned char shadowPattern, const Colour &shfgc) = 0;
  virtual void collectCharIXStyle(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, double fontSize,
                                  bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                                  bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, VSDFont fontFace) = 0;
  virtual void collectParaIXStyle(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                                  double spLine, double spBefore, double spAfter, unsigned char align, unsigned flags) = 0;
  virtual void collectTextBlockStyle(unsigned id, unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin,
                                     unsigned char verticalAlign, bool isBgFilled, const Colour &bgColour, double defaultTabStop,
                                     unsigned char textDirection) = 0;
  // Field list
  virtual void collectFieldList(unsigned id, unsigned level) = 0;
  virtual void collectTextField(unsigned id, unsigned level, int nameId, int formatStringId) = 0;
  virtual void collectNumericField(unsigned id, unsigned level, unsigned short format, double number, int formatStringId) = 0;

  // Temporary hack
  virtual void startPage(unsigned pageId) = 0;
  virtual void endPage() = 0;
  virtual void endPages() = 0;

protected:
  const ::WPXString getColourString(const Colour &c) const
  {
    ::WPXString sColour;
    sColour.sprintf("#%.2x%.2x%.2x", c.r, c.g, c.b);
    return sColour;
  }

private:
  VSDCollector(const VSDCollector &);
  VSDCollector &operator=(const VSDCollector &);
};

} // namespace libvisio

#endif /* VSDCOLLECTOR_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
