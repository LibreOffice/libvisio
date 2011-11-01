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

#ifndef VSDXCONTENTCOLLECTOR_H
#define VSDXCONTENTCOLLECTOR_H

#include <locale.h>
#include <sstream>
#include <string>
#include <cmath>
#include <map>
#include <list>
#include <vector>
#include <libwpg/libwpg.h>
#include "libvisio_utils.h"
#include "VSDXCollector.h"
#include "VSDXParser.h"
#include "VSDXOutputElementList.h"
#include "VSDXStyles.h"
#include "VSDXPages.h"

namespace libvisio
{

class VSDXContentCollector : public VSDXCollector
{
public:
  VSDXContentCollector(
    libwpg::WPGPaintInterface *painter,
    std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
    std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
    std::vector<std::list<unsigned> > &documentPageShapeOrders,
    VSDXStyles &styles, VSDXStencils &stencils
  );
  virtual ~VSDXContentCollector()
  {
    if (m_txtxform) delete(m_txtxform);
  };

  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData);
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop);
  void collectLine(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern, unsigned char startMarker, unsigned char endMarker, unsigned lineCap);
  void collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern,
                            unsigned fillFGTransparency, unsigned fillBGTransparency, unsigned shadowPattern, Colour shfgc,
                            double shadowOffsetX, double shadowOffsetY);
  void collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern,
                            unsigned fillFGTransparency, unsigned fillBGTransparency, unsigned shadowPattern, Colour shfgc);
  void collectGeomList(unsigned id, unsigned level);
  void collectGeometry(unsigned id, unsigned level, unsigned char geomFlags);
  void collectMoveTo(unsigned id, unsigned level, double x, double y);
  void collectLineTo(unsigned id, unsigned level, double x, double y);
  void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
                      std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID);
  void collectPolylineTo(unsigned id , unsigned level, double x, double y, unsigned char xType, unsigned char yType,
                         std::vector<std::pair<double, double> > &points);
  void collectPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID);
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, unsigned degree, double lastKnot,
                        std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights);
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points);
  void collectXFormData(unsigned id, unsigned level, const XForm &xform);
  void collectTxtXForm(unsigned id, unsigned level, const XForm &txtxform);
  void collectShapeId(unsigned id, unsigned level, unsigned shapeId);
  void collectShapeList(unsigned id, unsigned level);
  void collectForeignDataType(unsigned id, unsigned level, unsigned foreignType, unsigned foreignFormat);
  void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale);
  void collectPage(unsigned id, unsigned level, unsigned backgroundPageID, unsigned currentPageID);
  void collectShape(unsigned id, unsigned level, unsigned masterPage, unsigned masterShape, unsigned lineStyle, unsigned fillStyle, unsigned textStyle);
  void collectSplineStart(unsigned id, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree);
  void collectSplineKnot(unsigned id, unsigned level, double x, double y, double knot);
  void collectSplineEnd();
  void collectInfiniteLine(unsigned id, unsigned level, double x1, double y1, double x2, double y2);

  void collectUnhandledChunk(unsigned id, unsigned level);

  void collectColours(const std::vector<Colour> &colours);
  void collectFont(unsigned short fontID, const std::vector<unsigned char> &textStream, TextFormat format);
  void collectCharList(unsigned id, unsigned level);
  void collectParaList(unsigned id, unsigned level);
  void collectText(unsigned id, unsigned level, const std::vector<unsigned char> &textStream, TextFormat format);
  void collectVSDXCharStyle(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, unsigned langId,
                            double fontSize, bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                            bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, WPXString fontFace);
  void collectVSDXParaStyle(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                            double spLine, double spBefore, double spAfter, unsigned char align);
  void collectTextBlock(unsigned id, unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin, unsigned char verticalAlign,
                        unsigned char bgClrId, const Colour &bgColour, double defaultTabStop, unsigned char textDirection);


  // Style collectors
  void collectStyleSheet(unsigned id, unsigned level, unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle);
  void collectLineStyle(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned char linePattern, unsigned char startMarker, unsigned char endMarker, unsigned char lineCap);
  void collectFillStyle(unsigned id, unsigned level, unsigned char colourIndexFG, unsigned char colourIndexBG, unsigned char fillPattern,
                        unsigned char fillFGTransparency, unsigned char fillBGTransparency, unsigned char shadowPattern, Colour shfgc,
                        double shadowOffsetX, double shadowOffsetY);
  void collectFillStyle(unsigned id, unsigned level, unsigned char colourIndexFG, unsigned char colourIndexBG, unsigned char fillPattern,
                        unsigned char fillFGTransparency, unsigned char fillBGTransparency, unsigned char shadowPattern, Colour shfgc);
  void collectCharIXStyle(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, unsigned langId,
                          double fontSize, bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                          bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, WPXString fontFace);
  void collectParaIXStyle(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                          double spLine, double spBefore, double spAfter, unsigned char align);
  void collectTextBlockStyle(unsigned id, unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin, unsigned char verticalAlign,
                             unsigned char bgClrId, const Colour &bgColour, double defaultTabStop, unsigned char textDirection);

  void startPage();
  void endPage();
  void endPages();


