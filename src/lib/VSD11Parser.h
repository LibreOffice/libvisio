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

  typedef void (VSD11Parser::*StreamMethod)(VSDInternalStream&, libwpg::WPGPaintInterface*);
  struct StreamHandler { unsigned int type; const char *name; StreamMethod handler;};
  static const struct StreamHandler streamHandlers[32];

  typedef void (VSD11Parser::*ChunkMethod)(VSDInternalStream&, libwpg::WPGPaintInterface*);
  struct ChunkHandler { unsigned int type; const char *name; ChunkMethod handler;};
  static const struct ChunkHandler chunkHandlers[3];

  // Stream handlers
  void handlePages(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);
  void handlePage(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);

  // Chunk handlers
  void shapeChunk(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);
  void foreignChunk(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);

  // Utilities
  struct ChunkHeader
  {
    unsigned int chunkType;  // 4 bytes
    unsigned int id;         // 4 bytes
    unsigned int list;       // 4 bytes
    unsigned int dataLength; // 4 bytes
    unsigned int level;      // 2 bytes
    unsigned int unknown;    // 1 byte
    unsigned int trailer; // Derived
  };

  void getChunkHeader(VSDInternalStream &stream, ChunkHeader &header);
};

} // namespace libvisio

#endif // __VSD11PARSER_H__
