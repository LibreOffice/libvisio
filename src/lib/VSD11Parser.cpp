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
#include "VSDXDocumentStructure.h"

#define DUMP_BITMAP 0

#if DUMP_BITMAP
static unsigned bitmapId = 0;
#include <sstream>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const ::WPXString libvisio::VSD11Parser::getColourString(const struct Colour &c) const
{
    ::WPXString sColour;
    sColour.sprintf("#%.2x%.2x%.2x", c.r, c.g, c.b);
    return sColour;
}

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
    for (int j = 0; (index < 0) && streamHandlers[j].type; j++)
    {
      if (streamHandlers[j].type == ptrType)
        index = j;
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
        (this->*streamHandler)(stream);
      }
    }
  }

  // End page if one is started
  if (m_isPageStarted)
    m_painter->endGraphics();
  return true;
}

void libvisio::VSD11Parser::handlePages(VSDInternalStream &stream)
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
    for (int j = 0; (index < 0) && streamHandlers[j].type; j++)
    {
      if (streamHandlers[j].type == ptrType)
        index = j;
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
        VSDInternalStream tmpStream(m_input, ptrLength, compressed);
        (this->*streamHandler)(tmpStream);
      }
    }
  }
}

void libvisio::VSD11Parser::handlePage(VSDInternalStream &stream)
{
  m_groupXForms.clear();

  while (!stream.atEOS())
  {
    getChunkHeader(stream);
    int index = -1;
    for (int i = 0; (index < 0) && chunkHandlers[i].type; i++)
    {
      if (chunkHandlers[i].type == m_header.chunkType)
        index = i;
    }

    if (index >= 0)
    {
      // Skip rest of this chunk
      stream.seek(m_header.dataLength + m_header.trailer, WPX_SEEK_CUR);
      ChunkMethod chunkHandler = chunkHandlers[index].handler;
      if (chunkHandler)
      {
        m_currentShapeId = m_header.id;
        (this->*chunkHandler)(stream);
      }
      continue;
    }

    VSD_DEBUG_MSG(("Parsing chunk type %02x with trailer (%d) and length %x\n",
                   m_header.chunkType, m_header.trailer, m_header.dataLength));

    if (m_header.chunkType == VSD_PAGE_PROPS) // Page properties
    {
      // Skip bytes representing unit to *display* (value is always inches)
      stream.seek(1, WPX_SEEK_CUR);
      m_pageWidth = readDouble(&stream);
      stream.seek(1, WPX_SEEK_CUR);
      m_pageHeight = readDouble(&stream);
      stream.seek(19, WPX_SEEK_CUR);
      /* m_scale = */ readDouble(&stream);

      WPXPropertyList pageProps;
      pageProps.insert("svg:width", m_scale*m_pageWidth);
      pageProps.insert("svg:height", m_scale*m_pageHeight);

      if (m_isPageStarted)
        m_painter->endGraphics();
      m_painter->startGraphics(pageProps);
      m_isPageStarted = true;

      stream.seek(m_header.dataLength+m_header.trailer-45, WPX_SEEK_CUR);
    }
    else // Skip chunk
    {
      m_header.dataLength += m_header.trailer;
      VSD_DEBUG_MSG(("Skipping chunk by %x (%d) bytes\n",
                     m_header.dataLength, m_header.dataLength));
      stream.seek(m_header.dataLength, WPX_SEEK_CUR);
    }
  }
}

void libvisio::VSD11Parser::handleColours(VSDInternalStream &stream)
{
  stream.seek(6, WPX_SEEK_SET);
  unsigned int numColours = readU8(&stream);
  Colour tmpColour;

  stream.seek(1, WPX_SEEK_CUR);

  for (unsigned int i = 0; i < numColours; i++)
  {
    tmpColour.r = readU8(&stream);
    tmpColour.g = readU8(&stream);
    tmpColour.b = readU8(&stream);
    tmpColour.a = readU8(&stream);

    m_colours.push_back(tmpColour);
  }
}

