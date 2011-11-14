/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* libvisio
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
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
  : m_input(input), m_painter(painter), m_header(), m_collector(0), m_geomList(new VSDXGeometryList()),
    m_geomListVector(), m_fieldList(), m_charList(new VSDXCharacterList()),
    m_paraList(new VSDXParagraphList()), m_charListVector(), m_paraListVector(),
    m_shapeList(), m_currentLevel(0), m_stencils(), m_currentStencil(0),
    m_stencilShape(), m_isStencilStarted(false), m_isInStyles(false), m_currentPageID(0)
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
  if (m_paraList)
  {
    m_paraList->clear();
    delete m_paraList;
  }
  if (m_currentStencil)
    delete m_currentStencil;
}

bool libvisio::VSDXParser::parseMain()
{
  if (!m_input)
  {
    return false;
  }
  // Seek to trailer stream pointer
  m_input->seek(0x24, WPX_SEEK_SET);

  m_input->seek(8, WPX_SEEK_CUR);
  unsigned offset = readU32(m_input);
  unsigned length = readU32(m_input);
  unsigned short format = readU16(m_input);
  bool compressed = ((format & 2) == 2);

  m_input->seek(offset, WPX_SEEK_SET);
  VSDInternalStream trailerStream(m_input, length, compressed);

  std::vector<std::map<unsigned, XForm> > groupXFormsSequence;
  std::vector<std::map<unsigned, unsigned> > groupMembershipsSequence;
  std::vector<std::list<unsigned> > documentPageShapeOrders;

  VSDXStylesCollector stylesCollector(groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders);
  m_collector = &stylesCollector;
  if (!parseDocument(&trailerStream))
    return false;

  VSDXStyles styles = stylesCollector.getStyleSheets();

  VSDXContentCollector contentCollector(m_painter, groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders, styles, m_stencils);
  m_collector = &contentCollector;
  if (!parseDocument(&trailerStream))
    return false;

  return true;
}

bool libvisio::VSDXParser::parseDocument(WPXInputStream *input)
{
  const unsigned SHIFT = 4;

//  unsigned ptrType;
// unsigned ptrOffset;
//  unsigned ptrLength;
//  unsigned ptrFormat;
  std::vector<libvisio::Pointer> PtrList;
  Pointer ptr;

  // Parse out pointers to other streams from trailer
  input->seek(SHIFT, WPX_SEEK_SET);
  unsigned offset = readU32(input);
  input->seek(offset+SHIFT, WPX_SEEK_SET);
  unsigned pointerCount = readU32(input);
  input->seek(SHIFT, WPX_SEEK_CUR);
  for (unsigned i = 0; i < pointerCount; i++)
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
  for (unsigned j = 0; j < PtrList.size(); j++)
  {
    ptr = PtrList[j];
    bool compressed = ((ptr.Format & 2) == 2);
    m_input->seek(ptr.Offset, WPX_SEEK_SET);
    VSDInternalStream tmpInput(m_input, ptr.Length, compressed);
    unsigned shift = compressed ? 4 : 0;
    switch (ptr.Type)
    {
    case VSD_PAGE:           // shouldn't happen
    case VSD_FONT_LIST:      // ver6 stream contains chunk 0x18 (FontList) and chunks 0x19 (Font)
      handlePage(&tmpInput);
      break;
    case VSD_PAGES:
    case VSD_FONTFACES:      // ver11 stream contains streams 0xd7 (FontFace)
      handlePages(&tmpInput, shift);
      break;
    case VSD_COLORS:
      readColours(&tmpInput);
      break;
    case VSD_STYLES:
      handleStyles(&tmpInput);
      break;
    case VSD_STENCILS:
      handleStencils(&tmpInput, shift);
      break;
    default:
      break;
    }
  }

  return true;
}

