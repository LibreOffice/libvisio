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
#include <boost/optional.hpp>
#include "VSDParser.h"

namespace libvisio
{

class VSDCollector
{
public:
  VSDCollector() {};
  virtual ~VSDCollector() {}

  virtual void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) = 0;
  virtual void collectForeignData(unsigned level, const WPXBinaryData &binaryData) = 0;
  virtual void collectOLEList(unsigned id, unsigned level) = 0;
  virtual void collectOLEData(unsigned id, unsigned level, const WPXBinaryData &oleData) = 0;
  virtual void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop) = 0;
  virtual void collectLine(unsigned level, const boost::optional<double> &strokeWidth, const boost::optional<Colour> &c, const boost::optional<unsigned char> &linePattern,
                           const boost::optional<unsigned char> &startMarker, const boost::optional<unsigned char> &endMarker,
                           const boost::optional<unsigned char> &lineCap) = 0;
  virtual void collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                    const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                                    const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                                    const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX, const boost::optional<double> &shadowOffsetY) = 0;
  virtual void collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                    const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                                    const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                                    const boost::optional<Colour> &shfgc) = 0;
  virtual void collectGeometry(unsigned id, unsigned level, bool noFill, bool noLine, bool noShow) = 0;
  virtual void collectMoveTo(unsigned id, unsigned level, double x, double y) = 0;
  virtual void collectLineTo(unsigned id, unsigned level, double x, double y) = 0;
  virtual void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow) = 0;
  virtual void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
                              const std::vector<std::pair<double, double> > &ctrlPnts, const std::vector<double> &kntVec, const std::vector<double> &weights) = 0;
  virtual void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID) = 0;
  virtual void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, const NURBSData &data) = 0;
  virtual void collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned char xType, unsigned char yType, const std::vector<std::pair<double, double> > &points) = 0;
  virtual void collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned dataID) = 0;
  virtual void collectPolylineTo(unsigned id, unsigned level, double x, double y, const PolylineData &data) = 0;
  virtual void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, unsigned degree, double lastKnot,
                                std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) = 0;
  virtual void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points) = 0;
  virtual void collectXFormData(unsigned level, const XForm &xform) = 0;
  virtual void collectTxtXForm(unsigned level, const XForm &txtxform) = 0;
  virtual void collectShapesOrder(unsigned id, unsigned level, const std::vector<unsigned> &shapeIds) = 0;
  virtual void collectForeignDataType(unsigned level, unsigned foreignType, unsigned foreignFormat, double offsetX, double offsetY, double width, double height) = 0;
  virtual void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale) = 0;
  virtual void collectPage(unsigned id, unsigned level, unsigned backgroundPageID, bool isBackgroundPage, const VSDName &pageName) = 0;
  virtual void collectShape(unsigned id, unsigned level, unsigned parent, unsigned masterPage, unsigned masterShape, unsigned lineStyle, unsigned fillStyle, unsigned textStyle) = 0;
  virtual void collectSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree) = 0;
  virtual void collectSplineKnot(unsigned id, unsigned level, double x, double y, double knot) = 0;
  virtual void collectSplineEnd() = 0;
  virtual void collectInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2) = 0;
  virtual void collectRelCubBezTo(unsigned id, unsigned level, double x, double y, double a, double b, double c, double d) = 0;
  virtual void collectRelEllipticalArcTo(unsigned id, unsigned level, double x, double y, double a, double b, double c, double d) = 0;
  virtual void collectRelLineTo(unsigned id, unsigned level, double x, double y) = 0;
  virtual void collectRelMoveTo(unsigned id, unsigned level, double x, double y) = 0;
  virtual void collectRelQuadBezTo(unsigned id, unsigned level, double x, double y, double a, double b) = 0;

  virtual void collectUnhandledChunk(unsigned id, unsigned level) = 0;

  virtual void collectText(unsigned level, const ::WPXBinaryData &textStream, TextFormat format) = 0;
  virtual void collectCharIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<VSDName> &font,
                             const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                             const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
                             const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps,
                             const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript,
                             const boost::optional<bool> &subscript) = 0;
  virtual void collectDefaultCharStyle(unsigned charCount, const boost::optional<VSDName> &font, const boost::optional<Colour> &fontColour,
                                       const boost::optional<double> &fontSize, const boost::optional<bool> &bold, const boost::optional<bool> &italic,
                                       const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline, const boost::optional<bool> &strikeout,
                                       const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps,
                                       const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript, const boost::optional<bool> &subscript) = 0;
  virtual void collectParaIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                             const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                             const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                             const boost::optional<unsigned> &flags) = 0;
  virtual void collectDefaultParaStyle(unsigned charCount, const boost::optional<double> &indFirst, const boost::optional<double> &indLeft,
                                       const boost::optional<double> &indRight, const boost::optional<double> &spLine, const boost::optional<double> &spBefore,
                                       const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align, const boost::optional<unsigned> &flags) = 0;
  virtual void collectTextBlock(unsigned level, const boost::optional<double> &leftMargin, const boost::optional<double> &rightMargin,
                                const boost::optional<double> &topMargin, const boost::optional<double> &bottomMargin,
                                const boost::optional<unsigned char> &verticalAlign, const boost::optional<bool> &isBgFilled,
                                const boost::optional<Colour> &bgColour, const boost::optional<double> &defaultTabStop,
                                const boost::optional<unsigned char> &textDirection) = 0;
  virtual void collectNameList(unsigned id, unsigned level) = 0;
  virtual void collectName(unsigned id, unsigned level,  const ::WPXBinaryData &name, TextFormat format) = 0;
  virtual void collectPageSheet(unsigned id, unsigned level) = 0;
  virtual void collectMisc(unsigned level, const VSDMisc &misc) = 0;

  // Style collectors
  virtual void collectStyleSheet(unsigned id, unsigned level,unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle) = 0;
  virtual void collectLineStyle(unsigned level, const boost::optional<double> &strokeWidth, const boost::optional<Colour> &c, const boost::optional<unsigned char> &linePattern,
                                const boost::optional<unsigned char> &startMarker, const boost::optional<unsigned char> &endMarker,
                                const boost::optional<unsigned char> &lineCap) = 0;
  virtual void collectFillStyle(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                                const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                                const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX, const boost::optional<double> &shadowOffsetY) = 0;
  virtual void collectFillStyle(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                                const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                                const boost::optional<Colour> &shfgc) = 0;
  virtual void collectCharIXStyle(unsigned id, unsigned level, unsigned charCount, const boost::optional<VSDName> &font,
                                  const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                                  const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
                                  const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps,
                                  const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript,
                                  const boost::optional<bool> &subscript) = 0;
  virtual void collectParaIXStyle(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                                  const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                                  const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                                  const boost::optional<unsigned> &flags) = 0;
  virtual void collectTextBlockStyle(unsigned level, const boost::optional<double> &leftMargin, const boost::optional<double> &rightMargin,
                                     const boost::optional<double> &topMargin, const boost::optional<double> &bottomMargin,
                                     const boost::optional<unsigned char> &verticalAlign, const boost::optional<bool> &isBgFilled,
                                     const boost::optional<Colour> &bgColour, const boost::optional<double> &defaultTabStop,
                                     const boost::optional<unsigned char> &textDirection) = 0;
  // Field list
  virtual void collectFieldList(unsigned id, unsigned level) = 0;
  virtual void collectTextField(unsigned id, unsigned level, int nameId, int formatStringId) = 0;
  virtual void collectNumericField(unsigned id, unsigned level, unsigned short format, double number, int formatStringId) = 0;

  // Temporary hack
  virtual void startPage(unsigned pageId) = 0;
  virtual void endPage() = 0;
  virtual void endPages() = 0;

private:
  VSDCollector(const VSDCollector &);
  VSDCollector &operator=(const VSDCollector &);
};

} // namespace libvisio

#endif /* VSDCOLLECTOR_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
