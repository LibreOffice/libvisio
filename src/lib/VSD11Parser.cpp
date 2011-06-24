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

#include <libwpd-stream/libwpd-stream.h>
#include <locale.h>
#include <sstream>
#include <string>
#include "libvisio_utils.h"
#include "VSD11Parser.h"
#include "VSDInternalStream.h"
#include "VSDXDocumentStructure.h"

const libvisio::VSD11Parser::StreamHandler libvisio::VSD11Parser::streamHandlers[] = {
  {VSD_PAGE, "Page", &libvisio::VSD11Parser::handlePage},
  {VSD_COLORS, "Colors", &libvisio::VSD11Parser::handleColours},
  {VSD_PAGES, "Pages", &libvisio::VSD11Parser::handlePages},
  {0, 0, 0}
};

const struct libvisio::VSD11Parser::ChunkHandler libvisio::VSD11Parser::chunkHandlers[] =
{
  {VSD_SHAPE_GROUP, "ShapeType=\"Group\"", &libvisio::VSD11Parser::shapeChunk},
  {VSD_SHAPE_SHAPE, "ShapeType=\"Shape\"", &libvisio::VSD11Parser::shapeChunk},
  {VSD_SHAPE_FOREIGN, "ShapeType=\"Foreign\"", &libvisio::VSD11Parser::shapeChunk},
#if 0
  {VSD_XFORM_DATA, "XForm Data", &libvisio::VSD11Parser::readXFormData},
  {VSD_SHAPE_ID, "Shape Id", &libvisio::VSD11Parser::readShapeID},
  {VSD_LINE, "Line", &libvisio::VSD11Parser::readLine},
  {VSD_FILL_AND_SHADOW, "Fill and Shadow", &libvisio::VSD11Parser::readFillAndShadow},
  {VSD_GEOM_LIST, "Geom List", &libvisio::VSD11Parser::readGeomList},
  {VSD_GEOMETRY, "Geometry", &libvisio::VSD11Parser::readGeometry},
  {VSD_MOVE_TO, "Move To", &libvisio::VSD11Parser::readMoveTo},
  {VSD_LINE_TO, "Line To", &libvisio::VSD11Parser::readLineTo},
  {VSD_ARC_TO, "Arc To", &libvisio::VSD11Parser::readArcTo},
  {VSD_ELLIPSE, "Ellipse", &libvisio::VSD11Parser::readEllipse},
  {VSD_ELLIPTICAL_ARC_TO, "Elliptical Arc To", &libvisio::VSD11Parser::readEllipticalArcTo},
  {VSD_FOREIGN_DATA_TYPE, "Foreign Data Type", &libvisio::VSD11Parser::readForeignDataType},
  {VSD_FOREIGN_DATA, "Foreign Data", &libvisio::VSD11Parser::readForeignData},
#endif
  {0, 0, 0}
};

libvisio::VSD11Parser::VSD11Parser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : VSDXParser(input, painter)
{}

libvisio::VSD11Parser::~VSD11Parser()
{}

/** Parses VSD 2003 input stream content, making callbacks to functions provided
by WPGPaintInterface class implementation as needed.
\param iface A WPGPaintInterface implementation
\return A value indicating whether parsing was successful
*/
bool libvisio::VSD11Parser::parse()
{
  if (!m_input)
  {
    return false;
  }
  // Seek to trailer stream pointer
  m_input->seek(0x24, WPX_SEEK_SET);

  m_input->seek(8, WPX_SEEK_CUR);
  unsigned int offset = readU32(m_input);
  unsigned int length = readU32(m_input);
  unsigned short format = readU16(m_input);
  bool compressed = ((format & 2) == 2);

  m_input->seek(offset, WPX_SEEK_SET);
  WPXInputStream *trailerStream = new VSDInternalStream(m_input, length, compressed);

/*  
  VSDXStylesCollector stylesCollector;
  if (!parseDocument(trailerStream, &stylesCollector))
  {
    delete trailerStream;
    return false;
  }
*/
  VSDXContentCollector contentCollector;
  if (!parseDocument(trailerStream, &contentCollector))
  {
    delete trailerStream;
    return false;
  }
  
  delete trailerStream;
  return true;
}

bool libvisio::VSD11Parser::parseDocument(WPXInputStream *input, VSDXCollector *collector)
{
  const unsigned int SHIFT = 4;

  unsigned int ptrType;
  unsigned int ptrOffset;
  unsigned int ptrLength;
  unsigned int ptrFormat;

  // Parse out pointers to other streams from trailer
  input->seek(SHIFT, WPX_SEEK_SET);
  unsigned offset = readU32(input);
  input->seek(offset+SHIFT, WPX_SEEK_SET);
  unsigned int pointerCount = readU32(input);
  input->seek(SHIFT, WPX_SEEK_CUR);
  for (unsigned int i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(input);
    ptrLength = readU32(input);
    ptrFormat = readU16(input);

    int index = -1;
    for (int j = 0; (index < 0) && streamHandlers[j].type; j++)
    {
      if (streamHandlers[j].type == ptrType)
        index = j;
    }

    if (index < 0)
    {
      VSD_DEBUG_MSG(("Unknown stream pointer type 0x%02x found in trailer at %li\n",
                     ptrType, input->tell() - 18));
    }
    else
    {
      StreamMethod streamHandler = streamHandlers[index].handler;
      if (!streamHandler)
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li ignored\n",
                       streamHandlers[index].name, streamHandlers[index].type, ptrFormat,
                       input->tell() - 18));
      else
      {
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li handled\n",
                       streamHandlers[index].name, streamHandlers[index].type, ptrFormat,
                       input->tell() - 18));

        bool compressed = ((ptrFormat & 2) == 2);
        m_input->seek(ptrOffset, WPX_SEEK_SET);
        WPXInputStream *input = new VSDInternalStream(m_input, ptrLength, compressed);
        (this->*streamHandler)(input);
        delete input;
      }
    }
  }

  // End page if one is started
  if (m_isPageStarted)
    m_painter->endGraphics();
  return true;
}

