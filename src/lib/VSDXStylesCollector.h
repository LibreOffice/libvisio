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
  void collectForeignData(unsigned id, unsigned level) {}
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double aa, double bb, double cc, double dd) {}
  void collectLine(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern) {}
  void collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern) {}
  void collectGeomList(unsigned id, unsigned level) {}
  void collectGeometry(unsigned id, unsigned level) {}
  void collectMoveTo(unsigned id, unsigned level) {}
  void collectLineTo(unsigned id, unsigned level) {}
  void collectArcTo(unsigned id, unsigned level) {}
  void collectXFormData(unsigned id, unsigned level) {}
  void collectShapeID(unsigned id, unsigned level) {}
  void collectForeignDataType(unsigned id, unsigned level) {}
  void collectPageProps(unsigned id, unsigned level) {}

  void collectUnhandledChunk(unsigned id, unsigned level) {}


private:
  VSDXStylesCollector(const VSDXStylesCollector&);
  VSDXStylesCollector& operator=(const VSDXStylesCollector&);
};

}

#endif /* VSDXSTYLESCOLLECTOR_H */