void libvisio::VSD11Parser::shapeChunk(VSDInternalStream &stream)
{
  WPXPropertyListVector path;
  m_gradientProps = WPXPropertyListVector();
  m_foreignType = 0; // Tracks current foreign data type
  m_foreignFormat = 0; // Tracks foreign data format
  unsigned long streamPos = 0;

  m_x = 0; m_y = 0;
  m_xform = XForm();
  m_header = ChunkHeader();

  // Geometry flags
  m_noLine = false;
  m_noFill = false;
  m_noShow = false;

  // Save line colour and pattern, fill type and pattern
  m_lineColour = "black";
  m_fillType = "none";
  m_linePattern = 1; // same as "solid"
  m_fillPattern = 1; // same as "solid"

  // Reset style
  m_styleProps.clear();
  m_styleProps.insert("svg:stroke-width", m_scale*0.0138889);
  m_styleProps.insert("svg:stroke-color", m_lineColour);
  m_styleProps.insert("draw:fill", m_fillType);
  m_styleProps.insert("svg:stroke-dasharray", "solid");

  while (!stream.atEOS())
  {
    getChunkHeader(stream);
    streamPos = stream.tell();

    // Break once a chunk that is not nested in the shape is found
    if (m_header.level < 2)
    {
      stream.seek(-19, WPX_SEEK_CUR);
      break;
    }

    // Until the next geometry chunk, don't parse if geometry is not to be shown
    if (m_header.chunkType != VSD_GEOM_LIST && m_noShow)
    {
      stream.seek(m_header.dataLength+m_header.trailer, WPX_SEEK_CUR);
      continue;
    }

    VSD_DEBUG_MSG(("Shape: parsing chunk type %x\n", m_header.chunkType));
    switch (m_header.chunkType)
    {
    case VSD_XFORM_DATA: // XForm data
      m_xform = _transformXForm(_parseXForm(&stream));
      break;
    case VSD_SHAPE_ID:
      m_groupXForms[readU32(&stream)] = m_xform;
      break;
    case VSD_LINE:
      readLine(&stream);
      break;
    case VSD_FILL_AND_SHADOW:
      readFillAndShadow(&stream);
      break;
    case VSD_GEOM_LIST:
      readGeomList(&stream);
      break;
    case VSD_GEOMETRY:
      readGeometry(&stream);
      break;
    case VSD_MOVE_TO:
      readMoveTo(&stream);
      break;
    case VSD_LINE_TO:
      readLineTo(&stream);
      break;
    case VSD_ARC_TO:
      readArcTo(&stream);
      break;
    case VSD_ELLIPSE:
      readEllipse(&stream);
      break;
    case VSD_ELLIPTICAL_ARC_TO:
      readEllipticalArcTo(&stream);
      break;
    case VSD_FOREIGN_DATA_TYPE:
      stream.seek(0x24, WPX_SEEK_CUR);
      m_foreignType = readU16(&stream);
      stream.seek(0xb, WPX_SEEK_CUR);
      m_foreignFormat = readU32(&stream);

      stream.seek(m_header.dataLength+m_header.trailer-0x35, WPX_SEEK_CUR);
      VSD_DEBUG_MSG(("Found foreign data, type %d format %d\n", m_foreignType, m_foreignFormat));

      break;
    case VSD_FOREIGN_DATA:
      readForeignData(&stream);
      break;
    }

    stream.seek(m_header.dataLength+m_header.trailer-(stream.tell()-streamPos), WPX_SEEK_CUR);
  }
  _flushCurrentPath();
  _flushCurrentForeignData();
  m_x = 0.0; m_y = 0.0;
}

