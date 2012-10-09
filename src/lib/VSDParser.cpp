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
#include "VSDParser.h"
#include "VSDInternalStream.h"
#include "VSDDocumentStructure.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"

libvisio::VSDParser::VSDParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : m_input(input), m_painter(painter), m_header(), m_collector(0), m_geomList(),
    m_geomListVector(), m_fieldList(), m_charList(), m_paraList(), m_charListVector(),
    m_paraListVector(), m_shapeList(), m_currentLevel(0), m_stencils(), m_currentStencil(0),
    m_shape(), m_isStencilStarted(false), m_isInStyles(false), m_currentShapeLevel(0),
    m_currentShapeID((unsigned)-1), m_extractStencils(false), m_colours(), m_isBackgroundPage(false)
{}

libvisio::VSDParser::~VSDParser()
{
}

bool libvisio::VSDParser::parseMain()
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

  VSDStylesCollector stylesCollector(groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders);
  m_collector = &stylesCollector;
  m_stencils.clear();
  if (!parseDocument(&trailerStream))
    return false;

  VSDStyles styles = stylesCollector.getStyleSheets();

  VSDContentCollector contentCollector(m_painter, groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders, styles, m_stencils);
  m_collector = &contentCollector;
  if (!parseDocument(&trailerStream))
    return false;

  return true;
}

