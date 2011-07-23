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
  : m_input(input), m_painter(painter), m_header(), m_collector(0), m_geomList(new VSDXGeometryList()), m_geomListVector(),
    m_charList(new VSDXCharacterList()), m_charListVector(), m_shapeList(), m_currentLevel(0)
{}

libvisio::VSDXParser::~VSDXParser()
{
  if (m_geomList)
  {
    m_geomList->clear();
    delete m_geomList;
  }
  if (m_charList)
  {
    m_charList->clear();
    delete m_charList;
  }
}

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

//  unsigned int ptrType;
// unsigned int ptrOffset;
//  unsigned int ptrLength;
//  unsigned int ptrFormat;
  std::vector<libvisio::Pointer> PtrList;
  Pointer ptr;
  
  // Parse out pointers to other streams from trailer
  input->seek(SHIFT, WPX_SEEK_SET);
  unsigned offset = readU32(input);
  input->seek(offset+SHIFT, WPX_SEEK_SET);
  unsigned int pointerCount = readU32(input);
  input->seek(SHIFT, WPX_SEEK_CUR);
  for (unsigned int i = 0; i < pointerCount; i++)
  {
    ptr.Type = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip dword
    ptr.Offset = readU32(input);
    ptr.Length = readU32(input);
    ptr.Format = readU16(input);
    
    if (ptr.Type == VSD_FONTFACES)
      PtrList.insert(PtrList.begin(),ptr);
//      PtrList.push_back(ptr);
    else if (ptr.Type != 0)
      PtrList.push_back(ptr);
  }
  for (unsigned int i = 0; i < PtrList.size(); i++)
  {
    ptr = PtrList[i];
    bool compressed = ((ptr.Format & 2) == 2);
    m_input->seek(ptr.Offset, WPX_SEEK_SET);
    VSDInternalStream tmpInput(m_input, ptr.Length, compressed);

    switch (ptr.Type)
    {
    case VSD_PAGE:           // shouldn't happen
    case VSD_FONT_LIST:      // ver6 stream contains chunk 0x18 (FontList) and chunks 0x19 (Font)
      handlePage(&tmpInput);
      break;
    case VSD_PAGES:
    case VSD_FONTFACES:      // ver11 stream contains streams 0xd7 (FontFace)
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
    case VSD_PAGES:             // shouldn't happen
      handlePages(tmpInput);
      break;
    case VSD_COLORS:            // shouldn't happen
      readColours(tmpInput);
      break;
    case VSD_FONTFACE:          // substreams of FONTAFACES stream, ver 11 only
      readFont(tmpInput, i);
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
    m_geomListVector.push_back(m_geomList);
    m_charListVector.push_back(m_charList);
    // reinitialize, but don't clear, because we want those pointers to be valid until we handle the whole vector
    m_geomList = new VSDXGeometryList();
    m_charList = new VSDXCharacterList();
    m_shapeList.handle(m_collector);
    m_shapeList.clear();
  }
  if (level < 2)
  {
    for (std::vector<VSDXGeometryList *>::iterator iter = m_geomListVector.begin(); iter != m_geomListVector.end(); iter++)
    {
      (*iter)->handle(m_collector);
      (*iter)->clear();
      delete *iter;
    }
    m_geomListVector.clear();
    for (std::vector<VSDXCharacterList *>::iterator iter = m_charListVector.begin(); iter != m_charListVector.end(); iter++)
    {
      (*iter)->handle(m_collector);
      (*iter)->clear();
      delete *iter;
    }
    m_charListVector.clear();
  }
  m_currentLevel = level;
}

