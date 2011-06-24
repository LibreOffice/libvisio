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
  explicit VSD11Parser(WPXInputStream *input);
  ~VSD11Parser();
  bool parse(libwpg::WPGPaintInterface *iface);
private:
  // reader functions
  void readEllipticalArcTo(WPXInputStream *input);

  typedef void (VSD11Parser::*StreamMethod)(VSDInternalStream&, libwpg::WPGPaintInterface*);
  struct StreamHandler { unsigned int type; const char *name; StreamMethod handler;};
  static const StreamHandler streamHandlers[];

  typedef void (VSD11Parser::*ChunkMethod)(VSDInternalStream&, libwpg::WPGPaintInterface*);
  struct ChunkHandler { unsigned int type; const char *name; ChunkMethod handler;};
  static const struct ChunkHandler chunkHandlers[4];

  // Stream handlers
  void handlePages(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);
  void handlePage(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);
  void handleColours(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);

  // Chunk handlers
  void shapeChunk(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);
  void foreignChunk(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);

  struct Colour
  {
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int a;
  };

  void getChunkHeader(VSDInternalStream &stream);
  void rotatePoint(double &x, double &y, const XForm &xform);
  void flipPoint(double &x, double &y, const XForm &xform);
  
  XForm _parseXForm(WPXInputStream *input);
  XForm _transformXForm(const XForm &xform);
  void _flushCurrentPath(libwpg::WPGPaintInterface *painter);
  void _flushCurrentForeignData(libwpg::WPGPaintInterface *painter);
  
  const ::WPXString getColourString(const Colour& c) const;

  std::vector<Colour> m_colours;
};

} // namespace libvisio

#endif // __VSD11PARSER_H__
