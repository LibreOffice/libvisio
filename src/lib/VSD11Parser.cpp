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
#include <cmath>
#include <set>
#include "libvisio_utils.h"
#include "VSD11Parser.h"
#include "VSDInternalStream.h"

#define DUMP_BITMAP 0

#if DUMP_BITMAP
static unsigned bitmapId = 0;
#include <sstream>
#endif

const struct libvisio::VSD11Parser::StreamHandler libvisio::VSD11Parser::streamHandlers[] =
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

const struct libvisio::VSD11Parser::ChunkHandler libvisio::VSD11Parser::chunkHandlers[] =
{
  {0x47, "ShapeType=\"Group\"", &libvisio::VSD11Parser::groupChunk},
  {0x48, "ShapeType=\"Shape\"", &libvisio::VSD11Parser::shapeChunk},
  {0x4e, "ShapeType=\"Foreign\"", &libvisio::VSD11Parser::foreignChunk},
  {0, 0, 0}
};

libvisio::VSD11Parser::VSD11Parser(WPXInputStream *input)
  : VSDXParser(input)
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
    for (int i = 0; (index < 0) && streamHandlers[i].type; i++)
    {
      if (streamHandlers[i].type == ptrType)
        index = i;
    }

    if (index < 0)
    {
      VSD_DEBUG_MSG(("Unknown stream pointer type 0x%02x found in trailer at %li\n",
                     ptrType, trailerStream.tell() - 18));
    }
    else
    {
      StreamMethod streamHandler = streamHandlers[index].handler;
      if (!streamHandler)
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li ignored\n",
                       streamHandlers[index].name, streamHandlers[index].type, ptrFormat,
                       trailerStream.tell() - 18));
      else
      {
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li handled\n",
                       streamHandlers[index].name, streamHandlers[index].type, ptrFormat,
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
    for (int i = 0; (index < 0) && streamHandlers[i].type; i++)
    {
      if (streamHandlers[i].type == ptrType)
        index = i;
    }

    if (index < 0)
    {
      VSD_DEBUG_MSG(("Unknown stream pointer type 0x%02x found in pages at %li\n",
                     ptrType, stream.tell() - 18));
    }
    else
    {
      StreamMethod streamHandler = streamHandlers[index].handler;
      if (!streamHandler)
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li ignored\n",
                       streamHandlers[index].name, streamHandlers[index].type, ptrFormat,
                       stream.tell() - 18));
      else
      {
        VSD_DEBUG_MSG(("Stream '%s', type 0x%02x, format 0x%02x at %li handled\n",
                       streamHandlers[index].name, streamHandlers[index].type, ptrFormat,
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
  ChunkHeader header = {0};

  //double x = 0; double y = 0;

  while (!stream.atEOS())
  {
    getChunkHeader(stream, header);
    int index = -1;
    for (int i = 0; (index < 0) && chunkHandlers[i].type; i++)
    {
      if (chunkHandlers[i].type == header.chunkType)
        index = i;
    }

    if (index >= 0)
    {
      // Skip rest of this chunk
      stream.seek(header.dataLength + header.trailer, WPX_SEEK_CUR);
      ChunkMethod chunkHandler = chunkHandlers[index].handler;
      if (chunkHandler)
        (this->*chunkHandler)(stream, painter);
      continue;
    }

    VSD_DEBUG_MSG(("Parsing chunk type %02x with trailer (%d) and length %x\n",
                   header.chunkType, header.trailer, header.dataLength));

    if (header.chunkType == 0x92) // Page properties
    {
      // Skip bytes representing unit to *display* (value is always inches)
      stream.seek(1, WPX_SEEK_CUR);
      double width = readDouble(&stream);
      stream.seek(1, WPX_SEEK_CUR);
      double height = readDouble(&stream);
      m_pageWidth = width;
      m_pageHeight = height;

      WPXPropertyList pageProps;
      pageProps.insert("svg:width", width);
      pageProps.insert("svg:height", height);

      if (m_isPageStarted)
        painter->endGraphics();
      painter->startGraphics(pageProps);
      m_isPageStarted = true;

      stream.seek(header.dataLength+header.trailer-18, WPX_SEEK_CUR);      
    }
    else // Skip chunk
    {
      header.dataLength += header.trailer;
      VSD_DEBUG_MSG(("Skipping chunk by %x (%d) bytes\n",
                     header.dataLength, header.dataLength));
      stream.seek(header.dataLength, WPX_SEEK_CUR);
    }
  }
}

void libvisio::VSD11Parser::groupChunk(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter)
{
  WPXPropertyListVector vertices;
  WPXPropertyList styleProps;
  WPXPropertyListVector gradientProps;
  XForm xform = {0}; // Shape xform data
  std::set<unsigned int> shapeIDs;
  ChunkHeader header = {0};
  double x = 0; double y = 0;
  bool done = false;
  int geomCount = -1;

  // Reset style
  styleProps.clear();
  styleProps.insert("svg:stroke-width", 0.0138889);
  styleProps.insert("svg:stroke-color", "black");

  while (!stream.atEOS())
  {
    getChunkHeader(stream, header);
    VSD_DEBUG_MSG(("Shape: parsing chunk type %x\n", header.chunkType));

    // Break once a chunk that is not nested in the group is found
    if (header.level < 2)
    {
      stream.seek(-19, WPX_SEEK_CUR);
      break;
    }

    switch (header.chunkType)
    {
    case 0x9b: // XForm data      
      xform = _parseXForm(&stream);
      stream.seek(header.dataLength+header.trailer-65, WPX_SEEK_CUR);
      break;
    case 0x83: // ShapeID
      shapeIDs.insert(readU32(&stream));
      stream.seek(header.dataLength+header.trailer-4, WPX_SEEK_CUR);
      break;
    case 0x85: // Line properties
      stream.seek(1, WPX_SEEK_CUR);
      styleProps.insert("svg:stroke-width", readDouble(&stream));
      stream.seek(header.dataLength+header.trailer-9, WPX_SEEK_CUR);      
      break;
    case 0x6c: // GeomList
      if (vertices.count())
        painter->drawPolyline(vertices);
      vertices = WPXPropertyListVector();
      painter->setStyle(styleProps, gradientProps);
      stream.seek(header.dataLength+header.trailer, WPX_SEEK_CUR);
      geomCount = header.list;
      continue; // Avoid geomCount decrement below
    case 0x8a: // MoveTo
    {
      WPXPropertyList end1;
      stream.seek(1, WPX_SEEK_CUR);
      x = readDouble(&stream) + xform.x;
      stream.seek(1, WPX_SEEK_CUR);
      y = (xform.height - readDouble(&stream)) + xform.y;
      rotatePoint(x, y, xform);

      end1.insert("svg:x", x);
      end1.insert("svg:y", y); 
      vertices.append(end1);

      stream.seek(header.dataLength+header.trailer-18, WPX_SEEK_CUR);
    }
      break;
    case 0x8b: // LineTo
    {
      WPXPropertyList end2;
      stream.seek(1, WPX_SEEK_CUR);
      x = readDouble(&stream) + xform.x;
      stream.seek(1, WPX_SEEK_CUR);
      y = (xform.height - readDouble(&stream)) + xform.y;
      rotatePoint(x, y, xform);

      end2.insert("svg:x", x);
      end2.insert("svg:y", y);
      vertices.append(end2);

      stream.seek(header.dataLength+header.trailer-18, WPX_SEEK_CUR);
    }
      break;
    default:
      stream.seek(header.dataLength+header.trailer, WPX_SEEK_CUR);
    }
    if (geomCount > 0) geomCount--;
    if (geomCount == 0) done = true;
  }
}

void libvisio::VSD11Parser::shapeChunk(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter)
{
  WPXPropertyListVector vertices;
  WPXPropertyList styleProps;
  WPXPropertyListVector gradientProps;
  XForm xform = {0}; // Shape xform data
  ChunkHeader header = {0};
  bool done = false;
  int geomCount = -1;

  double x = 0; double y = 0;

  // Reset style
  styleProps.clear();
  styleProps.insert("svg:stroke-width", 0.0138889);
  styleProps.insert("svg:stroke-color", "black");

  while (!done && !stream.atEOS())
  {
    getChunkHeader(stream, header);

    VSD_DEBUG_MSG(("Shape: parsing chunk type %x\n", header.chunkType));
    switch (header.chunkType)
    {
    case 0x9b: // XForm data      
      xform = _parseXForm(&stream);
      stream.seek(header.dataLength+header.trailer-65, WPX_SEEK_CUR);
      break;
    case 0x85: // Line properties
      stream.seek(1, WPX_SEEK_CUR);
      styleProps.insert("svg:stroke-width", readDouble(&stream));
      stream.seek(header.dataLength+header.trailer-9, WPX_SEEK_CUR);      
      break;
    case 0x6c: // GeomList
      if (vertices.count())
        painter->drawPolyline(vertices);
      vertices = WPXPropertyListVector();
      painter->setStyle(styleProps, gradientProps);
      stream.seek(header.dataLength+header.trailer, WPX_SEEK_CUR);
      geomCount = header.list;
      continue; // Avoid geomCount decrement below
    case 0x8a: // MoveTo
    {
      WPXPropertyList end1;
      stream.seek(1, WPX_SEEK_CUR);
      x = readDouble(&stream) + xform.x;
      stream.seek(1, WPX_SEEK_CUR);
      y = (xform.height - readDouble(&stream)) + xform.y;
      rotatePoint(x, y, xform);

      end1.insert("svg:x", x);
      end1.insert("svg:y", y); 
      vertices.append(end1);

      stream.seek(header.dataLength+header.trailer-18, WPX_SEEK_CUR);
    }
      break;
    case 0x8b: // LineTo
    {
      WPXPropertyList end2;
      stream.seek(1, WPX_SEEK_CUR);
      x = readDouble(&stream) + xform.x;
      stream.seek(1, WPX_SEEK_CUR);
      y = (xform.height - readDouble(&stream)) + xform.y;
      rotatePoint(x, y, xform);

      end2.insert("svg:x", x);
      end2.insert("svg:y", y);
      vertices.append(end2);

      stream.seek(header.dataLength+header.trailer-18, WPX_SEEK_CUR);
    }
      break;
    default:
      stream.seek(header.dataLength+header.trailer, WPX_SEEK_CUR);
    }
    if (geomCount > 0) geomCount--;
    if (geomCount == 0) done = true;
  }
  if (vertices.count())
     painter->drawPolyline(vertices);
}

void libvisio::VSD11Parser::foreignChunk(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter)
{
  XForm xform = {0}; // Shape xform data
  unsigned int foreignType = 0; // Tracks current foreign data type
  unsigned int foreignFormat = 0; // Tracks foreign data format
  ChunkHeader header = {0};
  unsigned long tmpBytesRead = 0;
  bool done = false;

  while (!done && !stream.atEOS())
  {
    getChunkHeader(stream, header);

    switch(header.chunkType)
    {
    case 0x9b: // XForm data      
      xform = _parseXForm(&stream);
      stream.seek(header.dataLength+header.trailer-65, WPX_SEEK_CUR);
      break;
    case 0x98:
      stream.seek(0x24, WPX_SEEK_CUR);
      foreignType = readU16(&stream);
      stream.seek(0xb, WPX_SEEK_CUR);
      foreignFormat = readU32(&stream);

      stream.seek(header.dataLength+header.trailer-0x35, WPX_SEEK_CUR);
      VSD_DEBUG_MSG(("Found foreign data, type %d format %d\n", foreignType, foreignFormat));
      
      break;
    case 0x0c:
      if (foreignType == 1 || foreignType == 4) // Image
      {
        const unsigned char *buffer = stream.read(header.dataLength, tmpBytesRead);
        WPXBinaryData binaryData;
        // If bmp data found, reconstruct header
        if (foreignType == 1 && foreignFormat == 0)
        {
          binaryData.append(0x42);
          binaryData.append(0x4d);

          binaryData.append((unsigned char)((tmpBytesRead + 14) & 0x000000ff));
          binaryData.append((unsigned char)(((tmpBytesRead + 14) & 0x0000ff00) >> 8));
          binaryData.append((unsigned char)(((tmpBytesRead + 14) & 0x00ff0000) >> 16));
          binaryData.append((unsigned char)(((tmpBytesRead + 14) & 0xff000000) >> 24));

          binaryData.append(0x00);
          binaryData.append(0x00);
          binaryData.append(0x00);
          binaryData.append(0x00);

          binaryData.append(0x36);
          binaryData.append(0x00);
          binaryData.append(0x00);
          binaryData.append(0x00);
        }
        binaryData.append(buffer, tmpBytesRead);
        
#if DUMP_BITMAP
        if (foreignType == 1 || foreignType == 4)
        {
          std::ostringstream filename;
          switch(foreignFormat)
          {
          case 0:
            filename << "binarydump" << bitmapId++ << ".bmp"; break;
          case 1:
            filename << "binarydump" << bitmapId++ << ".jpeg"; break;
          case 2:
            filename << "binarydump" << bitmapId++ << ".gif"; break;
          case 3:
            filename << "binarydump" << bitmapId++ << ".tiff"; break;
          case 4:
            filename << "binarydump" << bitmapId++ << ".png"; break;
          default:
            filename << "binarydump" << bitmapId++ << ".bin"; break;
          }
          FILE *f = fopen(filename.str().c_str(), "wb");
          if (f)
          {
            const unsigned char *tmpBuffer = binaryData.getDataBuffer();
            for (unsigned long k = 0; k < binaryData.size(); k++)
              fprintf(f, "%c",tmpBuffer[k]);
            fclose(f);
          }
        }
#endif

        WPXPropertyList foreignProps;
        foreignProps.insert("svg:width", xform.width);
        foreignProps.insert("svg:height", xform.height);
        foreignProps.insert("svg:x", xform.pinX - xform.pinLocX);
        // Y axis starts at the bottom not top
        foreignProps.insert("svg:y", m_pageHeight - 
                            xform.pinY + xform.pinLocY - xform.height);

        if (foreignType == 1)
        {
          switch(foreignFormat)
          {
          case 0:
            foreignProps.insert("libwpg:mime-type", "image/bmp"); break;
          case 1:
            foreignProps.insert("libwpg:mime-type", "image/jpeg"); break;
          case 2:
            foreignProps.insert("libwpg:mime-type", "image/gif"); break;
          case 3:
            foreignProps.insert("libwpg:mime-type", "image/tiff"); break;
          case 4:
            foreignProps.insert("libwpg:mime-type", "image/png"); break;
          }
        }
        else if (foreignType == 4)
        {
          const unsigned char *tmpBinData = binaryData.getDataBuffer();
		  // Check for EMF signature
          if (tmpBinData[0x28] == 0x20 && tmpBinData[0x29] == 0x45 && tmpBinData[0x2A] == 0x4D && tmpBinData[0x2B] == 0x46)
          {
            foreignProps.insert("libwpg:mime-type", "image/emf");
          }
          else
          {
            foreignProps.insert("libwpg:mime-type", "image/wmf");
          }
        }

        painter->drawGraphicObject(foreignProps, binaryData);
      }
      else
      {
        stream.seek(header.dataLength+header.trailer, WPX_SEEK_CUR);
      }
      break;

    default:
      stream.seek(header.dataLength+header.trailer, WPX_SEEK_CUR);

    }
  }
}

void libvisio::VSD11Parser::getChunkHeader(VSDInternalStream &stream, libvisio::VSD11Parser::ChunkHeader &header)
{
  do
  {
    header.chunkType = readU32(&stream);
  } while (header.chunkType == 0 && !stream.atEOS());
  
  if (stream.atEOS())
    return;

  header.id = readU32(&stream);
  header.list = readU32(&stream);

   // Certain chunk types seem to always have a trailer
  header.trailer = 0;
  if (header.list != 0 || header.chunkType == 0x71 || header.chunkType == 0x70 ||
      header.chunkType == 0x6b || header.chunkType == 0x6a || header.chunkType == 0x69 || 
      header.chunkType == 0x66 || header.chunkType == 0x65 || header.chunkType == 0x2c)
    header.trailer += 8; // 8 byte trailer

  header.dataLength = readU32(&stream);
  header.level = readU16(&stream);
  header.unknown = readU8(&stream);

  // Add word separator under certain circumstances for v11
  // Below are known conditions, may be more or a simpler pattern
  if (header.list != 0 || (header.level == 2 && header.unknown == 0x55) ||
      (header.level == 2 && header.unknown == 0x54 && header.chunkType == 0xaa) 
      || (header.level == 3 && header.unknown != 0x50 && header.unknown != 0x54) ||
      header.chunkType == 0x69 || header.chunkType == 0x6a || header.chunkType == 0x6b || 
      header.chunkType == 0x71 || header.chunkType == 0xb6 || header.chunkType == 0xb9 || 
      header.chunkType == 0xa9 || header.chunkType == 0x92)
  {
    header.trailer += 4;
  }
  // 0x1f (OLE data) and 0xc9 (Name ID) never have trailer 
  if (header.chunkType == 0x1f || header.chunkType == 0xc9)
  {
    header.trailer = 0;
  }
}

void libvisio::VSD11Parser::rotatePoint(double &x, double &y, const XForm &xform)
{
  if (xform.angle == 0.0) return;

  // Calculate co-ordinates using pin position as origin
  double tmpX = x - xform.pinX; 
  double tmpY = (m_pageHeight - y) - xform.pinY; // Start from bottom left

  // Rotate around pin and move back to bottom left as origin
  x = (tmpX * cos(xform.angle)) - (tmpY * sin(xform.angle)) + xform.pinX;
  y = (tmpX * sin(xform.angle)) + (tmpY * cos(xform.angle)) + xform.pinY;
  y = m_pageHeight - y; // Flip Y for screen co-ordinate
}

libvisio::VSDXParser::XForm libvisio::VSD11Parser::_parseXForm(WPXInputStream *input)
{
  libvisio::VSDXParser::XForm xform;
  input->seek(1, WPX_SEEK_CUR);
  xform.pinX = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  xform.pinY = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  xform.width = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  xform.height = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  xform.pinLocX = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  xform.pinLocY = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  xform.angle = readDouble(input);
  xform.flipX = (readU8(input) != 0);
  xform.flipY = (readU8(input) != 0);

  xform.x = xform.pinX - xform.pinLocX;
  xform.y = m_pageHeight - xform.pinY + xform.pinLocY - xform.height;
    
  return xform;
}