bool libvisio::VSDParser::parseDocument(WPXInputStream *input)
{
  try
  {
    handleStreams(input, 4, 0);
    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool libvisio::VSDParser::extractStencils()
{
  m_extractStencils = true;
  return parseMain();
}

void libvisio::VSDParser::handleStreams(WPXInputStream *input, unsigned shift, unsigned level)
{
  std::vector<unsigned> pointerOrder;
  std::map<unsigned, libvisio::Pointer> PtrList;
  std::map<unsigned, libvisio::Pointer> FontFaces;
  unsigned i = 0;

  try
  {
    // Parse out pointers to streams
    input->seek(shift, WPX_SEEK_SET);
    unsigned offset = readU32(input);
    input->seek(offset+shift-4, WPX_SEEK_SET);
    unsigned listSize = readU32(input);
    unsigned pointerCount = readU32(input);
    input->seek(4, WPX_SEEK_CUR);
    for (i = 0; i < pointerCount; i++)
    {
      Pointer ptr;
      ptr.Type = readU32(input);
      input->seek(4, WPX_SEEK_CUR); // Skip dword
      ptr.Offset = readU32(input);
      ptr.Length = readU32(input);
      ptr.Format = readU16(input);
      if (ptr.Type == VSD_FONTFACES)
        FontFaces[i] = ptr;
      else if (ptr.Type != 0)
        PtrList[i] = ptr;
    }
    for (i = 0; i < listSize; ++i)
      pointerOrder.push_back(readU32(input));
  }
  catch (const EndOfStreamException &)
  {
    pointerOrder.clear();
    PtrList.clear();
    FontFaces.clear();
  }

  std::map<unsigned, libvisio::Pointer>::iterator iter;
  for (iter = FontFaces.begin(); iter != FontFaces.end(); ++iter)
    handleStream(iter->second, iter->first, level+1);

  if (!pointerOrder.empty())
  {
    for (i=0; i < pointerOrder.size(); ++i)
    {
      iter = PtrList.find(pointerOrder[i]);
      if (iter != PtrList.end())
      {
        handleStream(iter->second, iter->first, level+1);
        PtrList.erase(iter);
      }
    }
  }
  for (iter = PtrList.begin(); iter != PtrList.end(); ++iter)
    handleStream(iter->second, iter->first, level+1);

}


void libvisio::VSDParser::handleStream(const Pointer &ptr, unsigned idx, unsigned level)
{
  m_header.level = level;
  m_header.id = idx;
  m_header.chunkType = ptr.Type;
  _handleLevelChange(level);
  VSDStencil tmpStencil;
  bool compressed = ((ptr.Format & 2) == 2);
  m_input->seek(ptr.Offset, WPX_SEEK_SET);
  VSDInternalStream tmpInput(m_input, ptr.Length, compressed);
  unsigned shift = compressed ? 4 : 0;

  VSD_DEBUG_MSG(("VSDParser::handleStream: level %i, ptr.Type 0x%.8x, ptr.Offset 0x%.8x, ptr.Length 0x%.8x, ptr.Format 0x%.4x\n",
                 level, ptr.Type, ptr.Offset, ptr.Length, ptr.Format));

  switch (ptr.Type)
  {
  case VSD_STYLES:
    m_isInStyles = true;
    break;
  case VSD_PAGES:
    if (m_extractStencils)
      return;
    break;
  case VSD_PAGE:
    if (m_extractStencils)
      return;
    if (ptr.Format == 0xd2 || ptr.Format == 0xd6)
      m_isBackgroundPage = true;
    else
      m_isBackgroundPage = false;
    m_collector->startPage(idx);
    break;
  case VSD_STENCILS:
    if (m_extractStencils)
      break;
    if (m_stencils.count())
      return;
    m_isStencilStarted = true;
    break;
  case VSD_STENCIL_PAGE:
    if (m_extractStencils)
    {
      m_isBackgroundPage = false;
      m_collector->startPage(idx);
    }
    else
      m_currentStencil = &tmpStencil;
    break;
  case VSD_SHAPE_GROUP:
  case VSD_SHAPE_GUIDE:
  case VSD_SHAPE_SHAPE:
  case VSD_SHAPE_FOREIGN:
    m_currentShapeID = idx;
    break;
  case VSD_OLE_LIST:
    if (m_isStencilStarted)
    {
      if (!m_shape.m_foreign)
        m_shape.m_foreign = new ForeignData();
      m_shape.m_foreign->dataId = idx;
    }
    break;
  default:
    break;
  }

  if ((ptr.Format >> 4) == 0x4 || (ptr.Format >> 4) == 0x5 || (ptr.Format >> 4) == 0x0)
  {
    if (ptr.Length > 4)
      handleBlob(&tmpInput, level+1);
    if ((ptr.Format >> 4) == 0x5 && ptr.Type != VSD_COLORS)
      handleStreams(&tmpInput, shift, level+1);
  }
  else if ((ptr.Format >> 4) == 0xd || (ptr.Format >> 4) == 0x8)
    handleChunks(&tmpInput, level+1);

  switch (ptr.Type)
  {
  case VSD_STYLES:
    _handleLevelChange(0);
    m_isInStyles = false;
    break;
  case VSD_PAGE:
    _handleLevelChange(0);
    m_collector->endPage();
    break;
  case VSD_PAGES:
    m_collector->endPages();
    break;
  case VSD_STENCILS:
    if (m_extractStencils)
      m_collector->endPages();
    else
      m_isStencilStarted = false;
    break;
  case VSD_STENCIL_PAGE:
    if (m_extractStencils)
    {
      _handleLevelChange(0);
      m_collector->endPage();
    }
    else
    {
      m_stencils.addStencil(idx, *m_currentStencil);
      m_currentStencil = 0;
    }
    break;
  case VSD_SHAPE_GROUP:
  case VSD_SHAPE_GUIDE:
  case VSD_SHAPE_SHAPE:
  case VSD_SHAPE_FOREIGN:
    if (m_isStencilStarted)
    {
      _handleLevelChange(0);
      m_currentStencil->addStencilShape(idx, m_shape);
    }
    break;
  default:
    break;
  }

}

void libvisio::VSDParser::handleBlob(WPXInputStream *input, unsigned level)
{
  try
  {
    m_header.level = level;
    m_header.trailer = 0;
    m_header.dataLength = readU32(input);
    _handleLevelChange(m_header.level);
    handleChunk(input);
  }
  catch (EndOfStreamException &)
  {
    VSD_DEBUG_MSG(("VSDParser::handleBlob - catching EndOfStreamException\n"));
  }
}


void libvisio::VSDParser::handleChunks(WPXInputStream *input, unsigned level)
{
  long endPos = 0;

  while (!input->atEOS())
  {
    getChunkHeader(input);
    m_header.level += level;
    endPos = m_header.dataLength+m_header.trailer+input->tell();

    _handleLevelChange(m_header.level);
    VSD_DEBUG_MSG(("Shape: parsing chunk type 0x%x\n", m_header.chunkType));
    handleChunk(input);
    input->seek(endPos, WPX_SEEK_SET);
  }
}

void libvisio::VSDParser::handleChunk(WPXInputStream *input)
{
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
  case VSD_OLE_LIST:
    readOLEList(input);
    break;
  case VSD_OLE_DATA:
    readOLEData(input);
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
  case VSD_FONT_IX: // ver 6 only
    readFontIX(input);
    break;
  case VSD_PAGE:
    readPage(input);
    break;
  case VSD_STENCIL_PAGE:
    if (m_extractStencils)
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
  case VSD_STYLE_SHEET:
    readStyleSheet(input);
    break;
  case VSD_PAGE_SHEET:
    readPageSheet(input);
    break;
  case VSD_COLORS:
    readColours(input);
    break;
  case VSD_FONTFACE: // substreams of FONTAFACES stream, ver 11 only
    readFont(input);
    break;
  default:
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  }
}

void libvisio::VSDParser::_flushShape(const libvisio::VSDShape &shape)
{
  m_collector->collectXFormData(m_currentShapeLevel+1, shape.m_xform);
  if (shape.m_txtxform)
    m_collector->collectTxtXForm(m_currentShapeLevel+1, *(shape.m_txtxform));
  if (shape.m_lineStyle)
    m_collector->collectLine(m_currentShapeLevel+1, shape.m_lineStyle->width, shape.m_lineStyle->colour, shape.m_lineStyle->pattern,
                             shape.m_lineStyle->startMarker, shape.m_lineStyle->endMarker, shape.m_lineStyle->cap);
  if (shape.m_fillStyle)
    m_collector->collectFillAndShadow(m_currentShapeLevel+1, m_shape.m_fillStyle->fgColour, m_shape.m_fillStyle->bgColour, m_shape.m_fillStyle->pattern,
                                      m_shape.m_fillStyle->fgTransparency, m_shape.m_fillStyle->bgTransparency, m_shape.m_fillStyle->shadowPattern,
                                      m_shape.m_fillStyle->shadowFgColour, m_shape.m_fillStyle->shadowOffsetX, m_shape.m_fillStyle->shadowOffsetY);
  if (shape.m_textBlockStyle)
    m_collector->collectTextBlock(m_currentShapeLevel+1, m_shape.m_textBlockStyle->leftMargin, m_shape.m_textBlockStyle->rightMargin,
                                  m_shape.m_textBlockStyle->topMargin, m_shape.m_textBlockStyle->bottomMargin, m_shape.m_textBlockStyle->verticalAlign,
                                  m_shape.m_textBlockStyle->isTextBkgndFilled, m_shape.m_textBlockStyle->textBkgndColour,
                                  m_shape.m_textBlockStyle->defaultTabStop, m_shape.m_textBlockStyle->textDirection);

  if (m_shape.m_foreign)
  {
    m_collector->collectForeignDataType(m_currentShapeLevel+1, m_shape.m_foreign->type, m_shape.m_foreign->format,
                                        m_shape.m_foreign->offsetX, m_shape.m_foreign->offsetY, m_shape.m_foreign->width, m_shape.m_foreign->height);
    m_collector->collectForeignData(m_currentShapeLevel+1, m_shape.m_foreign->data);
  }


  for (std::map<unsigned, NURBSData>::const_iterator iterNurbs = shape.m_nurbsData.begin(); iterNurbs != shape.m_nurbsData.end(); ++iterNurbs)
    m_collector->collectShapeData(iterNurbs->first, m_currentShapeLevel+1, iterNurbs->second.xType, iterNurbs->second.yType,
                                  iterNurbs->second.degree, iterNurbs->second.lastKnot, iterNurbs->second.points,
                                  iterNurbs->second.knots, iterNurbs->second.weights);

  for (std::map<unsigned, PolylineData>::const_iterator iterPoly = shape.m_polylineData.begin(); iterPoly != shape.m_polylineData.end(); ++iterPoly)
    m_collector->collectShapeData(iterPoly->first, m_currentShapeLevel+1, iterPoly->second.xType, iterPoly->second.yType, iterPoly->second.points);

  for (std::map<unsigned, VSDName>::const_iterator iterName = shape.m_names.begin(); iterName != shape.m_names.end(); ++iterName)
    m_collector->collectName(iterName->first, m_currentShapeLevel+1, iterName->second.m_data, iterName->second.m_format);


  m_collector->collectText(m_currentShapeLevel+1, shape.m_text, shape.m_textFormat);


  for (std::vector<VSDGeometryList>::const_iterator iterGeom = shape.m_geometries.begin(); iterGeom != shape.m_geometries.end(); ++iterGeom)
    iterGeom->handle(m_collector);

  for (std::vector<VSDCharacterList>::const_iterator iterChar = shape.m_charListVector.begin(); iterChar != shape.m_charListVector.end(); ++iterChar)
    iterChar->handle(m_collector);

  for (std::vector<VSDParagraphList>::const_iterator iterPara = shape.m_paraListVector.begin(); iterPara != shape.m_paraListVector.end(); ++iterPara)
    iterPara->handle(m_collector);

  if (!shape.m_fields.empty())
    shape.m_fields.handle(m_collector);

#if 0
  unsigned m_lineStyleId, m_fillStyleId, m_textStyleId;
  VSDCharStyle *m_charStyle;
  VSDParaStyle *m_paraStyle;
#endif
}

void libvisio::VSDParser::_handleLevelChange(unsigned level)
{
  if (level == m_currentLevel)
    return;
  if (level <= m_currentShapeLevel+1)
  {
    m_geomListVector.push_back(m_geomList);
    m_geomList = VSDGeometryList();
    m_charListVector.push_back(m_charList);
    m_charList = VSDCharacterList();
    m_paraListVector.push_back(m_paraList);
    m_paraList = VSDParagraphList();
    m_shapeList.handle(m_collector);
    m_shapeList.clear();
  }
  if (level <= m_currentShapeLevel)
  {
    for (std::vector<VSDGeometryList>::iterator iter = m_geomListVector.begin(); iter != m_geomListVector.end(); ++iter)
    {
      iter->handle(m_collector);
      iter->clear();
    }
    m_geomListVector.clear();
    for (std::vector<VSDCharacterList>::iterator iterChar = m_charListVector.begin(); iterChar != m_charListVector.end(); ++iterChar)
    {
      iterChar->handle(m_collector);
      iterChar->clear();
    }
    m_charListVector.clear();
    for (std::vector<VSDParagraphList>::iterator iterPara = m_paraListVector.begin(); iterPara != m_paraListVector.end(); ++iterPara)
    {
      iterPara->handle(m_collector);
      iterPara->clear();
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

// --- READERS ---

void libvisio::VSDParser::readEllipticalArcTo(WPXInputStream *input)
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
    m_shape.m_geometries.back().addEllipticalArcTo(m_header.id, m_header.level, x3, y3, x2, y2, angle, ecc);
  else
    m_geomList.addEllipticalArcTo(m_header.id, m_header.level, x3, y3, x2, y2, angle, ecc);
}


void libvisio::VSDParser::readForeignData(WPXInputStream *input)
{
  unsigned long tmpBytesRead = 0;
  const unsigned char *buffer = input->read(m_header.dataLength, tmpBytesRead);
  if (m_header.dataLength != tmpBytesRead)
    return;
  WPXBinaryData binaryData(buffer, tmpBytesRead);

  if (!m_shape.m_foreign)
    m_shape.m_foreign = new ForeignData();
  m_shape.m_foreign->dataId = m_header.id;
  m_shape.m_foreign->dataLevel = m_header.level;
  m_shape.m_foreign->data = binaryData;

  if (!m_isStencilStarted)
    m_collector->collectForeignData(m_header.level, binaryData);
}

void libvisio::VSDParser::readOLEList(WPXInputStream * /* input */)
{
  m_collector->collectOLEList(m_header.id, m_header.level);
}

void libvisio::VSDParser::readOLEData(WPXInputStream *input)
{
  unsigned long tmpBytesRead = 0;
  const unsigned char *buffer = input->read(m_header.dataLength, tmpBytesRead);
  if (m_header.dataLength != tmpBytesRead)
    return;
  WPXBinaryData oleData(buffer, tmpBytesRead);

  if (!m_shape.m_foreign)
    m_shape.m_foreign = new ForeignData();
  // Append data instead of setting it - allows multi-stream OLE objects
  m_shape.m_foreign->data.append(oleData);
  m_shape.m_foreign->dataLevel = m_header.level;

  if (!m_isStencilStarted)
    m_collector->collectOLEData(m_header.id, m_header.level, oleData);
}

void libvisio::VSDParser::readEllipse(WPXInputStream *input)
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
    m_shape.m_geometries.back().addEllipse(m_header.id, m_header.level, cx, cy, xleft, yleft, xtop, ytop);
  else
    m_geomList.addEllipse(m_header.id, m_header.level, cx, cy, xleft, yleft, xtop, ytop);
}

void libvisio::VSDParser::readLine(WPXInputStream *input)
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
    m_collector->collectLineStyle(m_header.level, strokeWidth, c, linePattern, startMarker, endMarker, lineCap);
  else
  {
    if (m_shape.m_lineStyle)
      delete m_shape.m_lineStyle;
    m_shape.m_lineStyle = new VSDLineStyle(strokeWidth, c, linePattern, startMarker, endMarker, lineCap);
    if (!m_isStencilStarted)
      m_collector->collectLine(m_header.level, strokeWidth, c, linePattern, startMarker, endMarker, lineCap);
  }
}

void libvisio::VSDParser::readTextBlock(WPXInputStream *input)
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
  bool isBgFilled = (!!readU8(input));
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
    m_collector->collectTextBlockStyle(m_header.level, leftMargin, rightMargin, topMargin, bottomMargin,
                                       verticalAlign, isBgFilled, c, defaultTabStop, textDirection);
  else
  {
    if (m_shape.m_textBlockStyle)
      delete m_shape.m_textBlockStyle;
    m_shape.m_textBlockStyle = new VSDTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin,
        verticalAlign, isBgFilled, c, defaultTabStop, textDirection);
    if (!m_isStencilStarted)
      m_collector->collectTextBlock(m_header.level, leftMargin, rightMargin, topMargin, bottomMargin,
                                    verticalAlign, isBgFilled, c, defaultTabStop, textDirection);
  }
}