private:
  VSDXContentCollector(const VSDXContentCollector &);
  VSDXContentCollector &operator=(const VSDXContentCollector &);
  libwpg::WPGPaintInterface *m_painter;

  void applyXForm(double &x, double &y, const XForm &xform);

  void transformPoint(double &x, double &y, XForm *txtxform = 0);
  void transformAngle(double &angle, XForm *txtxform = 0);

  double _NURBSBasis(unsigned knot, unsigned degree, double point, const std::vector<double> &knotVector);

  void _flushCurrentPath();
  void _flushText();
  void _flushCurrentForeignData();
  void _flushCurrentPage();

  void _handleLevelChange(unsigned level);
  void _appendUTF16LE(WPXString &text, WPXInputStream *input);
  void _appendUCS4(WPXString &text, unsigned ucs4Character);

  void _handleForeignData(const WPXBinaryData &data);

  void lineStyleFromStyleSheet(unsigned styleId);
  void fillStyleFromStyleSheet(unsigned styleId);
  void lineStyleFromStyleSheet(const VSDXLineStyle *style);
  void fillStyleFromStyleSheet(const VSDXFillStyle *style);

  void _lineProperties(double strokeWidth, Colour c, unsigned linePattern, unsigned startMarker, unsigned endMarker, unsigned lineCap);
  void _fillAndShadowProperties(unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern,
                                unsigned fillFGTransparency, unsigned fillBGTransparency,
                                unsigned shadowPattern, Colour shfgc, double shadowOffsetX, double shadowOffsetY);

  bool m_isPageStarted;
  double m_pageWidth;
  double m_pageHeight;
  double m_shadowOffsetX;
  double m_shadowOffsetY;
  double m_scale;
  double m_x;
  double m_y;
  double m_originalX;
  double m_originalY;
  XForm m_xform;
  XForm *m_txtxform;
  std::vector<WPXPropertyList> m_currentGeometry;
  std::map<unsigned, XForm> &m_groupXForms;
  WPXBinaryData m_currentForeignData;
  WPXPropertyList m_currentForeignProps;
  unsigned m_currentShapeId;
  unsigned m_foreignType;
  unsigned m_foreignFormat;
  WPXPropertyList m_styleProps;
  ::WPXString m_lineColour;
  ::WPXString m_fillType;
  unsigned m_linePattern;
  unsigned m_fillPattern;
  unsigned m_fillFGTransparency;
  unsigned m_fillBGTransparency;
  bool m_noLine;
  bool m_noFill;
  bool m_noShow;
  std::vector<Colour> m_colours;
  std::map<unsigned short, WPXString> m_fonts;
  unsigned m_currentLevel;
  bool m_isShapeStarted;
  std::map<unsigned, unsigned> &m_groupMemberships;
  std::vector<std::map<unsigned, XForm> > &m_groupXFormsSequence;
  std::vector<std::map<unsigned, unsigned> > &m_groupMembershipsSequence;
  unsigned m_currentPageNumber;
  VSDXOutputElementList *m_shapeOutputDrawing, *m_shapeOutputText;
  std::map<unsigned, VSDXOutputElementList> m_pageOutputDrawing;
  std::map<unsigned, VSDXOutputElementList> m_pageOutputText;
  std::vector<std::list<unsigned> > &m_documentPageShapeOrders;
  std::list<unsigned> &m_pageShapeOrder;
  bool m_isFirstGeometry;

  std::map<unsigned, NURBSData> m_NURBSData;
  std::map<unsigned, PolylineData> m_polylineData;
  std::vector<unsigned char> m_textStream;
  TextFormat m_textFormat;
  std::vector<VSDXCharStyle> m_charFormats;
  std::vector<VSDXParaStyle> m_paraFormats;

  VSDXTextBlockStyle m_textBlockStyle;

  VSDXCharStyle m_defaultCharStyle;
  VSDXParaStyle m_defaultParaStyle;

  VSDXStyles m_styles;

  VSDXStencils m_stencils;
  const VSDXStencilShape *m_stencilShape;
  bool m_isStencilStarted;

  unsigned m_currentGeometryCount;

  unsigned m_backgroundPageID;
  unsigned m_currentPageID;
  VSDXPage m_currentPage;
  VSDXPages m_pages;

  std::vector<std::pair<double, double> > m_splineControlPoints;
  std::vector<double> m_splineKnotVector;
  double m_splineX, m_splineY;
  double m_splineLastKnot;
  unsigned m_splineDegree;
  unsigned m_splineLevel;
};

} // namespace libvisio

#endif /* VSDXCONTENTCOLLECTOR_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
