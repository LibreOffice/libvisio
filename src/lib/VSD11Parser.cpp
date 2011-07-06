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
#include "VSDXContentCollector.h"
#include "VSDXStylesCollector.h"

libvisio::VSD11Parser::VSD11Parser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : VSDXParser(input, painter)
{}

libvisio::VSD11Parser::~VSD11Parser()
{}

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

void libvisio::VSD11Parser::readNURBSTo(WPXInputStream *input)
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