void libvisio::VSDParser::readGeomList(WPXInputStream *input)
{
  if (m_isStencilStarted)
    m_shape.m_geometries.push_back(VSDGeometryList());
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  std::vector<unsigned> geometryOrder;
  geometryOrder.reserve(childrenListLength / sizeof(uint32_t));
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    geometryOrder.push_back(readU32(input));

  if (m_isStencilStarted)
    m_shape.m_geometries.back().setElementsOrder(geometryOrder);
  else
  {
    m_geomList.setElementsOrder(geometryOrder);
    // We want the collectors to still get the level information
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  }
}

void libvisio::VSDParser::readCharList(WPXInputStream *input)
{
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  std::vector<unsigned> characterOrder;
  characterOrder.reserve(childrenListLength / sizeof(uint32_t));
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    characterOrder.push_back(readU32(input));

  m_charList.setElementsOrder(characterOrder);
  // We want the collectors to still get the level information
  m_collector->collectUnhandledChunk(m_header.id, m_header.level);
}

void libvisio::VSDParser::readParaList(WPXInputStream *input)
{
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  std::vector<unsigned> paragraphOrder;
  paragraphOrder.reserve(childrenListLength / sizeof(uint32_t));
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    paragraphOrder.push_back(readU32(input));

  m_paraList.setElementsOrder(paragraphOrder);
  // We want the collectors to still get the level information
  m_collector->collectUnhandledChunk(m_header.id, m_header.level);
}

