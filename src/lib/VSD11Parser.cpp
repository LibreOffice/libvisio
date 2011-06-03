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

const struct libvisio::VSD11Parser::StreamHandler libvisio::VSD11Parser::handlers[] =
    {
      {0xa, "Name", 0},
      {0xb, "Name Idx", 0},
      {0x14, "Trailer, 0"},
      {0x15, "Page", &libvisio::VSD11Parser::handlePage},
      {0x16, "Colors"},
      {0x17, "??? seems to have no data", 0},
      {0x18, "FontFaces (ver.11)", 0},
      {0x1a, "Styles", 0},
      {0x1b, "??? saw 'Acrobat PDFWriter' string here", 0},
      {0x1c, "??? saw 'winspool.Acrobat PDFWriter.LPT1' string here", 0},
      {0x1d, "Stencils"},
      {0x1e, "Stencil Page (collection of Shapes, one collection per each stencil item)", 0},
      {0x20, "??? seems to have no data", 0},
      {0x21, "??? seems to have no data", 0},
      {0x23, "Icon (for stencil only?)", 0},
      {0x24, "??? seems to have no data", 0},
      {0x26, "??? some fractions, maybe PrintProps", 0},
      {0x27, "Pages", &libvisio::VSD11Parser::handlePages},
      {0x29, "Collection of Type 2a streams", 0},
      {0x2a, "???", 0},
      {0x31, "Document", 0},
      {0x32, "NameList", 0},
      {0x33, "Name", 0},
      {0x3d, "??? found in VSS, seems to have no data", 0},
      {0x3f, "List of name indexes collections", 0},
      {0x40, "Name Idx + ?? group (contains 0xb type streams)", 0},
      {0x44, "??? Collection of Type 0x45 streams", 0},
      {0x45, "??? match number of Pages or Stencil Pages in Pages/Stencils", 0},
      {0xc9, "some collection of 13 bytes structures related to 3f/40 streams", 0},
      {0xd7, "FontFace (ver.11)", 0},
      {0xd8, "FontFaces (ver.6)", 0},
      {0, 0, 0}
      };

libvisio::VSD11Parser::VSD11Parser(WPXInputStream *input)
  : VSDXParser(input), m_isPageStarted(false)
{}

libvisio::VSD11Parser::~VSD11Parser()
{}

/** Parses VSD 2003 input stream content, making callbacks to functions provided
by WPGPaintInterface class implementation as needed.
\param iface A WPGPaintInterface implementation
\return A value indicating whether parsing was successful
*/
bool libvisio::VSD11Parser::parse(libwpg::WPGPaintInterface *painter)
{
  const unsigned int SHIFT = 4;
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
  VSDInternalStream trailerStream(m_input, length, compressed);

  // Parse out pointers to other streams from trailer
  trailerStream.seek(SHIFT, WPX_SEEK_SET);
  offset = readU32(&trailerStream);
  trailerStream.seek(offset+SHIFT, WPX_SEEK_SET);
  unsigned int pointerCount = readU32(&trailerStream);
  trailerStream.seek(SHIFT, WPX_SEEK_CUR);

  unsigned int ptrType;
  unsigned int ptrOffset;
  unsigned int ptrLength;
  unsigned int ptrFormat;
  for (unsigned int i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(&trailerStream);
    trailerStream.seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(&trailerStream);
    ptrLength = readU32(&trailerStream);
    ptrFormat = readU16(&trailerStream);

    int index = -1;
    for (int i = 0; (index < 0) && handlers[i].type; i++)
    {
      if (handlers[i].type == ptrType)
        index = i;
    }

    if (index < 0)
    {
      VSD_DEBUG_MSG(("Unknown stream pointer type 0x%02x found in trailer at %li\n",
                     ptrType, trailerStream.tell() - 18));
    }
    else
    {
      Method streamHandler = handlers[index].handler;
      if (!streamHandler)
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li ignored\n",
                       handlers[index].name, handlers[index].type, ptrFormat,
                       trailerStream.tell() - 18));
      else
      {
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li handled\n",
                       handlers[index].name, handlers[index].type, ptrFormat,
                       trailerStream.tell() - 18));

        compressed = ((ptrFormat & 2) == 2);
        m_input->seek(ptrOffset, WPX_SEEK_SET);
        VSDInternalStream stream(m_input, ptrLength, compressed);
        (this->*streamHandler)(stream, painter);
      }
    }
  }

  // End page if one is started
  if (m_isPageStarted)
    painter->endGraphics();
  return true;
}

