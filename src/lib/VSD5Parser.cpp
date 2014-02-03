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
#include <stack>
#include "libvisio_utils.h"
#include "VSD5Parser.h"
#include "VSDInternalStream.h"
#include "VSDDocumentStructure.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"

libvisio::VSD5Parser::VSD5Parser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : VSD6Parser(input, painter)
{}

libvisio::VSD5Parser::~VSD5Parser()
{}

void libvisio::VSD5Parser::readPointer(WPXInputStream *input, Pointer &ptr)
{
  ptr.Type = readU16(input) & 0x00ff;
  ptr.Format = readU16(input) & 0x00ff;
  input->seek(4, WPX_SEEK_CUR); // Skip dword
  ptr.Offset = readU32(input);
  ptr.Length = readU32(input);
}

void libvisio::VSD5Parser::readPointerInfo(WPXInputStream *input, unsigned ptrType, unsigned shift, unsigned &listSize, int &pointerCount)
{
  VSD_DEBUG_MSG(("VSD5Parser::readPointerInfo\n"));
  switch (ptrType)
  {
  case VSD_TRAILER_STREAM:
    input->seek(shift+0x82, WPX_SEEK_SET);
    break;
  case VSD_PAGE:
    input->seek(shift+0x42, WPX_SEEK_SET);
    break;
  case VSD_FONT_LIST:
    input->seek(shift+0x2e, WPX_SEEK_SET);
    break;
  case VSD_STYLES:
    input->seek(shift+0x12, WPX_SEEK_SET);
    break;
  case VSD_STENCILS:
  case VSD_SHAPE_FOREIGN:
    input->seek(shift+0x1e, WPX_SEEK_SET);
    break;
  case VSD_STENCIL_PAGE:
    input->seek(shift+0x36, WPX_SEEK_SET);
    break;
  default:
    if (ptrType > 0x45)
      input->seek(shift+0x1e, WPX_SEEK_SET);
    else
      input->seek(shift+0xa, WPX_SEEK_SET);
    break;
  }
  pointerCount = readS16(input);
  listSize = 0;
  VSD_DEBUG_MSG(("VSD5Parser::readPointerInfo ptrType %u shift %u pointerCount %i\n", ptrType, shift, pointerCount));
}

bool libvisio::VSD5Parser::getChunkHeader(WPXInputStream *input)
{
  unsigned char tmpChar = 0;
  while (!input->atEOS() && !tmpChar)
    tmpChar = readU8(input);

  if (input->atEOS())
    return false;
  else
    input->seek(-1, WPX_SEEK_CUR);

  m_header.chunkType = getUInt(input);
  m_header.id = getUInt(input);
  m_header.level = readU8(input);
  m_header.unknown = readU8(input);

  m_header.trailer = 0;

  m_header.list = getUInt(input);

  m_header.dataLength = readU32(input);

  return true;
}

void libvisio::VSD5Parser::handleChunkRecords(WPXInputStream *input)
{
  long startPosition = input->tell();
  long endPosition = input->tell() + m_header.dataLength;
  input->seek(endPosition - 4, WPX_SEEK_SET);
  unsigned numRecords = readU16(input);
  unsigned endOffset = readU16(input);
  std::map<unsigned, ChunkHeader> records;
  input->seek(endPosition-4*(numRecords+1), WPX_SEEK_SET);
  unsigned i = 0;
  for (i = 0; i < numRecords; ++i)
  {
    ChunkHeader header;
    header.chunkType = readU16(input);
    unsigned offset = readU16(input);
    unsigned tmpStart = offset;
    while (tmpStart % 4)
      tmpStart++;
    header.dataLength = endOffset - tmpStart;
    header.level = m_header.level + 1;
    records[tmpStart] = header;
    endOffset = offset;
  }
  i = 0;
  for (std::map<unsigned, ChunkHeader>::iterator iter = records.begin(); iter != records.end(); ++iter)
  {
    m_header = iter->second;
    m_header.id = i++;
    input->seek(startPosition + iter->first, WPX_SEEK_SET);
    handleChunk(input);
  }
}

void libvisio::VSD5Parser::readGeomList(WPXInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readGeomList\n"));
  if (!m_shape.m_geometries.empty() && m_currentGeometryList->empty())
    m_shape.m_geometries.erase(--m_currentGeomListCount);
  m_currentGeometryList = &m_shape.m_geometries[m_currentGeomListCount++];

  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  handleChunkRecords(input);
}

void libvisio::VSD5Parser::readList(WPXInputStream *input)
{
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  handleChunkRecords(input);
}