void libvisio::VSDParser::readPage(WPXInputStream *input)
{
  input->seek(8, WPX_SEEK_CUR); //sub header length and children list length
  uint32_t backgroundPageID = readU32(input);
  m_collector->collectPage(m_header.id, m_header.level, backgroundPageID, m_isBackgroundPage);
}

void libvisio::VSDParser::readGeometry(WPXInputStream *input)
{
  unsigned char geomFlags = readU8(input);
  bool noFill = (!!(geomFlags & 1));
  bool noLine = (!!(geomFlags & 2));
  bool noShow = (!!(geomFlags & 4));

  if (m_isStencilStarted)
    m_shape.m_geometries.back().addGeometry(m_header.id, m_header.level, noFill, noLine, noShow);
  else
    m_geomList.addGeometry(m_header.id, m_header.level, noFill, noLine, noShow);
}

void libvisio::VSDParser::readMoveTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  if (m_isStencilStarted)
    m_shape.m_geometries.back().addMoveTo(m_header.id, m_header.level, x, y);
  else
    m_geomList.addMoveTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDParser::readLineTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);

  if (m_isStencilStarted)
    m_shape.m_geometries.back().addLineTo(m_header.id, m_header.level, x, y);
  else
    m_geomList.addLineTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDParser::readArcTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y2 = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double bow = readDouble(input);

  if (m_isStencilStarted)
    m_shape.m_geometries.back().addArcTo(m_header.id, m_header.level, x2, y2, bow);
  else
    m_geomList.addArcTo(m_header.id, m_header.level, x2, y2, bow);
}

