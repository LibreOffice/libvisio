/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
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
#include "VSDXParser.h"
#include "VSDInternalStream.h"
#include "VSDXDocumentStructure.h"
#include "VSDXCollector.h"

#define DUMP_BITMAP 0

#if DUMP_BITMAP
static unsigned bitmapId = 0;
#include <sstream>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

libvisio::VSDXParser::VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : m_input(input), m_painter(painter), m_header(), m_collector(0)
{}

libvisio::VSDXParser::~VSDXParser()
{}


// --- READERS ---

void libvisio::VSDXParser::readEllipticalArcTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x3 = readDouble(input); // End x
  input->seek(1, WPX_SEEK_CUR);
  double y3 = readDouble(input); // End y
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input); // Mid x
  input->seek(1, WPX_SEEK_CUR);
  double y2 = readDouble(input); // Mid y
  input->seek(1, WPX_SEEK_CUR);
  double angle = readDouble(input); // Angle
  input->seek(1, WPX_SEEK_CUR);
  double ecc = readDouble(input); // Eccentricity

  m_collector->collectEllipticalArcTo(m_header.id, m_header.level, x3, y3, x2, y2, angle, ecc);
}


void libvisio::VSDXParser::readForeignData(WPXInputStream *input)
{
  unsigned long tmpBytesRead = 0;
  const unsigned char *buffer = input->read(m_header.dataLength, tmpBytesRead);
  if (m_header.dataLength != tmpBytesRead)
    return;
  WPXBinaryData binaryData(buffer, tmpBytesRead);

  m_collector->collectForeignData(m_header.id, m_header.level, binaryData);
}

void libvisio::VSDXParser::readEllipse(WPXInputStream *input)
{
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

  m_collector->collectEllipse(m_header.id, m_header.level, cx, cy, aa, dd);
}

void libvisio::VSDXParser::readLine(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double strokeWidth = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  Colour c;
  c.r = readU8(input);
  c.g = readU8(input);
  c.b = readU8(input);
  c.a = readU8(input);
  unsigned linePattern = readU8(input);

  m_collector->collectLine(m_header.id, m_header.level, strokeWidth, c, linePattern);
}

void libvisio::VSDXParser::readFillAndShadow(WPXInputStream *input)
{
  unsigned int colourIndexFG = readU8(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned int colourIndexBG = readU8(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned fillPattern = readU8(input);

  m_collector->collectFillAndShadow(m_header.id, m_header.level, colourIndexFG, colourIndexBG, fillPattern);
}


void libvisio::VSDXParser::readGeomList(WPXInputStream *input)
{
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  std::vector<unsigned> geometryOrder;
  geometryOrder.reserve(childrenListLength / sizeof(uint32_t));
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    geometryOrder.push_back(readU32(input));

  m_collector->collectGeomList(m_header.id, m_header.level, geometryOrder);
}

void libvisio::VSDXParser::readGeometry(WPXInputStream *input)
{
  unsigned geomFlags = readU8(input);
  
  m_collector->collectGeometry(m_header.id, m_header.level, geomFlags);
}

void libvisio::VSDXParser::readMoveTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  m_collector->collectMoveTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDXParser::readLineTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  m_collector->collectLineTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDXParser::readArcTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y2 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double bow = readDouble(input);

  m_collector->collectArcTo(m_header.id, m_header.level, x2, y2, bow);
}

void libvisio::VSDXParser::readXFormData(WPXInputStream *input)
{
  XForm xform;
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

  m_collector->collectXFormData(m_header.id, m_header.level, xform);
}

void libvisio::VSDXParser::readShapeID(WPXInputStream *input)
{
  unsigned shapeId = readU32(input);

  m_collector->collectShapeID(m_header.id, m_header.level, shapeId);
}

void libvisio::VSDXParser::readForeignDataType(WPXInputStream *input)
{
  input->seek(0x24, WPX_SEEK_CUR);
  unsigned foreignType = readU16(input);
  input->seek(0xb, WPX_SEEK_CUR);
  unsigned foreignFormat = readU32(input);

  VSD_DEBUG_MSG(("Found foreign data, type %d format %d\n", foreignType, foreignFormat));

  m_collector->collectForeignDataType(m_header.id, m_header.level, foreignType, foreignFormat);
}

void libvisio::VSDXParser::readPageProps(WPXInputStream *input)
{
  // Skip bytes representing unit to *display* (value is always inches)
  input->seek(1, WPX_SEEK_CUR);
  double pageWidth = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double pageHeight = readDouble(input);
  input->seek(19, WPX_SEEK_CUR);
  /* m_scale = */ readDouble(input);

  m_collector->collectPageProps(m_header.id, m_header.level, pageWidth, pageHeight);
}

void libvisio::VSDXParser::readColours(WPXInputStream *input)
{
  input->seek(6, WPX_SEEK_SET);
  unsigned int numColours = readU8(input);
  Colour tmpColour;

  input->seek(1, WPX_SEEK_CUR);

  std::vector<Colour> colours;

  for (unsigned int i = 0; i < numColours; i++)
  {
    tmpColour.r = readU8(input);
    tmpColour.g = readU8(input);
    tmpColour.b = readU8(input);
    tmpColour.a = readU8(input);

    colours.push_back(tmpColour);
  }
  m_collector->collectColours(colours);
}

void libvisio::VSDXParser::shapeChunk(WPXInputStream *input)
{
  long endPos = 0;

  m_collector->shapeChunkBegin(m_header.id, m_header.level);

  while (!input->atEOS())
  {
    if (!getChunkHeader(input))
      break;
    endPos = m_header.dataLength+m_header.trailer+input->tell();

    // Break once a chunk that is not nested in the shape is found
    if (m_header.level < 2)
    {
      input->seek(-19, WPX_SEEK_CUR);
      break;
    }

    VSD_DEBUG_MSG(("Shape: parsing chunk type %x\n", m_header.chunkType));
    switch (m_header.chunkType)
    {
    case VSD_XFORM_DATA: // XForm data
      readXFormData(input);
      break;
    case VSD_SHAPE_ID:
      readShapeID(input);
      break;
    case VSD_LINE:
      readLine(input);
      break;
    case VSD_FILL_AND_SHADOW:
      readFillAndShadow(input);
      break;
    case VSD_GEOM_LIST:
      readGeomList(input);
      break;
    case VSD_GEOMETRY:
      readGeometry(input);
      break;
    case VSD_MOVE_TO:
      readMoveTo(input);
      break;
    case VSD_LINE_TO:
      readLineTo(input);
      break;
    case VSD_ARC_TO:
      readArcTo(input);
      break;
    case VSD_ELLIPSE:
      readEllipse(input);
      break;
    case VSD_ELLIPTICAL_ARC_TO:
      readEllipticalArcTo(input);
      break;
    case VSD_FOREIGN_DATA_TYPE:
      readForeignDataType(input);
      break;
    case VSD_FOREIGN_DATA:
      readForeignData(input);
      break;
    }

    input->seek(endPos, WPX_SEEK_SET);
  }
  m_collector->shapeChunkEnd(m_header.id, m_header.level);
}

