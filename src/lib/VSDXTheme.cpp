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

libvisio::VSDXVariationStyleScheme::VSDXVariationStyleScheme()
  : m_varStyles()
{
}

libvisio::VSDXTheme::VSDXTheme()
  : m_clrScheme(),
    m_fontScheme(),
    m_fillStyleLst(std::vector<std::optional<libvisio::Colour>>(6)),
    m_variationStyleSchemeLst()
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
      case XML_A_FMTSCHEME:
        readFmtScheme(reader.get());
        break;
      case XML_VT_VARIATIONSTYLESCHEMELST:
        readVariationStyleSchemeLst(reader.get());
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

std::optional<libvisio::Colour> libvisio::VSDXTheme::readSrgbClr(xmlTextReaderPtr reader)
{
  std::optional<libvisio::Colour> retVal;
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

std::optional<libvisio::Colour> libvisio::VSDXTheme::readSysClr(xmlTextReaderPtr reader)
{
  std::optional<libvisio::Colour> retVal;
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

bool libvisio::VSDXTheme::readThemeColour(xmlTextReaderPtr reader, int idToken, Colour &clr)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  std::optional<libvisio::Colour> colour;
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
  {
    clr = *colour;
    return true;
  }
  return false;
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

std::optional<libvisio::Colour> libvisio::VSDXTheme::getThemeColour(unsigned value, unsigned variationIndex) const
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
  return std::optional<libvisio::Colour>();
}

void libvisio::VSDXTheme::readVariationStyleSchemeLst(xmlTextReaderPtr reader)
{
  VSD_DEBUG_MSG(("VSDXTheme::readVariationStyleSchemeLst\n"));
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  m_variationStyleSchemeLst.clear();
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readVariationStyleSchemeLst: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_VT_VARIATIONSTYLESCHEME:
    {
      VSDXVariationStyleScheme vaStyleSch;
      readVariationStyleScheme(reader, tokenId, vaStyleSch);
      m_variationStyleSchemeLst.push_back(vaStyleSch);
      break;
    }
    default:
      break;
    }
  }
  while ((XML_VT_VARIATIONCLRSCHEMELST != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXTheme::readVariationStyleScheme(xmlTextReaderPtr reader, int idToken, VSDXVariationStyleScheme &vaStyleSch)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  size_t iVNum = 0;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readVariationStyleScheme: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
      case XML_VT_VARSTYLE:
      {
        if (vaStyleSch.m_varStyles.size() > iVNum)
        {
          readVarIdx(reader, vaStyleSch.m_varStyles[iVNum++]);
        }
        else
        {
          VSD_DEBUG_MSG(("Only four XML_VT_VARSTYLE can exists.\n"));
        }
        break;
      }
      default:
        break;
    }
  }
  while ((idToken != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXTheme::readVarIdx(xmlTextReaderPtr reader, std::array<unsigned, 4>& varStyle)
{
  if (XML_VT_VARSTYLE == getElementToken(reader))
  {
    // https://learn.microsoft.com/en-us/openspecs/sharepoint_protocols/ms-vsdx/07c6a18a-16ec-4c36-942b-8c611dd3d140
    const shared_ptr<xmlChar> valFillIdx(xmlTextReaderGetAttribute(reader, BAD_CAST("fillIdx")), xmlFree);
    varStyle[0] = valFillIdx ? (unsigned)xmlStringToLong(valFillIdx) : MINUS_ONE;

    const shared_ptr<xmlChar> valLineIdx(xmlTextReaderGetAttribute(reader, BAD_CAST("lineIdx")), xmlFree);
    varStyle[1] = valLineIdx ? (unsigned)xmlStringToLong(valLineIdx) : MINUS_ONE;

    const shared_ptr<xmlChar> valEffIdx(xmlTextReaderGetAttribute(reader, BAD_CAST("effectIdx")), xmlFree);
    varStyle[2] = valEffIdx ? (unsigned)xmlStringToLong(valEffIdx) : MINUS_ONE;

    const shared_ptr<xmlChar> valFontIdx(xmlTextReaderGetAttribute(reader, BAD_CAST("fontIdx")), xmlFree);
    varStyle[3] = valFontIdx ? (unsigned)xmlStringToLong(valFontIdx) : MINUS_ONE;
  }
}

std::optional<libvisio::Colour> libvisio::VSDXTheme::getStyleColour(unsigned value, unsigned variationIndex) const
{
  if (!m_variationStyleSchemeLst.empty())
  {
    // https://learn.microsoft.com/en-us/openspecs/sharepoint_protocols/ms-vsdx/25689058-b1e7-4d3c-a833-0a4c7180f5f2
    if (variationIndex >= m_variationStyleSchemeLst.size())
      variationIndex = 0;
    switch (value)
    {
    case 100:
    case 200:
    {
      std::array<unsigned, 4> varStyle = m_variationStyleSchemeLst[variationIndex].m_varStyles[0];
      return getFillStyleColour(varStyle[0]);
    }
    case 101:
    case 201:
    {
      std::array<unsigned, 4> varStyle = m_variationStyleSchemeLst[variationIndex].m_varStyles[1];
      return getFillStyleColour(varStyle[0]);
    }
    case 102:
    case 202:
    {
      std::array<unsigned, 4> varStyle = m_variationStyleSchemeLst[variationIndex].m_varStyles[2];
      return getFillStyleColour(varStyle[0]);
    }
    case 103:
    case 203:
    {
      std::array<unsigned, 4> varStyle = m_variationStyleSchemeLst[variationIndex].m_varStyles[3];
      return getFillStyleColour(varStyle[0]);
    }
    default:
      break;
    }
  }
  return std::optional<libvisio::Colour>();
}

void libvisio::VSDXTheme::readFmtScheme(xmlTextReaderPtr reader)
{
  VSD_DEBUG_MSG(("VSDXTheme::readFmtScheme\n"));
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readFmtScheme: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_A_FILLSTYLELST:
    {
      readFillStyleLst(reader);
      break;
    }
    default:
      // Other style lists not implemented
      break;
    }
  }
  while ((XML_A_FMTSCHEME != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXTheme::skipUnimplemented(xmlTextReaderPtr reader, int idToken)
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
      VSD_DEBUG_MSG(("VSDXTheme::skipUnimplemented: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
  }
  while ((idToken != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXTheme::readFillStyleLst(xmlTextReaderPtr reader)
{
  VSD_DEBUG_MSG(("VSDXTheme::readFillStyleLst\n"));
  int ret = xmlTextReaderRead(reader);
  int tokenId = getElementToken(reader);
  if (XML_TOKEN_INVALID == tokenId)
  {
    VSD_DEBUG_MSG(("VSDXTheme::readFillStyleLst: unknown token %s\n", xmlTextReaderConstName(reader)));
  }
  int tokenType = xmlTextReaderNodeType(reader);
  std::size_t i = 0;
  while ((XML_A_FILLSTYLELST != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret)
  {
    switch (tokenId)
    {
    case XML_A_SOLIDFILL:
    {
      Colour colour;
      if (readThemeColour(reader, tokenId, colour))
      {
        if (i < m_fillStyleLst.size())
        {
          m_fillStyleLst[i] = colour;
        }
        else
        {
          VSD_DEBUG_MSG(("VSDXTheme::readFillStyleLst Error: Unable to add colour #%02x%02x%02x\n", colour.r, colour.g, colour.b));
        }
      }
      break;
    }
    default:
      // Skip unimplemented fill type
      skipUnimplemented(reader, tokenId);
      break;
    }
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXTheme::readFillStyleLst: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    i++;
  }
}

std::optional<libvisio::Colour> libvisio::VSDXTheme::getFillStyleColour(unsigned value) const
{
  if (value == 0 || value > m_fillStyleLst.size())
    return std::optional<libvisio::Colour>();
  return m_fillStyleLst[value - 1];
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