void libvisio::VSDParser::readXFormData(WPXInputStream *input)
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

  if (m_isStencilStarted)
    m_shape.m_xform = xform;
  else
    m_collector->collectXFormData(m_header.level, xform);
}

void libvisio::VSDParser::readTxtXForm(WPXInputStream *input)
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

  if (m_isStencilStarted)
  {
    if (m_shape.m_txtxform)
      delete (m_shape.m_txtxform);
    m_shape.m_txtxform = new XForm(txtxform);
  }
  else
    m_collector->collectTxtXForm(m_header.level, txtxform);
}

void libvisio::VSDParser::readShapeId(WPXInputStream *input)
{
  unsigned shapeId = readU32(input);

  m_shapeList.addShapeId(m_header.id, m_header.level, shapeId);
}

void libvisio::VSDParser::readShapeList(WPXInputStream *input)
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

void libvisio::VSDParser::readForeignDataType(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double imgOffsetX = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double imgOffsetY = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double imgWidth = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double imgHeight = readDouble(input);
  unsigned foreignType = readU16(input);
  input->seek(0xb, WPX_SEEK_CUR);
  unsigned foreignFormat = readU32(input);

  if (!m_shape.m_foreign)
    m_shape.m_foreign = new ForeignData();
  m_shape.m_foreign->typeId = m_header.id;
  m_shape.m_foreign->typeLevel = m_header.level;
  m_shape.m_foreign->type = foreignType;
  m_shape.m_foreign->format = foreignFormat;
  m_shape.m_foreign->offsetX = imgOffsetX;
  m_shape.m_foreign->offsetY = imgOffsetY;
  m_shape.m_foreign->width = imgWidth;
  m_shape.m_foreign->height = imgHeight;

  if (!m_isStencilStarted)
    m_collector->collectForeignDataType(m_header.level, foreignType, foreignFormat, imgOffsetX, imgOffsetY, imgWidth, imgHeight);
}

