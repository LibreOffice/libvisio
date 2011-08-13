/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02111-1301 USA
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

namespace libvisio {

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
  virtual ~VSDXContentCollector() {};

  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData);
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop);
  void collectLine(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern, unsigned lineCap);
  void collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern, unsigned fillFGTransparency, unsigned fillBGTransparency, unsigned shadowPattern, Colour shfgc, double shadowOffsetX, double shadowOffsetY);
  void collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern, unsigned fillFGTransparency, unsigned fillBGTransparency, unsigned shadowPattern, Colour shfgc);
  void collectGeomList(unsigned id, unsigned level);
  void collectGeometry(unsigned id, unsigned level, unsigned geomFlags);
  void collectMoveTo(unsigned id, unsigned level, double x, double y);
  void collectLineTo(unsigned id, unsigned level, double x, double y);
  void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned xType, unsigned yType, unsigned degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID);
  void collectPolylineTo(unsigned id , unsigned level, double x, double y, unsigned xType, unsigned yType, std::vector<std::pair<double, double> > &points);
  void collectPolylineTo(unsigned id , unsigned level, double x, double y, unsigned dataID);
  void collectShapeData(unsigned id, unsigned level, unsigned xType, unsigned yType, unsigned degree, double lastKnot, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights);
  void collectShapeData(unsigned id, unsigned level, unsigned xType, unsigned yType, std::vector<std::pair<double, double> > points);
  void collectXFormData(unsigned id, unsigned level, const XForm &xform);
  void collectTxtXForm(unsigned id, unsigned level, const XForm &txtxform);
  void collectShapeId(unsigned id, unsigned level, unsigned shapeId);
  void collectShapeList(unsigned id, unsigned level);
  void collectForeignDataType(unsigned id, unsigned level, unsigned foreignType, unsigned foreignFormat);
  void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY);
  void collectShape(unsigned id, unsigned level, unsigned masterPage, unsigned masterShape, unsigned lineStyle, unsigned fillStyle, unsigned textStyle);

  void collectUnhandledChunk(unsigned id, unsigned level);

  void collectColours(const std::vector<Colour> &colours);
  void collectFont(unsigned short fontID, const std::vector<unsigned char> &textStream, TextFormat format);
  void collectCharList(unsigned id, unsigned level);
  void collectText(unsigned id, unsigned level, const std::vector<unsigned char> &textStream, TextFormat format);
  void collectCharFormat(unsigned id , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, unsigned langId, double fontSize, bool bold, bool italic, bool underline, WPXString fontFace);

  // Style collectors
  void collectStyleSheet(unsigned id, unsigned level, unsigned parentLineStyle, unsigned parentFillStyle, unsigned parentTextStyle);
  void collectLineStyle(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern, unsigned lineCap);
  void collectFillStyle(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern, unsigned fillFGTransparency, unsigned fillBGTransparency, unsigned shadowPattern, Colour shfgc, double shadowOffsetX, double shadowOffsetY);
  void collectFillStyle(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern, unsigned fillFGTransparency, unsigned fillBGTransparency, unsigned shadowPattern, Colour shfgc);

  void startPage();
  void endPage();


private:
  VSDXContentCollector(const VSDXContentCollector&);
  VSDXContentCollector& operator=(const VSDXContentCollector&);
  libwpg::WPGPaintInterface *m_painter;

  void transformPoint(double &x, double &y);
  void transformAngle(double &angle);

  double _NURBSBasis(unsigned knot, unsigned degree, double point, const std::vector<double> &knotVector);

  void _flushCurrentPath();
  void _flushCurrentForeignData();
  void _flushCurrentPage();

  void _handleLevelChange(unsigned level);
  void _appendUTF16LE(WPXString &text, WPXInputStream *input);

  void lineStyleFromStyleSheet(unsigned styleId);
  void fillStyleFromStyleSheet(unsigned styleId);
  void lineStyleFromStyleSheet(const VSDXLineStyle &style);
  void fillStyleFromStyleSheet(const VSDXFillStyle &style);

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
  XForm m_txtxform;
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
  std::vector<unsigned> m_shapeList;
  VSDXOutputElementList *m_shapeOutput;
  std::map<unsigned, VSDXOutputElementList> m_pageOutput;
  std::vector<std::list<unsigned> > &m_documentPageShapeOrders;
  std::list<unsigned> &m_pageShapeOrder;
  bool m_isFirstGeometry;

  std::map<unsigned, NURBSData> m_NURBSData;
  std::map<unsigned, PolylineData> m_polylineData;
  std::vector<unsigned char> m_textStream;
  TextFormat m_textFormat;
  bool m_outputTextStart;

  VSDXStyles m_styles;
  bool m_isTextUnstyled;
  bool m_hasLocalLineStyle, m_hasLocalFillStyle;

  VSDXStencils m_stencils;
  const VSDXStencilShape * m_stencilShape;
};

} // namespace libvisio

#endif /* VSDXCONTENTCOLLECTOR_H */
