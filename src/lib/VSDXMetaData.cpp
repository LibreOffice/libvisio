/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDXMetaData.h"
#include "VSDXMLTokenMap.h"
#include "libvisio_utils.h"

libvisio::VSDXMetaData::VSDXMetaData()
  : m_metaData()
{
}

libvisio::VSDXMetaData::~VSDXMetaData()
{
}

librevenge::RVNGString libvisio::VSDXMetaData::readString(xmlTextReaderPtr reader, int stringTokenId)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  librevenge::RVNGString string;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    tokenType = xmlTextReaderNodeType(reader);
    if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT)
      string.append((const char *)xmlTextReaderConstValue(reader));
  }
  while ((stringTokenId != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  return string;
}

void libvisio::VSDXMetaData::readCoreProperties(xmlTextReaderPtr reader)
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
      VSD_DEBUG_MSG(("VSDXMetaData::readCoreProperties: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    if (tokenType != XML_READER_TYPE_ELEMENT)
      continue;

    switch (tokenId)
    {
    case XML_DC_TITLE:
      m_metaData.insert("dc:title", readString(reader, XML_DC_TITLE));
      break;
    case XML_DC_SUBJECT:
      m_metaData.insert("dc:subject", readString(reader, XML_DC_SUBJECT));
      break;
    case XML_DC_CREATOR:
      m_metaData.insert("meta:initial-creator", readString(reader, XML_DC_CREATOR));
      break;
    case XML_DCTERMS_CREATED:
      m_metaData.insert("meta:creation-date", readString(reader, XML_DCTERMS_CREATED));
      break;
    case XML_DCTERMS_MODIFIED:
      m_metaData.insert("dc:date", readString(reader, XML_DCTERMS_MODIFIED));
      break;
    case XML_CP_KEYWORDS:
      m_metaData.insert("meta:keyword", readString(reader, XML_CP_KEYWORDS));
      break;
    case XML_DC_DESCRIPTION:
      m_metaData.insert("dc:description", readString(reader, XML_DC_DESCRIPTION));
      break;
    case XML_CP_LASTMODIFIEDBY:
      m_metaData.insert("dc:creator", readString(reader, XML_CP_LASTMODIFIEDBY));
      break;
    default:
      break;
    }
  }
  while ((XML_CP_COREPROPERTIES != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

bool libvisio::VSDXMetaData::parse(librevenge::RVNGInputStream *input)
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
      case XML_CP_COREPROPERTIES:
        readCoreProperties(reader);
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

int libvisio::VSDXMetaData::getElementToken(xmlTextReaderPtr reader)
{
  return VSDXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
}

const librevenge::RVNGPropertyList &libvisio::VSDXMetaData::getMetaData()
{
  return m_metaData;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
