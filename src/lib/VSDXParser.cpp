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
#include "VSDXContentCollector.h"
#include "VSDXStylesCollector.h"

libvisio::VSDXParser::VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : m_input(input), m_painter(painter), m_header(), m_collector(0), m_geomList(),
    m_shapeList(), m_currentLevel(0)
{}

/** Parses Visio input stream content, making callbacks to functions provided
by WPGPaintInterface class implementation as needed.
\param iface A WPGPaintInterface implementation
\return A value indicating whether parsing was successful
*/
bool libvisio::VSDXParser::parse()
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

  std::vector<std::map<unsigned, XForm> > groupXFormsSequence;
  std::vector<std::map<unsigned, unsigned> > groupMembershipsSequence;
  std::vector<std::list<unsigned> > documentPageShapeOrders;

  VSDXStylesCollector stylesCollector(groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders);
  m_collector = &stylesCollector;
  if (!parseDocument(trailerStream))
  {
    delete trailerStream;
    return false;
  }

  VSDXContentCollector contentCollector(m_painter, groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders);
  m_collector = &contentCollector;
  if (!parseDocument(trailerStream))
  {
    delete trailerStream;
    return false;
  }

  delete trailerStream;
  return true;
}

bool libvisio::VSDXParser::parseDocument(WPXInputStream *input)
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

    bool compressed = ((ptrFormat & 2) == 2);
    m_input->seek(ptrOffset, WPX_SEEK_SET);
    VSDInternalStream tmpInput(m_input, ptrLength, compressed);

    switch (ptrType)
    {
    case VSD_PAGE:
      handlePage(&tmpInput);
      break;
    case VSD_PAGES:
      handlePages(&tmpInput);
      break;
    case VSD_COLORS:
      readColours(&tmpInput);
      break;
    default:
      break;
    }
  }

  return true;
}

void libvisio::VSDXParser::handlePages(WPXInputStream *input)
{
  unsigned int ptrType;
  unsigned int ptrOffset;
  unsigned int ptrLength;
  unsigned int ptrFormat;

  unsigned int offset = readU32(input);
  input->seek(offset, WPX_SEEK_SET);
  unsigned int pointerCount = readU32(input);
  input->seek(4, WPX_SEEK_CUR); // Ignore 0x0 dword

  for (unsigned int i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(input);
    ptrLength = readU32(input);
    ptrFormat = readU16(input);

    bool compressed = ((ptrFormat & 2) == 2);
    m_input->seek(ptrOffset, WPX_SEEK_SET);
    WPXInputStream *tmpInput = new VSDInternalStream(m_input, ptrLength, compressed);

    switch (ptrType)
    {
    case VSD_PAGE:
      handlePage(tmpInput);
      break;
    case VSD_PAGES:
      handlePages(tmpInput);
      break;
    case VSD_COLORS:
      readColours(tmpInput);
      break;
    default:
      break;
    }

    delete tmpInput;
  }
}

void libvisio::VSDXParser::_handleLevelChange(unsigned level)
{
  if (level == m_currentLevel)
    return;
  if (level < 3)
  {
    m_geomList.handle(m_collector);
    m_geomList.clear();
    m_shapeList.handle(m_collector);
    m_shapeList.clear();
  }
  m_currentLevel = level;
}

void libvisio::VSDXParser::handlePage(WPXInputStream *input)
{
  long endPos = 0;

  m_collector->startPage();

  while (!input->atEOS())
  {
    if (!getChunkHeader(input))
      break;
    endPos = m_header.dataLength+m_header.trailer+input->tell();

    _handleLevelChange(m_header.level);
    VSD_DEBUG_MSG(("Shape: parsing chunk type %x\n", m_header.chunkType));
    switch (m_header.chunkType)
    {
    case VSD_SHAPE_GROUP:
    case VSD_SHAPE_SHAPE:
    case VSD_SHAPE_FOREIGN:
      readShape(input);
      break;
    case VSD_XFORM_DATA:
      readXFormData(input);
      break;
    case VSD_SHAPE_LIST:
      readShapeList(input);
      break;
    case VSD_SHAPE_ID:
      readShapeId(input);
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
    case VSD_NURBS_TO:
      readNURBSTo(input);
      break;
    case VSD_FOREIGN_DATA_TYPE:
      readForeignDataType(input);
      break;
    case VSD_FOREIGN_DATA:
      readForeignData(input);
      break;
    case VSD_PAGE_PROPS:
      readPageProps(input);
    default:
      m_collector->collectUnhandledChunk(m_header.id, m_header.level);
    }

    input->seek(endPos, WPX_SEEK_SET);
  }
  _handleLevelChange(0);
  m_collector->endPage();
}

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

  m_geomList.addEllipticalArcTo(m_header.id, m_header.level, x3, y3, x2, y2, angle, ecc);
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
  double xleft = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double yleft = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double xtop = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double ytop = readDouble(input);

  m_geomList.addEllipse(m_header.id, m_header.level, cx, cy, xleft, yleft, xtop, ytop);
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

  m_geomList.setElementsOrder(geometryOrder);
  // We want the collectors to still get the level information
  m_collector->collectGeomList(m_header.id, m_header.level);
}

