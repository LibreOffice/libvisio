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

void libvisio::VSDXMetaData::readTitle(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  librevenge::RVNGString title;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    tokenType = xmlTextReaderNodeType(reader);
    if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT)
      title.append((const char *)xmlTextReaderConstValue(reader));
  }
  while ((XML_DC_TITLE != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  m_metaData.insert("dc:title", title);
}

void libvisio::VSDXMetaData::readSubject(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  librevenge::RVNGString subject;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    tokenType = xmlTextReaderNodeType(reader);
    if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT)
      subject.append((const char *)xmlTextReaderConstValue(reader));
  }
  while ((XML_DC_SUBJECT != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  m_metaData.insert("dc:subject", subject);
}

void libvisio::VSDXMetaData::readCreator(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  librevenge::RVNGString creator;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    tokenType = xmlTextReaderNodeType(reader);
    if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT)
      creator.append((const char *)xmlTextReaderConstValue(reader));
  }
  while ((XML_DC_CREATOR != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  m_metaData.insert("meta:initial-creator", creator);
}

void libvisio::VSDXMetaData::readCreated(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  librevenge::RVNGString created;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    tokenType = xmlTextReaderNodeType(reader);
    if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT)
      created.append((const char *)xmlTextReaderConstValue(reader));
  }
  while ((XML_DCTERMS_CREATED != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  m_metaData.insert("meta:creation-date", created);
}

void libvisio::VSDXMetaData::readModified(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  librevenge::RVNGString modified;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    tokenType = xmlTextReaderNodeType(reader);
    if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT)
      modified.append((const char *)xmlTextReaderConstValue(reader));
  }
  while ((XML_DCTERMS_MODIFIED != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  m_metaData.insert("dc:date", modified);
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
    switch (tokenId)
    {
    case XML_DC_TITLE:
      if (tokenType == XML_READER_TYPE_ELEMENT)
        readTitle(reader);
      break;
    case XML_DC_SUBJECT:
      if (tokenType == XML_READER_TYPE_ELEMENT)
        readSubject(reader);
      break;
    case XML_DC_CREATOR:
      if (tokenType == XML_READER_TYPE_ELEMENT)
        readCreator(reader);
      break;
    case XML_DCTERMS_CREATED:
      if (tokenType == XML_READER_TYPE_ELEMENT)
        readCreated(reader);
      break;
    case XML_DCTERMS_MODIFIED:
      if (tokenType == XML_READER_TYPE_ELEMENT)
        readModified(reader);
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
