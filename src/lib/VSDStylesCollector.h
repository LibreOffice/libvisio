/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VSDSTYLESCOLLECTOR_H
#define VSDSTYLESCOLLECTOR_H

#include <map>
#include <vector>
#include <list>
#include "VSDCollector.h"
#include "VSDParser.h"
#include "libvisio_utils.h"
#include "VSDStyles.h"

namespace libvisio
{

class VSDStylesCollector : public VSDCollector
{
public:
  VSDStylesCollector(
    std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
    std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
    std::vector<std::list<unsigned> > &documentPageShapeOrders
  );
  ~VSDStylesCollector() override {}

  void collectDocumentTheme(const VSDXTheme * /* theme */) override {}
  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) override;
  void collectForeignData(unsigned level, const librevenge::RVNGBinaryData &binaryData) override;
  void collectOLEList(unsigned id, unsigned level) override
  {
    collectUnhandledChunk(id, level);
  }
  void collectOLEData(unsigned id, unsigned level, const librevenge::RVNGBinaryData &oleData) override;
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop) override;
  void collectLine(unsigned level, const std::optional<double> &strokeWidth, const std::optional<Colour> &c, const std::optional<unsigned char> &linePattern,
                   const std::optional<unsigned char> &startMarker, const std::optional<unsigned char> &endMarker,
                   const std::optional<unsigned char> &lineCap, const std::optional<double> &rounding,
                   const std::optional<long> &qsLineColour, const std::optional<long> &qsLineMatrix) override;
  void collectFillAndShadow(unsigned level, const std::optional<Colour> &colourFG, const std::optional<Colour> &colourBG,
                            const std::optional<unsigned char> &fillPattern, const std::optional<double> &fillFGTransparency,
                            const std::optional<double> &fillBGTransparency, const std::optional<unsigned char> &shadowPattern,
                            const std::optional<Colour> &shfgc, const std::optional<double> &shadowOffsetX, const std::optional<double> &shadowOffsetY,
                            const std::optional<long> &qsFc, const std::optional<long> &qsSc, const std::optional<long> &qsLm) override;
  void collectFillAndShadow(unsigned level, const std::optional<Colour> &colourFG, const std::optional<Colour> &colourBG,
                            const std::optional<unsigned char> &fillPattern, const std::optional<double> &fillFGTransparency,
                            const std::optional<double> &fillBGTransparency, const std::optional<unsigned char> &shadowPattern,
                            const std::optional<Colour> &shfgc) override;
  void collectGeometry(unsigned id, unsigned level, bool noFill, bool noLine, bool noShow) override;
  void collectMoveTo(unsigned id, unsigned level, double x, double y) override;
  void collectLineTo(unsigned id, unsigned level, double x, double y) override;
  void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow) override;
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
                      const std::vector<std::pair<double, double> > &ctrlPnts, const std::vector<double> &kntVec, const std::vector<double> &weights) override;
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID) override;
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, const NURBSData &data) override;
  void collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned char xType, unsigned char yType, const std::vector<std::pair<double, double> > &points) override;
  void collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned dataID) override;
  void collectPolylineTo(unsigned id, unsigned level, double x, double y, const PolylineData &data) override;
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, unsigned degree, double lastKnot,
                        std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) override;
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points) override;
  void collectXFormData(unsigned level, const XForm &xform) override;
  void collectTxtXForm(unsigned level, const XForm &txtxform) override;
  void collectShapesOrder(unsigned id, unsigned level, const std::vector<unsigned> &shapeIds) override;
  void collectForeignDataType(unsigned level, unsigned foreignType, unsigned foreignFormat, double offsetX, double offsetY, double width, double height) override;
  void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale,
                        unsigned char drawingScaleUnit, const std::optional<unsigned> variationColorIndex, const std::optional<unsigned> variationStyleIndex) override;
  void collectPage(unsigned id, unsigned level, unsigned backgroundPageID, bool isBackgroundPage, const VSDName &pageName) override;
  void collectShape(unsigned id, unsigned level, unsigned parent, unsigned masterPage, unsigned masterShape, unsigned lineStyle, unsigned fillStyle, unsigned textStyle, const VSDName &aShapeType) override;
  void collectSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree) override;
  void collectSplineKnot(unsigned id, unsigned level, double x, double y, double knot) override;
  void collectSplineEnd() override;
  void collectInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2) override;
  void collectRelCubBezTo(unsigned id, unsigned level, double x, double y, double a, double b, double c, double d) override;
  void collectRelEllipticalArcTo(unsigned id, unsigned level, double x, double y, double a, double b, double c, double d) override;
  void collectRelLineTo(unsigned id, unsigned level, double x, double y) override;
  void collectRelMoveTo(unsigned id, unsigned level, double x, double y) override;
  void collectRelQuadBezTo(unsigned id, unsigned level, double x, double y, double a, double b) override;
  void collectUnhandledChunk(unsigned id, unsigned level) override;

  void collectText(unsigned level, const librevenge::RVNGBinaryData &textStream, TextFormat format) override;
  void collectCharIX(unsigned id, unsigned level, unsigned charCount, const std::optional<VSDName> &font,
                     const std::optional<Colour> &fontColour, const std::optional<double> &fontSize, const std::optional<bool> &bold,
                     const std::optional<bool> &italic, const std::optional<bool> &underline, const std::optional<bool> &doubleunderline,
                     const std::optional<bool> &strikeout, const std::optional<bool> &doublestrikeout, const std::optional<bool> &allcaps,
                     const std::optional<bool> &initcaps, const std::optional<bool> &smallcaps, const std::optional<bool> &superscript,
                     const std::optional<bool> &subscript, const std::optional<double> &scaleWidth) override;
  void collectDefaultCharStyle(unsigned charCount, const std::optional<VSDName> &font, const std::optional<Colour> &fontColour,
                               const std::optional<double> &fontSize, const std::optional<bool> &bold, const std::optional<bool> &italic,
                               const std::optional<bool> &underline, const std::optional<bool> &doubleunderline, const std::optional<bool> &strikeout,
                               const std::optional<bool> &doublestrikeout, const std::optional<bool> &allcaps, const std::optional<bool> &initcaps,
                               const std::optional<bool> &smallcaps, const std::optional<bool> &superscript, const std::optional<bool> &subscript,
                               const std::optional<double> &scaleWidth) override;
  void collectParaIX(unsigned id, unsigned level, unsigned charCount, const std::optional<double> &indFirst,
                     const std::optional<double> &indLeft, const std::optional<double> &indRight, const std::optional<double> &spLine,
                     const std::optional<double> &spBefore, const std::optional<double> &spAfter, const std::optional<unsigned char> &align,
                     const std::optional<unsigned char> &bullet, const std::optional<VSDName> &bulletStr, const std::optional<VSDName> &bulletFont,
                     const std::optional<double> &bulletFontSize, const std::optional<double> &textPosAfterBullet,
                     const std::optional<unsigned> &flags) override;
  void collectDefaultParaStyle(unsigned charCount, const std::optional<double> &indFirst, const std::optional<double> &indLeft,
                               const std::optional<double> &indRight, const std::optional<double> &spLine, const std::optional<double> &spBefore,
                               const std::optional<double> &spAfter, const std::optional<unsigned char> &align,
                               const std::optional<unsigned char> &bullet, const std::optional<VSDName> &bulletStr,
                               const std::optional<VSDName> &bulletFont, const std::optional<double> &bulletFontSize,
                               const std::optional<double> &textPosAfterBullet, const std::optional<unsigned> &flags) override;
  void collectTextBlock(unsigned level, const std::optional<double> &leftMargin, const std::optional<double> &rightMargin,
                        const std::optional<double> &topMargin, const std::optional<double> &bottomMargin,
                        const std::optional<unsigned char> &verticalAlign, const std::optional<bool> &isBgFilled,
                        const std::optional<Colour> &bgColour, const std::optional<double> &defaultTabStop,
                        const std::optional<unsigned char> &textDirection) override;
  void collectNameList(unsigned id, unsigned level) override
  {
    collectUnhandledChunk(id, level);
  }
  void collectName(unsigned id, unsigned level, const librevenge::RVNGBinaryData &name, TextFormat format) override;
  void collectPageSheet(unsigned id, unsigned level) override;
  void collectMisc(unsigned level, const VSDMisc &misc) override;
  void collectLayer(unsigned id, unsigned level, const VSDLayer &layer) override;
  void collectLayerMem(unsigned level, const VSDName &layerMem) override;
  void collectTabsDataList(unsigned level, const std::map<unsigned, VSDTabSet> &tabSets) override;

  // Style collectors
  void collectStyleSheet(unsigned id, unsigned level,unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle) override;
  void collectLineStyle(unsigned level, const std::optional<double> &strokeWidth, const std::optional<Colour> &c, const std::optional<unsigned char> &linePattern,
                        const std::optional<unsigned char> &startMarker, const std::optional<unsigned char> &endMarker,
                        const std::optional<unsigned char> &lineCap, const std::optional<double> &rounding,
                        const std::optional<long> &qsLineColour, const std::optional<long> &qsLineMatrix) override;
  void collectFillStyle(unsigned level, const std::optional<Colour> &colourFG, const std::optional<Colour> &colourBG,
                        const std::optional<unsigned char> &fillPattern, const std::optional<double> &fillFGTransparency,
                        const std::optional<double> &fillBGTransparency, const std::optional<unsigned char> &shadowPattern,
                        const std::optional<Colour> &shfgc, const std::optional<double> &shadowOffsetX, const std::optional<double> &shadowOffsetY,
                        const std::optional<long> &qsFillColour, const std::optional<long> &qsShadowColour,
                        const std::optional<long> &qsFillMatrix) override;
  void collectFillStyle(unsigned level, const std::optional<Colour> &colourFG, const std::optional<Colour> &colourBG,
                        const std::optional<unsigned char> &fillPattern, const std::optional<double> &fillFGTransparency,
                        const std::optional<double> &fillBGTransparency, const std::optional<unsigned char> &shadowPattern,
                        const std::optional<Colour> &shfgc) override;
  void collectCharIXStyle(unsigned id, unsigned level, unsigned charCount, const std::optional<VSDName> &font,
                          const std::optional<Colour> &fontColour, const std::optional<double> &fontSize, const std::optional<bool> &bold,
                          const std::optional<bool> &italic, const std::optional<bool> &underline, const std::optional<bool> &doubleunderline,
                          const std::optional<bool> &strikeout, const std::optional<bool> &doublestrikeout, const std::optional<bool> &allcaps,
                          const std::optional<bool> &initcaps, const std::optional<bool> &smallcaps, const std::optional<bool> &superscript,
                          const std::optional<bool> &subscript, const std::optional<double> &scaleWidth) override;
  void collectParaIXStyle(unsigned id, unsigned level, unsigned charCount, const std::optional<double> &indFirst,
                          const std::optional<double> &indLeft, const std::optional<double> &indRight, const std::optional<double> &spLine,
                          const std::optional<double> &spBefore, const std::optional<double> &spAfter, const std::optional<unsigned char> &align,
                          const std::optional<unsigned char> &bullet, const std::optional<VSDName> &bulletStr,
                          const std::optional<VSDName> &bulletFont, const std::optional<double> &bulletFontSize,
                          const std::optional<double> &textPosAfterBullet, const std::optional<unsigned> &flags) override;
  void collectTextBlockStyle(unsigned level, const std::optional<double> &leftMargin, const std::optional<double> &rightMargin,
                             const std::optional<double> &topMargin, const std::optional<double> &bottomMargin,
                             const std::optional<unsigned char> &verticalAlign, const std::optional<bool> &isBgFilled,
                             const std::optional<Colour> &bgColour, const std::optional<double> &defaultTabStop,
                             const std::optional<unsigned char> &textDirection) override;

  // Field list
  void collectFieldList(unsigned id, unsigned level) override;
  void collectTextField(unsigned id, unsigned level, int nameId, int formatStringId) override;
  void collectNumericField(unsigned id, unsigned level, unsigned short format, unsigned short cellType, double number, int formatStringId) override;

  void collectMetaData(const librevenge::RVNGPropertyList &) override { }

  // Temporary hack
  void startPage(unsigned pageID) override;
  void endPage() override;
  void endPages() override {}

  const VSDStyles &getStyleSheets() const
  {
    return m_styles;
  }
  const std::optional<unsigned>& getvariationColorIndex() const { return m_variationColorIndex; }
  const std::optional<unsigned>& getvariationStyleIndex() const { return m_variationStyleIndex; }

private:
  VSDStylesCollector(const VSDStylesCollector &);
  VSDStylesCollector &operator=(const VSDStylesCollector &);

  void _handleLevelChange(unsigned level);
  void _flushShapeList();

  unsigned m_currentLevel;
  bool m_isShapeStarted;

  double m_shadowOffsetX;
  double m_shadowOffsetY;

  unsigned m_currentShapeId;
  std::map<unsigned, XForm> m_groupXForms;
  std::map<unsigned, unsigned> m_groupMemberships;
  std::vector<std::map<unsigned, XForm> > &m_groupXFormsSequence;
  std::vector<std::map<unsigned, unsigned> > &m_groupMembershipsSequence;
  std::list<unsigned> m_pageShapeOrder;
  std::vector<std::list<unsigned> > &m_documentPageShapeOrders;
  std::map<unsigned, std::list<unsigned> > m_groupShapeOrder;
  std::list<unsigned> m_shapeList;

  unsigned m_currentStyleSheet;
  VSDStyles m_styles;

  std::optional<unsigned> m_variationColorIndex;
  std::optional<unsigned> m_variationStyleIndex;

  unsigned m_currentShapeLevel;
};

}

#endif /* VSDSTYLESCOLLECTOR_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
