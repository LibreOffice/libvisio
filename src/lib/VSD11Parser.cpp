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
#include "libvisio_utils.h"
#include "VSD11Parser.h"
#include "VSDInternalStream.h"
#include "VSDDocumentStructure.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"

libvisio::VSD11Parser::VSD11Parser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : VSDParser(input, painter)
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

  unsigned trailerChunks [14] = {0x64, 0x65, 0x66, 0x69, 0x6a, 0x6b, 0x6f, 0x71,
                                 0x92, 0xa9, 0xb4, 0xb6, 0xb9, 0xc7
                                };
  // Add word separator under certain circumstances for v11
  // Below are known conditions, may be more or a simpler pattern
  if (m_header.list != 0 || (m_header.level == 2 && m_header.unknown == 0x55) ||
      (m_header.level == 2 && m_header.unknown == 0x54 && m_header.chunkType == 0xaa)
      || (m_header.level == 3 && m_header.unknown != 0x50 && m_header.unknown != 0x54))
  {
    m_header.trailer += 4;
  }

  for (unsigned i = 0; i < 14; i++)
  {
    if (m_header.chunkType == trailerChunks[i] && m_header.trailer != 12 && m_header.trailer != 4)
    {
      m_header.trailer += 4;
      break;
    }
  }

  // Some chunks never have a trailer
  if (m_header.chunkType == 0x1f || m_header.chunkType == 0xc9 ||
      m_header.chunkType == 0x2d || m_header.chunkType == 0xd1)
  {
    m_header.trailer = 0;
  }
  return true;
}

void libvisio::VSD11Parser::readText(WPXInputStream *input)
{
  input->seek(8, WPX_SEEK_CUR);
  ::WPXBinaryData textStream;

  // Read up to end of chunk in byte pairs (except from last 2 bytes)
  unsigned long numBytesRead = 0;
  const unsigned char *tmpBuffer = input->read(m_header.dataLength - 8, numBytesRead);
  if (numBytesRead)
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil text\n"));
    }
    textStream.append(tmpBuffer, numBytesRead);
    m_shape.m_text = textStream;
    m_shape.m_textFormat = libvisio::VSD_TEXT_UTF16;
  }

}

void libvisio::VSD11Parser::readCharIX(WPXInputStream *input)
{
  VSDFont fontFace;
  unsigned charCount = readU32(input);
  unsigned short fontID = readU16(input);
  input->seek(1, WPX_SEEK_CUR);  // Color ID
  Colour fontColour;            // Font Colour
  fontColour.r = readU8(input);
  fontColour.g = readU8(input);
  fontColour.b = readU8(input);
  fontColour.a = readU8(input);

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

  fontMod = readU8(input);
  if (fontMod & 1) doubleunderline = true;
  if (fontMod & 4) strikeout = true;
  if (fontMod & 0x20) doublestrikeout = true;

  if (m_isInStyles)
    m_collector->collectCharIXStyle(m_header.id, m_header.level, charCount, fontID, fontColour, fontSize,
                                    bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                    allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil character style\n"));
    }

    if (!m_shape.m_charStyle)
      m_shape.m_charStyle= new VSDCharStyle(charCount, fontID, fontColour, fontSize,
                                            bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                            allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
    m_shape.m_charListVector.back().addCharIX(m_header.id, m_header.level, charCount, fontID, fontColour, fontSize,
        bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
        allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
  }
}

void libvisio::VSD11Parser::readParaIX(WPXInputStream *input)
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
  input->seek(26, WPX_SEEK_CUR);
  unsigned flags = readU32(input);

  if (m_isInStyles)
    m_collector->collectParaIXStyle(m_header.id, m_header.level, charCount, indFirst, indLeft, indRight,
                                    spLine, spBefore, spAfter, align, flags);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil paragraph style\n"));
    }

    if (!m_shape.m_paraStyle)
      m_shape.m_paraStyle= new VSDParaStyle(charCount, indFirst, indLeft, indRight,
                                            spLine, spBefore, spAfter, align, flags);
    m_shape.m_paraListVector.back().addParaIX(m_header.id, m_header.level, charCount, indFirst, indLeft, indRight,
        spLine, spBefore, spAfter, align, flags);
  }
}


void libvisio::VSD11Parser::readFillAndShadow(WPXInputStream *input)
{
  Colour colourFG = _colourFromIndex(readU8(input));
  input->seek(3, WPX_SEEK_CUR);
  double fillFGTransparency = (double)readU8(input) / 255.0;
  Colour colourBG = _colourFromIndex(readU8(input));
  input->seek(3, WPX_SEEK_CUR);
  double fillBGTransparency = (double)readU8(input) / 255.0;
  unsigned char fillPattern = readU8(input);
  input->seek(1, WPX_SEEK_CUR);
  Colour shfgc;            // Shadow Foreground Colour
  shfgc.r = readU8(input);
  shfgc.g = readU8(input);
  shfgc.b = readU8(input);
  shfgc.a = readU8(input);
  input->seek(5, WPX_SEEK_CUR); // Shadow Background Colour skipped
  unsigned char shadowPattern = readU8(input);
// only version 11 after that point
  input->seek(2, WPX_SEEK_CUR); // Shadow Type and Value format byte
  double shadowOffsetX = readDouble(input);
  input->seek(1, WPX_SEEK_CUR); // Value format byte
  double shadowOffsetY = -readDouble(input);



  if (m_isInStyles)
    m_collector->collectFillStyle(m_header.level, colourFG, colourBG, fillPattern,
                                  fillFGTransparency, fillBGTransparency, shadowPattern, shfgc,
                                  shadowOffsetX, shadowOffsetY);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil fill\n"));
    }
    if (!m_shape.m_fillStyle)
      m_shape.m_fillStyle = new VSDFillStyle(colourFG, colourBG, fillPattern, fillFGTransparency,
                                             fillBGTransparency, shfgc, shadowPattern, shadowOffsetX, shadowOffsetY);
  }
}

void libvisio::VSD11Parser::readName(WPXInputStream *input)
{
  unsigned long numBytesRead = 0;
  const unsigned char *tmpBuffer = input->read(m_header.dataLength, numBytesRead);
  if (numBytesRead)
  {
    ::WPXBinaryData name(tmpBuffer, numBytesRead);
    m_shape.m_names[m_header.id] = VSDName(name, libvisio::VSD_TEXT_UTF16);
  }
}

void libvisio::VSD11Parser::readTextField(WPXInputStream *input)
{
  unsigned long initialPosition = input->tell();
  input->seek(7, WPX_SEEK_CUR);
  unsigned char tmpCode = readU8(input);
  if (tmpCode == 0xe8)
  {
    int nameId = (int)readU32(input);
    input->seek(6, WPX_SEEK_CUR);
    int formatStringId = (int)readU32(input);
    m_shape.m_fields.addTextField(m_header.id, m_header.level, nameId, formatStringId);
  }
  else
  {
    double numericValue = readDouble(input);
    input->seek(2, WPX_SEEK_CUR);
    int formatStringId = (int)readU32(input);

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

    m_shape.m_fields.addNumericField(m_header.id, m_header.level, formatNumber, numericValue, formatStringId);
  }
}


/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
