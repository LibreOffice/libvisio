/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDMetaData.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <unicode/ucnv.h>
#include <ctime>

libvisio::VSDMetaData::VSDMetaData()
  : m_idsAndOffsets(), m_typedPropertyValues(), m_metaData()
{
}

libvisio::VSDMetaData::~VSDMetaData()
{
}

enum PIDDSI
{
  PIDDSI_CODEPAGE          = 0x00000001,
  PIDDSI_CATEGORY          = 0x00000002,
  PIDDSI_PRESFORMAT        = 0x00000003,
  PIDDSI_BYTECOUNT         = 0x00000004,
  PIDDSI_LINECOUNT         = 0x00000005,
  PIDDSI_PARACOUNT         = 0x00000006,
  PIDDSI_SLIDECOUNT        = 0x00000007,
  PIDDSI_NOTECOUNT         = 0x00000008,
  PIDDSI_HIDDENCOUNT       = 0x00000009,
  PIDDSI_MMCLIPCOUNT       = 0x0000000A,
  PIDDSI_SCALE             = 0x0000000B,
  PIDDSI_HEADINGPAIR       = 0x0000000C,
  PIDDSI_DOCPARTS          = 0x0000000D,
  PIDDSI_MANAGER           = 0x0000000E,
  PIDDSI_COMPANY           = 0x0000000F,
  PIDDSI_LINKSDIRTY        = 0x00000010,
  PIDDSI_CCHWITHSPACES     = 0x00000011,
  PIDDSI_SHAREDDOC         = 0x00000013,
  PIDDSI_LINKBASE          = 0x00000014,
  PIDDSI_HLINKS            = 0x00000015,
  PIDDSI_HYPERLINKSCHANGED = 0x00000016,
  PIDDSI_VERSION           = 0x00000017,
  PIDDSI_DIGSIG            = 0x00000018,
  PIDDSI_CONTENTTYPE       = 0x0000001A,
  PIDDSI_CONTENTSTATUS     = 0x0000001B,
  PIDDSI_LANGUAGE          = 0x0000001C,
  PIDDSI_DOCVERSION        = 0x0000001D
};

enum PIDSI
{
  CODEPAGE_PROPERTY_IDENTIFIER = 0x00000001,
  PIDSI_TITLE                  = 0x00000002,
  PIDSI_SUBJECT                = 0x00000003,
  PIDSI_AUTHOR                 = 0x00000004,
  PIDSI_KEYWORDS               = 0x00000005,
  PIDSI_COMMENTS               = 0x00000006,
  PIDSI_TEMPLATE               = 0x00000007,
  PIDSI_LASTAUTHOR             = 0x00000008,
  PIDSI_REVNUMBER              = 0x00000009,
  PIDSI_EDITTIME               = 0x0000000A,
  PIDSI_LASTPRINTED            = 0x0000000B,
  PIDSI_CREATE_DTM             = 0x0000000C,
  PIDSI_LASTSAVE_DTM           = 0x0000000D,
  PIDSI_PAGECOUNT              = 0x0000000E,
  PIDSI_WORDCOUNT              = 0x0000000F,
  PIDSI_CHARCOUNT              = 0x00000010,
  PIDSI_THUMBNAIL              = 0x00000011,
  PIDSI_APPNAME                = 0x00000012,
  PIDSI_DOC_SECURITY           = 0x00000013
};

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
  //input->seek(16, librevenge::RVNG_SEEK_CUR);
  uint32_t data1 = readU32(input);
  uint16_t data2 = readU16(input);
  uint16_t data3 = readU16(input);
  uint8_t data4[8];
  for (unsigned char &i : data4)
  {
    i = readU8(input);
  }
  // Pretty-printed GUID is 36 bytes + the terminating null-character.
  char FMTID0[37];
  sprintf(FMTID0, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", data1, data2, data3,
          data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]);

  uint32_t offset0 = readU32(input);
  readPropertySet(input, offset0, FMTID0);
}

void libvisio::VSDMetaData::readPropertySet(librevenge::RVNGInputStream *input, uint32_t offset, char *FMTID)
{
  input->seek(offset, librevenge::RVNG_SEEK_SET);

  // Size
  input->seek(4, librevenge::RVNG_SEEK_CUR);
  uint32_t numProperties = readU32(input);
  // The exact size of a property is not known beforehand: check upper bound
  if (numProperties > getRemainingLength(input) / 12)
    numProperties = getRemainingLength(input) / 12;
  for (uint32_t i = 0; i < numProperties; ++i)
    readPropertyIdentifierAndOffset(input);
  for (uint32_t i = 0; i < numProperties; ++i)
  {
    if (i >= m_idsAndOffsets.size())
      break;
    readTypedPropertyValue(input, i, offset + m_idsAndOffsets[i].second, FMTID);
  }
}

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

void libvisio::VSDMetaData::readTypedPropertyValue(librevenge::RVNGInputStream *input,
                                                   uint32_t index,
                                                   uint32_t offset,
                                                   char *FMTID)
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

      if (!strcmp(FMTID, "f29f85e0-4ff9-1068-ab91-08002b27b3d9"))
      {
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
        case PIDSI_TEMPLATE:
          std::string templateHref(string.cstr());
          size_t found = templateHref.find_last_of("/\\");
          if (found != std::string::npos)
            string = librevenge::RVNGString(templateHref.substr(found+1).c_str());
          m_metaData.insert("librevenge:template", string);
          break;
        }
      }
      else if (!strcmp(FMTID,"d5cdd502-2e9c-101b-9397-08002b2cf9ae"))
      {
        switch (m_idsAndOffsets[index].first)
        {
        case PIDDSI_CATEGORY:
          m_metaData.insert("librevenge:category", string);
          break;
        case PIDDSI_LINECOUNT:
          // this should actually be PIDDSI_COMPANY but this
          // is what company is mapped to
          m_metaData.insert("librevenge:company", string);
          break;
        case PIDDSI_LANGUAGE:
          m_metaData.insert("dc:language", string);
          break;
        }
      }
    }
  }
}

librevenge::RVNGString libvisio::VSDMetaData::readCodePageString(librevenge::RVNGInputStream *input)
{
  uint32_t size = readU32(input);
  if (size > getRemainingLength(input))
    size = getRemainingLength(input);

  if (size == 0)
    return librevenge::RVNGString();

  std::vector<unsigned char> characters;
  for (uint32_t i = 0; i < size; ++i)
    characters.push_back(readU8(input));

  uint32_t codepage = getCodePage();
  librevenge::RVNGString string;

  if (codepage == 65001)
  {
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd374130%28v=vs.85%29.aspx
    // says this is UTF-8.
    characters.push_back(0);
    string.append(reinterpret_cast<const char *>(characters.data()));
  }
  else
  {
    UErrorCode status = U_ZERO_ERROR;
    UConverter *conv = nullptr;

    switch (codepage)
    {
    case 1252:
      // http://msdn.microsoft.com/en-us/goglobal/bb964654
      conv = ucnv_open("windows-1252", &status);
      break;
    }

    if (U_SUCCESS(status) && conv)
    {
      assert(!characters.empty());
      const auto *src = (const char *)&characters[0];
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
  size_t sectorSize = std::pow(2, sectorShift);
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
  const uint64_t epoch = uint64_t(116444736UL) * 100;
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
