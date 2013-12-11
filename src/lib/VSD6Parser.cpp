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

#include <librevenge-stream/librevenge-stream.h>
#include <locale.h>
#include <sstream>
#include <string>
#include "libvisio_utils.h"
#include "VSD6Parser.h"
#include "VSDInternalStream.h"
#include "VSDDocumentStructure.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"

libvisio::VSD6Parser::VSD6Parser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter)
  : VSDParser(input, painter)
{}

libvisio::VSD6Parser::~VSD6Parser()
{}

bool libvisio::VSD6Parser::getChunkHeader(librevenge::RVNGInputStream *input)
{
  unsigned char tmpChar = 0;
  while (!input->isEnd() && !tmpChar)
    tmpChar = readU8(input);

  if (input->isEnd())
    return false;
  else
    input->seek(-1, librevenge::RVNG_SEEK_CUR);

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

void libvisio::VSD6Parser::readText(librevenge::RVNGInputStream *input)
{
  input->seek(8, librevenge::RVNG_SEEK_CUR);
  librevenge::RVNGBinaryData  textStream;

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
    m_shape.m_textFormat = libvisio::VSD_TEXT_ANSI;
  }
}

void libvisio::VSD6Parser::readCharIX(librevenge::RVNGInputStream *input)
{
  unsigned charCount = readU32(input);
  unsigned fontID = readU16(input);
  VSDName font;
  std::map<unsigned, VSDName>::const_iterator iter = m_fonts.find(fontID);
  if (iter != m_fonts.end())
    font = iter->second;
  input->seek(1, librevenge::RVNG_SEEK_CUR);  // Color ID
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

  input->seek(4, librevenge::RVNG_SEEK_CUR);
  double fontSize = readDouble(input);

  fontMod = readU8(input);
  if (fontMod & 1) doubleunderline = true;
  if (fontMod & 4) strikeout = true;
  if (fontMod & 0x20) doublestrikeout = true;

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

void libvisio::VSD6Parser::readParaIX(librevenge::RVNGInputStream *input)
{
  unsigned charCount = getUInt(input);
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

  if (m_isInStyles)
    m_collector->collectParaIXStyle(m_header.id, m_header.level, charCount, indFirst, indLeft, indRight,
                                    spLine, spBefore, spAfter, align, (unsigned)0);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil paragraph style\n"));
    }

    m_shape.m_paraStyle.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight,
                                                      spLine, spBefore, spAfter, align, (unsigned)0));
    m_shape.m_paraList.addParaIX(m_header.id, m_header.level, charCount, indFirst, indLeft, indRight,
                                 spLine, spBefore, spAfter, align, (unsigned)0);
  }
}


void libvisio::VSD6Parser::readFillAndShadow(librevenge::RVNGInputStream *input)
{
  unsigned char colourFGIndex = readU8(input);
  Colour colourFG;
  colourFG.r = readU8(input);
  colourFG.g = readU8(input);
  colourFG.b = readU8(input);
  colourFG.a = readU8(input);
  unsigned char colourBGIndex = readU8(input);
  Colour colourBG;
  colourBG.r = readU8(input);
  colourBG.g = readU8(input);
  colourBG.b = readU8(input);
  colourBG.a = readU8(input);
  if (!colourFG && !colourBG)
  {
    colourFG = _colourFromIndex(colourFGIndex);
    colourBG = _colourFromIndex(colourBGIndex);
  }
  double fillFGTransparency = (double)colourFG.a / 255.0;
  double fillBGTransparency = (double)colourBG.a / 255.0;

  unsigned char fillPattern = readU8(input);

  unsigned char shadowFGIndex = readU8(input);
  Colour shadowFG;
  shadowFG.r = readU8(input);
  shadowFG.g = readU8(input);
  shadowFG.b = readU8(input);
  shadowFG.a = readU8(input);
  unsigned char shadowBGIndex = readU8(input);
  Colour shadowBG;
  shadowBG.r = readU8(input);
  shadowBG.g = readU8(input);
  shadowBG.b = readU8(input);
  shadowBG.a = readU8(input);
  if (!shadowFG && !shadowBG)
  {
    shadowFG = _colourFromIndex(shadowFGIndex);
    shadowBG = _colourFromIndex(shadowBGIndex);
  }

  unsigned char shadowPattern = readU8(input);

  if (m_isInStyles)
    m_collector->collectFillStyle(m_header.level, colourFG, colourBG, fillPattern,
                                  fillFGTransparency, fillBGTransparency, shadowPattern, shadowFG);
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
    m_shape.m_fillStyle.override(VSDOptionalFillStyle(colourFG, colourBG, fillPattern, fillFGTransparency,
                                                      fillBGTransparency, shadowFG, shadowPattern, shadowOffsetX, shadowOffsetY));
  }
}

void libvisio::VSD6Parser::readName(librevenge::RVNGInputStream *input)
{
  unsigned long numBytesRead = 0;
  const unsigned char *tmpBuffer = input->read(m_header.dataLength, numBytesRead);
  if (numBytesRead)
  {
    librevenge::RVNGBinaryData name(tmpBuffer, numBytesRead);
    m_shape.m_names[m_header.id] = VSDName(name, libvisio::VSD_TEXT_ANSI);
  }
}

void libvisio::VSD6Parser::readName2(librevenge::RVNGInputStream *input)
{
  unsigned char character = 0;
  librevenge::RVNGBinaryData name;
  getInt(input); // skip a dword that seems to be always 1
  while ((character = readU8(input)))
    name.append(character);
  name.append(character);
  m_names[m_header.id] = VSDName(name, libvisio::VSD_TEXT_ANSI);
}

void libvisio::VSD6Parser::readTextField(librevenge::RVNGInputStream *input)
{
  unsigned long initialPosition = input->tell();
  input->seek(7, librevenge::RVNG_SEEK_CUR);
  unsigned char tmpCode = readU8(input);
  if (tmpCode == 0xe8)
  {
    int nameId = readS32(input);
    input->seek(6, librevenge::RVNG_SEEK_CUR);
    int formatStringId = readS32(input);
    m_shape.m_fields.addTextField(m_header.id, m_header.level, nameId, formatStringId);
  }
  else
  {
    double numericValue = readDouble(input);
    input->seek(2, librevenge::RVNG_SEEK_CUR);
    int formatStringId = readS32(input);

    unsigned blockIdx = 0;
    unsigned length = 0;
    unsigned short formatNumber = 0;
    input->seek(initialPosition+0x24, librevenge::RVNG_SEEK_SET);
    while (blockIdx != 2 && !input->isEnd() && (unsigned long) input->tell() < (unsigned long)(initialPosition+m_header.dataLength+m_header.trailer))
    {
      unsigned long inputPos = input->tell();
      length = readU32(input);
      if (!length)
        break;
      input->seek(1, librevenge::RVNG_SEEK_CUR);
      blockIdx = readU8(input);
      if (blockIdx != 2)
        input->seek(inputPos + length, librevenge::RVNG_SEEK_SET);
      else
      {
        input->seek(1, librevenge::RVNG_SEEK_CUR);
        formatNumber = readU16(input);
        if (0x80 != readU8(input))
        {
          input->seek(inputPos + length, librevenge::RVNG_SEEK_SET);
          blockIdx = 0;
        }
        else
        {
          if (0xc2 != readU8(input))
          {
            input->seek(inputPos + length, librevenge::RVNG_SEEK_SET);
            blockIdx = 0;
          }
          else
            break;
        }
      }
    }

    if (input->isEnd())
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
