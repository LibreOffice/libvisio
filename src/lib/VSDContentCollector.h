/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VSDCONTENTCOLLECTOR_H
#define VSDCONTENTCOLLECTOR_H

#include <locale.h>
#include <sstream>
#include <string>
#include <cmath>
#include <map>
#include <memory>
#include <list>
#include <vector>
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
    librevenge::RVNGDrawingInterface *painter,
    std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
    std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
    std::vector<std::list<unsigned> > &documentPageShapeOrders,
    VSDStyles &styles, VSDStencils &stencils
  );

  void collectDocumentTheme(const VSDXTheme *theme) override;
  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) override;
  void collectForeignData(unsigned level, const librevenge::RVNGBinaryData &binaryData) override;
  void collectOLEList(unsigned id, unsigned level) override;
  void collectOLEData(unsigned id, unsigned level, const librevenge::RVNGBinaryData &oleData) override;
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop) override;
  void collectLine(unsigned level, const boost::optional<double> &strokeWidth, const boost::optional<Colour> &c, const boost::optional<unsigned char> &linePattern,
                   const boost::optional<unsigned char> &startMarker, const boost::optional<unsigned char> &endMarker,
                   const boost::optional<unsigned char> &lineCap, const boost::optional<double> &rounding,
                   const boost::optional<long> &qsLineColour, const boost::optional<long> &qsLineMatrix) override;
  void collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                            const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                            const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                            const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX, const boost::optional<double> &shadowOffsetY,
                            const boost::optional<long> &qsFc, const boost::optional<long> &qsSc, const boost::optional<long> &qsLm) override;
  void collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                            const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                            const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                            const boost::optional<Colour> &shfgc) override;
  void collectGeometry(unsigned id, unsigned level, bool noFill, bool noLine, bool noShow) override;
  void collectMoveTo(unsigned id, unsigned level, double x, double y) override;
  void collectLineTo(unsigned id, unsigned level, double x, double y) override;
  void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow) override;
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree,
                      const std::vector<std::pair<double, double> > &ctrlPnts, const std::vector<double> &kntVec, const std::vector<double> &weights) override;
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID) override;
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, const NURBSData &data) override;
  void collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned char xType, unsigned char yType,
                         const std::vector<std::pair<double, double> > &points) override;
  void collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned dataID) override;
  void collectPolylineTo(unsigned id, unsigned level, double x, double y, const PolylineData &data) override;
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, unsigned degree, double lastKnot,
                        std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) override;
  void collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points) override;
  void collectXFormData(unsigned level, const XForm &xform) override;
  void collectTxtXForm(unsigned level, const XForm &txtxform) override;
  void collectShapesOrder(unsigned id, unsigned level, const std::vector<unsigned> &shapeIds) override;
  void collectForeignDataType(unsigned level, unsigned foreignType, unsigned foreignFormat, double offsetX, double offsetY, double width, double height) override;
  void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale) override;
  void collectPage(unsigned id, unsigned level, unsigned backgroundPageID, bool isBackgroundPage, const VSDName &pageName) override;
  void collectShape(unsigned id, unsigned level, unsigned parent, unsigned masterPage, unsigned masterShape, unsigned lineStyle, unsigned fillStyle, unsigned textStyle) override;
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
  void collectCharIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<VSDName> &font,
                     const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                     const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
                     const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps,
                     const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript,
                     const boost::optional<bool> &subscript, const boost::optional<double> &scaleWidth) override;
  void collectDefaultCharStyle(unsigned charCount, const boost::optional<VSDName> &font, const boost::optional<Colour> &fontColour,
                               const boost::optional<double> &fontSize, const boost::optional<bool> &bold, const boost::optional<bool> &italic,
                               const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline, const boost::optional<bool> &strikeout,
                               const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps,
                               const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript, const boost::optional<bool> &subscript,
                               const boost::optional<double> &scaleWidth) override;
  void collectParaIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                     const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                     const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                     const boost::optional<unsigned char> &bullet, const boost::optional<VSDName> &bulletStr, const boost::optional<VSDName> &bulletFont,
                     const boost::optional<double> &bulletFontSize, const boost::optional<double> &textPosAfterBullet,
                     const boost::optional<unsigned> &flags) override;
  void collectDefaultParaStyle(unsigned charCount, const boost::optional<double> &indFirst, const boost::optional<double> &indLeft,
                               const boost::optional<double> &indRight, const boost::optional<double> &spLine, const boost::optional<double> &spBefore,
                               const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                               const boost::optional<unsigned char> &bullet, const boost::optional<VSDName> &bulletStr,
                               const boost::optional<VSDName> &bulletFont, const boost::optional<double> &bulletFontSize,
                               const boost::optional<double> &textPosAfterBullet, const boost::optional<unsigned> &flags) override;
  void collectTextBlock(unsigned level, const boost::optional<double> &leftMargin, const boost::optional<double> &rightMargin,
                        const boost::optional<double> &topMargin, const boost::optional<double> &bottomMargin,
                        const boost::optional<unsigned char> &verticalAlign, const boost::optional<bool> &isBgFilled,
                        const boost::optional<Colour> &bgColour, const boost::optional<double> &defaultTabStop,
                        const boost::optional<unsigned char> &textDirection) override;
  void collectNameList(unsigned id, unsigned level) override;
  void collectName(unsigned id, unsigned level,  const librevenge::RVNGBinaryData &name, TextFormat format) override;
  void collectPageSheet(unsigned id, unsigned level) override;
  void collectMisc(unsigned level, const VSDMisc &misc) override;
  void collectLayer(unsigned id, unsigned level, const VSDLayer &layer) override;
  void collectLayerMem(unsigned level, const VSDName &layerMem) override;
  void collectTabsDataList(unsigned level, const std::map<unsigned, VSDTabSet> &tabSets) override;

  // Style collectors
  void collectStyleSheet(unsigned id, unsigned level, unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle) override;
  void collectLineStyle(unsigned level, const boost::optional<double> &strokeWidth, const boost::optional<Colour> &c, const boost::optional<unsigned char> &linePattern,
                        const boost::optional<unsigned char> &startMarker, const boost::optional<unsigned char> &endMarker,
                        const boost::optional<unsigned char> &lineCap, const boost::optional<double> &rounding,
                        const boost::optional<long> &qsLineColour, const boost::optional<long> &qsLineMatrix) override;
  void collectFillStyle(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                        const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                        const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                        const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX, const boost::optional<double> &shadowOffsetY,
                        const boost::optional<long> &qsFillColour, const boost::optional<long> &qsShadowColour,
                        const boost::optional<long> &qsFillMatrix) override;
  void collectFillStyle(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                        const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                        const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                        const boost::optional<Colour> &shfgc) override;
  void collectCharIXStyle(unsigned id, unsigned level, unsigned charCount, const boost::optional<VSDName> &font,
                          const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                          const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
                          const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps,
                          const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript,
                          const boost::optional<bool> &subscript, const boost::optional<double> &scaleWidth) override;
  void collectParaIXStyle(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                          const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                          const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                          const boost::optional<unsigned char> &bullet, const boost::optional<VSDName> &bulletStr,
                          const boost::optional<VSDName> &bulletFont, const boost::optional<double> &bulletFontSize,
                          const boost::optional<double> &textPosAfterBullet, const boost::optional<unsigned> &flags) override;
  void collectTextBlockStyle(unsigned level, const boost::optional<double> &leftMargin, const boost::optional<double> &rightMargin,
                             const boost::optional<double> &topMargin, const boost::optional<double> &bottomMargin,
                             const boost::optional<unsigned char> &verticalAlign, const boost::optional<bool> &isBgFilled,
                             const boost::optional<Colour> &bgColour, const boost::optional<double> &defaultTabStop,
                             const boost::optional<unsigned char> &textDirection) override;

  void collectMetaData(const librevenge::RVNGPropertyList &metaData) override;


  // Field list
  void collectFieldList(unsigned id, unsigned level) override;
  void collectTextField(unsigned id, unsigned level, int nameId, int formatStringId) override;
  void collectNumericField(unsigned id, unsigned level, unsigned short format, unsigned short cellType, double number, int formatStringId) override;

  void startPage(unsigned pageId) override;
  void endPage() override;
  void endPages() override;


