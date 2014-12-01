/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDMetaData.h"
#include <unicode/ucnv.h>

libvisio::VSDMetaData::VSDMetaData()
{
}

libvisio::VSDMetaData::~VSDMetaData()
{
}

bool libvisio::VSDMetaData::parse(librevenge::RVNGInputStream *input)
{
  if (!input)
    return false;

  readPropertySetStream(input);

  return true;
}

void libvisio::VSDMetaData::readPropertySetStream(librevenge::RVNGInputStream *input)
{
  // ByteOrder
  input->seek(2, librevenge::RVNG_SEEK_CUR);
  // Version
  input->seek(2, librevenge::RVNG_SEEK_CUR);
  // SystemIdentifier
  input->seek(4, librevenge::RVNG_SEEK_CUR);
  // CLSID
  input->seek(16, librevenge::RVNG_SEEK_CUR);
  // NumPropertySets
  input->seek(4, librevenge::RVNG_SEEK_CUR);
  // FMTID0
  input->seek(16, librevenge::RVNG_SEEK_CUR);
  uint32_t offset0 = readU32(input);
  readPropertySet(input, offset0);
}

void libvisio::VSDMetaData::readPropertySet(librevenge::RVNGInputStream *input, uint32_t offset)
{
  input->seek(offset, librevenge::RVNG_SEEK_SET);

  // Size
  input->seek(4, librevenge::RVNG_SEEK_CUR);
  uint32_t numProperties = readU32(input);
  for (uint32_t i = 0; i < numProperties; ++i)
    readPropertyIdentifierAndOffset(input);
  for (uint32_t i = 0; i < numProperties; ++i)
  {
    if (i >= m_idsAndOffsets.size())
      break;
    readTypedPropertyValue(input, i, offset + m_idsAndOffsets[i].second);
  }
}

#define CODEPAGE_PROPERTY_IDENTIFIER 0x00000001

uint32_t libvisio::VSDMetaData::getCodePage()
{
  for (size_t i = 0; i < m_idsAndOffsets.size(); ++i)
  {
    if (m_idsAndOffsets[i].first == CODEPAGE_PROPERTY_IDENTIFIER)
    {
      if (i >= m_typedPropertyValues.size())
        break;
      return m_typedPropertyValues[i];
    }
  }

  return 0;
}

void libvisio::VSDMetaData::readPropertyIdentifierAndOffset(librevenge::RVNGInputStream *input)
{
  uint32_t propertyIdentifier = readU32(input);
  uint32_t offset = readU32(input);
  m_idsAndOffsets.push_back(std::make_pair(propertyIdentifier, offset));
}

#define VT_I2 0x0002
#define VT_LPSTR 0x001E

#define PIDSI_TITLE 0x00000002

void libvisio::VSDMetaData::readTypedPropertyValue(librevenge::RVNGInputStream *input, uint32_t index, uint32_t offset)
{
  input->seek(offset, librevenge::RVNG_SEEK_SET);
  uint16_t type = readU16(input);
  // Padding
  input->seek(2, librevenge::RVNG_SEEK_CUR);

  if (type == VT_I2)
  {
    uint16_t value = readU16(input);
    m_typedPropertyValues[index] = value;
  }
  else if (type == VT_LPSTR)
  {
    librevenge::RVNGString string = readCodePageString(input);
    if (!string.empty())
    {
      if (index >= m_idsAndOffsets.size())
        return;

      switch (m_idsAndOffsets[index].first)
      {
      case PIDSI_TITLE:
        m_metaData.insert("dc:title", string);
        break;
      }
    }
  }
}

librevenge::RVNGString libvisio::VSDMetaData::readCodePageString(librevenge::RVNGInputStream *input)
{
  uint32_t size = readU32(input);

  std::vector<unsigned char> characters;
  for (uint32_t i = 0; i < size; ++i)
    characters.push_back(readU8(input));

  uint32_t codepage = getCodePage();
  librevenge::RVNGString string;

  if (codepage == 65001)
  {
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd374130%28v=vs.85%29.aspx
    // says this is UTF-8.
    for (std::vector<unsigned char>::const_iterator i = characters.begin(); i != characters.end(); ++i)
      string.append((const char)*i);
  }
  else
  {
    UErrorCode status = U_ZERO_ERROR;
    UConverter *conv = 0;

    switch (codepage)
    {
    case 1252:
      // http://msdn.microsoft.com/en-us/goglobal/bb964654
      conv = ucnv_open("windows-1252", &status);
      break;
    }

    if (U_SUCCESS(status) && conv)
    {
      const char *src = (const char *)&characters[0];
      const char *srcLimit = (const char *)src + characters.size();
      while (src < srcLimit)
      {
        UChar32 ucs4Character = ucnv_getNextUChar(conv, &src, srcLimit, &status);
        if (U_SUCCESS(status) && U_IS_UNICODE_CHAR(ucs4Character))
          appendUCS4(string, ucs4Character);
      }
    }

    if (conv)
      ucnv_close(conv);
  }

  return string;
}

const librevenge::RVNGPropertyList &libvisio::VSDMetaData::getMetaData()
{
  return m_metaData;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