void libvisio::VSDXParser::handlePages(WPXInputStream *input, unsigned shift)
{
  unsigned ptrType;
  unsigned ptrOffset;
  unsigned ptrLength;
  unsigned ptrFormat;

  input->seek(shift, WPX_SEEK_CUR);
  unsigned offset = readU32(input);
  input->seek(offset+shift, WPX_SEEK_SET);
  unsigned pointerCount = readU32(input);
  input->seek(4, WPX_SEEK_CUR); // Ignore 0x0 dword

  for (unsigned i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(input);
    ptrLength = readU32(input);
    ptrFormat = readU16(input);

    bool compressed = ((ptrFormat & 2) == 2);
    m_input->seek(ptrOffset, WPX_SEEK_SET);
    VSDInternalStream tmpInput(m_input, ptrLength, compressed);
    m_currentPageID = 0;
    switch (ptrType)
    {
    case VSD_PAGE:
      m_currentPageID = i;
      handlePage(&tmpInput);
      break;
    case VSD_PAGES:             // shouldn't happen
      handlePages(&tmpInput, shift);
      break;
    case VSD_COLORS:            // shouldn't happen
      readColours(&tmpInput);
      break;
    case VSD_FONTFACE:          // substreams of FONTAFACES stream, ver 11 only
      readFont(&tmpInput, i);
      break;
    default:
      break;
    }
  }
  m_collector->endPages();
}

void libvisio::VSDXParser::handleStyles(WPXInputStream *input)
{
  m_isInStyles = true;

  try
  {
    long endPos = 0;

    while (!input->atEOS())
    {
      getChunkHeader(input);
      endPos = m_header.dataLength+m_header.trailer+input->tell();

      _handleLevelChange(m_header.level);
      switch (m_header.chunkType)
      {
      case VSD_STYLE_SHEET:
        readStyleSheet(input);
        break;
      case VSD_LINE:
        readLine(input);
        break;
      case VSD_FILL_AND_SHADOW:
        readFillAndShadow(input);
        break;
      case VSD_CHAR_IX:
        readCharIX(input);
        break;
      case VSD_PARA_IX:
        readParaIX(input);
        break;
      case VSD_TEXT_BLOCK:
        readTextBlock(input);
        break;
      default:
        m_collector->collectUnhandledChunk(m_header.id, m_header.level);
      }

      input->seek(endPos, WPX_SEEK_SET);
    }
    _handleLevelChange(0);
  }
  catch (EndOfStreamException)
  {
    _handleLevelChange(0);
  }

  m_isInStyles = false;
}

void libvisio::VSDXParser::handleStencils(WPXInputStream *input, unsigned shift)
{
  if (m_stencils.count() > 0) return;
  m_isStencilStarted = true;
  unsigned ptrType;
  unsigned ptrOffset;
  unsigned ptrLength;
  unsigned ptrFormat;

  input->seek(shift, WPX_SEEK_CUR);
  unsigned offset = readU32(input);
  input->seek(offset+shift, WPX_SEEK_SET);
  unsigned pointerCount = readU32(input);
  input->seek(4, WPX_SEEK_CUR); // Ignore 0x0 dword
  for (unsigned i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(input);
    ptrLength = readU32(input);
    ptrFormat = readU16(input);

    bool compressed = ((ptrFormat & 2) == 2);
    m_input->seek(ptrOffset, WPX_SEEK_SET);
    VSDInternalStream tmpInput(m_input, ptrLength, compressed);
    unsigned shift2 = compressed ? 4 : 0;
    switch (ptrType)
    {
    case VSD_STENCIL_PAGE:
      {
        VSDXStencil tmpStencil;
        m_currentStencil = &tmpStencil;
        handleStencilPage(&tmpInput, shift2);
        m_stencils.addStencil(i, *m_currentStencil);
        m_currentStencil = 0;
      }
      break;
    default:
      break;
    }
  }
  m_isStencilStarted = false;
}