void libvisio::VSD5Parser::readCharList(WPXInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readCharList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readParaList(WPXInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readParaList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readShapeList(WPXInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readShapeList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readPropList(WPXInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readPropList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readFieldList(WPXInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readFieldList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readNameList2(WPXInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readNameList2\n"));
  readList(input);
}

void libvisio::VSD5Parser::readLine(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double strokeWidth = readDouble(input);
  unsigned char colourIndex = readU8(input);
  Colour c = _colourFromIndex(colourIndex);
  unsigned char linePattern = readU8(input);
  input->seek(10, WPX_SEEK_CUR);
  unsigned char startMarker = readU8(input);
  unsigned char endMarker = readU8(input);
  unsigned char lineCap = readU8(input);

  if (m_isInStyles)
    m_collector->collectLineStyle(m_header.level, strokeWidth, c, linePattern, startMarker, endMarker, lineCap);
  else
    m_shape.m_lineStyle.override(VSDOptionalLineStyle(strokeWidth, c, linePattern, startMarker, endMarker, lineCap));
}

void libvisio::VSD5Parser::readCharIX(WPXInputStream *input)
{
  unsigned charCount = readU16(input);
  unsigned fontID = readU16(input);
  VSDName font;
  std::map<unsigned, VSDName>::const_iterator iter = m_fonts.find(fontID);
  if (iter != m_fonts.end())
    font = iter->second;
  Colour fontColour = _colourFromIndex(readU8(input));

  bool bold(false);
  bool italic(false);
  bool underline(false);
  bool doubleunderline(false);
  bool strikeout(false);
  bool doublestrikeout(false);
  bool allcaps(false);
  bool initcaps(false);
  bool smallcaps(false);
  bool superscript(false);
  bool subscript(false);
  unsigned char fontMod = readU8(input);
  if (fontMod & 1) bold = true;
  if (fontMod & 2) italic = true;
  if (fontMod & 4) underline = true;
  if (fontMod & 8) smallcaps = true;
  fontMod = readU8(input);
  if (fontMod & 1) allcaps = true;
  if (fontMod & 2) initcaps = true;
  fontMod = readU8(input);
  if (fontMod & 1) superscript = true;
  if (fontMod & 2) subscript = true;

  input->seek(4, WPX_SEEK_CUR);
  double fontSize = readDouble(input);

#if 0
  fontMod = readU8(input);
  if (fontMod & 1) doubleunderline = true;
  if (fontMod & 4) strikeout = true;
  if (fontMod & 0x20) doublestrikeout = true;
#endif

  if (m_isInStyles)
    m_collector->collectCharIXStyle(m_header.id, m_header.level, charCount, font, fontColour, fontSize,
                                    bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                    allcaps, initcaps, smallcaps, superscript, subscript);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil character style\n"));
    }

    m_shape.m_charStyle.override(VSDOptionalCharStyle(charCount, font, fontColour, fontSize,
                                                      bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                                      allcaps, initcaps, smallcaps, superscript, subscript));
    m_shape.m_charList.addCharIX(m_header.id, m_header.level, charCount, font, fontColour, fontSize,
                                 bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                 allcaps, initcaps, smallcaps, superscript, subscript);
  }
}

void libvisio::VSD5Parser::readFillAndShadow(WPXInputStream *input)
{
  Colour colourFG = _colourFromIndex(readU8(input));
  Colour colourBG = _colourFromIndex(readU8(input));
  unsigned char fillPattern = readU8(input);
  Colour shfgc = _colourFromIndex(readU8(input));
  input->seek(1, WPX_SEEK_CUR); // Shadow Background Colour skipped
  unsigned char shadowPattern = readU8(input);

  if (m_isInStyles)
    m_collector->collectFillStyle(m_header.level, colourFG, colourBG, fillPattern,
                                  0.0, 0.0, shadowPattern, shfgc);
  else
  {
    double shadowOffsetX = 0.0;
    double shadowOffsetY = 0.0;
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil fill\n"));
      shadowOffsetX = m_currentStencil->m_shadowOffsetX;
      shadowOffsetY = m_currentStencil->m_shadowOffsetY;
    }
    else
    {
      shadowOffsetX = m_shadowOffsetX;
      shadowOffsetY = m_shadowOffsetY;
    }
    m_shape.m_fillStyle.override(VSDOptionalFillStyle(colourFG, colourBG, fillPattern, 0.0,
                                                      0.0, shfgc, shadowPattern, shadowOffsetX, shadowOffsetY));
  }
}

void libvisio::VSD5Parser::readStyleSheet(WPXInputStream *input)
{
  input->seek(10, WPX_SEEK_CUR);
  unsigned lineStyle = getUInt(input);
  unsigned fillStyle = getUInt(input);
  unsigned textStyle = getUInt(input);

  m_collector->collectStyleSheet(m_header.id, m_header.level, lineStyle, fillStyle, textStyle);
}

void libvisio::VSD5Parser::readShape(WPXInputStream *input)
{
  m_currentGeomListCount = 0;
  m_isShapeStarted = true;
  m_shapeList.clear();
  if (m_header.id != MINUS_ONE)
    m_currentShapeID = m_header.id;
  m_currentShapeLevel = m_header.level;
  unsigned parent = 0;
  unsigned masterPage = MINUS_ONE;
  unsigned masterShape = MINUS_ONE;
  unsigned lineStyle = MINUS_ONE;
  unsigned fillStyle = MINUS_ONE;
  unsigned textStyle = MINUS_ONE;

  try
  {
    input->seek(2, WPX_SEEK_CUR);
    parent = getUInt(input);
    input->seek(2, WPX_SEEK_CUR);
    masterPage = getUInt(input);
    masterShape = getUInt(input);
    lineStyle = getUInt(input);
    fillStyle = getUInt(input);
    textStyle = getUInt(input);
  }
  catch (const EndOfStreamException &)
  {
  }

  m_shape.clear();
  const VSDShape *tmpShape = m_stencils.getStencilShape(masterPage, masterShape);
  if (tmpShape)
  {
    if (tmpShape->m_foreign)
      m_shape.m_foreign = new ForeignData(*(tmpShape->m_foreign));
    m_shape.m_text = tmpShape->m_text;
    m_shape.m_textFormat = tmpShape->m_textFormat;
  }

  m_shape.m_lineStyleId = lineStyle;
  m_shape.m_fillStyleId = fillStyle;
  m_shape.m_textStyleId = textStyle;
  m_shape.m_parent = parent;
  m_shape.m_masterPage = masterPage;
  m_shape.m_masterShape = masterShape;
  m_shape.m_shapeId = m_currentShapeID;
  m_currentShapeID = MINUS_ONE;
}

void libvisio::VSD5Parser::readPage(WPXInputStream *input)
{
  unsigned backgroundPageID = getUInt(input);
  m_collector->collectPage(m_header.id, m_header.level, backgroundPageID, m_isBackgroundPage, m_currentPageName);
}

void libvisio::VSD5Parser::readTextBlock(WPXInputStream *input)
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
  unsigned char colourIndex = readU8(input);
  bool isBgFilled = !!colourIndex;
  Colour c;
  if (isBgFilled)
    c = _colourFromIndex(colourIndex-1);

  if (m_isInStyles)
    m_collector->collectTextBlockStyle(m_header.level, leftMargin, rightMargin, topMargin, bottomMargin,
                                       verticalAlign, isBgFilled, c, 0.0, (unsigned char)0);
  else
    m_shape.m_textBlockStyle.override(VSDOptionalTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin,
                                                                verticalAlign, isBgFilled, c, 0.0, (unsigned char)0));
}

