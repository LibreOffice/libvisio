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
  virtual ~VSDStylesCollector() {};

  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData);
  void collectOLEList(unsigned id, unsigned level)
  {
    collectUnhandledChunk(id, level);
  }
  void collectOLEData(unsigned id, unsigned level, const WPXBinaryData &oleData);
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop);
  void collectLine(unsigned id, unsigned level, double strokeWidth, const Colour &c, unsigned linePattern, unsigned char startMarker,
                   unsigned char endMarker, unsigned lineCap);
  void collectFillAndShadow(unsigned id, unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned fillPattern,
                            double fillFGTransparency, double fillBGTransparency, unsigned shadowPattern, const Colour &shfgc,
                            double shadowOffsetX, double shadowOffsetY);
  void collectFillAndShadow(unsigned id, unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned fillPattern,
                            double fillFGTransparency, double fillBGTransparency, unsigned shadowPattern, const Colour &shfgc);
  void collectGeometry(unsigned id, unsigned level, unsigned char geomFlags);
  void collectMoveTo(unsigned id, unsigned level, double x, double y);
  void collectLineTo(unsigned id, unsigned level, double x, double y);
  void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType,
                      unsigned degree, std::vector<std::pair<double, double> > controlPoints,
                      std::vector<double> knotVector, std::vector<double> weights);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID);
  void collectPolylineTo(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > &points);
  void collectPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID);
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, unsigned degree, double lastKnot,
                        std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights);
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points);
  void collectXFormData(unsigned id, unsigned level, const XForm &xform);
  void collectTxtXForm(unsigned id, unsigned level, const XForm &txtxform);
  void collectShapeId(unsigned id, unsigned level, unsigned shapeId);
  void collectForeignDataType(unsigned id, unsigned level, unsigned foreignType, unsigned foreignFormat, double offsetX, double offsetY, double width, double height);
  void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale);
  void collectPage(unsigned id, unsigned level, unsigned backgroundPageID, bool isBackgroundPage);
  void collectShape(unsigned id, unsigned level, unsigned masterPage, unsigned masterShape, unsigned lineStyle, unsigned fillStyle, unsigned textStyle);
  void collectSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree);
  void collectSplineKnot(unsigned id, unsigned level, double x, double y, double knot);
  void collectSplineEnd();
  void collectInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2);
  void collectUnhandledChunk(unsigned id, unsigned level);

  void collectFont(unsigned short fontID, const ::WPXBinaryData &textStream, TextFormat format);

  void collectText(unsigned id, unsigned level, const ::WPXBinaryData &textStream, TextFormat format);
  void collectVSDCharStyle(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour,
                           double fontSize, bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                           bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, VSDFont fontFace);
  void collectVSDParaStyle(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                           double spLine, double spBefore, double spAfter, unsigned char align, unsigned flags);
  void collectTextBlock(unsigned id, unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin,
                        unsigned char verticalAlign, bool isBgFilled, const Colour &bgColour, double defaultTabStop, unsigned char textDirection);
  void collectNameList(unsigned id, unsigned level)
  {
    collectUnhandledChunk(id, level);
  }
  void collectName(unsigned id, unsigned level,  const ::WPXBinaryData &name, TextFormat format);
  void collectPageSheet(unsigned id, unsigned level);

  // Style collectors
  void collectStyleSheet(unsigned id, unsigned level, unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle);
  void collectLineStyle(unsigned id, unsigned level, double strokeWidth, const Colour &c, unsigned char linePattern,
                        unsigned char startMarker, unsigned char endMarker, unsigned char lineCap);
  void collectFillStyle(unsigned id, unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned char fillPattern,
                        double fillFGTransparency, double fillBGTransparency, unsigned char shadowPattern,
                        const Colour &shfgc, double shadowOffsetX, double shadowOffsetY);
  void collectFillStyle(unsigned id, unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned char fillPattern,
                        double fillFGTransparency, double fillBGTransparency, unsigned char shadowPattern,
                        const Colour &shfgc);
  void collectCharIXStyle(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour,
                          double fontSize, bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                          bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, VSDFont fontFace);
  void collectParaIXStyle(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                          double spLine, double spBefore, double spAfter, unsigned char align, unsigned flags);
  void collectTextBlockStyle(unsigned id, unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin,
                             unsigned char verticalAlign, bool isBgFilled, const Colour &bgColour, double defaultTabStop, unsigned char textDirection);

  // Field list
  void collectFieldList(unsigned id, unsigned level);
  void collectTextField(unsigned id, unsigned level, int nameId, int formatStringId);
  void collectNumericField(unsigned id, unsigned level, unsigned short format, double number, int formatStringId);

  // Temporary hack
  void startPage(unsigned pageID);
  void endPage();
  void endPages() {}

  VSDStyles getStyleSheets() const
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
  VSDLineStyle *m_lineStyle;
  VSDFillStyle *m_fillStyle;
  VSDTextBlockStyle *m_textBlockStyle;
  VSDCharStyle *m_charStyle;
  VSDParaStyle *m_paraStyle;

  unsigned m_lineStyleMaster;
  unsigned m_fillStyleMaster;
  unsigned m_textStyleMaster;
  bool m_isStyleStarted;
  unsigned m_currentShapeLevel;
};

}

#endif /* VSDSTYLESCOLLECTOR_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
