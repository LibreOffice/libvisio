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

#ifndef __VSDXPARSER_H__
#define __VSDXPARSER_H__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <libwpd/libwpd.h>
#include <libwpd-stream/libwpd-stream.h>
#include <libwpg/libwpg.h>
#include "VSDXTypes.h"
#include "VSDXGeometryList.h"
#include "VSDXShapeList.h"

namespace libvisio
{

class VSDXCollector;

class VSDXParser
{
public:
  explicit VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter);
  virtual ~VSDXParser() {}
  virtual bool parse();

protected:
  // reader functions
  void readEllipticalArcTo(WPXInputStream *input);
  void readForeignData(WPXInputStream *input);
  void readEllipse(WPXInputStream *input);
  void readLine(WPXInputStream *input);
  void readFillAndShadow(WPXInputStream *input);
  void readGeomList(WPXInputStream *input);
  void readGeometry(WPXInputStream *input);
  void readMoveTo(WPXInputStream *input);
  void readLineTo(WPXInputStream *input);
  void readArcTo(WPXInputStream *input);
  void readXFormData(WPXInputStream *input);
  void readShapeId(WPXInputStream *input);
  void readShapeList(WPXInputStream *input);
  void readForeignDataType(WPXInputStream *input);
  void readPageProps(WPXInputStream *input);
  void readShape(WPXInputStream *input);
  void readColours(WPXInputStream *input);

  // parser of one pass
  bool parseDocument(WPXInputStream *input);

  // Stream handlers
  void handlePages(WPXInputStream *input);
  void handlePage(WPXInputStream *input);

  virtual bool getChunkHeader(WPXInputStream *input) = 0;
  void _handleLevelChange(unsigned level);

  WPXInputStream *m_input;
  libwpg::WPGPaintInterface *m_painter;
  ChunkHeader m_header;
  VSDXCollector *m_collector;
  VSDXGeometryList m_geomList;
  VSDXShapeList m_shapeList;
  unsigned m_currentLevel;

private:
  VSDXParser();
  VSDXParser(const VSDXParser&);
  VSDXParser &operator=(const VSDXParser&);
};

} // namespace libvisio

#endif // __VSDXPARSER_H__
