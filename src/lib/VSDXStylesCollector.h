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

#ifndef VSDXSTYLESCOLLECTOR_H
#define VSDXSTYLESCOLLECTOR_H

#include <map>
#include <vector>
#include <list>
#include "VSDXCollector.h"
#include "VSDXParser.h"

namespace libvisio {

class VSDXStylesCollector : public VSDXCollector
{
public:
  VSDXStylesCollector(
    std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
    std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
    std::vector<std::list<unsigned> > &documentPageShapeOrders
  );
  virtual ~VSDXStylesCollector() {};

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
  void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight);
  void collectShape(unsigned id, unsigned level);
  void collectUnhandledChunk(unsigned id, unsigned level);

  void collectColours(const std::vector<Colour> &colours);

  void collectCharList(unsigned id, unsigned level);
  void collectText(unsigned id, unsigned level, const WPXString &text);

  // Temporary hack
  void startPage();
  void endPage();


private:
  VSDXStylesCollector(const VSDXStylesCollector&);
  VSDXStylesCollector& operator=(const VSDXStylesCollector&);

  void _handleLevelChange(unsigned level);
  void _flushShapeList();

  unsigned m_currentLevel;
  bool m_isShapeStarted;

  unsigned m_currentShapeId;
  std::map<unsigned, XForm> m_groupXForms;
  std::map<unsigned, unsigned> m_groupMemberships;
  std::vector<std::map<unsigned, XForm> > &m_groupXFormsSequence;
  std::vector<std::map<unsigned, unsigned> > &m_groupMembershipsSequence;
  std::list<unsigned> m_pageShapeOrder;
  std::vector<std::list<unsigned> > &m_documentPageShapeOrders;
  std::map<unsigned, std::list<unsigned> > m_groupShapeOrder;
  std::list<unsigned> m_shapeList;
};

}

#endif /* VSDXSTYLESCOLLECTOR_H */
