/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VSDCOLLECTOR_H
#define VSDCOLLECTOR_H

#include <vector>
#include <optional>
#include "VSDParser.h"

namespace libvisio
{

class VSDCollector
{
public:
  VSDCollector() {};
  virtual ~VSDCollector() {}

  virtual void collectDocumentTheme(const VSDXTheme *theme) = 0;
  virtual void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) = 0;
  virtual void collectForeignData(unsigned level, const librevenge::RVNGBinaryData &binaryData) = 0;
  virtual void collectOLEList(unsigned id, unsigned level) = 0;
  virtual void collectOLEData(unsigned id, unsigned level, const librevenge::RVNGBinaryData &oleData) = 0;
  virtual void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop) = 0;
  virtual void collectLine(unsigned level, const std::optional<double> &strokeWidth, const std::optional<Colour> &c, const std::optional<unsigned char> &linePattern,
                           const std::optional<unsigned char> &startMarker, const std::optional<unsigned char> &endMarker,
                           const std::optional<unsigned char> &lineCap, const std::optional<double> &rounding,
                           const std::optional<long> &qsLineColour, const std::optional<long> &qsLineMatrix) = 0;
  virtual void collectFillAndShadow(unsigned level, const std::optional<Colour> &colourFG, const std::optional<Colour> &colourBG,
                                    const std::optional<unsigned char> &fillPattern, const std::optional<double> &fillFGTransparency,
                                    const std::optional<double> &fillBGTransparency, const std::optional<unsigned char> &shadowPattern,
                                    const std::optional<Colour> &shfgc, const std::optional<double> &shadowOffsetX, const std::optional<double> &shadowOffsetY,
                                    const std::optional<long> &qsFc, const std::optional<long> &qsSc, const std::optional<long> &qsLm) = 0;
  virtual void collectFillAndShadow(unsigned level, const std::optional<Colour> &colourFG, const std::optional<Colour> &colourBG,
                                    const std::optional<unsigned char> &fillPattern, const std::optional<double> &fillFGTransparency,
                                    const std::optional<double> &fillBGTransparency, const std::optional<unsigned char> &shadowPattern,
                                    const std::optional<Colour> &shfgc) = 0;
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
  virtual void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale,
                                unsigned char drawingScaleUnit, const std::optional<unsigned> variationColorIndex, const std::optional<unsigned> variationStyleIndex) = 0;
  virtual void collectPage(unsigned id, unsigned level, unsigned backgroundPageID, bool isBackgroundPage, const VSDName &pageName) = 0;
  virtual void collectShape(unsigned id, unsigned level, unsigned parent, unsigned masterPage, unsigned masterShape, unsigned lineStyle, unsigned fillStyle, unsigned textStyle, const VSDName &aShapeType) = 0;
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

  virtual void collectText(unsigned level, const librevenge::RVNGBinaryData &textStream, TextFormat format) = 0;
  virtual void collectCharIX(unsigned id, unsigned level, unsigned charCount, const std::optional<VSDName> &font,
                             const std::optional<Colour> &fontColour, const std::optional<double> &fontSize, const std::optional<bool> &bold,
                             const std::optional<bool> &italic, const std::optional<bool> &underline, const std::optional<bool> &doubleunderline,
                             const std::optional<bool> &strikeout, const std::optional<bool> &doublestrikeout, const std::optional<bool> &allcaps,
                             const std::optional<bool> &initcaps, const std::optional<bool> &smallcaps, const std::optional<bool> &superscript,
                             const std::optional<bool> &subscript, const std::optional<double> &scaleWidth) = 0;
  virtual void collectDefaultCharStyle(unsigned charCount, const std::optional<VSDName> &font, const std::optional<Colour> &fontColour,
                                       const std::optional<double> &fontSize, const std::optional<bool> &bold, const std::optional<bool> &italic,
                                       const std::optional<bool> &underline, const std::optional<bool> &doubleunderline, const std::optional<bool> &strikeout,
                                       const std::optional<bool> &doublestrikeout, const std::optional<bool> &allcaps, const std::optional<bool> &initcaps,
                                       const std::optional<bool> &smallcaps, const std::optional<bool> &superscript, const std::optional<bool> &subscript,
                                       const std::optional<double> &scaleWidth) = 0;
  virtual void collectParaIX(unsigned id, unsigned level, unsigned charCount, const std::optional<double> &indFirst,
                             const std::optional<double> &indLeft, const std::optional<double> &indRight, const std::optional<double> &spLine,
                             const std::optional<double> &spBefore, const std::optional<double> &spAfter, const std::optional<unsigned char> &align,
                             const std::optional<unsigned char> &bullet, const std::optional<VSDName> &bulletStr,
                             const std::optional<VSDName> &bulletFont, const std::optional<double> &bulletFontSize,
                             const std::optional<double> &textPosAfterBullet, const std::optional<unsigned> &flags) = 0;
  virtual void collectDefaultParaStyle(unsigned charCount, const std::optional<double> &indFirst, const std::optional<double> &indLeft,
                                       const std::optional<double> &indRight, const std::optional<double> &spLine, const std::optional<double> &spBefore,
                                       const std::optional<double> &spAfter, const std::optional<unsigned char> &align,
                                       const std::optional<unsigned char> &bullet, const std::optional<VSDName> &bulletStr,
                                       const std::optional<VSDName> &bulletFont, const std::optional<double> &bulletFontSize,
                                       const std::optional<double> &textPosAfterBullet, const std::optional<unsigned> &flags) = 0;
  virtual void collectTextBlock(unsigned level, const std::optional<double> &leftMargin, const std::optional<double> &rightMargin,
                                const std::optional<double> &topMargin, const std::optional<double> &bottomMargin,
                                const std::optional<unsigned char> &verticalAlign, const std::optional<bool> &isBgFilled,
                                const std::optional<Colour> &bgColour, const std::optional<double> &defaultTabStop,
                                const std::optional<unsigned char> &textDirection) = 0;
  virtual void collectNameList(unsigned id, unsigned level) = 0;
  virtual void collectName(unsigned id, unsigned level,  const librevenge::RVNGBinaryData &name, TextFormat format) = 0;
  virtual void collectPageSheet(unsigned id, unsigned level) = 0;
  virtual void collectMisc(unsigned level, const VSDMisc &misc) = 0;
  virtual void collectLayer(unsigned id, unsigned level, const VSDLayer &layer) = 0;
  virtual void collectLayerMem(unsigned level, const VSDName &layerMem) = 0;
  virtual void collectTabsDataList(unsigned level, const std::map<unsigned, VSDTabSet> &tabSets) = 0;

  // Style collectors
  virtual void collectStyleSheet(unsigned id, unsigned level,unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle) = 0;
  virtual void collectLineStyle(unsigned level, const std::optional<double> &strokeWidth, const std::optional<Colour> &c, const std::optional<unsigned char> &linePattern,
                                const std::optional<unsigned char> &startMarker, const std::optional<unsigned char> &endMarker,
                                const std::optional<unsigned char> &lineCap, const std::optional<double> &rounding,
                                const std::optional<long> &qsLineColour, const std::optional<long> &qsLineMatrix) = 0;
  virtual void collectFillStyle(unsigned level, const std::optional<Colour> &colourFG, const std::optional<Colour> &colourBG,
                                const std::optional<unsigned char> &fillPattern, const std::optional<double> &fillFGTransparency,
                                const std::optional<double> &fillBGTransparency, const std::optional<unsigned char> &shadowPattern,
                                const std::optional<Colour> &shfgc, const std::optional<double> &shadowOffsetX, const std::optional<double> &shadowOffsetY,
                                const std::optional<long> &qsFillColour, const std::optional<long> &qsShadowColour,
                                const std::optional<long> &qsFillMatrix) = 0;
  virtual void collectFillStyle(unsigned level, const std::optional<Colour> &colourFG, const std::optional<Colour> &colourBG,
                                const std::optional<unsigned char> &fillPattern, const std::optional<double> &fillFGTransparency,
                                const std::optional<double> &fillBGTransparency, const std::optional<unsigned char> &shadowPattern,
                                const std::optional<Colour> &shfgc) = 0;
  virtual void collectCharIXStyle(unsigned id, unsigned level, unsigned charCount, const std::optional<VSDName> &font,
                                  const std::optional<Colour> &fontColour, const std::optional<double> &fontSize, const std::optional<bool> &bold,
                                  const std::optional<bool> &italic, const std::optional<bool> &underline, const std::optional<bool> &doubleunderline,
                                  const std::optional<bool> &strikeout, const std::optional<bool> &doublestrikeout, const std::optional<bool> &allcaps,
                                  const std::optional<bool> &initcaps, const std::optional<bool> &smallcaps, const std::optional<bool> &superscript,
                                  const std::optional<bool> &subscript, const std::optional<double> &scaleWidth) = 0;
  virtual void collectParaIXStyle(unsigned id, unsigned level, unsigned charCount, const std::optional<double> &indFirst,
                                  const std::optional<double> &indLeft, const std::optional<double> &indRight, const std::optional<double> &spLine,
                                  const std::optional<double> &spBefore, const std::optional<double> &spAfter, const std::optional<unsigned char> &align,
                                  const std::optional<unsigned char> &bullet, const std::optional<VSDName> &bulletStr,
                                  const std::optional<VSDName> &bulletFont, const std::optional<double> &bulletFontSize,
                                  const std::optional<double> &textPosAfterBullet, const std::optional<unsigned> &flags) = 0;
  virtual void collectTextBlockStyle(unsigned level, const std::optional<double> &leftMargin, const std::optional<double> &rightMargin,
                                     const std::optional<double> &topMargin, const std::optional<double> &bottomMargin,
                                     const std::optional<unsigned char> &verticalAlign, const std::optional<bool> &isBgFilled,
                                     const std::optional<Colour> &bgColour, const std::optional<double> &defaultTabStop,
                                     const std::optional<unsigned char> &textDirection) = 0;

  // Field list
  virtual void collectFieldList(unsigned id, unsigned level) = 0;
  virtual void collectTextField(unsigned id, unsigned level, int nameId, int formatStringId) = 0;
  virtual void collectNumericField(unsigned id, unsigned level, unsigned short format, unsigned short cellType, double number, int formatStringId) = 0;

  // Metadata
  virtual void collectMetaData(const librevenge::RVNGPropertyList &metaData) = 0;

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
