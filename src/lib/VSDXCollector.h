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

#ifndef VSDXCOLLECTOR_H
#define VSDXCOLLECTOR_H

#include <vector>
#include "VSDXParser.h"

namespace libvisio {

class VSDXCollector
{
public:
  VSDXCollector();
  virtual ~VSDXCollector() {}

  virtual void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) = 0;
  virtual void collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData) = 0;
  virtual void collectEllipse(unsigned id, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop) = 0;
  virtual void collectLine(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern) = 0;
  virtual void collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern) = 0;
  virtual void collectGeomList(unsigned id, unsigned level) = 0;
  virtual void collectGeometry(unsigned id, unsigned level, unsigned geomFlags) = 0;
  virtual void collectMoveTo(unsigned id, unsigned level, double x, double y) = 0;
  virtual void collectLineTo(unsigned id, unsigned level, double x, double y) = 0;
  virtual void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow) = 0;
  virtual void collectNURBSTo(unsigned id, unsigned level, double x2, double y2, unsigned xType, unsigned yType, double degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights) = 0;
  virtual void collectXFormData(unsigned id, unsigned level, const XForm &xform) = 0;
  virtual void collectShapeId(unsigned id, unsigned level, unsigned shapeId) = 0;
  virtual void collectShapeList(unsigned id, unsigned level) = 0;
  virtual void collectForeignDataType(unsigned id, unsigned level, unsigned foreignType, unsigned foreignFormat) = 0;
  virtual void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight) = 0;
  virtual void collectShape(unsigned id, unsigned level) = 0;

  virtual void collectUnhandledChunk(unsigned id, unsigned level) = 0;

  virtual void collectColours(const std::vector<Colour> &colours) = 0;

  // Temporary hack
  virtual void startPage() = 0;
  virtual void endPage() = 0;



private:
    VSDXCollector(const VSDXCollector&);
    VSDXCollector& operator=(const VSDXCollector&);
};

} // namespace libvisio

#endif /* VSDXCOLLECTOR_H */