void libvisio::VSDXParser::handlePage(WPXInputStream *input)
{
  try
  {

    long endPos = 0;

    m_collector->startPage();

    while (!input->atEOS())
    {
      getChunkHeader(input);
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
      case VSD_TEXT_XFORM:
        readTxtXForm(input);
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
      case VSD_POLYLINE_TO:
        readPolylineTo(input);
        break;
      case VSD_SHAPE_DATA:
        readShapeData(input);
        break;
      case VSD_FOREIGN_DATA_TYPE:
        readForeignDataType(input);
        break;
      case VSD_FOREIGN_DATA:
        readForeignData(input);
        break;
      case VSD_PAGE_PROPS:
        readPageProps(input);
        break;
      case VSD_CHAR_LIST:
        readCharList(input);
        break;
      case VSD_TEXT:
        readText(input);
        break;
      case VSD_CHAR_IX:
        readCharIX(input);
        break;
//    case VSD_FONT_LIST: // ver 6 only, don't need to handle that
      case VSD_FONT_IX: // ver 6 only
        readFontIX(input);
        break;
      default:
        m_collector->collectUnhandledChunk(m_header.id, m_header.level);
      }

      input->seek(endPos, WPX_SEEK_SET);
    }
    _handleLevelChange(0);
    m_collector->endPage();
  }
  catch (EndOfStreamException)
  {
    _handleLevelChange(0);
    m_collector->endPage();
  }
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

  m_geomList->addEllipticalArcTo(m_header.id, m_header.level, x3, y3, x2, y2, angle, ecc);
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

  m_geomList->addEllipse(m_header.id, m_header.level, cx, cy, xleft, yleft, xtop, ytop);
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
  input->seek(12, WPX_SEEK_CUR);
  unsigned lineCap = readU8(input);

  m_collector->collectLine(m_header.id, m_header.level, strokeWidth, c, linePattern, lineCap);
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

  m_geomList->setElementsOrder(geometryOrder);
  // We want the collectors to still get the level information
  m_collector->collectGeomList(m_header.id, m_header.level);
}

void libvisio::VSDXParser::readCharList(WPXInputStream *input)
{
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  std::vector<unsigned> characterOrder;
  characterOrder.reserve(childrenListLength / sizeof(uint32_t));
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    characterOrder.push_back(readU32(input));

  m_charList->setElementsOrder(characterOrder);
  // We want the collectors to still get the level information
  m_collector->collectCharList(m_header.id, m_header.level);
}

void libvisio::VSDXParser::readGeometry(WPXInputStream *input)
{
  unsigned geomFlags = readU8(input);

  m_geomList->addGeometry(m_header.id, m_header.level, geomFlags);
}

void libvisio::VSDXParser::readMoveTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  m_geomList->addMoveTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDXParser::readLineTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  m_geomList->addLineTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDXParser::readArcTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y2 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double bow = readDouble(input);

  m_geomList->addArcTo(m_header.id, m_header.level, x2, y2, bow);
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

void libvisio::VSDXParser::readTxtXForm(WPXInputStream *input)
{
  XForm txtxform;
  input->seek(1, WPX_SEEK_CUR);
  txtxform.pinX = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  txtxform.pinY = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  txtxform.width = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  txtxform.height = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  txtxform.pinLocX = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  txtxform.pinLocY = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  txtxform.angle = readDouble(input);

  m_collector->collectTxtXForm(m_header.id, m_header.level, txtxform);
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
  input->seek(1, WPX_SEEK_CUR);
  double shadowOffsetX = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double shadowOffsetY = -readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  /* m_scale = */ readDouble(input);

  m_collector->collectPageProps(m_header.id, m_header.level, pageWidth, pageHeight, shadowOffsetX, shadowOffsetY);
}

void libvisio::VSDXParser::readShape(WPXInputStream * /* input */)
{
  m_collector->collectShape(m_header.id, m_header.level);
}