void libvisio::VSDXParser::handleStencilPage(WPXInputStream *input, unsigned shift)
{
  unsigned ptrType;
  unsigned ptrOffset;
  unsigned ptrLength;
  unsigned ptrFormat;

  input->seek(shift, WPX_SEEK_CUR);
  unsigned offset = readU32(input);
  input->seek(offset+shift, WPX_SEEK_SET);
  unsigned pointerCount = readU32(input);
  input->seek(4, WPX_SEEK_CUR); // Ignore 0x0 dword

  for (unsigned i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(input);
    ptrLength = readU32(input);
    ptrFormat = readU16(input);

    bool compressed = ((ptrFormat & 2) == 2);
    m_input->seek(ptrOffset, WPX_SEEK_SET);
    VSDInternalStream tmpInput(m_input, ptrLength, compressed);

    shift = compressed ? 4 : 0;

    switch (ptrType)
    {
    case VSD_SHAPE_FOREIGN:
      m_stencilShape = VSDXStencilShape();
      m_stencilShape.m_foreign = new ForeignData();
      handleStencilForeign(&tmpInput, shift);
      m_currentStencil->addStencilShape(i, m_stencilShape);
      break;
    case VSD_SHAPE_GROUP:
    case VSD_SHAPE_GUIDE:
    case VSD_SHAPE_SHAPE:
      m_stencilShape = VSDXStencilShape();
      handleStencilShape(&tmpInput);
      m_currentStencil->addStencilShape(i, m_stencilShape);
      break;
    default:
      break;
    }
  }
}

void libvisio::VSDXParser::handleStencilForeign(WPXInputStream *input, unsigned shift)
{
  unsigned ptrType;
  unsigned ptrOffset;
  unsigned ptrLength;
  unsigned ptrFormat;

  input->seek(shift, WPX_SEEK_CUR);
  unsigned offset = readU32(input);
  input->seek(offset+shift, WPX_SEEK_SET);
  unsigned pointerCount = readU32(input);
  input->seek(4, WPX_SEEK_CUR); // Ignore 0x0 dword

  for (unsigned i = 0; i < pointerCount; i++)
  {
    ptrType = readU32(input);
    input->seek(4, WPX_SEEK_CUR); // Skip dword
    ptrOffset = readU32(input);
    ptrLength = readU32(input);
    ptrFormat = readU16(input);

    bool compressed = ((ptrFormat & 2) == 2);
    m_input->seek(ptrOffset, WPX_SEEK_SET);
    VSDInternalStream tmpInput(m_input, ptrLength, compressed);

    VSD_DEBUG_MSG(("Stencil foreign stream %x\n", ptrType));

    if (ptrType == VSD_PROP_LIST)
    {
      shift = compressed ? 4 : 0;
      tmpInput.seek(shift, WPX_SEEK_CUR);
      offset = readU32(&tmpInput);
      tmpInput.seek(offset+shift, WPX_SEEK_SET);
      unsigned pointerCount2 = readU32(&tmpInput);
      tmpInput.seek(4, WPX_SEEK_CUR); // Ignore 0x0 dword

      for (unsigned j = 0; j < pointerCount2; j++)
      {
        ptrType = readU32(&tmpInput);
        tmpInput.seek(4, WPX_SEEK_CUR); // Skip dword
        ptrOffset = readU32(&tmpInput);
        ptrLength = readU32(&tmpInput);
        ptrFormat = readU16(&tmpInput);

        compressed = ((ptrFormat & 2) == 2);
        m_input->seek(ptrOffset, WPX_SEEK_SET);
        VSDInternalStream tmpInput2(m_input, ptrLength, compressed);
        if (ptrType == VSD_FOREIGN_DATA_TYPE)
        {
          tmpInput2.seek(0x4, WPX_SEEK_CUR);
          readForeignDataType(&tmpInput2);
        }
      }
    }
    else if (ptrType == VSD_FOREIGN_DATA)
    {
      unsigned foreignLength = ptrLength - 4;
      if (compressed)
        foreignLength = readU32(&tmpInput);
      else
        tmpInput.seek(0x4, WPX_SEEK_CUR);

      unsigned long tmpBytesRead = 0;
      const unsigned char *buffer = tmpInput.read(foreignLength, tmpBytesRead);
      if (foreignLength == tmpBytesRead)
      {
        WPXBinaryData binaryData(buffer, tmpBytesRead);
        m_stencilShape.m_foreign->dataId = m_header.id;
        m_stencilShape.m_foreign->dataLevel = m_header.level;
        m_stencilShape.m_foreign->data = binaryData;
      }
    }
  }
}

