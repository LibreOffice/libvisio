/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDMetaData.h"
#include <cmath>
#include <unicode/ucnv.h>
#include <ctime>

libvisio::VSDMetaData::VSDMetaData()
  : m_idsAndOffsets(), m_typedPropertyValues(), m_metaData()
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
#define PIDSI_SUBJECT 0x00000003
#define PIDSI_AUTHOR 0x00000004
#define PIDSI_KEYWORDS 0x00000005
#define PIDSI_COMMENTS 0x00000006

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
      case PIDSI_SUBJECT:
        m_metaData.insert("dc:subject", string);
        break;
      case PIDSI_AUTHOR:
        m_metaData.insert("meta:initial-creator", string);
        m_metaData.insert("dc:creator", string);
        break;
      case PIDSI_KEYWORDS:
        m_metaData.insert("meta:keyword", string);
        break;
      case PIDSI_COMMENTS:
        m_metaData.insert("dc:description", string);
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

bool libvisio::VSDMetaData::parseTimes(librevenge::RVNGInputStream *input)
{
  // Parse the header
  // HeaderSignature: 8 bytes
  // HeaderCLSID: 16 bytes
  // MinorVersion: 2 bytes
  // MajorVersion: 2 bytes
  // ByteOrder: 2 bytes
  input->seek(30, librevenge::RVNG_SEEK_CUR);
  uint16_t sectorShift = readU16(input);
  // MiniSectorShift: 2 bytes
  // Reserved: 6 bytes
  // NumDirectorySectors: 4 bytes
  // NumFATSectors: 4 bytes
  input->seek(16, librevenge::RVNG_SEEK_CUR);
  uint32_t firstDirSectorLocation = readU32(input);

  // Seek to the Root Directory Entry
  size_t sectorSize = pow(2, sectorShift);
  input->seek((firstDirSectorLocation + 1) * sectorSize, librevenge::RVNG_SEEK_SET);
  // DirectoryEntryName: 64 bytes
  // DirectoryEntryNameLength: 2 bytes
  // ObjectType: 1 byte
  // ColorFlag: 1 byte
  // LeftSiblingID: 4 bytes
  // RightSiblingID: 4 bytes
  // ChildID: 4 bytes
  // CLSID: 16 bytes
  // StateBits: 4 bytes
  // CreationTime: 8 bytes
  input->seek(108, librevenge::RVNG_SEEK_CUR);
  uint64_t modifiedTime = readU64(input);

  // modifiedTime is number of 100ns since Jan 1 1601
  static const uint64_t epoch = 11644473600;
  time_t sec = (modifiedTime / 10000000) - epoch;
  const struct tm *time = localtime(&sec);
  if (time)
  {
    static const int MAX_BUFFER = 1024;
    char buffer[MAX_BUFFER];
    strftime(&buffer[0], MAX_BUFFER-1, "%Y-%m-%dT%H:%M:%SZ", time);
    librevenge::RVNGString result;
    result.append(buffer);
    // Visio UI uses modifiedTime for both purposes.
    m_metaData.insert("meta:creation-date", result);
    m_metaData.insert("dc:date", result);
    return true;
  }
  return false;
}

const librevenge::RVNGPropertyList &libvisio::VSDMetaData::getMetaData()
{
  return m_metaData;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