void libvisio::VSDXParser::readNURBSTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);
  double knot = readDouble(input); // Second last knot
  double weight = readDouble(input); // Last weight
  double knotPrev = readDouble(input); // First knot
  double weightPrev = readDouble(input); // First weight

  // Detect whether to use Shape Data block
  input->seek(1, WPX_SEEK_CUR);
  unsigned useData = readU8(input);
  if (useData == 0x8a)
  {
    input->seek(3, WPX_SEEK_CUR);
    unsigned dataId = readU32(input);
    m_geomList->addNURBSTo(m_header.id, m_header.level, x, y, knot, knotPrev, weight, weightPrev, dataId);
    return;
  }

  std::vector<double> knotVector;
  knotVector.push_back(knotPrev);
  std::vector<std::pair<double, double> > controlPoints;
  std::vector<double> weights;
  weights.push_back(weightPrev);

  input->seek(9, WPX_SEEK_CUR); // Seek to blocks at offset 0x50 (80)
  unsigned long chunkBytesRead = 0x50;

  // Find formula block referring to cell E (cell 6)
  unsigned cellRef = 0;
  unsigned length = 0;
  unsigned long inputPos = input->tell();
  unsigned long bytesRead = 0;
  while (cellRef != 6 && !input->atEOS() &&
         m_header.dataLength - chunkBytesRead > 4)
  {
    length = readU32(input);
    input->seek(1, WPX_SEEK_CUR);
    cellRef = readU8(input);
    if (cellRef < 6)
      input->seek(length - 6, WPX_SEEK_CUR);
    chunkBytesRead += input->tell() - inputPos;
    inputPos = input->tell();
  }

  if (input->atEOS())
    return;

  unsigned degree = 3;
  unsigned xType = 1; unsigned yType = 1;
  // Only read formula if block is found
  if (cellRef == 6)
  {
    // Indicates whether it's a "simple" NURBS block with a static format
    // or a complex block where parameters each have a type
    unsigned paramType = readU8(input);
    unsigned short valueType = 0;

    double lastKnot = 0;
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
      double knot_ = 0;
      double weight_ = 0;
      double controlX = 0;
      double controlY = 0;

      if (paramType == 0x8a) // Parameters have static format
      {
        controlX = readDouble(input);
        controlY = readDouble(input);
        knot_ = readDouble(input);
        weight_ = readDouble(input);
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
          knot_ = readDouble(input);
        else if (valueType == 0x62)
          knot_ = readU16(input);

        valueType = readU8(input);
        if (valueType == 0x20)
          weight_ = readDouble(input);
        else if (valueType == 0x62)
          weight_ = readU16(input);
      }
      controlPoints.push_back(std::pair<double, double>(controlX, controlY));
      knotVector.push_back(knot_);
      weights.push_back(weight_);

      if (paramType != 0x8a) flag = readU8(input);
      else repetitions--;
      bytesRead += input->tell() - inputPos;
    }
    knotVector.push_back(knot);
    knotVector.push_back(lastKnot);
    weights.push_back(weight);
    m_geomList->addNURBSTo(m_header.id, m_header.level, x, y, xType,
                           yType, degree, controlPoints, knotVector, weights);
  }
  else // No formula found, use line
  {
    m_geomList->addLineTo(m_header.id, m_header.level, x,  y);
  }
}

void libvisio::VSDXParser::readPolylineTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  // Detect whether to use Shape Data block
  input->seek(1, WPX_SEEK_CUR);
  unsigned useData = readU8(input);
  if (useData == 0x8b)
  {
    input->seek(3, WPX_SEEK_CUR);
    unsigned dataId = readU32(input);
    m_geomList->addPolylineTo(m_header.id, m_header.level, x, y, dataId);
    return;
  }

  // Blocks start at 0x30
  input->seek(0x9, WPX_SEEK_CUR);
  unsigned long chunkBytesRead = 0x30;

    // Find formula block referring to cell A (cell 2)
  unsigned cellRef = 0;
  unsigned length = 0;
  unsigned long inputPos = input->tell();
  unsigned long blockBytesRead = 0;
  while (cellRef != 2 && !input->atEOS() &&
         m_header.dataLength - chunkBytesRead > 4)
  {
    length = readU32(input);
    if (!length)
      break;
    input->seek(1, WPX_SEEK_CUR);
    cellRef = readU8(input);
    if (cellRef < 2)
      input->seek(length - 6, WPX_SEEK_CUR);
    chunkBytesRead += input->tell() - inputPos;
    inputPos = input->tell();
  }

  if (input->atEOS())
    return;

  // Default to local co-ordinates if unspecified
  unsigned xType = 1; unsigned yType = 1;
  std::vector<std::pair<double, double> > points;

  // Only formula if block is found
  if (cellRef == 2)
  {
    inputPos = input->tell();
    blockBytesRead += 6;

    // Parse static first two parameters to function
    input->seek(1, WPX_SEEK_CUR);
    xType = readU16(input);
    input->seek(1, WPX_SEEK_CUR);
    yType = readU16(input);

    // Parse pairs of x,y co-ordinates
    unsigned flag = readU8(input);
    unsigned valueType = 0; // Holds parameter type indicator
    blockBytesRead += input->tell() - inputPos;
    while (flag != 0x81 && blockBytesRead < length)
    {
      inputPos = input->tell();
      double x2 = 0; double y2 = 0;

      valueType = flag;
      if (valueType == 0x20)
        x2 = readDouble(input);
      else
        x2 = readU16(input);

      valueType = readU8(input);
      if (valueType == 0x20)
        y2 = readDouble(input);
      else
        y2 = readU16(input);

      points.push_back(std::pair<double, double>(x2, y2));
      flag = readU8(input);
      blockBytesRead += input->tell() - inputPos;
    }

    m_geomList->addPolylineTo(m_header.id, m_header.level, x, y, xType,
                              yType, points);
  }
  else
  {
    m_geomList->addLineTo(m_header.id, m_header.level, x, y);
  }
}

