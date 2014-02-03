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

libvisio::VSDXVariationClrScheme::VSDXVariationClrScheme()
  : m_varColor1()
  , m_varColor2()
  , m_varColor3()
  , m_varColor4()
  , m_varColor5()
  , m_varColor6()
  , m_varColor7()
{
}

libvisio::VSDXClrScheme::VSDXClrScheme()
  : m_dk1()
  , m_lt1()
  , m_dk2()
  , m_lt2()
  , m_accent1()
  , m_accent2()
  , m_accent3()
  , m_accent4()
  , m_accent5()
  , m_accent6()
  , m_hlink()
  , m_folHlink()
  , m_bkgnd()
  , m_variationClrSchemeLst()
{
}

libvisio::VSDXTheme::VSDXTheme()
  : m_clrScheme()
{
}

libvisio::VSDXTheme::~VSDXTheme()
{
}


int libvisio::VSDXTheme::getElementToken(xmlTextReaderPtr reader)
{
  return VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
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
      int tokenId = getElementToken(reader);

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
  if (XML_A_SRGBCLR == getElementToken(reader))
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
  if (XML_A_SYSCLR == getElementToken(reader))
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
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  m_clrScheme.m_variationClrSchemeLst.clear();
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readClrScheme: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_A_DK1:
      readThemeColour(reader, tokenId, m_clrScheme.m_dk1);
      break;
    case XML_A_DK2:
      readThemeColour(reader, tokenId, m_clrScheme.m_dk2);
      break;
    case XML_A_LT1:
      readThemeColour(reader, tokenId, m_clrScheme.m_lt1);
      break;
    case XML_A_LT2:
      readThemeColour(reader, tokenId, m_clrScheme.m_lt2);
      break;
    case XML_A_ACCENT1:
      readThemeColour(reader, tokenId, m_clrScheme.m_accent1);
      break;
    case XML_A_ACCENT2:
      readThemeColour(reader, tokenId, m_clrScheme.m_accent2);
      break;
    case XML_A_ACCENT3:
      readThemeColour(reader, tokenId, m_clrScheme.m_accent3);
      break;
    case XML_A_ACCENT4:
      readThemeColour(reader, tokenId, m_clrScheme.m_accent4);
      break;
    case XML_A_ACCENT5:
      readThemeColour(reader, tokenId, m_clrScheme.m_accent5);
      break;
    case XML_A_ACCENT6:
      readThemeColour(reader, tokenId, m_clrScheme.m_accent6);
      break;
    case XML_A_HLINK:
      readThemeColour(reader, tokenId, m_clrScheme.m_hlink);
      break;
    case XML_A_FOLHLINK:
      readThemeColour(reader, tokenId, m_clrScheme.m_folHlink);
      break;
    case XML_VT_BKGND:
      readThemeColour(reader, tokenId, m_clrScheme.m_bkgnd);
      break;
    case XML_VT_VARIATIONCLRSCHEMELST:
      readVariationClrSchemeLst(reader);
      break;
    default:
      break;
    }
  }
  while ((XML_A_CLRSCHEME != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXTheme::readThemeColour(xmlTextReaderPtr reader, int idToken, Colour &clr)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  boost::optional<libvisio::Colour> colour;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readThemeColour: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_A_SRGBCLR:
      colour = readSrgbClr(reader);
      break;
    case XML_A_SYSCLR:
      colour = readSysClr(reader);
      break;
    default:
      break;
    }
  }
  while ((idToken != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);

  if (colour)
    clr = *colour;
}

void libvisio::VSDXTheme::readVariationClrSchemeLst(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readVariationClrSchemeLst: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_VT_VARIATIONCLRSCHEME:
    {
      VSDXVariationClrScheme varClrSch;
      readVariationClrScheme(reader, varClrSch);
      m_clrScheme.m_variationClrSchemeLst.push_back(varClrSch);
      break;
    }
    default:
      break;
    }
  }
  while ((XML_VT_VARIATIONCLRSCHEMELST != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXTheme::readVariationClrScheme(xmlTextReaderPtr reader, VSDXVariationClrScheme &varClrSch)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readVariationClrScheme: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_VT_VARCOLOR1:
      readThemeColour(reader, tokenId, varClrSch.m_varColor1);
      break;
    case XML_VT_VARCOLOR2:
      readThemeColour(reader, tokenId, varClrSch.m_varColor2);
      break;
    case XML_VT_VARCOLOR3:
      readThemeColour(reader, tokenId, varClrSch.m_varColor3);
      break;
    case XML_VT_VARCOLOR4:
      readThemeColour(reader, tokenId, varClrSch.m_varColor4);
      break;
    case XML_VT_VARCOLOR5:
      readThemeColour(reader, tokenId, varClrSch.m_varColor5);
      break;
    case XML_VT_VARCOLOR6:
      readThemeColour(reader, tokenId, varClrSch.m_varColor6);
      break;
    case XML_VT_VARCOLOR7:
      readThemeColour(reader, tokenId, varClrSch.m_varColor7);
      break;
    default:
      break;
    }
  }
  while ((XML_VT_VARIATIONCLRSCHEME != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

boost::optional<libvisio::Colour> libvisio::VSDXTheme::getThemeColour(unsigned value, unsigned variationIndex) const
{
  if (value < 100)
  {
    switch (value)
    {
    case 0:
      return m_clrScheme.m_dk1;
    case 1:
      return m_clrScheme.m_lt1;
    case 2:
      return m_clrScheme.m_accent1;
    case 3:
      return m_clrScheme.m_accent2;
    case 4:
      return m_clrScheme.m_accent3;
    case 5:
      return m_clrScheme.m_accent4;
    case 6:
      return m_clrScheme.m_accent5;
    case 7:
      return m_clrScheme.m_accent6;
    case 8:
      return m_clrScheme.m_bkgnd;
    default:
      break;
    }
  }
  else if (!m_clrScheme.m_variationClrSchemeLst.empty())
  {
    if (variationIndex >= m_clrScheme.m_variationClrSchemeLst.size())
      variationIndex = 0;
    switch (value)
    {
    case 100:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor1;
    case 101:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor2;
    case 102:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor3;
    case 103:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor4;
    case 104:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor5;
    case 105:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor6;
    case 106:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor7;
    default:
      break;
    }
  }
  return boost::optional<libvisio::Colour>();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
