/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDXTheme.h"

#include <memory>

#include "VSDXMLTokenMap.h"
#include "libvisio_utils.h"
#include "libvisio_xml.h"

using std::shared_ptr;

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

libvisio::VSDXFont::VSDXFont()
  : m_latinTypeFace(),
    m_eaTypeFace(),
    m_csTypeFace(),
    m_typeFaces()
{
}

libvisio::VSDXFontScheme::VSDXFontScheme()
  : m_majorFont(),
    m_minorFont(),
    m_schemeId(0)
{
}

libvisio::VSDXTheme::VSDXTheme()
  : m_clrScheme(),
    m_fontScheme()
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
  VSD_DEBUG_MSG(("VSDXTheme::parse\n"));
  if (!input)
    return false;

  auto reader = xmlReaderForStream(input, nullptr, false);
  if (!reader)
    return false;

  try
  {
    int ret = xmlTextReaderRead(reader.get());
    while (1 == ret)
    {
      int tokenId = getElementToken(reader.get());

      switch (tokenId)
      {
      case XML_A_CLRSCHEME:
        readClrScheme(reader.get());
        break;
      case XML_A_FONTSCHEME:
        readFontScheme(reader.get());
        break;
      default:
        break;
      }
      ret = xmlTextReaderRead(reader.get());
    }
  }
  catch (...)
  {
    return false;
  }
  return true;
}

boost::optional<libvisio::Colour> libvisio::VSDXTheme::readSrgbClr(xmlTextReaderPtr reader)
{
  boost::optional<libvisio::Colour> retVal;
  if (XML_A_SRGBCLR == getElementToken(reader))
  {
    const shared_ptr<xmlChar> val(xmlTextReaderGetAttribute(reader, BAD_CAST("val")), xmlFree);
    if (val)
    {
      try
      {
        retVal = xmlStringToColour(val);
      }
      catch (const XmlParserException &)
      {
      }
    }
  }
  return retVal;
}

boost::optional<libvisio::Colour> libvisio::VSDXTheme::readSysClr(xmlTextReaderPtr reader)
{
  boost::optional<libvisio::Colour> retVal;
  if (XML_A_SYSCLR == getElementToken(reader))
  {
    const shared_ptr<xmlChar> lastClr(xmlTextReaderGetAttribute(reader, BAD_CAST("lastClr")), xmlFree);
    if (lastClr)
    {
      try
      {
        retVal = xmlStringToColour(lastClr);
      }
      catch (const XmlParserException &)
      {
      }
    }
  }
  return retVal;
}

void libvisio::VSDXTheme::readFontScheme(xmlTextReaderPtr reader)
{
  VSD_DEBUG_MSG(("VSDXTheme::readFontScheme\n"));
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readFontScheme: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_A_MAJORFONT:
      readFont(reader, tokenId, m_fontScheme.m_majorFont);
      break;
    case XML_A_MINORFONT:
      readFont(reader, tokenId, m_fontScheme.m_minorFont);
      break;
    case XML_VT_SCHEMEID:
      break;
    default:
      break;
    }
  }
  while ((XML_A_FONTSCHEME != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXTheme::readFont(xmlTextReaderPtr reader, int idToken, VSDXFont &font)
{
  VSD_DEBUG_MSG(("VSDXTheme::readFont\n"));
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readFont: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_A_LATIN:
      readTypeFace(reader, font.m_latinTypeFace);
      break;
    case XML_A_EA:
      readTypeFace(reader, font.m_eaTypeFace);
      break;
    case XML_A_CS:
      readTypeFace(reader, font.m_csTypeFace);
      break;
    case XML_A_FONT:
    {
      int script;
      librevenge::RVNGString typeFace;
      if (readTypeFace(reader, script, typeFace) && !typeFace.empty())
        font.m_typeFaces[script] = typeFace;
      break;
    }
    default:
      break;
    }
  }
  while ((idToken != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

bool libvisio::VSDXTheme::readTypeFace(xmlTextReaderPtr reader, librevenge::RVNGString &typeFace)
{
  const shared_ptr<xmlChar> sTypeFace(xmlTextReaderGetAttribute(reader, BAD_CAST("typeface")), xmlFree);
  if (sTypeFace)
  {
    typeFace.clear();
    typeFace.sprintf("%s", (const char *)sTypeFace.get());
  }
  return bool(sTypeFace);
}

bool libvisio::VSDXTheme::readTypeFace(xmlTextReaderPtr reader, int &script, librevenge::RVNGString &typeFace)
{
  const shared_ptr<xmlChar> sScript(xmlTextReaderGetAttribute(reader, BAD_CAST("script")), xmlFree);
  bool knownScript = false;
  if (sScript)
  {
    int token = libvisio::VSDXMLTokenMap::getTokenId(sScript.get());
    knownScript = XML_TOKEN_INVALID != token;
    if (knownScript)
      script = token;
  }
  return readTypeFace(reader, typeFace) && knownScript;
}

void libvisio::VSDXTheme::readClrScheme(xmlTextReaderPtr reader)
{
  VSD_DEBUG_MSG(("VSDXTheme::readClrScheme\n"));
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
  VSD_DEBUG_MSG(("VSDXTheme::readVariationClrSchemeLst\n"));
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
  VSD_DEBUG_MSG(("VSDXTheme::readVariationClrScheme\n"));
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
    case 200:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor1;
    case 101:
    case 201:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor2;
    case 102:
    case 202:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor3;
    case 103:
    case 203:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor4;
    case 104:
    case 204:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor5;
    case 105:
    case 205:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor6;
    case 106:
    case 206:
      return m_clrScheme.m_variationClrSchemeLst[variationIndex].m_varColor7;
    default:
      break;
    }
  }
  return boost::optional<libvisio::Colour>();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
