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

namespace libvisio {

class VSDXContentCollector : public VSDXCollector
{
public:
  VSDXContentCollector(
    libwpg::WPGPaintInterface *painter,
    std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
    std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
    std::vector<std::list<unsigned> > &documentPageShapeOrders
  );
  virtual ~VSDXContentCollector() {};

  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData);
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop);
  void collectLine(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern);
  void collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern);
  void collectGeomList(unsigned id, unsigned level);
  void collectGeometry(unsigned id, unsigned level, unsigned geomFlags);
  void collectMoveTo(unsigned id, unsigned level, double x, double y);
  void collectLineTo(unsigned id, unsigned level, double x, double y);
  void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow);
  void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned xType, unsigned yType, double degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights);
  void collectXFormData(unsigned id, unsigned level, const XForm &xform);
  void collectShapeId(unsigned id, unsigned level, unsigned shapeId);
  void collectShapeList(unsigned id, unsigned level);
  void collectForeignDataType(unsigned id, unsigned level, unsigned foreignType, unsigned foreignFormat);
  void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight);
  void collectShape(unsigned id, unsigned level);

  void collectUnhandledChunk(unsigned id, unsigned level);

  void collectColours(const std::vector<Colour> &colours);

  void startPage();
  void endPage();


private:
  VSDXContentCollector(const VSDXContentCollector&);
  VSDXContentCollector& operator=(const VSDXContentCollector&);
  libwpg::WPGPaintInterface *m_painter;

  void transformPoint(double &x, double &y);
  void transformAngle(double &angle);

  double _NURBSBasis(unsigned knot, double degree, double point, unsigned controlCount, const std::vector<double> &knotVector);

  void _flushCurrentPath();
  void _flushCurrentForeignData();
  void _flushCurrentPage();

  const ::WPXString getColourString(const Colour& c) const;

  void _handleLevelChange(unsigned level);

  bool m_isPageStarted;
  double m_pageWidth;
  double m_pageHeight;
  double m_scale;
  double m_x;
  double m_y;
  double m_originalX;
  double m_originalY;
  XForm m_xform;
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
  bool m_noLine;
  bool m_noFill;
  bool m_noShow;
  std::vector<Colour> m_colours;
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
};

} // namespace libvisio

#endif /* VSDXCONTENTCOLLECTOR_H */
