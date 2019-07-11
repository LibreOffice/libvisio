/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSD5Parser.h"

#include <librevenge-stream/librevenge-stream.h>
#include <locale.h>
#include <sstream>
#include <string>
#include <stack>
#include "libvisio_utils.h"
#include "VSDInternalStream.h"
#include "VSDDocumentStructure.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"

libvisio::VSD5Parser::VSD5Parser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter)
  : VSD6Parser(input, painter)
{}

libvisio::VSD5Parser::~VSD5Parser()
{}

void libvisio::VSD5Parser::readPointer(librevenge::RVNGInputStream *input, Pointer &ptr)
{
  ptr.Type = readU16(input) & 0x00ff;
  ptr.Format = readU16(input) & 0x00ff;
  input->seek(4, librevenge::RVNG_SEEK_CUR); // Skip dword
  ptr.Offset = readU32(input);
  ptr.Length = readU32(input);
}

void libvisio::VSD5Parser::readPointerInfo(librevenge::RVNGInputStream *input, unsigned ptrType, unsigned shift, unsigned &listSize, int &pointerCount)
{
  VSD_DEBUG_MSG(("VSD5Parser::readPointerInfo\n"));
  switch (ptrType)
  {
  case VSD_TRAILER_STREAM:
    input->seek(shift+0x82, librevenge::RVNG_SEEK_SET);
    break;
  case VSD_PAGE:
    input->seek(shift+0x42, librevenge::RVNG_SEEK_SET);
    break;
  case VSD_FONT_LIST:
    input->seek(shift+0x2e, librevenge::RVNG_SEEK_SET);
    break;
  case VSD_STYLES:
    input->seek(shift+0x12, librevenge::RVNG_SEEK_SET);
    break;
  case VSD_STENCILS:
  case VSD_SHAPE_FOREIGN:
    input->seek(shift+0x1e, librevenge::RVNG_SEEK_SET);
    break;
  case VSD_STENCIL_PAGE:
    input->seek(shift+0x36, librevenge::RVNG_SEEK_SET);
    break;
  default:
    if (ptrType > 0x45)
      input->seek(shift+0x1e, librevenge::RVNG_SEEK_SET);
    else
      input->seek(shift+0xa, librevenge::RVNG_SEEK_SET);
    break;
  }
  pointerCount = readS16(input);
  listSize = 0;
  VSD_DEBUG_MSG(("VSD5Parser::readPointerInfo ptrType %u shift %u pointerCount %i\n", ptrType, shift, pointerCount));
}

bool libvisio::VSD5Parser::getChunkHeader(librevenge::RVNGInputStream *input)
{
  unsigned char tmpChar = 0;
  while (!input->isEnd() && !tmpChar)
    tmpChar = readU8(input);

  if (input->isEnd())
    return false;
  else
    input->seek(-1, librevenge::RVNG_SEEK_CUR);

  m_header.chunkType = getUInt(input);
  m_header.id = getUInt(input);
  m_header.level = readU8(input);
  m_header.unknown = readU8(input);

  m_header.trailer = 0;

  m_header.list = getUInt(input);

  m_header.dataLength = readU32(input);

  return true;
}

void libvisio::VSD5Parser::handleChunkRecords(librevenge::RVNGInputStream *input)
{
  long startPosition = input->tell();
  long endPosition = input->tell() + m_header.dataLength;
  input->seek(endPosition - 4, librevenge::RVNG_SEEK_SET);
  unsigned numRecords = readU16(input);
  const long headerPosition = endPosition - 4 * (numRecords + 1);
  if (headerPosition <= startPosition) // no records to read
    return;
  unsigned endOffset = readU16(input);
  if (long(endOffset) > (headerPosition - startPosition))
    endOffset = unsigned(headerPosition - startPosition); // try to read something anyway
  std::map<unsigned, ChunkHeader> records;
  input->seek(headerPosition, librevenge::RVNG_SEEK_SET);
  unsigned i = 0;
  for (i = 0; i < numRecords; ++i)
  {
    ChunkHeader header;
    header.chunkType = readU16(input);
    unsigned offset = readU16(input);
    unsigned tmpStart = offset;
    while (tmpStart % 4)
      tmpStart++;
    if (tmpStart < endOffset)
    {
      header.dataLength = endOffset - tmpStart;
      header.level = m_header.level + 1;
      records[tmpStart] = header;
      endOffset = offset;
    }
  }
  i = 0;
  for (auto &record : records)
  {
    m_header = record.second;
    m_header.id = i++;
    input->seek(startPosition + record.first, librevenge::RVNG_SEEK_SET);
    handleChunk(input);
  }
}