bool libvisio::VSD11Parser::getChunkHeader(WPXInputStream *input)
{
  unsigned char tmpChar = 0;
  while (!input->atEOS() && !tmpChar)
    tmpChar = readU8(input);

  if (input->atEOS())
    return false;
  else
    input->seek(-1, WPX_SEEK_CUR);

  m_header.chunkType = readU32(input);
  m_header.id = readU32(input);
  m_header.list = readU32(input);

   // Certain chunk types seem to always have a trailer
  m_header.trailer = 0;
  if (m_header.list != 0 || m_header.chunkType == 0x71 || m_header.chunkType == 0x70 ||
      m_header.chunkType == 0x6b || m_header.chunkType == 0x6a || m_header.chunkType == 0x69 ||
      m_header.chunkType == 0x66 || m_header.chunkType == 0x65 || m_header.chunkType == 0x2c)
    m_header.trailer += 8; // 8 byte trailer

  m_header.dataLength = readU32(input);
  m_header.level = readU16(input);
  m_header.unknown = readU8(input);

  // Add word separator under certain circumstances for v11
  // Below are known conditions, may be more or a simpler pattern
  if (m_header.list != 0 || (m_header.level == 2 && m_header.unknown == 0x55) ||
      (m_header.level == 2 && m_header.unknown == 0x54 && m_header.chunkType == 0xaa)
      || (m_header.level == 3 && m_header.unknown != 0x50 && m_header.unknown != 0x54) ||
      m_header.chunkType == 0x69 || m_header.chunkType == 0x6a || m_header.chunkType == 0x6b ||
      m_header.chunkType == 0x71 || m_header.chunkType == 0xb6 || m_header.chunkType == 0xb9 ||
      m_header.chunkType == 0xa9 || m_header.chunkType == 0x92)
  {
    m_header.trailer += 4;
  }
  // 0x1f (OLE data) and 0xc9 (Name ID) never have trailer
  if (m_header.chunkType == 0x1f || m_header.chunkType == 0xc9)
  {
    m_header.trailer = 0;
  }
  return true;
}

void libvisio::VSD11Parser::handlePages(WPXInputStream *input)
{
  unsigned int offset = readU32(input);
  input->seek(offset, WPX_SEEK_SET);
  unsigned int pointerCount = readU32(input);
  input->seek(4, WPX_SEEK_CUR); // Ignore 0x0 dword

  unsigned int ptrType;
  unsigned int ptrOffset;
  unsigned int ptrLength;
  unsigned int ptrFormat;
  for (unsigned int i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(input);
    ptrLength = readU32(input);
    ptrFormat = readU16(input);

    int index = -1;
    for (int j = 0; (index < 0) && streamHandlers[j].type; j++)
    {
      if (streamHandlers[j].type == ptrType)
        index = j;
    }

    if (index < 0)
    {
      VSD_DEBUG_MSG(("Unknown stream pointer type 0x%02x found in pages at %li\n",
                     ptrType, input->tell() - 18));
    }
    else
    {
      StreamMethod streamHandler = streamHandlers[index].handler;
      if (!streamHandler)
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li ignored\n",
                       streamHandlers[index].name, streamHandlers[index].type, ptrFormat,
                       input->tell() - 18));
      else
      {
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li handled\n",
                       streamHandlers[index].name, streamHandlers[index].type, ptrFormat,
                       input->tell() - 18));

        bool compressed = ((ptrFormat & 2) == 2);
        m_input->seek(ptrOffset, WPX_SEEK_SET);
        WPXInputStream *tmpInput = new VSDInternalStream(m_input, ptrLength, compressed);
        (this->*streamHandler)(tmpInput);
        delete tmpInput;
      }
    }
  }
}

void libvisio::VSD11Parser::handlePage(WPXInputStream *input)
{
  long endPos = 0;

  m_groupXForms.clear();

  while (!input->atEOS())
  {
    if (!getChunkHeader(input))
      break;
    endPos = m_header.dataLength+m_header.trailer+input->tell();
    int index = -1;
    for (int i = 0; (index < 0) && chunkHandlers[i].type; i++)
    {
      if (chunkHandlers[i].type == m_header.chunkType)
        index = i;
    }

    if (index >= 0)
    {
      // Skip rest of this chunk
      input->seek(m_header.dataLength + m_header.trailer, WPX_SEEK_CUR);
      ChunkMethod chunkHandler = chunkHandlers[index].handler;
      if (chunkHandler)
      {
        m_currentShapeId = m_header.id;
        (this->*chunkHandler)(input);
      }
      continue;
    }

    VSD_DEBUG_MSG(("Parsing chunk type %02x with trailer (%d) and length %x\n",
                   m_header.chunkType, m_header.trailer, m_header.dataLength));

    if (m_header.chunkType == VSD_PAGE_PROPS)
      readPageProps(input);

    input->seek(endPos, WPX_SEEK_SET);
  }
}

void libvisio::VSD11Parser::handleColours(WPXInputStream *input)
{
  input->seek(6, WPX_SEEK_SET);
  unsigned int numColours = readU8(input);
  Colour tmpColour;

  input->seek(1, WPX_SEEK_CUR);

  for (unsigned int i = 0; i < numColours; i++)
  {
    tmpColour.r = readU8(input);
    tmpColour.g = readU8(input);
    tmpColour.b = readU8(input);
    tmpColour.a = readU8(input);

    m_colours.push_back(tmpColour);
  }
}

