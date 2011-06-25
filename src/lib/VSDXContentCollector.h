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
#include <libwpg/libwpg.h>
#include "libvisio_utils.h"
#include "VSDXCollector.h"
#include "VSDXParser.h"

namespace libvisio {

class VSDXContentCollector : public VSDXCollector
{
public:
  VSDXContentCollector(libwpg::WPGPaintInterface *painter);
  virtual ~VSDXContentCollector() {};

  void collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc);
  void collectForeignData(unsigned id, unsigned level) {}
  void collectEllipse(unsigned id, unsigned level, double cx, double cy, double aa, double bb, double cc, double dd);
  void collectLine(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern);
  void collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern);
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
  VSDXContentCollector(const VSDXContentCollector&);
  VSDXContentCollector& operator=(const VSDXContentCollector&);
  libwpg::WPGPaintInterface *m_painter;

  void rotatePoint(double &x, double &y, const XForm &xform);
  void flipPoint(double &x, double &y, const XForm &xform);

  void _flushCurrentPath();
  void _flushCurrentForeignData();

  const ::WPXString getColourString(const Colour& c) const;

  bool m_isPageStarted;
  double m_pageWidth;
  double m_pageHeight;
  double m_scale;
  double m_x;
  double m_y;
  XForm m_xform;
  std::vector<unsigned> m_currentGeometryOrder;
  std::map<unsigned, WPXPropertyList> m_currentGeometry;
  std::map<unsigned, WPXPropertyListVector> m_currentComplexGeometry;
  std::map<unsigned, XForm> m_groupXForms;
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
  WPXPropertyListVector m_gradientProps;
  bool m_noLine;
  bool m_noFill;
  bool m_noShow;
  std::vector<Colour> m_colours;
};

} // namespace libvisio

#endif /* VSDXCONTENTCOLLECTOR_H */