void libvisio::VSD5Parser::readGeomList(librevenge::RVNGInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readGeomList\n"));
  if (!m_shape.m_geometries.empty() && m_currentGeometryList && m_currentGeometryList->empty())
    m_shape.m_geometries.erase(--m_currentGeomListCount);
  m_currentGeometryList = &m_shape.m_geometries[m_currentGeomListCount++];

  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  handleChunkRecords(input);
}

void libvisio::VSD5Parser::readList(librevenge::RVNGInputStream *input)
{
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  handleChunkRecords(input);
}

void libvisio::VSD5Parser::readCharList(librevenge::RVNGInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readCharList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readParaList(librevenge::RVNGInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readParaList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readShapeList(librevenge::RVNGInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readShapeList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readPropList(librevenge::RVNGInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readPropList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readFieldList(librevenge::RVNGInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readFieldList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readNameList2(librevenge::RVNGInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readNameList2\n"));
  readList(input);
}

void libvisio::VSD5Parser::readTabsDataList(librevenge::RVNGInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readTabsDataList\n"));
  readList(input);
}

void libvisio::VSD5Parser::readLine(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double strokeWidth = readDouble(input);
  unsigned char colourIndex = readU8(input);
  Colour c = _colourFromIndex(colourIndex);
  unsigned char linePattern = readU8(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double rounding = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  unsigned char startMarker = readU8(input);
  unsigned char endMarker = readU8(input);
  unsigned char lineCap = readU8(input);

  if (m_isInStyles)
    m_collector->collectLineStyle(m_header.level, strokeWidth, c, linePattern, startMarker, endMarker, lineCap, rounding, -1, -1);
  else
    m_shape.m_lineStyle.override(VSDOptionalLineStyle(strokeWidth, c, linePattern, startMarker, endMarker, lineCap, rounding, -1, -1));
}

void libvisio::VSD5Parser::readParaIX(librevenge::RVNGInputStream *input)
{
  unsigned charCount = readU16(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double indFirst = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double indLeft = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double indRight = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double spLine = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double spBefore = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double spAfter = readDouble(input);
  unsigned char align = readU8(input);

  unsigned char bullet(0);
  VSDName bulletStr;
  VSDName bulletFont;
  double bulletFontSize(0.0);
  double textPosAfterTab(0.0);
  unsigned flags(0);

  if (m_isInStyles)
    m_collector->collectParaIXStyle(m_header.id, m_header.level, charCount, indFirst, indLeft, indRight,
                                    spLine, spBefore, spAfter, align, bullet, bulletStr,
                                    bulletFont, bulletFontSize, textPosAfterTab, flags);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil paragraph style\n"));
    }

    m_shape.m_paraStyle.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight,
                                                      spLine, spBefore, spAfter, align, bullet,
                                                      bulletStr, bulletFont, bulletFontSize,
                                                      textPosAfterTab, flags));
    m_shape.m_paraList.addParaIX(m_header.id, m_header.level, charCount, indFirst, indLeft, indRight,
                                 spLine, spBefore, spAfter, align, bullet, bulletStr, bulletFont,
                                 bulletFontSize, textPosAfterTab, flags);
  }
}

void libvisio::VSD5Parser::readCharIX(librevenge::RVNGInputStream *input)
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

  double scaleWidth = (double)(readU16(input)) / 10000.0;
  input->seek(2, librevenge::RVNG_SEEK_CUR);
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
                                    allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil character style\n"));
    }

    m_shape.m_charStyle.override(VSDOptionalCharStyle(charCount, font, fontColour, fontSize,
                                                      bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                                      allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth));
    m_shape.m_charList.addCharIX(m_header.id, m_header.level, charCount, font, fontColour, fontSize,
                                 bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                 allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth);
  }
}

