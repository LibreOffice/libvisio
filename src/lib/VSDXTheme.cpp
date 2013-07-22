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
 * Copyright (C) 2013 Fridrich Strba <fridrich.strba@bluewin.ch>
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

#include "VSDXTheme.h"
#include "VSDXMLTokenMap.h"
#include "libvisio_utils.h"

libvisio::VSDXTheme::VSDXTheme()
{
}

libvisio::VSDXTheme::~VSDXTheme()
{
}

bool libvisio::VSDXTheme::parse(WPXInputStream *input)
{
  if (!input)
    return false;

  xmlTextReaderPtr reader = xmlReaderForStream(input, 0, 0, XML_PARSE_NOBLANKS|XML_PARSE_NOENT|XML_PARSE_NONET);
  if (!reader)
    return false;

  try
  {
    int ret = xmlTextReaderRead(reader);
    while (1 == ret)
    {
      int tokenId = VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
      int tokenType = xmlTextReaderNodeType(reader);

      switch (tokenId)
      {
      case XML_A_CLRSCHEME:
        readClrScheme(reader);
        break;
      default:
        break;
      }
      ret = xmlTextReaderRead(reader);
    }
  }
  catch (...)
  {
    xmlFreeTextReader(reader);
    return false;
  }
  xmlFreeTextReader(reader);
  return true;
}

boost::optional<libvisio::Colour> libvisio::VSDXTheme::readSrgbClr(xmlTextReaderPtr reader)
{
  boost::optional<libvisio::Colour> retVal;
  if (XML_A_SRGBCLR == VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader)))
  {
    xmlChar *val = xmlTextReaderGetAttribute(reader, BAD_CAST("val"));
    if (val)
    {
      try
      {
        retVal = xmlStringToColour(val);
      }
      catch (const XmlParserException &)
      {
      }
      xmlFree(val);
    }
  }
  return retVal;
}

boost::optional<libvisio::Colour> libvisio::VSDXTheme::readSysClr(xmlTextReaderPtr reader)
{
  boost::optional<libvisio::Colour> retVal;
  if (XML_A_SYSCLR == VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader)))
  {
    xmlChar *lastClr = xmlTextReaderGetAttribute(reader, BAD_CAST("lastClr"));
    if (lastClr)
    {
      try
      {
        retVal = xmlStringToColour(lastClr);
      }
      catch (const XmlParserException &)
      {
      }
      xmlFree(lastClr);
    }
  }
  return retVal;
}

void libvisio::VSDXTheme::readClrScheme(xmlTextReaderPtr reader)
{
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