void libvisio::VSD5Parser::readTextField(WPXInputStream *input)
{
  input->seek(3, WPX_SEEK_CUR);
  if (0xe8 == readU8(input))
  {
    int nameId = readS16(input);
    m_shape.m_fields.addTextField(m_header.id, m_header.level, nameId, 0xffff);
  }
  else
  {
    double numericValue = readDouble(input);
    m_shape.m_fields.addNumericField(m_header.id, m_header.level, 0xffff, numericValue, 0xffff);
  }
}

void libvisio::VSD5Parser::readNameIDX(WPXInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readNameIDX\n"));
  std::map<unsigned, VSDName> names;
  unsigned recordCount = readU16(input);
  for (unsigned i = 0; i < recordCount; ++i)
  {
    unsigned nameId = readU16(input);
    unsigned elementId = readU16(input);
    std::map<unsigned, VSDName>::const_iterator iter = m_names.find(nameId);
    if (iter != m_names.end())
      names[elementId] = iter->second;
  }
  m_namesMapMap[m_header.level] = names;
}


unsigned libvisio::VSD5Parser::getUInt(WPXInputStream *input)
{
  int value = readS16(input);
  return (unsigned)value;
}

int libvisio::VSD5Parser::getInt(WPXInputStream *input)
{
  return readS16(input);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
