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

void libvisio::VSD5Parser::readPointerInfo(WPXInputStream *input, unsigned ptrType, unsigned shift, unsigned &listSize, unsigned &pointerCount)
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
    input->seek(shift+0x1e, WPX_SEEK_SET);
    break;
  case VSD_STENCIL_PAGE:
    input->seek(shift+0x36, WPX_SEEK_SET);
    break;
  default:
    input->seek(shift+0xa, WPX_SEEK_SET);
    break;
  }
  pointerCount = readU16(input);
  VSD_DEBUG_MSG(("VSD5Parser::readPointerInfo ptrType %u shift %u listSize 0x%x pointerCount 0x%x\n", ptrType, shift, listSize, pointerCount));
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

  m_header.chunkType = readU16(input);
  m_header.id = readU16(input);
  m_header.level = readU8(input);
  m_header.unknown = readU8(input);

  m_header.list = readU16(input);

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

  // 0x1f (OLE data) and 0xc9 (Name ID) never have trailer
  if (m_header.chunkType == 0x1f || m_header.chunkType == 0xc9)
  {
    m_header.trailer = 0;
  }
  return true;
}



/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