void libvisio::VSDParser::readPageProps(WPXInputStream *input)
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
  m_collector->collectPageProps(m_header.id, m_header.level, pageWidth, pageHeight, shadowOffsetX, shadowOffsetY, scale);
}

void libvisio::VSDParser::readShape(WPXInputStream *input)
{
  if (m_header.id != (unsigned)-1)
    m_currentShapeID = m_header.id;
  m_currentShapeLevel = m_header.level;
  unsigned masterPage = (unsigned)-1;
  unsigned masterShape = (unsigned)-1;
  unsigned lineStyle = (unsigned)-1;
  unsigned fillStyle = (unsigned)-1;
  unsigned textStyle = (unsigned)-1;

  try
  {
    input->seek(0x12, WPX_SEEK_CUR);
    masterPage = readU32(input);
    input->seek(4, WPX_SEEK_CUR);
    masterShape = readU32(input);
    input->seek(0x4, WPX_SEEK_CUR);
    fillStyle = readU32(input);
    input->seek(4, WPX_SEEK_CUR);
    lineStyle = readU32(input);
    input->seek(4, WPX_SEEK_CUR);
    textStyle = readU32(input);
  }
  catch (const EndOfStreamException &)
  {
  }

  const VSDShape *tmpShape = m_stencils.getStencilShape(masterPage, masterShape);
  if (tmpShape)
    m_shape = *tmpShape;
  else
    m_shape = VSDShape();

  m_shape.m_lineStyleId = lineStyle;
  m_shape.m_fillStyleId = fillStyle;
  m_shape.m_textStyleId = textStyle;

  if (!m_isStencilStarted)
    m_collector->collectShape(m_currentShapeID, m_header.level, masterPage, masterShape, lineStyle, fillStyle, textStyle);
  m_currentShapeID = (unsigned)-1;
}

