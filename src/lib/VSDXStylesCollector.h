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

#include "VSDXCollector.h"
#include "VSDXParser.h"

namespace libvisio {

class VSDXStylesCollector : public VSDXCollector
{
public:
  VSDXStylesCollector();
  virtual ~VSDXStylesCollector() {};

  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc) {}
  void collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData) {}
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double aa, double dd) {}
  void collectLine(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern) {}
  void collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern) {}
  void collectGeomList(unsigned id, unsigned level, const std::vector<unsigned> &geometryOrder) {}
  void collectGeometry(unsigned id, unsigned level, unsigned geomFlags) {}
  void collectMoveTo(unsigned id, unsigned level, double x, double y) {}
  void collectLineTo(unsigned id, unsigned level, double x, double y) {}
  void collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow) {}
  void collectXFormData(unsigned id, unsigned level, const XForm &xform) {}
  void collectShapeID(unsigned id, unsigned level, unsigned shapeId) {}
  void collectForeignDataType(unsigned id, unsigned level, unsigned foreignType, unsigned foreignFormat) {}
  void collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight) {}

  void collectUnhandledChunk(unsigned id, unsigned level) {}
  
  void collectColours(const std::vector<Colour> &colours) {}

  // Temporary hack
  void shapeChunkBegin(unsigned id, unsigned level) {}
  void shapeChunkEnd(unsigned id, unsigned level) {}
  void pageChunkBegin(unsigned id, unsigned level) {}
  void startPage() {}
  void endPage() {}


private:
  VSDXStylesCollector(const VSDXStylesCollector&);
  VSDXStylesCollector& operator=(const VSDXStylesCollector&);
};

}

#endif /* VSDXSTYLESCOLLECTOR_H */
