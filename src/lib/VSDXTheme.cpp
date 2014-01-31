/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

bool libvisio::VSDXTheme::parse(librevenge::RVNGInputStream *input)
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
    case XML_A_SRGBCLR:
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
    case XML_VT_VARIATIONSTYLESCHEME:
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
  while ((XML_VT_VARIATIONSTYLESCHEMELST != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
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
  while ((XML_VT_VARIATIONSTYLESCHEME != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