void libvisio::VSDXParser::readShapeData(WPXInputStream *input)
{
  unsigned char dataType = readU8(input);

  input->seek(15, WPX_SEEK_CUR);
  // Polyline data
  if (dataType == 0x80)
  {
    unsigned pointCount = 0;
    std::vector<std::pair<double, double> > points;
    unsigned xType = readU8(input);
    unsigned yType = readU8(input);
    pointCount = readU32(input);

    for (unsigned i = 0; i < pointCount; i++)
    {
      double x = 0; double y = 0;
      x = readDouble(input);
      y = readDouble(input);
      points.push_back(std::pair<double, double>(x, y));
    }

    m_collector->collectShapeData(m_header.id, m_header.level, xType, yType, points);
  }

  // NURBS data
  else if (dataType == 0x82)
  {
    double lastKnot = readDouble(input);

    unsigned degree = readU16(input);
    unsigned xType = readU8(input);
    unsigned yType = readU8(input);
    unsigned pointCount = readU32(input);

    std::vector<double> knotVector;
    std::vector<std::pair<double, double> > controlPoints;
    std::vector<double> weights;

    for (unsigned i = 0; i < pointCount; i++)
    {
      double controlX = readDouble(input);
      double controlY = readDouble(input);
      double knot = readDouble(input);
      double weight = readDouble(input);

      knotVector.push_back(knot);
      weights.push_back(weight);
      controlPoints.push_back(std::pair<double, double>(controlX, controlY));
    }
    m_collector->collectShapeData(m_header.id, m_header.level, xType, yType, degree, lastKnot, controlPoints, knotVector, weights);
  }
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

void libvisio::VSDXParser::readFont(WPXInputStream *input, unsigned int fontID)
{
  input->seek(8, WPX_SEEK_CUR);
  std::vector<uint8_t> textStream;

  unsigned int curchar = 0;
  unsigned int nextchar = 0;
  for (unsigned int i = 0; i < 32; i++)
  {
    curchar = readU8(input);
    nextchar = readU8(input);
    if (curchar == 0 && nextchar == 0)
      break;
    textStream.push_back(curchar);
    textStream.push_back(nextchar);
  }
  m_collector->collectFont((unsigned short) fontID, textStream, libvisio::VSD_TEXT_UTF16);
}

void libvisio::VSDXParser::readFontIX(WPXInputStream *input)
{
  input->seek(6, WPX_SEEK_CUR);
  std::vector<uint8_t> textStream;

  unsigned int curchar = 0;
  for (unsigned int i = 0; i < m_header.dataLength - 6; i++)
  {
    curchar = readU8(input);
    if (curchar == 0)
      break;
    textStream.push_back(curchar);
  }
  m_collector->collectFont((unsigned short) m_header.id, textStream, libvisio::VSD_TEXT_ANSI);
}
