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

#include "VSDXParser.h"
 
namespace libvisio {

class VSDXCollector
{
public:
  VSDXCollector();
  virtual ~VSDXCollector() {}

  virtual void collectEllipticalArcTo(double x3, double y3, double x2, double y2, double angle, double ecc, unsigned id) = 0;
  virtual void collectForeignData() = 0;
  virtual void collectEllipse(double cx, double cy, double aa, double bb, double cc, double dd) = 0;
  virtual void collectLine() = 0;
  virtual void collectFillAndShadow() = 0;
  virtual void collectGeomList() = 0;
  virtual void collectGeometry() = 0;
  virtual void collectMoveTo() = 0;
  virtual void collectLineTo() = 0;
  virtual void collectArcTo() = 0;
  virtual void collectXFormData() = 0;
  virtual void collectShapeID() = 0;
  virtual void collectForeignDataType() = 0;
  virtual void collectPageProps() = 0;
  


private:
    VSDXCollector(const VSDXCollector&);
    VSDXCollector& operator=(const VSDXCollector&);
};

} // namespace libvisio

#endif /* VSDXCOLLECTOR_H */