void libvisio::VSD11Parser::getChunkHeader(VSDInternalStream &stream)
{
  unsigned char tmpChar = 0;
  while (!stream.atEOS() && !tmpChar)
    tmpChar = readU8(&stream);

  if (stream.atEOS())
    return;
  else
    stream.seek(-1, WPX_SEEK_CUR);

  m_header.chunkType = readU32(&stream);
  m_header.id = readU32(&stream);
  m_header.list = readU32(&stream);

   // Certain chunk types seem to always have a trailer
  m_header.trailer = 0;
  if (m_header.list != 0 || m_header.chunkType == 0x71 || m_header.chunkType == 0x70 ||
      m_header.chunkType == 0x6b || m_header.chunkType == 0x6a || m_header.chunkType == 0x69 ||
      m_header.chunkType == 0x66 || m_header.chunkType == 0x65 || m_header.chunkType == 0x2c)
    m_header.trailer += 8; // 8 byte trailer

  m_header.dataLength = readU32(&stream);
  m_header.level = readU16(&stream);
  m_header.unknown = readU8(&stream);

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

void libvisio::VSD11Parser::flipPoint(double &x, double &y, const XForm &xform)
{
  if (!xform.flipX && !xform.flipY) return;

  double tmpX = x - xform.x;
  double tmpY = y - xform.y;

  if (xform.flipX)
    tmpX = xform.width - tmpX;
  if (xform.flipY)
    tmpY = xform.height - tmpY;
  x = tmpX + xform.x;
  y = tmpY + xform.y;
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

libvisio::VSDXParser::XForm libvisio::VSD11Parser::_transformXForm(const libvisio::VSDXParser::XForm &xform)
{
  libvisio::VSDXParser::XForm tmpXForm = xform;
  std::map<unsigned int, XForm>::iterator iter = m_groupXForms.find(m_currentShapeId);
  if  (iter != m_groupXForms.end()) {
    tmpXForm.pinX += iter->second.pinX;
    tmpXForm.pinY += iter->second.pinY;
    tmpXForm.pinLocX += iter->second.pinLocX;
    tmpXForm.pinLocY += iter->second.pinLocY;
  }
  tmpXForm.x = tmpXForm.pinX - tmpXForm.pinLocX;
  tmpXForm.y = m_pageHeight - tmpXForm.pinY + tmpXForm.pinLocY - tmpXForm.height;

  return tmpXForm;
}

void libvisio::VSD11Parser::_flushCurrentPath()
{
  double startX = 0; double startY = 0;
  double x = 0; double y = 0;
  bool firstPoint = true;

  WPXPropertyListVector path;
  std::map<unsigned int, WPXPropertyList>::iterator iter;
  std::map<unsigned int, WPXPropertyListVector>::iterator itervec;
  if (m_currentGeometryOrder.size())
  {
    for (unsigned i = 0; i < m_currentGeometryOrder.size(); i++)
    {
      iter = m_currentGeometry.find(m_currentGeometryOrder[i]);
      if (iter != m_currentGeometry.end())
      {
        if (firstPoint)
        {
          x = (iter->second)["svg:x"]->getDouble();
          y = (iter->second)["svg:y"]->getDouble();
          startX = x;
          startY = y;
          firstPoint = false;
        }
        else if ((iter->second)["libwpg:path-action"]->getStr() == "M")
        {
          if (startX == x && startY == y)
          {
             WPXPropertyList closedPath;
             closedPath.insert("libwpg:path-action", "Z");
             path.append(closedPath);
          }
          if (path.count())
            m_painter->drawPath(path);

          path = WPXPropertyListVector();
          x = (iter->second)["svg:x"]->getDouble();
          y = (iter->second)["svg:y"]->getDouble();
          startX = x;
          startY = y;
        }
        else
        {
          x = (iter->second)["svg:x"]->getDouble();
          y = (iter->second)["svg:y"]->getDouble();
        }
        path.append(iter->second);
      }
      else
      {
        itervec = m_currentComplexGeometry.find(m_currentGeometryOrder[i]);
        if (itervec != m_currentComplexGeometry.end())
        {
          WPXPropertyListVector::Iter iter2(itervec->second);
          for (; iter2.next();)
          {
            if (firstPoint)
            {
              x = (iter2())["svg:x"]->getDouble();
              y = (iter2())["svg:y"]->getDouble();
              startX = x;
              startY = y;
              firstPoint = false;
            }
            else if ((iter2())["libwpg:path-action"]->getStr() == "M")
            {
              if (startX == x && startY == y)
              {
                WPXPropertyList closedPath;
                closedPath.insert("libwpg:path-action", "Z");
                path.append(closedPath);
              }
              if (path.count())
                m_painter->drawPath(path);

              path = WPXPropertyListVector();
              x = (iter2())["svg:x"]->getDouble();
              y = (iter2())["svg:y"]->getDouble();
              startX = x;
              startY = y;
            }
            else
            {
              x = (iter2())["svg:x"]->getDouble();
              y = (iter2())["svg:y"]->getDouble();
            }
          path.append(iter2());
          }
        }
      }
    }

    if (startX == x && startY == y && path.count())
    {
      WPXPropertyList closedPath;
      closedPath.insert("libwpg:path-action", "Z");
      path.append(closedPath);
    }
    if (path.count())
      m_painter->drawPath(path);
  }
  else
  {
    for (iter=m_currentGeometry.begin(); iter != m_currentGeometry.end(); iter++)
      path.append(iter->second);
    for (itervec=m_currentComplexGeometry.begin(); itervec != m_currentComplexGeometry.end(); itervec++)
    {
      WPXPropertyListVector::Iter iter2(itervec->second);
      for (; iter2.next();)
        path.append(iter2());
    }
    if (path.count())
      m_painter->drawPath(path);
  }
  m_currentGeometry.clear();
  m_currentComplexGeometry.clear();
  m_currentGeometryOrder.clear();
}

void libvisio::VSD11Parser::_flushCurrentForeignData()
{
  if (m_currentForeignData.size() && m_currentForeignProps["libwpg:mime-type"])
    m_painter->drawGraphicObject(m_currentForeignProps, m_currentForeignData);
  m_currentForeignData.clear();
  m_currentForeignProps.clear();
}

// --- READERS ---

void libvisio::VSD11Parser::readEllipticalArcTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x3 = readDouble(input) + m_xform.x; // End x
  input->seek(1, WPX_SEEK_CUR);
  double y3 = (m_xform.height - readDouble(input)) + m_xform.y; // End y
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input) + m_xform.x; // Mid x
  input->seek(1, WPX_SEEK_CUR);
  double y2 = (m_xform.height - readDouble(input)) + m_xform.y; // Mid y
  input->seek(1, WPX_SEEK_CUR);
  double angle = readDouble(input); // Angle
  input->seek(1, WPX_SEEK_CUR);
  double ecc = readDouble(input); // Eccentricity

  rotatePoint(x2, y2, m_xform);
  rotatePoint(x3, y3, m_xform);
  flipPoint(x2, y2, m_xform);
  flipPoint(x3, y3, m_xform);

  double x1 = m_x;
  double y1 = m_y;
  double x0 = ((x1-x2)*(x1+x2)*(y2-y3) - (x2-x3)*(x2+x3)*(y1-y2) +
               ecc*ecc*(y1-y2)*(y2-y3)*(y1-y3)) /
               (2*((x1-x2)*(y2-y3) - (x2-x3)*(y1-y2)));
  double y0 = ((x1-x2)*(x2-x3)*(x1-x3) + ecc*ecc*(x2-x3)*(y1-y2)*(y1+y2) -
               ecc*ecc*(x1-x2)*(y2-y3)*(y2+y3)) /
               (2*ecc*ecc*((x2-x3)*(y1-y2) - (x1-x2)*(y2-y3)));
  VSD_DEBUG_MSG(("Centre: (%f,%f), angle %f\n", x0, y0, angle));
  double rx = sqrt(pow(x1-x0, 2) + ecc*ecc*pow(y1-y0, 2));
  double ry = rx / ecc;

  m_x = x3; m_y = y3;
  WPXPropertyList arc;
  int largeArc = 0;
  int sweep = 1;

  // Calculate side of chord that ellipse centre and control point fall on
  double centreSide = (x3-x1)*(y0-y1) - (y3-y1)*(x0-x1);
  double midSide = (x3-x1)*(y2-y1) - (y3-y1)*(x2-x1);
  // Large arc if centre and control point are on the same side
  if ((centreSide > 0 && midSide > 0) || (centreSide < 0 && midSide < 0))
    largeArc = 1;
  // Change direction depending of side of control point
  if (midSide > 0)
    sweep = 0;

  arc.insert("svg:rx", m_scale*rx);
  arc.insert("svg:ry", m_scale*ry);
  arc.insert("libwpg:rotate", -(angle * (180 / M_PI) + m_xform.angle * (180 / M_PI)));
  arc.insert("libwpg:large-arc", largeArc);
  arc.insert("libwpg:sweep", sweep);
  arc.insert("svg:x", m_scale*m_x);
  arc.insert("svg:y", m_scale*m_y);
  arc.insert("libwpg:path-action", "A");
  m_currentGeometry[m_header.id] = arc;
}