void libvisio::VSDParser::readNURBSTo(WPXInputStream *input)
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
      m_shape.m_geometries.back().addNURBSTo(m_header.id, m_header.level, x, y, knot, knotPrev, weight, weightPrev, dataId);
    else
      m_geomList.addNURBSTo(m_header.id, m_header.level, x, y, knot, knotPrev, weight, weightPrev, dataId);
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

  // Only read formula if block is found
  if (cellRef == 6)
  {
    unsigned char xType = 1;
    unsigned char yType = 1;
    unsigned degree = 3;
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
    unsigned long bytesRead = input->tell() - inputPos;
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
      m_shape.m_geometries.back().addNURBSTo(m_header.id, m_header.level, x, y, xType, yType, degree, controlPoints, knotVector, weights);
    else
      m_geomList.addNURBSTo(m_header.id, m_header.level, x, y, xType,
                            yType, degree, controlPoints, knotVector, weights);
  }
  else // No formula found, use line
  {
    if (m_isStencilStarted)
      m_shape.m_geometries.back().addLineTo(m_header.id, m_header.level, x,  y);
    else
      m_geomList.addLineTo(m_header.id, m_header.level, x,  y);
  }
}

void libvisio::VSDParser::readPolylineTo(WPXInputStream *input)
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
      m_shape.m_geometries.back().addPolylineTo(m_header.id, m_header.level, x, y, dataId);
    else
      m_geomList.addPolylineTo(m_header.id, m_header.level, x, y, dataId);
    return;
  }

  // Blocks start at 0x30
  input->seek(0x9, WPX_SEEK_CUR);
  unsigned long chunkBytesRead = 0x30;

  // Find formula block referring to cell A (cell 2)
  unsigned cellRef = 0;
  unsigned length = 0;
  unsigned long inputPos = input->tell();
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
  std::vector<std::pair<double, double> > points;

  // Only formula if block is found
  if (cellRef == 2)
  {
    unsigned long blockBytesRead = 0;
    inputPos = input->tell();
    blockBytesRead += 6;

    // Parse static first two parameters to function
    input->seek(1, WPX_SEEK_CUR);
    unsigned char xType = readU16(input);
    input->seek(1, WPX_SEEK_CUR);
    unsigned char yType = readU16(input);

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
      m_shape.m_geometries.back().addPolylineTo(m_header.id, m_header.level, x, y, xType, yType, points);
    else
      m_geomList.addPolylineTo(m_header.id, m_header.level, x, y, xType,
                               yType, points);
  }
  else
  {
    if (m_isStencilStarted)
      m_shape.m_geometries.back().addLineTo(m_header.id, m_header.level, x, y);
    else
      m_geomList.addLineTo(m_header.id, m_header.level, x, y);
  }
}

void libvisio::VSDParser::readInfiniteLine(WPXInputStream *input)
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
    m_shape.m_geometries.back().addInfiniteLine(m_header.id, m_header.level, x1, y1, x2, y2);
  else
    m_geomList.addInfiniteLine(m_header.id, m_header.level, x1, y1, x2, y2);
}

void libvisio::VSDParser::readShapeData(WPXInputStream *input)
{
  unsigned char dataType = readU8(input);

  input->seek(15, WPX_SEEK_CUR);
  // Polyline data
  if (dataType == 0x80)
  {
    std::vector<std::pair<double, double> > points;
    unsigned char xType = readU8(input);
    unsigned char yType = readU8(input);
    unsigned pointCount = readU32(input);

    for (unsigned i = 0; i < pointCount; i++)
    {
      double x = readDouble(input);
      double y = readDouble(input);
      points.push_back(std::pair<double, double>(x, y));
    }

    if (m_isStencilStarted)
    {
      PolylineData data;
      data.xType = xType;
      data.yType = yType;
      data.points = points;
      m_shape.m_polylineData[m_header.id] = data;
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
      m_shape.m_nurbsData[m_header.id] = data;
    }
    else
      m_collector->collectShapeData(m_header.id, m_header.level, xType, yType, degree, lastKnot, controlPoints, knotVector, weights);
  }
}

void libvisio::VSDParser::readSplineStart(WPXInputStream *input)
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
    m_shape.m_geometries.back().addSplineStart(m_header.id, m_header.level, x, y, secondKnot, firstKnot, lastKnot, degree);
  else
    m_geomList.addSplineStart(m_header.id, m_header.level, x, y, secondKnot, firstKnot, lastKnot, degree);
}

