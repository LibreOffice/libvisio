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

#ifndef VSDCONTENTCOLLECTOR_H
#define VSDCONTENTCOLLECTOR_H

#include <locale.h>
#include <sstream>
#include <string>
#include <cmath>
#include <map>
#include <list>
#include <vector>
#include <libwpg/libwpg.h>
#include "libvisio_utils.h"
#include "VSDCollector.h"
#include "VSDParser.h"
#include "VSDOutputElementList.h"
#include "VSDStyles.h"
#include "VSDPages.h"

namespace libvisio
{

class VSDContentCollector : public VSDCollector
{
public:
  VSDContentCollector(
    libwpg::WPGPaintInterface *painter,
    std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
    std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
    std::vector<std::list<unsigned> > &documentPageShapeOrders,
    VSDStyles &styles, VSDStencils &stencils
  );
  virtual ~VSDContentCollector()
  {
    if (m_txtxform) delete(m_txtxform);
  };

  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData);
  void collectOLEList(unsigned id, unsigned level);
  void collectOLEData(unsigned id, unsigned level, const WPXBinaryData &oleData);
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop);
  void collectLine(unsigned level, double strokeWidth, const Colour &c, unsigned linePattern, unsigned char startMarker, unsigned char endMarker, unsigned lineCap);
  void collectFillAndShadow(unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned fillPattern,
                            double fillFGTransparency, double fillBGTransparency, unsigned shadowPattern, const Colour &shfgc,
                            double shadowOffsetX, double shadowOffsetY);
  void collectFillAndShadow(unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned fillPattern,
                            double fillFGTransparency, double fillBGTransparency, unsigned shadowPattern, const Colour &shfgc);
  void collectGeometry(unsigned id, unsigned level, bool noFill, bool noLine, bool noShow);
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
  void collectXFormData(unsigned level, const XForm &xform);
  void collectTxtXForm(unsigned level, const XForm &txtxform);
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

  void collectFont(unsigned short fontID, const WPXBinaryData &textStream, TextFormat format);
  void collectText(unsigned id, unsigned level, const WPXBinaryData &textStream, TextFormat format);
  void collectVSDCharStyle(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, double fontSize,
                           bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                           bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, VSDFont fontFace);
  void collectVSDParaStyle(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                           double spLine, double spBefore, double spAfter, unsigned char align, unsigned flags);
  void collectTextBlock(unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin,
                        unsigned char verticalAlign, bool isBgFilled, const Colour &bgColour, double defaultTabStop, unsigned char textDirection);
  void collectNameList(unsigned id, unsigned level);
  void collectName(unsigned id, unsigned level,  const WPXBinaryData &name, TextFormat format);
  void collectPageSheet(unsigned id, unsigned level);


  // Style collectors
  void collectStyleSheet(unsigned id, unsigned level, unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle);
  void collectLineStyle(unsigned level, double strokeWidth, const Colour &c, unsigned char linePattern,
                        unsigned char startMarker, unsigned char endMarker, unsigned char lineCap);
  void collectFillStyle(unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned char fillPattern,
                        double fillFGTransparency, double fillBGTransparency, unsigned char shadowPattern, const Colour &shfgc,
                        double shadowOffsetX, double shadowOffsetY);
  void collectFillStyle(unsigned level, const Colour &colourFG, const Colour &colourBG, unsigned char fillPattern,
                        double fillFGTransparency, double fillBGTransparency, unsigned char shadowPattern, const Colour &shfgc);
  void collectCharIXStyle(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, double fontSize,
                          bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                          bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, VSDFont fontFace);
  void collectParaIXStyle(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                          double spLine, double spBefore, double spAfter, unsigned char align, unsigned flags);
  void collectTextBlockStyle(unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin,
                             unsigned char verticalAlign, bool isBgFilled, const Colour &bgColour, double defaultTabStop, unsigned char textDirection);

  // Field list
  void collectFieldList(unsigned id, unsigned level);
  void collectTextField(unsigned id, unsigned level, int nameId, int formatStringId);
  void collectNumericField(unsigned id, unsigned level, unsigned short format, double number, int formatStringId);

  void startPage(unsigned pageId);
  void endPage();
  void endPages();