void libvisio::VSD11Parser::readForeignData(WPXInputStream *input)
{
  if (m_foreignType == 1 || m_foreignType == 4) // Image
  {
    unsigned long tmpBytesRead = 0;
    const unsigned char *buffer = input->read(m_header.dataLength, tmpBytesRead);
    // If bmp data found, reconstruct header
    if (m_foreignType == 1 && m_foreignFormat == 0)
    {
      m_currentForeignData.append(0x42);
      m_currentForeignData.append(0x4d);

      m_currentForeignData.append((unsigned char)((tmpBytesRead + 14) & 0x000000ff));
      m_currentForeignData.append((unsigned char)(((tmpBytesRead + 14) & 0x0000ff00) >> 8));
      m_currentForeignData.append((unsigned char)(((tmpBytesRead + 14) & 0x00ff0000) >> 16));
      m_currentForeignData.append((unsigned char)(((tmpBytesRead + 14) & 0xff000000) >> 24));

      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);

      m_currentForeignData.append(0x36);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
    }
    m_currentForeignData.append(buffer, tmpBytesRead);

#if DUMP_BITMAP
    if (m_foreignType == 1 || m_foreignType == 4)
    {
      ::WPXString filename;
      switch(m_foreignFormat)
      {
      case 0:
        filename.sprintf("binarydump%i.bmp", bitmapId++); break;
      case 1:
        filename.sprintf("binarydump%i.jpeg", bitmapId++); break;
      case 2:
        filename.sprintf("binarydump%i.gif", bitmapId++); break;
      case 3:
        filename.sprintf("binarydump%i.tiff", bitmapId++); break;
      case 4:
        filename.sprintf("binarydump%i.png", bitmapId++); break;
      default:
        filename.sprintf("binarydump%i.bin", bitmapId++); break;
      }
      FILE *f = fopen(filename.cstr(), "wb");
      if (f)
      {
        const unsigned char *tmpBuffer = m_currentForeignData.getDataBuffer();
        for (unsigned long k = 0; k < m_currentForeignData.size(); k++)
          fprintf(f, "%c",tmpBuffer[k]);
        fclose(f);
      }
    }
#endif

    m_currentForeignProps.insert("svg:width", m_scale*m_xform.width);
    m_currentForeignProps.insert("svg:height", m_scale*m_xform.height);
    m_currentForeignProps.insert("svg:x", m_scale*(m_xform.pinX - m_xform.pinLocX));
    // Y axis starts at the bottom not top
    m_currentForeignProps.insert("svg:y", m_scale*(m_pageHeight -
                        m_xform.pinY + m_xform.pinLocY - m_xform.height));

    if (m_foreignType == 1)
    {
      switch(m_foreignFormat)
      {
      case 0:
        m_currentForeignProps.insert("libwpg:mime-type", "image/bmp"); break;
      case 1:
        m_currentForeignProps.insert("libwpg:mime-type", "image/jpeg"); break;
      case 2:
        m_currentForeignProps.insert("libwpg:mime-type", "image/gif"); break;
      case 3:
        m_currentForeignProps.insert("libwpg:mime-type", "image/tiff"); break;
      case 4:
        m_currentForeignProps.insert("libwpg:mime-type", "image/png"); break;
      }
    }
    else if (m_foreignType == 4)
    {
      const unsigned char *tmpBinData = m_currentForeignData.getDataBuffer();
      // Check for EMF signature
      if (tmpBinData[0x28] == 0x20 && tmpBinData[0x29] == 0x45 && tmpBinData[0x2A] == 0x4D && tmpBinData[0x2B] == 0x46)
      {
        m_currentForeignProps.insert("libwpg:mime-type", "image/emf");
      }
      else
      {
        m_currentForeignProps.insert("libwpg:mime-type", "image/wmf");
      }
    }
  }
}