void libvisio::VSDXParser::handleStencilShape(WPXInputStream *input)
{
  try
  {
    long endPos = 0;

    while (!input->atEOS())
    {
      getChunkHeader(input);
      endPos = m_header.dataLength+m_header.trailer+input->tell();

      _handleLevelChange(m_header.level);
      VSD_DEBUG_MSG(("Stencil: parsing chunk type %x\n", m_header.chunkType));
      switch (m_header.chunkType)
      {
      case VSD_SHAPE_GROUP:
      case VSD_SHAPE_GUIDE:
      case VSD_SHAPE_SHAPE:
        readShape(input);
        break;
      case VSD_GEOM_LIST:
        m_stencilShape.m_geometries.push_back(VSDXGeometryList());
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
      case VSD_LINE:
        readLine(input);
        break;
      case VSD_NURBS_TO:
        readNURBSTo(input);
        break;
      case VSD_POLYLINE_TO:
        readPolylineTo(input);
        break;
      case VSD_INFINITE_LINE:
        readInfiniteLine(input);
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
      case VSD_FILL_AND_SHADOW:
        readFillAndShadow(input);
        break;
      case VSD_PAGE_PROPS:
        readPageProps(input);
        break;
      case VSD_CHAR_LIST:
        readCharList(input);
        break;
      case VSD_PARA_LIST:
        readParaList(input);
        break;
      case VSD_TEXT:
        readText(input);
        break;
      case VSD_CHAR_IX:
        readCharIX(input);
        break;
      case VSD_PARA_IX:
        readParaIX(input);
        break;
      case VSD_TEXT_BLOCK:
        readTextBlock(input);
        break;
      case VSD_SPLINE_START:
        readSplineStart(input);
        break;
      case VSD_SPLINE_KNOT:
        readSplineKnot(input);
        break;
      case VSD_NAME_LIST:
        readNameList(input);
        break;
      case VSD_NAME:
        readName(input);
        break;
      case VSD_FIELD_LIST:
        readFieldList(input);
        break;
      case VSD_TEXT_FIELD:
        readTextField(input);
        break;
      default:
        m_collector->collectUnhandledChunk(m_header.id, m_header.level);
      }

      input->seek(endPos, WPX_SEEK_SET);
    }
    _handleLevelChange(0);
  }
  catch (EndOfStreamException)
  {
    _handleLevelChange(0);
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
    m_paraListVector.push_back(m_paraList);
    // reinitialize, but don't clear, because we want those pointers to be valid until we handle the whole vector
    m_geomList = new VSDXGeometryList();
    m_charList = new VSDXCharacterList();
    m_paraList = new VSDXParagraphList();
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
    for (std::vector<VSDXCharacterList *>::iterator iter2 = m_charListVector.begin(); iter2 != m_charListVector.end(); iter2++)
    {
      (*iter2)->handle(m_collector);
      (*iter2)->clear();
      delete *iter2;
    }
    m_charListVector.clear();
    for (std::vector<VSDXParagraphList *>::iterator iter3 = m_paraListVector.begin(); iter3 != m_paraListVector.end(); iter3++)
    {
      (*iter3)->handle(m_collector);
      (*iter3)->clear();
      delete *iter3;
    }
    m_paraListVector.clear();
    if (!m_fieldList.empty())
    {
      m_fieldList.handle(m_collector);
      m_fieldList.clear();
    }
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
      case VSD_SHAPE_GUIDE:
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
      case VSD_INFINITE_LINE:
        readInfiniteLine(input);
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
      case VSD_PARA_LIST:
        readParaList(input);
        break;
      case VSD_TEXT:
        readText(input);
        break;
      case VSD_CHAR_IX:
        readCharIX(input);
        break;
      case VSD_PARA_IX:
        readParaIX(input);
        break;
      case VSD_TEXT_BLOCK:
        readTextBlock(input);
        break;
//    case VSD_FONT_LIST: // ver 6 only, don't need to handle that
      case VSD_FONT_IX: // ver 6 only
        readFontIX(input);
        break;
      case VSD_PAGE:
        readPage(input);
        break;
      case VSD_SPLINE_START:
        readSplineStart(input);
        break;
      case VSD_SPLINE_KNOT:
        readSplineKnot(input);
        break;
      case VSD_NAME_LIST:
        readNameList(input);
        break;
      case VSD_NAME:
        readName(input);
        break;
      case VSD_FIELD_LIST:
        readFieldList(input);
        break;
      case VSD_TEXT_FIELD:
        readTextField(input);
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

  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().addEllipticalArcTo(m_header.id, m_header.level, x3, y3, x2, y2, angle, ecc);
  else
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

  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().addEllipse(m_header.id, m_header.level, cx, cy, xleft, yleft, xtop, ytop);
  else
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
  unsigned char linePattern = readU8(input);
  input->seek(10, WPX_SEEK_CUR);
  unsigned char startMarker = readU8(input);
  unsigned char endMarker = readU8(input);
  unsigned char lineCap = readU8(input);

  if (m_isInStyles)
    m_collector->collectLineStyle(m_header.id, m_header.level, strokeWidth, c, linePattern, startMarker, endMarker, lineCap);
  else if (m_isStencilStarted)
  {
    if (!m_stencilShape.m_lineStyle)
      m_stencilShape.m_lineStyle = new VSDXLineStyle(strokeWidth, c, linePattern, startMarker, endMarker, lineCap);
  }
  else
    m_collector->collectLine(m_header.id, m_header.level, strokeWidth, c, linePattern, startMarker, endMarker, lineCap);
}

void libvisio::VSDXParser::readTextBlock(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double leftMargin = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double rightMargin = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double topMargin = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double bottomMargin = readDouble(input);
  unsigned char verticalAlign = readU8(input);
  unsigned char bgClrId = readU8(input);
  Colour c;
  c.r = readU8(input);
  c.g = readU8(input);
  c.b = readU8(input);
  c.a = readU8(input);
  input->seek(1, WPX_SEEK_CUR);
  double defaultTabStop = readDouble(input);
  input->seek(12, WPX_SEEK_CUR);
  unsigned char textDirection = readU8(input);

  if (m_isInStyles)
    m_collector->collectTextBlockStyle(m_header.id, m_header.level, leftMargin, rightMargin, topMargin, bottomMargin,
                                       verticalAlign, bgClrId, c, defaultTabStop, textDirection);
  else if (m_isStencilStarted)
  {
    if (!m_stencilShape.m_textBlockStyle)
      m_stencilShape.m_textBlockStyle = new VSDXTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin,
          verticalAlign, bgClrId, c, defaultTabStop, textDirection);
  }
  else
    m_collector->collectTextBlock(m_header.id, m_header.level, leftMargin, rightMargin, topMargin, bottomMargin,
                                  verticalAlign, bgClrId, c, defaultTabStop, textDirection);
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

  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().setElementsOrder(geometryOrder);
  else
  {
    m_geomList->setElementsOrder(geometryOrder);
    // We want the collectors to still get the level information
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  }
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
  m_collector->collectUnhandledChunk(m_header.id, m_header.level);
}