void libvisio::VSDParser::readSplineKnot(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double y = readDouble(input);
  double knot = readDouble(input);

  if (m_isStencilStarted)
    m_shape.m_geometries.back().addSplineKnot(m_header.id, m_header.level, x, y, knot);
  else
    m_geomList.addSplineKnot(m_header.id, m_header.level, x, y, knot);
}

void libvisio::VSDParser::readNameList(WPXInputStream * /* input */)
{
  if (m_isStencilStarted)
    m_shape.m_names.clear();
  else
    m_collector->collectNameList(m_header.id, m_header.level);
}

void libvisio::VSDParser::readFieldList(WPXInputStream *input)
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
    m_shape.m_fields.clear();
    m_shape.m_fields.setElementsOrder(fieldOrder);
  }
  else
  {
    m_fieldList.setElementsOrder(fieldOrder);
    m_fieldList.addFieldList(m_header.id, m_header.level);
    // We want the collectors to still get the level information
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  }
}

void libvisio::VSDParser::readColours(WPXInputStream *input)
{
  input->seek(6, WPX_SEEK_SET);
  unsigned numColours = readU8(input);
  Colour tmpColour;
  input->seek(1, WPX_SEEK_CUR);
  m_colours.clear();

  for (unsigned i = 0; i < numColours; i++)
  {
    tmpColour.r = readU8(input);
    tmpColour.g = readU8(input);
    tmpColour.b = readU8(input);
    tmpColour.a = readU8(input);

    m_colours.push_back(tmpColour);
  }
}

void libvisio::VSDParser::readFont(WPXInputStream *input)
{
  input->seek(4, WPX_SEEK_CUR);
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
  m_collector->collectFont(m_header.id, textStream, libvisio::VSD_TEXT_UTF16);
}

void libvisio::VSDParser::readFontIX(WPXInputStream *input)
{
  input->seek(2, WPX_SEEK_CUR);
  unsigned char codePage = readU8(input);
  input->seek(3, WPX_SEEK_CUR);
  ::WPXBinaryData textStream;

  for (unsigned i = 0; i < m_header.dataLength - 6; i++)
  {
    unsigned char curchar = readU8(input);
    if (curchar == 0)
      break;
    textStream.append(curchar);
  }
  TextFormat format = libvisio::VSD_TEXT_ANSI;
  switch (codePage)
  {
  case 0: // ANSI
    format = libvisio::VSD_TEXT_ANSI;
    break;
  case 0xa1: // GREEK
    format = libvisio::VSD_TEXT_GREEK;
    break;
  case 0xa2: // TURKISH
    format = libvisio::VSD_TEXT_TURKISH;
    break;
  case 0xa3: // VIETNAMESE
    format = libvisio::VSD_TEXT_VIETNAMESE;
    break;
  case 0xb1: // HEBREW
    format = libvisio::VSD_TEXT_HEBREW;
    break;
  case 0xb2: // ARABIC
    format = libvisio::VSD_TEXT_ARABIC;
    break;
  case 0xba: // BALTIC
    format = libvisio::VSD_TEXT_BALTIC;
    break;
  case 0xcc: // RUSSIAN
    format = libvisio::VSD_TEXT_RUSSIAN;
    break;
  case 0xde: // THAI
    format = libvisio::VSD_TEXT_THAI;
    break;
  case 0xee: // CENTRAL EUROPE
    format = libvisio::VSD_TEXT_CENTRAL_EUROPE;
    break;
  default:
    break;
  }
  m_collector->collectFont((unsigned short) m_header.id, textStream, format);
}

/* StyleSheet readers */

void libvisio::VSDParser::readStyleSheet(WPXInputStream *input)
{
  input->seek(0x22, WPX_SEEK_CUR);
  unsigned lineStyle = readU32(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned fillStyle = readU32(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned textStyle = readU32(input);

  m_collector->collectStyleSheet(m_header.id, m_header.level, lineStyle, fillStyle, textStyle);
}

void libvisio::VSDParser::readPageSheet(WPXInputStream * /* input */)
{
  m_currentShapeLevel = m_header.level;
  m_collector->collectPageSheet(m_header.id, m_header.level);
}

libvisio::Colour libvisio::VSDParser::_colourFromIndex(unsigned idx)
{
  if (idx < m_colours.size())
    return m_colours[idx];
  return libvisio::Colour();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