void libvisio::VSD11Parser::readEllipse(WPXInputStream *input)
{
  WPXPropertyList ellipse;
  input->seek(1, WPX_SEEK_CUR);
  double cx = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double cy = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double aa = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  /* double bb = */ readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  /* double cc = */ readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double dd = readDouble(input);
  ellipse.insert("svg:rx", m_scale*(aa-cx));
  ellipse.insert("svg:ry", m_scale*(dd-cy));
  ellipse.insert("svg:cx", m_scale*(m_xform.x+cx));
  ellipse.insert("svg:cy", m_scale*(m_xform.y+cy));
  ellipse.insert("libwpg:rotate", m_xform.angle * (180/M_PI));
  m_painter->drawEllipse(ellipse);
}

void libvisio::VSD11Parser::readLine(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  m_styleProps.insert("svg:stroke-width", m_scale*readDouble(input));
  input->seek(1, WPX_SEEK_CUR);
  Colour c;
  c.r = readU8(input);
  c.g = readU8(input);
  c.b = readU8(input);
  c.a = readU8(input);
  m_lineColour = getColourString(c);
  m_styleProps.insert("svg:stroke-color", m_lineColour);
  m_linePattern = readU8(input);
  const char* patterns[] = {
    /*  0 */  "none",
    /*  1 */  "solid",
    /*  2 */  "6, 3",
    /*  3 */  "1, 3",
    /*  4 */  "6, 3, 1, 3",
    /*  5 */  "6, 3, 1, 3, 1, 3",
    /*  6 */  "6, 3, 6, 3, 1, 3",
    /*  7 */  "14, 2, 6, 2",
    /*  8 */  "14, 2, 6, 2, 6, 2",
    /*  9 */  "3, 1",
    /* 10 */  "1, 1",
    /* 11 */  "3, 1, 1, 1",
    /* 12 */  "3, 1, 1, 1, 1, 1",
    /* 13 */  "3, 1, 3, 1, 1, 1",
    /* 14 */  "7, 1, 3, 1",
    /* 15 */  "7, 1, 3, 1, 3, 1",
    /* 16 */  "11, 5",
    /* 17 */  "1, 5",
    /* 18 */  "11, 5, 1, 5",
    /* 19 */  "11, 5, 1, 5, 1, 5",
    /* 20 */  "11, 5, 11, 5, 1, 5",
    /* 21 */  "27, 5, 11, 5",
    /* 22 */  "27, 5, 11, 5, 11, 5",
    /* 23 */  "2, 1"
  };
  if (m_linePattern > 0 && m_linePattern < sizeof(patterns)/sizeof(patterns[0]))
    m_styleProps.insert("svg:stroke-dasharray", patterns[m_linePattern]);
  // FIXME: later it will require special treatment for custom line patterns
  // patt ID is 0xfe, link to stencil name is in 'Line' blocks
}