void libvisio::VSDXParser::readParaList(WPXInputStream *input)
{
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  std::vector<unsigned> paragraphOrder;
  paragraphOrder.reserve(childrenListLength / sizeof(uint32_t));
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    paragraphOrder.push_back(readU32(input));

  m_paraList->setElementsOrder(paragraphOrder);
  // We want the collectors to still get the level information
  m_collector->collectUnhandledChunk(m_header.id, m_header.level);
}

void libvisio::VSDXParser::readPage(WPXInputStream *input)
{
  input->seek(8, WPX_SEEK_CUR); //sub header length and children list length
  uint32_t backgroundPageID = readU32(input);
  m_collector->collectPage(m_header.id, m_header.level, backgroundPageID, m_currentPageID);
}

void libvisio::VSDXParser::readGeometry(WPXInputStream *input)
{
  unsigned char geomFlags = readU8(input);

  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().addGeometry(m_header.id, m_header.level, geomFlags);
  else
    m_geomList->addGeometry(m_header.id, m_header.level, geomFlags);
}

void libvisio::VSDXParser::readMoveTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().addMoveTo(m_header.id, m_header.level, x, y);
  else
    m_geomList->addMoveTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDXParser::readLineTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().addLineTo(m_header.id, m_header.level, x, y);
  else
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

  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().addArcTo(m_header.id, m_header.level, x2, y2, bow);
  else
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
  m_collector->collectUnhandledChunk(m_header.id, m_header.level);
}

