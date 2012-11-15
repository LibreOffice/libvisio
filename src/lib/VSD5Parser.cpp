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

void libvisio::VSD5Parser::readPointerInfo(WPXInputStream *input, unsigned ptrType, unsigned shift, unsigned & /*listSize*/, int &pointerCount)
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
  pointerCount = readS16(input);
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

  m_header.chunkType = readU16(input);
  if (!m_header.chunkType)
    input->seek(10, WPX_SEEK_CUR);
  m_header.id = readU16(input);
  m_header.level = readU8(input);
  m_header.unknown = readU8(input);

  m_header.trailer = 0;

  m_header.list = readU16(input);

  m_header.dataLength = readU32(input);

  return true;
}

void libvisio::VSD5Parser::handleChunkRecords(WPXInputStream *input)
{
  long startPosition = input->tell();
  long endPosition = input->tell() + m_header.dataLength;
  input->seek(endPosition - 4, WPX_SEEK_SET);
  unsigned numRecords = readU16(input);
  unsigned endOffset = m_header.dataLength - 4*(numRecords+1);
  std::map<unsigned, ChunkHeader> records;
  input->seek(endOffset+startPosition, WPX_SEEK_SET);
  unsigned i = 0;
  for (i = 0; i < numRecords; ++i)
  {
    ChunkHeader header;
    header.chunkType = readU16(input);
    if (!header.chunkType)
      header.chunkType = readU16(input);
    unsigned offset = readU16(input) + (i < numRecords - 1? 2 : 0);
    header.dataLength = endOffset - offset;
    header.level = m_header.level + 1;
    records[offset] = header;
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
  if (!m_shape.m_geometries.empty() && m_currentGeometryList->empty())
    m_shape.m_geometries.erase(--m_currentGeomListCount);
  m_currentGeometryList = &m_shape.m_geometries[m_currentGeomListCount++];

  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  handleChunkRecords(input);
}

void libvisio::VSD5Parser::readCharList(WPXInputStream *input)
{
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  handleChunkRecords(input);
}

void libvisio::VSD5Parser::readParaList(WPXInputStream *input)
{
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  handleChunkRecords(input);
}

void libvisio::VSD5Parser::readShapeList(WPXInputStream *input)
{
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  handleChunkRecords(input);
}

void libvisio::VSD5Parser::readPropList(WPXInputStream *input)
{
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  handleChunkRecords(input);
}

void libvisio::VSD5Parser::readShapeId(WPXInputStream *input)
{
  if (!m_isShapeStarted)
    m_shapeList.addShapeId(m_header.id, readU16(input));
  else
    m_shape.m_shapeList.addShapeId(m_header.id, readU16(input));
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
