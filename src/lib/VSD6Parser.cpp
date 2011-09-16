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
#include "libvisio_utils.h"
#include "VSD6Parser.h"
#include "VSDInternalStream.h"
#include "VSDXDocumentStructure.h"
#include "VSDXContentCollector.h"
#include "VSDXStylesCollector.h"

libvisio::VSD6Parser::VSD6Parser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : VSDXParser(input, painter)
{}

libvisio::VSD6Parser::~VSD6Parser()
{}

bool libvisio::VSD6Parser::getChunkHeader(WPXInputStream *input)
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
  if (m_header.list != 0 || m_header.chunkType == 0x76 || m_header.chunkType == 0x73 ||
      m_header.chunkType == 0x72 || m_header.chunkType == 0x71 || m_header.chunkType == 0x70 ||
      m_header.chunkType == 0x6f || m_header.chunkType == 0x6e || m_header.chunkType == 0x6d ||
      m_header.chunkType == 0x6c || m_header.chunkType == 0x6b || m_header.chunkType == 0x6a ||
      m_header.chunkType == 0x69 || m_header.chunkType == 0x68 || m_header.chunkType == 0x67 ||
      m_header.chunkType == 0x66 || m_header.chunkType == 0x65 || m_header.chunkType == 0x64 ||
      m_header.chunkType == 0x2c || m_header.chunkType == 0xd)
    m_header.trailer += 8; // 8 byte trailer

  m_header.dataLength = readU32(input);
  m_header.level = readU16(input);
  m_header.unknown = readU8(input);

  // 0x1f (OLE data) and 0xc9 (Name ID) never have trailer
  if (m_header.chunkType == 0x1f || m_header.chunkType == 0xc9)
  {
    m_header.trailer = 0;
  }
  return true;
}

void libvisio::VSD6Parser::readText(WPXInputStream *input)
{
  input->seek(8, WPX_SEEK_CUR);
  std::vector<uint8_t> textStream;

  for (unsigned bytesRead = 8; bytesRead < m_header.dataLength-1; bytesRead++)
    textStream.push_back(readU8(input));

  m_collector->collectText(m_header.id, m_header.level, textStream, VSD_TEXT_ANSI);
}

void libvisio::VSD6Parser::readCharIX(WPXInputStream *input)
{
  WPXString fontFace = "Arial";
  unsigned charCount = readU32(input);
  unsigned short fontID = readU16(input);
  input->seek(1, WPX_SEEK_CUR);  // Color ID
  Colour fontColour;            // Font Colour
  fontColour.r = readU8(input);
  fontColour.g = readU8(input);
  fontColour.b = readU8(input);
  fontColour.a = readU8(input);

  bool bold(false); bool italic(false); bool underline(false); bool doubleunderline(false);
  bool strikeout(false); bool doublestrikeout(false); bool allcaps(false); bool initcaps(false); bool smallcaps(false);
  bool superscript(false); bool subscript(false);
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
  
  fontMod = readU8(input);
  if (fontMod & 1) doubleunderline = true;
  if (fontMod & 4) strikeout = true;
  if (fontMod & 0x20) doublestrikeout = true;

  input->seek(42, WPX_SEEK_CUR);
  unsigned langId = readU32(input);

  m_charList->addCharIX(m_header.id, m_header.level, charCount, fontID, fontColour, langId, fontSize,
                        bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                        allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
}

void libvisio::VSD6Parser::readFillAndShadow(WPXInputStream *input)
{
  unsigned char colourIndexFG = readU8(input);
  input->seek(3, WPX_SEEK_CUR);
  unsigned char fillFGTransparency = readU8(input);
  unsigned char colourIndexBG = readU8(input);
  input->seek(3, WPX_SEEK_CUR);
  unsigned char fillBGTransparency = readU8(input);
  unsigned char fillPattern = readU8(input);
  input->seek(1, WPX_SEEK_CUR);
  Colour shfgc;            // Shadow Foreground Colour
  shfgc.r = readU8(input);
  shfgc.g = readU8(input);
  shfgc.b = readU8(input);
  shfgc.a = readU8(input);
  input->seek(5, WPX_SEEK_CUR);  // Shadow Background Colour skipped
  unsigned char shadowPattern = readU8(input);

  if (m_isStencilStarted)
  {
    if (m_stencilShape.m_fillStyle == 0) m_stencilShape.m_fillStyle = new VSDXFillStyle(colourIndexFG, colourIndexBG, fillPattern, fillFGTransparency, fillBGTransparency, shfgc, shadowPattern, m_currentStencil->m_shadowOffsetX, m_currentStencil->m_shadowOffsetY);
  }
  else
    m_collector->collectFillAndShadow(m_header.id, m_header.level, colourIndexFG, colourIndexBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc);
}

void libvisio::VSD6Parser::readFillStyle(WPXInputStream *input)
{
  unsigned char colourIndexFG = readU8(input);
  input->seek(3, WPX_SEEK_CUR);
  unsigned char fillFGTransparency = readU8(input);
  unsigned char colourIndexBG = readU8(input);
  input->seek(3, WPX_SEEK_CUR);
  unsigned char fillBGTransparency = readU8(input);
  unsigned char fillPattern = readU8(input);
  input->seek(1, WPX_SEEK_CUR);
  Colour shfgc;            // Shadow Foreground Colour
  shfgc.r = readU8(input);
  shfgc.g = readU8(input);
  shfgc.b = readU8(input);
  shfgc.a = readU8(input);
  input->seek(5, WPX_SEEK_CUR);  // Shadow Background Colour skipped
  unsigned char shadowPattern = readU8(input);

  m_collector->collectFillStyle(m_header.id, m_header.level, colourIndexFG, colourIndexBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc);
}

void libvisio::VSD6Parser::readCharIXStyle(WPXInputStream *input)
{
  WPXString fontFace = "Arial";
  unsigned charCount = readU32(input);
  unsigned short fontID = readU16(input);
  input->seek(1, WPX_SEEK_CUR);  // Color ID
  Colour fontColour;            // Font Colour
  fontColour.r = readU8(input);
  fontColour.g = readU8(input);
  fontColour.b = readU8(input);
  fontColour.a = readU8(input);

  bool bold(false); bool italic(false); bool underline(false); bool doubleunderline(false);
  bool strikeout(false); bool doublestrikeout(false); bool allcaps(false); bool initcaps(false); bool smallcaps(false);
  bool superscript(false); bool subscript(false);
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
  
  fontMod = readU8(input);
  if (fontMod & 1) doubleunderline = true;
  if (fontMod & 4) strikeout = true;
  if (fontMod & 0x20) doublestrikeout = true;

  input->seek(42, WPX_SEEK_CUR);
  unsigned langId = readU32(input);

  m_collector->collectCharIXStyle(m_header.id, m_header.level, charCount, fontID, fontColour, langId, fontSize,
                                  bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                  allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
}
