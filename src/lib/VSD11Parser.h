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

#ifndef __VSD11PARSER_H__
#define __VSD11PARSER_H__

#include <stdio.h>
#include <iostream>
#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>
#include "VSDXParser.h"
#include "VSDInternalStream.h"

namespace libvisio
{

class VSD11Parser : public VSDXParser
{
public:
  explicit VSD11Parser(WPXInputStream *input, libwpg::WPGPaintInterface *painter);
  ~VSD11Parser();
  bool parse();
private:
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
  void readShapeID(WPXInputStream *input);
  void readForeignDataType(WPXInputStream *input);
  void readPageProps(WPXInputStream *input);
  

  typedef void (VSD11Parser::*StreamMethod)(WPXInputStream*);
  struct StreamHandler { unsigned int type; const char *name; StreamMethod handler;};
  static const StreamHandler streamHandlers[];

  typedef void (VSD11Parser::*ChunkMethod)(WPXInputStream*);
  struct ChunkHandler { unsigned int type; const char *name; ChunkMethod handler;};
  static const struct ChunkHandler chunkHandlers[];

  // Stream handlers
  void handlePages(WPXInputStream *input);
  void handlePage(WPXInputStream *input);
  void handleColours(WPXInputStream *input);

  // Chunk handlers
  void shapeChunk(WPXInputStream *input);
  void foreignChunk(WPXInputStream *input);

  struct Colour
  {
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int a;
  };

  void getChunkHeader(WPXInputStream *input);
  void rotatePoint(double &x, double &y, const XForm &xform);
  void flipPoint(double &x, double &y, const XForm &xform);
  
  void _flushCurrentPath();
  void _flushCurrentForeignData();
  
  const ::WPXString getColourString(const Colour& c) const;

  std::vector<Colour> m_colours;
};

} // namespace libvisio

#endif // __VSD11PARSER_H__
