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
  virtual ~VSDStylesCollector() {}

  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void collectForeignData(unsigned level, const librevenge::RVNGBinaryData &binaryData);
  void collectOLEList(unsigned id, unsigned level)
  {
    collectUnhandledChunk(id, level);
  }
  void collectOLEData(unsigned id, unsigned level, const librevenge::RVNGBinaryData &oleData);
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop);
  void collectLine(unsigned level, const boost::optional<double> &strokeWidth, const boost::optional<Colour> &c, const boost::optional<unsigned char> &linePattern,
                   const boost::optional<unsigned char> &startMarker, const boost::optional<unsigned char> &endMarker,
                   const boost::optional<unsigned char> &lineCap);
  void collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                            const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                            const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                            const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX, const boost::optional<double> &shadowOffsetY);
  void collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                            const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                            const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                            const boost::optional<Colour> &shfgc);
  void collectThemeReference(unsigned level, const boost::optional<long> &lineColour, const boost::optional<long> &fillColour,
                             const boost::optional<long> &shadowColour, const boost::optional<long> &fontColour);
  void collectGeometry(unsigned id, unsigned level, bool noFill, bool noLine, bool noShow);
  void collectMoveTo(unsigned id, unsigned level, double x, double y);
  void collectLineTo(unsigned id, unsigned level, double x, double y);
  void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
                      const std::vector<std::pair<double, double> > &ctrlPnts, const std::vector<double> &kntVec, const std::vector<double> &weights);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, const NURBSData &data);
  void collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned char xType, unsigned char yType, const std::vector<std::pair<double, double> > &points);
  void collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned dataID);
  void collectPolylineTo(unsigned id, unsigned level, double x, double y, const PolylineData &data);
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, unsigned degree, double lastKnot,
                        std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights);
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points);
  void collectXFormData(unsigned level, const XForm &xform);
  void collectTxtXForm(unsigned level, const XForm &txtxform);
  void collectShapesOrder(unsigned id, unsigned level, const std::vector<unsigned> &shapeIds);
  void collectForeignDataType(unsigned level, unsigned foreignType, unsigned foreignFormat, double offsetX, double offsetY, double width, double height);
  void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale);
  void collectPage(unsigned id, unsigned level, unsigned backgroundPageID, bool isBackgroundPage, const VSDName &pageName);
  void collectShape(unsigned id, unsigned level, unsigned parent, unsigned masterPage, unsigned masterShape, unsigned lineStyle, unsigned fillStyle, unsigned textStyle);
  void collectSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree);
  void collectSplineKnot(unsigned id, unsigned level, double x, double y, double knot);
  void collectSplineEnd();
  void collectInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2);
  void collectRelCubBezTo(unsigned id, unsigned level, double x, double y, double a, double b, double c, double d);
  void collectRelEllipticalArcTo(unsigned id, unsigned level, double x, double y, double a, double b, double c, double d);
  void collectRelLineTo(unsigned id, unsigned level, double x, double y);
  void collectRelMoveTo(unsigned id, unsigned level, double x, double y);
  void collectRelQuadBezTo(unsigned id, unsigned level, double x, double y, double a, double b);
  void collectUnhandledChunk(unsigned id, unsigned level);

  void collectText(unsigned level, const librevenge::RVNGBinaryData &textStream, TextFormat format);
  void collectCharIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<VSDName> &font,
                     const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                     const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
                     const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps,
                     const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript,
                     const boost::optional<bool> &subscript);
  void collectDefaultCharStyle(unsigned charCount, const boost::optional<VSDName> &font, const boost::optional<Colour> &fontColour,
                               const boost::optional<double> &fontSize, const boost::optional<bool> &bold, const boost::optional<bool> &italic,
                               const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline, const boost::optional<bool> &strikeout,
                               const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps,
                               const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript, const boost::optional<bool> &subscript);
  void collectParaIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                     const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                     const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                     const boost::optional<unsigned> &flags);
  void collectDefaultParaStyle(unsigned charCount, const boost::optional<double> &indFirst, const boost::optional<double> &indLeft,
                               const boost::optional<double> &indRight, const boost::optional<double> &spLine, const boost::optional<double> &spBefore,
                               const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align, const boost::optional<unsigned> &flags);
  void collectTextBlock(unsigned level, const boost::optional<double> &leftMargin, const boost::optional<double> &rightMargin,
                        const boost::optional<double> &topMargin, const boost::optional<double> &bottomMargin,
                        const boost::optional<unsigned char> &verticalAlign, const boost::optional<bool> &isBgFilled,
                        const boost::optional<Colour> &bgColour, const boost::optional<double> &defaultTabStop,
                        const boost::optional<unsigned char> &textDirection);
  void collectNameList(unsigned id, unsigned level)
  {
    collectUnhandledChunk(id, level);
  }
  void collectName(unsigned id, unsigned level, const librevenge::RVNGBinaryData &name, TextFormat format);
  void collectPageSheet(unsigned id, unsigned level);
  void collectMisc(unsigned level, const VSDMisc &misc);

  // Style collectors
  void collectStyleSheet(unsigned id, unsigned level,unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle);
  void collectLineStyle(unsigned level, const boost::optional<double> &strokeWidth, const boost::optional<Colour> &c, const boost::optional<unsigned char> &linePattern,
                        const boost::optional<unsigned char> &startMarker, const boost::optional<unsigned char> &endMarker,
                        const boost::optional<unsigned char> &lineCap);
  void collectFillStyle(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                        const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                        const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                        const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX, const boost::optional<double> &shadowOffsetY);
  void collectFillStyle(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                        const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                        const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                        const boost::optional<Colour> &shfgc);
  void collectCharIXStyle(unsigned id, unsigned level, unsigned charCount, const boost::optional<VSDName> &font,
                          const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                          const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
                          const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps,
                          const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript,
                          const boost::optional<bool> &subscript);
  void collectParaIXStyle(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                          const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                          const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                          const boost::optional<unsigned> &flags);
  void collectTextBlockStyle(unsigned level, const boost::optional<double> &leftMargin, const boost::optional<double> &rightMargin,
                             const boost::optional<double> &topMargin, const boost::optional<double> &bottomMargin,
                             const boost::optional<unsigned char> &verticalAlign, const boost::optional<bool> &isBgFilled,
                             const boost::optional<Colour> &bgColour, const boost::optional<double> &defaultTabStop,
                             const boost::optional<unsigned char> &textDirection);
  void collectStyleThemeReference(unsigned level, const boost::optional<long> &lineColour, const boost::optional<long> &fillColour,
                                  const boost::optional<long> &shadowColour, const boost::optional<long> &fontColour);

  // Field list
  void collectFieldList(unsigned id, unsigned level);
  void collectTextField(unsigned id, unsigned level, int nameId, int formatStringId);
  void collectNumericField(unsigned id, unsigned level, unsigned short format, double number, int formatStringId);

  virtual void collectMetaData(const librevenge::RVNGPropertyList &) { }

  // Temporary hack
  void startPage(unsigned pageID);
  void endPage();
  void endPages() {}

  const VSDStyles &getStyleSheets() const
  {
    return m_styles;
  }


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

  unsigned m_currentShapeLevel;
};

}

#endif /* VSDSTYLESCOLLECTOR_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