void libvisio::VSD11Parser::handlePages(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter)
{
  unsigned int offset = readU32(&stream);
  stream.seek(offset, WPX_SEEK_SET);
  unsigned int pointerCount = readU32(&stream);
  stream.seek(4, WPX_SEEK_CUR); // Ignore 0x0 dword

  unsigned int ptrType;
  unsigned int ptrOffset;
  unsigned int ptrLength;
  unsigned int ptrFormat;
  for (unsigned int i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(&stream);
    stream.seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(&stream);
    ptrLength = readU32(&stream);
    ptrFormat = readU16(&stream);

    int index = -1;
    for (int i = 0; (index < 0) && handlers[i].type; i++)
    {
      if (handlers[i].type == ptrType)
        index = i;
    }

    if (index < 0)
    {
      VSD_DEBUG_MSG(("Unknown stream pointer type 0x%02x found in pages at %li\n",
                     ptrType, stream.tell() - 18));
    }
    else
    {
      Method streamHandler = handlers[index].handler;
      if (!streamHandler)
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li ignored\n",
                       handlers[index].name, handlers[index].type, ptrFormat,
                       stream.tell() - 18));
      else
      {
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li handled\n",
                       handlers[index].name, handlers[index].type, ptrFormat,
                       stream.tell() - 18));

        bool compressed = ((ptrFormat & 2) == 2);
        m_input->seek(ptrOffset, WPX_SEEK_SET);
        VSDInternalStream stream(m_input, ptrLength, compressed);
        (this->*streamHandler)(stream, painter);
      }
    }
  }
}

void libvisio::VSD11Parser::handlePage(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter)
{
  while (!stream.atEOS())
  {
    unsigned int chunkType = readU32(&stream);
    //unsigned int id = readU32(&stream);
    stream.seek(4, WPX_SEEK_CUR); // Skip id field
    unsigned int trailer = 0;
    if (readU32(&stream) != 0)
      trailer += 8; // 8 byte trailer
    unsigned int dataLength = readU32(&stream);

    unsigned int level = readU16(&stream);
    unsigned int unknown = readU8(&stream);

    // Add 4 byte separator under certain circumstances
    if (trailer != 0 || (level == 2 && unknown == 0x55) ||
        (level == 2 && unknown == 0x54) || (level == 3 && unknown == 0x50) ||
        chunkType == 0x69 || chunkType == 0x6a || chunkType == 0x6b || 
        chunkType == 0x71 || chunkType == 0xb6 || chunkType == 0xb9 || 
        chunkType == 0xa9 || chunkType == 0x92)
    {
      trailer += 4;
    }
    if (chunkType == 0x1f || chunkType == 0xc9)
    {
      trailer = 0;
    }
    VSD_DEBUG_MSG(("Parsing chunk type %02x with trailer (%d) and length %x\n",
                   chunkType, trailer, dataLength));

    if (chunkType == 0x92) // Page properties
    {
      stream.seek(1, WPX_SEEK_CUR); // Skip separator
      double width = readDouble(&stream);
      stream.seek(1, WPX_SEEK_CUR); // Skip separator
      double height = readDouble(&stream);

      WPXPropertyList pageProps;
      pageProps.insert("svg:width", width);
      pageProps.insert("svg:height", height);
      if (m_isPageStarted)
        painter->endGraphics();
      painter->startGraphics(pageProps);
      m_isPageStarted = true;

      stream.seek(dataLength+trailer-18, WPX_SEEK_CUR);
      VSD_DEBUG_MSG(("Moved to %lx\n", stream.tell()));
    }
    else // Skip chunk
    {
      dataLength += trailer;
      VSD_DEBUG_MSG(("Skipping chunk by %x (%d) bytes\n",
                     dataLength, dataLength));
      stream.seek(dataLength, WPX_SEEK_CUR);
    }
  }
}
