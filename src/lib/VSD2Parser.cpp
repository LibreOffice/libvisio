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
#include "VSD2Parser.h"
#include "VSDInternalStream.h"
#include "VSDDocumentStructure.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"

libvisio::VSD2Parser::VSD2Parser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : VSD5Parser(input, painter)
{}

libvisio::VSD2Parser::~VSD2Parser()
{}

bool libvisio::VSD2Parser::getChunkHeader(WPXInputStream * /* input */)
{
  return true;
}

void libvisio::VSD2Parser::readColours(WPXInputStream *input)
{
  input->seek(2, WPX_SEEK_SET);
  unsigned numColours = readU8(input);
  input->seek(1, WPX_SEEK_CUR);
  m_colours.clear();

  for (unsigned i = 0; i < numColours; i++)
  {
    Colour tmpColour;
    tmpColour.r = readU8(input);
    tmpColour.g = readU8(input);
    tmpColour.b = readU8(input);
    tmpColour.a = readU8(input);

    m_colours.push_back(tmpColour);
  }
}

void libvisio::VSD2Parser::handleBlob(WPXInputStream *input, unsigned level)
{
  try
  {
    m_header.level = level;
    _handleLevelChange(m_header.level);
    handleChunk(input);
  }
  catch (EndOfStreamException &)
  {
    VSD_DEBUG_MSG(("VSD2Parser::handleBlob - catching EndOfStreamException\n"));
  }
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