void libvisio::VSD11Parser::readFillAndShadow(WPXInputStream *input)
{
  unsigned int colourIndexFG = readU8(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned int colourIndexBG = readU8(input);
  input->seek(4, WPX_SEEK_CUR);
  m_fillPattern = readU8(input);
  if (m_fillPattern == 1)
  {
    m_fillType = "solid";
    m_styleProps.insert("draw:fill", "solid");
    m_styleProps.insert("draw:fill-color", getColourString(m_colours[colourIndexFG]));
  }
  else if (m_fillPattern >= 25 && m_fillPattern <= 34)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:fill", "gradient");
    WPXPropertyList startColour;
    startColour.insert("svg:stop-color",
                       getColourString(m_colours[colourIndexFG]));
    startColour.insert("svg:offset", 0, WPX_PERCENT);
    startColour.insert("svg:stop-opacity", 1, WPX_PERCENT);
    WPXPropertyList endColour;
    endColour.insert("svg:stop-color",
                     getColourString(m_colours[colourIndexBG]));
    endColour.insert("svg:offset", 1, WPX_PERCENT);
    endColour.insert("svg:stop-opacity", 1, WPX_PERCENT);

    switch(m_fillPattern)
    {
    case 25:
      m_styleProps.insert("draw:angle", -90);
      break;
    case 26:
      m_styleProps.insert("draw:angle", -90);
      endColour.insert("svg:offset", 0, WPX_PERCENT);
      m_gradientProps.append(endColour);
      endColour.insert("svg:offset", 1, WPX_PERCENT);
      startColour.insert("svg:offset", 0.5, WPX_PERCENT);
      break;
    case 27:
      m_styleProps.insert("draw:angle", 90);
      break;
    case 28:
      m_styleProps.insert("draw:angle", 0);
      break;
    case 29:
      m_styleProps.insert("draw:angle", 0);
      endColour.insert("svg:offset", 0, WPX_PERCENT);
      m_gradientProps.append(endColour);
      endColour.insert("svg:offset", 1, WPX_PERCENT);
      startColour.insert("svg:offset", 0.5, WPX_PERCENT);
      break;
    case 30:
      m_styleProps.insert("draw:angle", 180);
      break;
    case 31:
      m_styleProps.insert("draw:angle", -45);
      break;
    case 32:
      m_styleProps.insert("draw:angle", 45);
      break;
    case 33:
      m_styleProps.insert("draw:angle", 225);
      break;
    case 34:
      m_styleProps.insert("draw:angle", 135);
      break;
    }
    m_gradientProps.append(startColour);
    m_gradientProps.append(endColour);
  }
}