void libvisio::VSDXParser::readGeometry(WPXInputStream *input)
{
  unsigned geomFlags = readU8(input);

  m_geomList.addGeometry(m_header.id, m_header.level, geomFlags);
}

void libvisio::VSDXParser::readMoveTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  m_geomList.addMoveTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDXParser::readLineTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  m_geomList.addLineTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDXParser::readArcTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y2 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double bow = readDouble(input);

  m_geomList.addArcTo(m_header.id, m_header.level, x2, y2, bow);
}

void libvisio::VSDXParser::readNURBSTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);
  double a = readDouble(input); // Second last knot
  double b = readDouble(input); // Last weight
  double c = readDouble(input); // First knot
  double d = readDouble(input); // First weight

  std::vector<double> knotVector;
  knotVector.push_back(c);
  std::vector<std::pair<double, double> > controlPoints;
  std::vector<double> weights;
  weights.push_back(d);

  input->seek(11, WPX_SEEK_CUR); // Seek to blocks at offset 0x50 (80)

  // Find formula block referring to cell E (cell 6)
  unsigned cellRef = 0;
  unsigned length = 0;
  unsigned long inputPos = input->tell();
  unsigned long bytesRead = 0;
  while (cellRef != 6)
  {
    length = readU32(input);
    input->seek(1, WPX_SEEK_CUR);
    cellRef = readU8(input);
    if (cellRef != 6)
      input->seek(length - 6, WPX_SEEK_CUR);
  }

  // Indicates whether it's a "simple" NURBS block with a static format
  // or a complex block where parameters each have a type
  unsigned paramType = readU8(input);
  unsigned short valueType = 0;

  double lastKnot = 0; unsigned degree = 0; 
  unsigned xType = 0; unsigned yType = 0;
  unsigned repetitions = 0;

  // Read formula's static first four parameters
  if (paramType == 0x8a)
  {
    lastKnot = readDouble(input);
    degree = readU16(input);
    xType = readU8(input);
    yType = readU8(input);
    repetitions = readU32(input);
  }
  else
  {
    valueType = paramType;
    if (valueType == 0x20)
      lastKnot = readDouble(input);
    else
      lastKnot = readU16(input);

    input->seek(1, WPX_SEEK_CUR);
    degree = readU16(input);
    input->seek(1, WPX_SEEK_CUR);
    xType = readU16(input);
    input->seek(1, WPX_SEEK_CUR);
    yType = readU16(input);
  }

  // Read sequences of (x, y, knot, weight) until finished
  bytesRead = input->tell() - inputPos;
  unsigned flag = 0;
  if (paramType != 0x8a) flag = readU8(input);
  while ((flag != 0x81 || (paramType == 0x8a && repetitions > 0)) && bytesRead < length)
  {
    inputPos = input->tell();
    double knot = 0;
    double weight = 0;
    double controlX = 0;
    double controlY = 0;

    if (paramType == 0x8a) // Parameters have static format
    {
      controlX = readDouble(input);
      controlY = readDouble(input);
      knot = readDouble(input);
      weight = readDouble(input);
    }
    else // Parameters have types
    {
      valueType = flag;
      if (valueType == 0x20)
        controlX = readDouble(input);
      else
        controlX = readU16(input);

      valueType = readU8(input);
      if (valueType == 0x20)
        controlY = readDouble(input);
      else
        controlY = readU16(input);

      valueType = readU8(input);
      if (valueType == 0x20)
        knot = readDouble(input);
      else if (valueType == 0x62)
        knot = readU16(input);

      valueType = readU8(input);
      if (valueType == 0x20)
        weight = readDouble(input);
      else if (valueType == 0x62)
        weight = readU16(input);
    }
    controlPoints.push_back(std::pair<double, double>(controlX, controlY));
    knotVector.push_back(knot);
    weights.push_back(weight);

    if (paramType != 0x8a) flag = readU8(input);
    else repetitions--;
    bytesRead += input->tell() - inputPos;
  }

  knotVector.push_back(a);
  knotVector.push_back(lastKnot);
  weights.push_back(b);
#if DEBUG
  VSD_DEBUG_MSG(("Control points: %d, knots: %d, weights: %d, degree: %d\n", controlPoints.size(), knotVector.size(), weights.size(), degree));
#endif

  m_geomList.addNURBSTo(m_header.id, m_header.level, x, y, xType,
  yType, degree, controlPoints, knotVector, weights);
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

void libvisio::VSDXParser::readShapeId(WPXInputStream *input)
{
  unsigned shapeId = readU32(input);

  m_shapeList.addShapeId(m_header.id, m_header.level, shapeId);
}

void libvisio::VSDXParser::readShapeList(WPXInputStream *input)
{
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  std::vector<unsigned> shapeOrder;
  shapeOrder.reserve(childrenListLength / sizeof(uint32_t));
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    shapeOrder.push_back(readU32(input));

  m_shapeList.setElementsOrder(shapeOrder);
  // We want the collectors to still get the level information
  m_collector->collectShapeList(m_header.id, m_header.level);
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

void libvisio::VSDXParser::readShape(WPXInputStream * /* input */)
{
  m_collector->collectShape(m_header.id, m_header.level);
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