void libvisio::VSDXParser::readForeignDataType(WPXInputStream *input)
{
  input->seek(0x24, WPX_SEEK_CUR);
  unsigned foreignType = readU16(input);
  input->seek(0xb, WPX_SEEK_CUR);
  unsigned foreignFormat = readU32(input);

  if (m_isStencilStarted)
  {
    m_stencilShape.m_foreign->typeId = m_header.id;
    m_stencilShape.m_foreign->typeLevel = m_header.level;
    m_stencilShape.m_foreign->type = foreignType;
    m_stencilShape.m_foreign->format = foreignFormat;
  }
  else
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
  double scale = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  scale /= readDouble(input);

  if (m_isStencilStarted)
  {
    m_currentStencil->m_shadowOffsetX = shadowOffsetX;
    m_currentStencil->m_shadowOffsetY = shadowOffsetY;
  }
  else
    m_collector->collectPageProps(m_header.id, m_header.level, pageWidth, pageHeight, shadowOffsetX, shadowOffsetY, scale);
}

void libvisio::VSDXParser::readShape(WPXInputStream *input)
{
  input->seek(0x12, WPX_SEEK_CUR);
  unsigned masterPage = readU32(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned masterShape = readU32(input);
  input->seek(0x4, WPX_SEEK_CUR);
  unsigned fillStyle = readU32(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned lineStyle = readU32(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned textStyle = readU32(input);

  if (m_isStencilStarted)
  {
    m_stencilShape.m_lineStyleId = lineStyle;
    m_stencilShape.m_fillStyleId = fillStyle;
    m_stencilShape.m_textStyleId = textStyle;
  }
  else
    m_collector->collectShape(m_header.id, m_header.level, masterPage, masterShape, lineStyle, fillStyle, textStyle);
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
  unsigned char useData = readU8(input);
  if (useData == 0x8a)
  {
    input->seek(3, WPX_SEEK_CUR);
    unsigned dataId = readU32(input);

    if (m_isStencilStarted)
      m_stencilShape.m_geometries.back().addNURBSTo(m_header.id, m_header.level, x, y, knot, knotPrev, weight, weightPrev, dataId);
    else
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
  unsigned char xType = 1;
  unsigned char yType = 1;
  // Only read formula if block is found
  if (cellRef == 6)
  {
    // Indicates whether it's a "simple" NURBS block with a static format
    // or a complex block where parameters each have a type
    unsigned char paramType = readU8(input);
    unsigned char valueType = 0;

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
    unsigned char flag = 0;
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

    if (m_isStencilStarted)
      m_stencilShape.m_geometries.back().addNURBSTo(m_header.id, m_header.level, x, y, xType, yType, degree, controlPoints, knotVector, weights);
    else
      m_geomList->addNURBSTo(m_header.id, m_header.level, x, y, xType,
                             yType, degree, controlPoints, knotVector, weights);
  }
  else // No formula found, use line
  {
    if (m_isStencilStarted)
      m_stencilShape.m_geometries.back().addLineTo(m_header.id, m_header.level, x,  y);
    else
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

    if (m_isStencilStarted)
      m_stencilShape.m_geometries.back().addPolylineTo(m_header.id, m_header.level, x, y, dataId);
    else
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
  unsigned char xType = 1;
  unsigned char yType = 1;
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
      double x2 = 0;
      double y2 = 0;

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

    if (m_isStencilStarted)
      m_stencilShape.m_geometries.back().addPolylineTo(m_header.id, m_header.level, x, y, xType, yType, points);
    else
      m_geomList->addPolylineTo(m_header.id, m_header.level, x, y, xType,
                                yType, points);
  }
  else
  {
    if (m_isStencilStarted)
      m_stencilShape.m_geometries.back().addLineTo(m_header.id, m_header.level, x, y);
    else
      m_geomList->addLineTo(m_header.id, m_header.level, x, y);
  }
}

void libvisio::VSDXParser::readInfiniteLine(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x1 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y1 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y2 = readDouble(input);
  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().addInfiniteLine(m_header.id, m_header.level, x1, y1, x2, y2);
  else
    m_geomList->addInfiniteLine(m_header.id, m_header.level, x1, y1, x2, y2);
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
    unsigned char xType = readU8(input);
    unsigned char yType = readU8(input);
    pointCount = readU32(input);

    for (unsigned i = 0; i < pointCount; i++)
    {
      double x = 0;
      double y = 0;
      x = readDouble(input);
      y = readDouble(input);
      points.push_back(std::pair<double, double>(x, y));
    }

    if (m_isStencilStarted)
    {
      PolylineData data;
      data.xType = xType;
      data.yType = yType;
      data.points = points;
      m_stencilShape.m_polylineData[m_header.id] = data;
    }
    else
      m_collector->collectShapeData(m_header.id, m_header.level, xType, yType, points);
  }

  // NURBS data
  else if (dataType == 0x82)
  {
    double lastKnot = readDouble(input);

    unsigned degree = readU16(input);
    unsigned char xType = readU8(input);
    unsigned char yType = readU8(input);
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
    if (m_isStencilStarted)
    {
      NURBSData data;
      data.lastKnot = lastKnot;
      data.degree = degree;
      data.xType = xType;
      data.yType = yType;
      data.knots = knotVector;
      data.weights = weights;
      data.points = controlPoints;
      m_stencilShape.m_nurbsData[m_header.id] = data;
    }
    else
      m_collector->collectShapeData(m_header.id, m_header.level, xType, yType, degree, lastKnot, controlPoints, knotVector, weights);
  }
}

void libvisio::VSDXParser::readSplineStart(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);
  double secondKnot = readDouble(input);
  double firstKnot = readDouble(input);
  double lastKnot = readDouble(input);
  unsigned degree = readU8(input);

  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().addSplineStart(m_header.id, m_header.level, x, y, secondKnot, firstKnot, lastKnot, degree);
  else
    m_geomList->addSplineStart(m_header.id, m_header.level, x, y, secondKnot, firstKnot, lastKnot, degree);
}

void libvisio::VSDXParser::readSplineKnot(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);
  double knot = readDouble(input);

  if (m_isStencilStarted)
    m_stencilShape.m_geometries.back().addSplineKnot(m_header.id, m_header.level, x, y, knot);
  else
    m_geomList->addSplineKnot(m_header.id, m_header.level, x, y, knot);
}

void libvisio::VSDXParser::readParaIX(WPXInputStream *input)
{
  unsigned charCount = readU32(input);
  input->seek(1, WPX_SEEK_CUR);
  double indFirst = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double indLeft = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double indRight = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double spLine = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double spBefore = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double spAfter = readDouble(input);
  unsigned char align = readU8(input);

  if (m_isInStyles)
    m_collector->collectParaIXStyle(m_header.id, m_header.level, charCount, indFirst, indLeft, indRight,
                                    spLine, spBefore, spAfter, align);
  else if (m_isStencilStarted)
  {
    VSD_DEBUG_MSG(("Found stencil paragraph style\n"));
    if (!m_stencilShape.m_paraStyle)
      m_stencilShape.m_paraStyle= new VSDXParaStyle(charCount, indFirst, indLeft, indRight,
          spLine, spBefore, spAfter, align);
  }
  else
    m_paraList->addParaIX(m_header.id, m_header.level, charCount, indFirst, indLeft, indRight,
                          spLine, spBefore, spAfter, align);
}


void libvisio::VSDXParser::readNameList(WPXInputStream * /* input */)
{
  if (m_isStencilStarted)
    m_stencilShape.m_names.clear();
  else
    m_collector->collectNameList(m_header.id, m_header.level);
}

void libvisio::VSDXParser::readFieldList(WPXInputStream *input)
{
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  std::vector<unsigned> fieldOrder;
  fieldOrder.reserve(childrenListLength / sizeof(uint32_t));
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    fieldOrder.push_back(readU32(input));

  if (m_isStencilStarted)
  {
    m_stencilShape.m_fields.clear();
    m_stencilShape.m_fields.setElementsOrder(fieldOrder);
  }
  else
  {
    m_fieldList.setElementsOrder(fieldOrder);
    m_fieldList.addFieldList(m_header.id, m_header.level);
    // We want the collectors to still get the level information
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  }
}

void libvisio::VSDXParser::readTextField(WPXInputStream *input)
{
  unsigned long initialPosition = input->tell();
  input->seek(7, WPX_SEEK_CUR);
  unsigned char tmpCode = readU8(input);
  if (tmpCode == 0xe8)
  {
    int nameId = (int)readU32(input);
    input->seek(6, WPX_SEEK_CUR);
    if (m_isStencilStarted)
      m_stencilShape.m_fields.addTextField(m_header.id, m_header.level, nameId);
    else
      m_fieldList.addTextField(m_header.id, m_header.level, nameId);
  }
  else
  {
    double numericValue = readDouble(input);
    input->seek(2, WPX_SEEK_CUR);

    unsigned blockIdx = 0;
    unsigned length = 0;
    unsigned short formatNumber = 0;
    input->seek(initialPosition+0x36, WPX_SEEK_SET);
    while (blockIdx != 2 && !input->atEOS() && (unsigned long) input->tell() < (unsigned long)(initialPosition+m_header.dataLength+m_header.trailer))
    {
      unsigned long inputPos = input->tell();
      length = readU32(input);
      if (!length)
        break;
      input->seek(1, WPX_SEEK_CUR);
      blockIdx = readU8(input);
      if (blockIdx != 2)
        input->seek(inputPos + length, WPX_SEEK_SET);
      else
      {
        input->seek(1, WPX_SEEK_CUR);
        formatNumber = readU16(input);
        if (0x80 != readU8(input))
        {
          input->seek(inputPos + length, WPX_SEEK_SET);
          blockIdx = 0;
        }
        else
        {
          if (0xc2 != readU8(input))
          {
            input->seek(inputPos + length, WPX_SEEK_SET);
            blockIdx = 0;
          }
          else
            break;
        }
      }
    }

    if (input->atEOS())
      return;

    if (blockIdx != 2)
    {
      if (tmpCode == 0x28)
        formatNumber = 200;
      else
        formatNumber = 0xffff;
    }

    printf("Fridrich is a good guy 0x%.4x\n", formatNumber);

    if (m_isStencilStarted)
      m_stencilShape.m_fields.addNumericField(m_header.id, m_header.level, formatNumber, numericValue);
    else
      m_fieldList.addNumericField(m_header.id, m_header.level, formatNumber, numericValue);
  }
}

void libvisio::VSDXParser::readColours(WPXInputStream *input)
{
  input->seek(6, WPX_SEEK_SET);
  unsigned numColours = readU8(input);
  Colour tmpColour;

  input->seek(1, WPX_SEEK_CUR);

  std::vector<Colour> colours;

  for (unsigned i = 0; i < numColours; i++)
  {
    tmpColour.r = readU8(input);
    tmpColour.g = readU8(input);
    tmpColour.b = readU8(input);
    tmpColour.a = readU8(input);

    colours.push_back(tmpColour);
  }
  m_collector->collectColours(colours);
}

void libvisio::VSDXParser::readFont(WPXInputStream *input, unsigned fontID)
{
  input->seek(8, WPX_SEEK_CUR);
  ::WPXBinaryData textStream;

  for (unsigned i = 0; i < 32; i++)
  {
    unsigned char curchar = readU8(input);
    unsigned char nextchar = readU8(input);
    if (curchar == 0 && nextchar == 0)
      break;
    textStream.append(curchar);
    textStream.append(nextchar);
  }
  m_collector->collectFont((unsigned short) fontID, textStream, libvisio::VSD_TEXT_UTF16);
}

void libvisio::VSDXParser::readFontIX(WPXInputStream *input)
{
  input->seek(6, WPX_SEEK_CUR);
  ::WPXBinaryData textStream;

  for (unsigned i = 0; i < m_header.dataLength - 6; i++)
  {
    unsigned char curchar = readU8(input);
    if (curchar == 0)
      break;
    textStream.append(curchar);
  }
  m_collector->collectFont((unsigned short) m_header.id, textStream, libvisio::VSD_TEXT_ANSI);
}

/* StyleSheet readers */

void libvisio::VSDXParser::readStyleSheet(WPXInputStream *input)
{
  input->seek(0x22, WPX_SEEK_CUR);
  unsigned lineStyle = readU32(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned fillStyle = readU32(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned textStyle = readU32(input);

  m_collector->collectStyleSheet(m_header.id, m_header.level, lineStyle, fillStyle, textStyle);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