void libvisio::VSD5Parser::readFillAndShadow(librevenge::RVNGInputStream *input)
{
  Colour colourFG = _colourFromIndex(readU8(input));
  Colour colourBG = _colourFromIndex(readU8(input));
  unsigned char fillPattern = readU8(input);
  Colour shfgc = _colourFromIndex(readU8(input));
  input->seek(1, librevenge::RVNG_SEEK_CUR); // Shadow Background Colour skipped
  unsigned char shadowPattern = readU8(input);

  if (m_isInStyles)
    m_collector->collectFillStyle(m_header.level, colourFG, colourBG, fillPattern,
                                  0.0, 0.0, shadowPattern, shfgc);
  else
  {
    double shadowOffsetX = 0.0;
    double shadowOffsetY = 0.0;
    if (m_isStencilStarted && m_currentStencil)
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
                                                      0.0, shfgc, shadowPattern, shadowOffsetX,
                                                      shadowOffsetY, -1, -1, -1));
  }
}

void libvisio::VSD5Parser::readStyleSheet(librevenge::RVNGInputStream *input)
{
  input->seek(10, librevenge::RVNG_SEEK_CUR);
  unsigned lineStyle = getUInt(input);
  unsigned fillStyle = getUInt(input);
  unsigned textStyle = getUInt(input);

  m_collector->collectStyleSheet(m_header.id, m_header.level, lineStyle, fillStyle, textStyle);
}

void libvisio::VSD5Parser::readShape(librevenge::RVNGInputStream *input)
{
  m_currentGeomListCount = 0;
  m_currentGeometryList = nullptr;
  m_isShapeStarted = true;
  m_shapeList.clear();
  if (m_header.id != MINUS_ONE)
    m_currentShapeID = m_header.id;
  m_currentShapeLevel = m_header.level;
  unsigned parent = 0;
  auto masterPage = MINUS_ONE;
  auto masterShape = MINUS_ONE;
  auto lineStyle = MINUS_ONE;
  auto fillStyle = MINUS_ONE;
  auto textStyle = MINUS_ONE;

  try
  {
    input->seek(2, librevenge::RVNG_SEEK_CUR);
    parent = getUInt(input);
    input->seek(2, librevenge::RVNG_SEEK_CUR);
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
      m_shape.m_foreign = make_unique<ForeignData>(*(tmpShape->m_foreign));
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

void libvisio::VSD5Parser::readPage(librevenge::RVNGInputStream *input)
{
  unsigned backgroundPageID = getUInt(input);
  m_collector->collectPage(m_header.id, m_header.level, backgroundPageID, m_isBackgroundPage, m_currentPageName);
}

void libvisio::VSD5Parser::readTextBlock(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double leftMargin = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double rightMargin = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double topMargin = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
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

void libvisio::VSD5Parser::readTextField(librevenge::RVNGInputStream *input)
{
  input->seek(3, librevenge::RVNG_SEEK_CUR);
  if (0xe8 == readU8(input))
  {
    int nameId = readS16(input);
    m_shape.m_fields.addTextField(m_header.id, m_header.level, nameId, 0xffff);
  }
  else
  {
    double numericValue = readDouble(input);
    m_shape.m_fields.addNumericField(m_header.id, m_header.level, VSD_FIELD_FORMAT_Unknown, CELL_TYPE_NoCast, numericValue, 0xffff);
  }
}

void libvisio::VSD5Parser::readNameIDX(librevenge::RVNGInputStream *input)
{
  VSD_DEBUG_MSG(("VSD5Parser::readNameIDX\n"));
  std::map<unsigned, VSDName> names;
  unsigned recordCount = readU16(input);
  if (recordCount > getRemainingLength(input) / 4)
    recordCount = getRemainingLength(input) / 4;
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

void libvisio::VSD5Parser::readMisc(librevenge::RVNGInputStream *input)
{
  unsigned char flags = readU8(input);
  if (flags & 0x20)
    m_shape.m_misc.m_hideText = true;
  else
    m_shape.m_misc.m_hideText = false;
}

void libvisio::VSD5Parser::readXForm1D(librevenge::RVNGInputStream *input)
{
  if (!m_shape.m_xform1d)
    m_shape.m_xform1d = make_unique<XForm1D>();
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform1d->beginX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform1d->beginY = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform1d->endX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform1d->endY = readDouble(input);
}

unsigned libvisio::VSD5Parser::getUInt(librevenge::RVNGInputStream *input)
{
  int value = readS16(input);
  return (unsigned)value;
}

int libvisio::VSD5Parser::getInt(librevenge::RVNGInputStream *input)
{
  return readS16(input);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