private:
  VSDContentCollector(const VSDContentCollector &);
  VSDContentCollector &operator=(const VSDContentCollector &);
  librevenge::RVNGDrawingInterface *m_painter;

  void applyXForm(double &x, double &y, const XForm &xform);

  void transformPoint(double &x, double &y, XForm *txtxform = nullptr);
  void transformAngle(double &angle, XForm *txtxform = nullptr);
  void transformFlips(bool &flipX, bool &flipY);

  double _NURBSBasis(unsigned knot, unsigned degree, double point, const std::vector<double> &knotVector);

  void _flushShape();
  void _flushCurrentPath(unsigned id);
  void _flushText();
  void _flushCurrentForeignData();
  void _flushCurrentPage();

  void _handleLevelChange(unsigned level);

  void _handleForeignData(const librevenge::RVNGBinaryData &data);

  void _lineProperties(const VSDLineStyle &style, librevenge::RVNGPropertyList &styleProps);
  void _fillAndShadowProperties(const VSDFillStyle &style, librevenge::RVNGPropertyList &styleProps);

  void _applyLinePattern();
  const char *_linePropertiesMarkerViewbox(unsigned marker);
  const char *_linePropertiesMarkerPath(unsigned marker);
  double _linePropertiesMarkerScale(unsigned marker);

  void appendCharacters(librevenge::RVNGString &text, const std::vector<unsigned char> &characters, TextFormat format);
  void appendCharacters(librevenge::RVNGString &text, const std::vector<unsigned char> &characters);
  void _convertDataToString(librevenge::RVNGString &result, const librevenge::RVNGBinaryData &data, TextFormat format);
  bool parseFormatId(const char *formatString, unsigned short &result);
  void _appendField(librevenge::RVNGString &text);

  // NURBS processing functions
  bool _isUniform(const std::vector<double> &weights) const;
  void _generatePolylineFromNURBS(unsigned degree, const std::vector<std::pair<double, double> > &controlPoints,
                                  const std::vector<double> &knotVector, const std::vector<double> &weights);
  void _generateBezierSegmentsFromNURBS(unsigned degree, const std::vector<std::pair<double, double> > &controlPoints,
                                        const std::vector<double> &knotVector);
  void _outputCubicBezierSegment(const std::vector<std::pair<double, double> > &points);
  void _outputQuadraticBezierSegment(const std::vector<std::pair<double, double> > &points);
  void _outputLinearBezierSegment(const std::vector<std::pair<double, double> > &points);
  void _appendVisibleAndPrintable(librevenge::RVNGPropertyList &propList);
  void _bulletFromParaFormat(VSDBullet &bullet, const VSDParaStyle &paraStyle);
  void _listLevelFromBullet(librevenge::RVNGPropertyList &propList, const VSDBullet &bullet);
  void _fillParagraphProperties(librevenge::RVNGPropertyList &propList, const VSDParaStyle &style);
  void _fillTabSet(librevenge::RVNGPropertyList &propList, const VSDTabSet &tabSet);
  void _fillCharProperties(librevenge::RVNGPropertyList &propList, const VSDCharStyle &style);
  void _convertToPath(const std::vector<librevenge::RVNGPropertyList> &segmentVector,
                      librevenge::RVNGPropertyListVector &path, double rounding);

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
  std::unique_ptr<XForm> m_txtxform;
  VSDMisc m_misc;
  std::vector<librevenge::RVNGPropertyList> m_currentFillGeometry;
  std::vector<librevenge::RVNGPropertyList> m_currentLineGeometry;
  std::map<unsigned, XForm> *m_groupXForms;
  librevenge::RVNGBinaryData m_currentForeignData;
  librevenge::RVNGBinaryData m_currentOLEData;
  librevenge::RVNGPropertyList m_currentForeignProps;
  unsigned m_currentShapeId;
  unsigned m_foreignType;
  unsigned m_foreignFormat;
  double m_foreignOffsetX;
  double m_foreignOffsetY;
  double m_foreignWidth;
  double m_foreignHeight;
  bool m_noLine;
  bool m_noFill;
  bool m_noShow;
  std::map<unsigned short, VSDFont> m_fonts;
  unsigned m_currentLevel;
  bool m_isShapeStarted;
  std::vector<std::map<unsigned, XForm> > &m_groupXFormsSequence;
  std::vector<std::map<unsigned, unsigned> > &m_groupMembershipsSequence;
  std::vector<std::map<unsigned, unsigned> >::iterator m_groupMemberships;
  unsigned m_currentPageNumber;
  VSDOutputElementList *m_shapeOutputDrawing, *m_shapeOutputText;
  std::map<unsigned, VSDOutputElementList> m_pageOutputDrawing;
  std::map<unsigned, VSDOutputElementList> m_pageOutputText;
  std::vector<std::list<unsigned> > &m_documentPageShapeOrders;
  std::vector<std::list<unsigned> >::iterator m_pageShapeOrder;
  bool m_isFirstGeometry;

  std::map<unsigned, NURBSData> m_NURBSData;
  std::map<unsigned, PolylineData> m_polylineData;
  libvisio::VSDName m_currentText;
  std::map<unsigned, librevenge::RVNGString> m_names, m_stencilNames;
  std::vector<librevenge::RVNGString> m_fields;
  VSDFieldList m_stencilFields;
  unsigned m_fieldIndex;
  std::vector<VSDCharStyle> m_charFormats;
  std::vector<VSDParaStyle> m_paraFormats;

  VSDLineStyle m_lineStyle;
  VSDFillStyle m_fillStyle;
  VSDTextBlockStyle m_textBlockStyle;

  VSDCharStyle m_defaultCharStyle;
  VSDParaStyle m_defaultParaStyle;

  unsigned m_currentStyleSheet;
  VSDStyles m_styles;

  VSDStencils m_stencils;
  const VSDShape *m_stencilShape;
  bool m_isStencilStarted;

  unsigned m_currentGeometryCount;

  unsigned m_backgroundPageID;
  unsigned m_currentPageID;
  VSDPage m_currentPage;
  VSDPages m_pages;

  VSDLayerList m_layerList;

  std::vector<std::pair<double, double> > m_splineControlPoints;
  std::vector<double> m_splineKnotVector;
  double m_splineX, m_splineY;
  double m_splineLastKnot;
  unsigned m_splineDegree;
  unsigned m_splineLevel;
  unsigned m_currentShapeLevel;
  bool m_isBackgroundPage;

  VSDLayerList m_currentLayerList;
  std::vector<unsigned> m_currentLayerMem;
  std::vector<VSDTabSet> m_tabSets;

  const VSDXTheme *m_documentTheme;
};

} // namespace libvisio

#endif /* VSDCONTENTCOLLECTOR_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