private:
  VSDContentCollector(const VSDContentCollector &);
  VSDContentCollector &operator=(const VSDContentCollector &);
  libwpg::WPGPaintInterface *m_painter;

  void applyXForm(double &x, double &y, const XForm &xform);

  void transformPoint(double &x, double &y, XForm *txtxform = 0);
  void transformAngle(double &angle, XForm *txtxform = 0);
  void transformFlips(bool &flipX, bool &flipY);

  double _NURBSBasis(unsigned knot, unsigned degree, double point, const std::vector<double> &knotVector);

  void _flushCurrentPath();
  void _flushText();
  void _flushCurrentForeignData();
  void _flushCurrentPage();

  void _handleLevelChange(unsigned level);

  void _handleForeignData(const WPXBinaryData &data);

  void lineStyleFromStyleSheet(unsigned styleId);
  void fillStyleFromStyleSheet(unsigned styleId);
  void lineStyleFromStyleSheet(const VSDLineStyle *style);
  void fillStyleFromStyleSheet(const VSDFillStyle *style);

  void _applyLinePattern();
  void _lineProperties(double strokeWidth, Colour c, unsigned linePattern, unsigned startMarker, unsigned endMarker, unsigned lineCap);
  const char *_linePropertiesMarkerViewbox(unsigned marker);
  const char *_linePropertiesMarkerPath(unsigned marker);
  double _linePropertiesMarkerScale(unsigned marker);
  void _fillAndShadowProperties(const Colour &colourFG, const Colour &colourBG, unsigned fillPattern, double fillFGTransparency,
                                double fillBGTransparency, unsigned shadowPattern, const Colour &shfgc, double shadowOffsetX, double shadowOffsetY);

  void appendCharacters(WPXString &text, const std::vector<unsigned char> &characters, TextFormat format);
  void appendCharacters(WPXString &text, const std::vector<unsigned char> &characters);
  void _convertDataToString(WPXString &result, const WPXBinaryData &data, TextFormat format);
  bool parseFormatId( const char *formatString, unsigned short &result );

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
  std::vector<WPXPropertyList> m_currentFillGeometry;
  std::vector<WPXPropertyList> m_currentLineGeometry;
  std::map<unsigned, XForm> *m_groupXForms;
  WPXBinaryData m_currentForeignData;
  WPXBinaryData m_currentOLEData;
  WPXPropertyList m_currentForeignProps;
  unsigned m_currentShapeId;
  unsigned m_foreignType;
  unsigned m_foreignFormat;
  double m_foreignOffsetX;
  double m_foreignOffsetY;
  double m_foreignWidth;
  double m_foreignHeight;
  WPXPropertyList m_styleProps;
  ::WPXString m_lineColour;
  ::WPXString m_fillType;
  unsigned m_linePattern;
  unsigned m_fillPattern;
  double m_fillFGTransparency;
  double m_fillBGTransparency;
  bool m_noLine;
  bool m_noFill;
  bool m_noShow;
  std::map<unsigned short, VSDFont> m_fonts;
  unsigned m_currentLevel;
  bool m_isShapeStarted;
  std::map<unsigned, unsigned> &m_groupMemberships;
  std::vector<std::map<unsigned, XForm> > &m_groupXFormsSequence;
  std::vector<std::map<unsigned, unsigned> > &m_groupMembershipsSequence;
  unsigned m_currentPageNumber;
  VSDOutputElementList *m_shapeOutputDrawing, *m_shapeOutputText;
  std::map<unsigned, VSDOutputElementList> m_pageOutputDrawing;
  std::map<unsigned, VSDOutputElementList> m_pageOutputText;
  std::vector<std::list<unsigned> > &m_documentPageShapeOrders;
  std::list<unsigned> &m_pageShapeOrder;
  bool m_isFirstGeometry;

  std::map<unsigned, NURBSData> m_NURBSData;
  std::map<unsigned, PolylineData> m_polylineData;
  WPXBinaryData m_textStream;
  std::map<unsigned, WPXString> m_names, m_stencilNames;
  std::vector<WPXString> m_fields;
  VSDFieldList m_stencilFields;
  unsigned m_fieldIndex;
  TextFormat m_textFormat;
  std::vector<VSDCharStyle> m_charFormats;
  std::vector<VSDParaStyle> m_paraFormats;

  VSDTextBlockStyle m_textBlockStyle;

  VSDCharStyle m_defaultCharStyle;
  VSDParaStyle m_defaultParaStyle;

  VSDStyles m_styles;

  VSDStencils m_stencils;
  const VSDShape *m_stencilShape;
  bool m_isStencilStarted;

  unsigned m_currentGeometryCount;

  unsigned m_backgroundPageID;
  unsigned m_currentPageID;
  VSDPage m_currentPage;
  VSDPages m_pages;

  std::vector<std::pair<double, double> > m_splineControlPoints;
  std::vector<double> m_splineKnotVector;
  double m_splineX, m_splineY;
  double m_splineLastKnot;
  unsigned m_splineDegree;
  unsigned m_splineLevel;
  unsigned m_currentShapeLevel;
  bool m_isBackgroundPage;
};

} // namespace libvisio

#endif /* VSDCONTENTCOLLECTOR_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