void libvisio::VSD11Parser::readGeomList(WPXInputStream *input)
{
  _flushCurrentPath();
  m_painter->setStyle(m_styleProps, m_gradientProps);
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    m_currentGeometryOrder.push_back(readU32(input));
  m_noShow = false;
}

void libvisio::VSD11Parser::readGeometry(WPXInputStream *input)
{
  m_x = 0.0; m_x = 0.0;
  unsigned int geomFlags = readU8(input);
  m_noFill = ((geomFlags & 1) == 1);
  m_noLine = ((geomFlags & 2) == 2);
  m_noShow = ((geomFlags & 4) == 4);
  if (m_noLine || m_linePattern == 0)
    m_styleProps.insert("svg:stroke-color", "none");
  else
    m_styleProps.insert("svg:stroke-color", m_lineColour);
  if (m_noFill || m_fillPattern == 0)
    m_styleProps.insert("svg:fill", "none");
  else
    m_styleProps.insert("svg:fill", m_fillType);
  VSD_DEBUG_MSG(("Flag: %d NoFill: %d NoLine: %d NoShow: %d\n", geomFlags, m_noFill, m_noLine, m_noShow));
  m_painter->setStyle(m_styleProps, m_gradientProps);
}

void libvisio::VSD11Parser::readMoveTo(WPXInputStream *input)
{
  WPXPropertyList end;
  input->seek(1, WPX_SEEK_CUR);
  m_x = readDouble(input) + m_xform.x;
  input->seek(1, WPX_SEEK_CUR);
  m_y = (m_xform.height - readDouble(input)) + m_xform.y;
  rotatePoint(m_x, m_y, m_xform);
  flipPoint(m_x, m_y, m_xform);

  end.insert("svg:x", m_scale*m_x);
  end.insert("svg:y", m_scale*m_y);
  end.insert("libwpg:path-action", "M");
  m_currentGeometry[m_header.id] = end;
}

void libvisio::VSD11Parser::readLineTo(WPXInputStream *input)
{
  WPXPropertyList end;
  input->seek(1, WPX_SEEK_CUR);
  m_x = readDouble(input) + m_xform.x;
  input->seek(1, WPX_SEEK_CUR);
  m_y = (m_xform.height - readDouble(input)) + m_xform.y;
  rotatePoint(m_x, m_y, m_xform);
  flipPoint(m_x, m_y, m_xform);

  end.insert("svg:x", m_scale*m_x);
  end.insert("svg:y", m_scale*m_y);
  end.insert("libwpg:path-action", "L");
  m_currentGeometry[m_header.id] = end;
}

void libvisio::VSD11Parser::readArcTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input) + m_xform.x;
  input->seek(1, WPX_SEEK_CUR);
  double y2 = (m_xform.height - readDouble(input)) + m_xform.y;
  input->seek(1, WPX_SEEK_CUR);
  double bow = readDouble(input);

  rotatePoint(x2, y2, m_xform);
  flipPoint(x2, y2, m_xform);

  if (bow == 0)
  {
    m_x = x2; m_y = y2;
    WPXPropertyList end;
    end.insert("svg:x", m_scale*m_x);
    end.insert("svg:y", m_scale*m_y);
    end.insert("libwpg:path-action", "L");
    m_currentGeometry[m_header.id] = end;
  }
  else
  {
    WPXPropertyList arc;
    double chord = sqrt(pow((y2 - m_y),2) + pow((x2 - m_x),2));
    double radius = (4 * bow * bow + chord * chord) / (8 * fabs(bow));
    VSD_DEBUG_MSG(("ArcTo with bow %f radius %f and chord %f\n", bow, radius, chord));
    int largeArc = fabs(bow) > radius ? 1 : 0;
    int sweep = bow < 0 ? 1 : 0;
    m_x = x2; m_y = y2;
    arc.insert("svg:rx", m_scale*radius);
    arc.insert("svg:ry", m_scale*radius);
    arc.insert("libwpg:rotate", m_xform.angle * (180/M_PI));
    arc.insert("libwpg:large-arc", largeArc);
    arc.insert("libwpg:sweep", sweep);
    arc.insert("svg:x", m_scale*m_x);
    arc.insert("svg:y", m_scale*m_y);
    arc.insert("libwpg:path-action", "A");
    m_currentGeometry[m_header.id] = arc;
  }
}
