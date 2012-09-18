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
 * Copyright (C) 2012 Fridrich Strba <fridrich.strba@bluewin.ch>
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

#include <string.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlreader.h>
#include <libwpd-stream/libwpd-stream.h>
#include "VSDXParser.h"

namespace
{

extern "C" {

  int vsdxInputOpenFunction(void * /* context */)
  {
    return 0;
  }

  int vsdxInputReadFunc(void *context, char *buffer, int len)
  {
    WPXInputStream *input = (WPXInputStream *)context;

    if ((!input) || (!buffer) || (len < 0))
      return -1;

    unsigned long tmpNumBytesRead = 0;
    const unsigned char *tmpBuffer = input->read(len, tmpNumBytesRead);

    if (!tmpBuffer)
      return -1;

    if (len > (int)tmpNumBytesRead)
      len = (int)tmpNumBytesRead;

    memcpy(buffer, tmpBuffer, len);
    return len;
  }

}

}

libvisio::VSDXParser::VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : m_input(input), m_painter(painter), m_extractStencils(false)
{}

libvisio::VSDXParser::~VSDXParser()
{
}

bool libvisio::VSDXParser::parseMain()
{
  if (!m_input)
  {
    return false;
  }

  return false;
}

bool libvisio::VSDXParser::extractStencils()
{
  m_extractStencils = true;
  return parseMain();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
