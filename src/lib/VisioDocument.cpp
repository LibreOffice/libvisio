/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <librevenge/librevenge.h>
#include <libvisio/libvisio.h>
#include "libvisio_utils.h"
#include "VDXParser.h"
#include "VSDParser.h"
#include "VSDXParser.h"
#include "VSD5Parser.h"
#include "VSD6Parser.h"
#include "VSDXMLHelper.h"

namespace
{

#define VISIO_MAGIC_LENGTH 21

static bool checkVisioMagic(librevenge::RVNGInputStream *input)
{
  int startPosition = (int)input->tell();
  try
  {
    unsigned long numBytesRead = 0;
    const unsigned char *buffer = input->read(VISIO_MAGIC_LENGTH, numBytesRead);
    bool returnValue = true;
    if (VISIO_MAGIC_LENGTH != numBytesRead)
      returnValue = false;
    else if (0x56 != buffer[0])
      returnValue = false;
    else if (0x69 != buffer[1])
      returnValue = false;
    else if (0x73 != buffer[2])
      returnValue = false;
    else if (0x69 != buffer[3])
      returnValue = false;
    else if (0x6f != buffer[4])
      returnValue = false;
    else if (0x20 != buffer[5])
      returnValue = false;
    else if (0x28 != buffer[6])
      returnValue = false;
    else if (0x54 != buffer[7])
      returnValue = false;
    else if (0x4d != buffer[8])
      returnValue = false;
    else if (0x29 != buffer[9])
      returnValue = false;
    else if (0x20 != buffer[10])
      returnValue = false;
    else if (0x44 != buffer[11])
      returnValue = false;
    else if (0x72 != buffer[12])
      returnValue = false;
    else if (0x61 != buffer[13])
      returnValue = false;
    else if (0x77 != buffer[14])
      returnValue = false;
    else if (0x69 != buffer[15])
      returnValue = false;
    else if (0x6e != buffer[16])
      returnValue = false;
    else if (0x67 != buffer[17])
      returnValue = false;
    else if (0x0d != buffer[18])
      returnValue = false;
    else if (0x0a != buffer[19])
      returnValue = false;
    else if (0x00 != buffer[20])
      returnValue = false;
    input->seek(startPosition, librevenge::RVNG_SEEK_SET);
    return returnValue;
  }
  catch (...)
  {
    input->seek(startPosition, librevenge::RVNG_SEEK_SET);
    return false;
  }
}

static bool isBinaryVisioDocument(librevenge::RVNGInputStream *input)
{
  librevenge::RVNGInputStream *docStream = 0;
  try
  {
    input->seek(0, librevenge::RVNG_SEEK_SET);
    if (input->isStructured())
    {
      input->seek(0, librevenge::RVNG_SEEK_SET);
      docStream = input->getSubStreamByName("VisioDocument");
    }
    if (!docStream)
      docStream = input;

    docStream->seek(0, librevenge::RVNG_SEEK_SET);
    unsigned char version = 0;
    if (checkVisioMagic(docStream))
    {
      docStream->seek(0x1A, librevenge::RVNG_SEEK_SET);
      version = libvisio::readU8(docStream);
    }
    input->seek(0, librevenge::RVNG_SEEK_SET);
    if (docStream && docStream != input)
      delete docStream;
    docStream = 0;

    VSD_DEBUG_MSG(("VisioDocument: version %i\n", version));

    // Versions 2k (6) and 2k3 (11)
    if ((version >= 1 && version <= 6) || version == 11)
    {
      return true;
    }
  }
  catch (...)
  {
    if (docStream && docStream != input)
      delete docStream;
    return false;
  }

  return false;
}

static bool parseBinaryVisioDocument(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter, bool isStencilExtraction)
{
  VSD_DEBUG_MSG(("Parsing Binary Visio Document\n"));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  librevenge::RVNGInputStream *docStream = 0;
  if (input->isStructured())
    docStream = input->getSubStreamByName("VisioDocument");
  if (!docStream)
    docStream = input;

  docStream->seek(0x1A, librevenge::RVNG_SEEK_SET);

  libvisio::VSDParser *parser = 0;
  try
  {
    unsigned char version = libvisio::readU8(docStream);
    switch (version)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      parser = new libvisio::VSD5Parser(docStream, painter);
      break;
    case 6:
      parser = new libvisio::VSD6Parser(docStream, painter);
      break;
    case 11:
      parser = new libvisio::VSDParser(docStream, painter);
      break;
    default:
      break;
    }

    bool retValue = false;
    if (parser)
    {
      if (isStencilExtraction)
        retValue = parser->extractStencils();
      else if (!isStencilExtraction)
        retValue = parser->parseMain();
    }
    else
    {
      if (docStream != input)
        delete docStream;
      return false;
    }

    delete parser;
    if (docStream != input)
      delete docStream;

    return retValue;
  }
  catch (...)
  {
    delete parser;
    if (docStream != input)
      delete docStream;
  }

  return false;
}

static bool isOpcVisioDocument(librevenge::RVNGInputStream *input)
{
  librevenge::RVNGInputStream *tmpInput = 0;
  try
  {
    input->seek(0, librevenge::RVNG_SEEK_SET);
    if (!input->isStructured())
      return false;

    tmpInput = input->getSubStreamByName("_rels/.rels");
    if (!tmpInput)
      return false;

    libvisio::VSDXRelationships rootRels(tmpInput);
    delete tmpInput;

    // Check whether the relationship points to a Visio document stream
    const libvisio::VSDXRelationship *rel = rootRels.getRelationshipByType("http://schemas.microsoft.com/visio/2010/relationships/document");
    if (!rel)
      return false;

    // check whether the pointed Visio document stream exists in the document
    tmpInput = input->getSubStreamByName(rel->getTarget().c_str());
    if (!tmpInput)
      return false;
    delete tmpInput;
    return true;
  }
  catch (...)
  {
    if (tmpInput)
      delete tmpInput;
    return false;
  }
}

static bool parseOpcVisioDocument(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter, bool isStencilExtraction)
{
  VSD_DEBUG_MSG(("Parsing Visio Document based on Open Packaging Convention\n"));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  libvisio::VSDXParser parser(input, painter);
  if (isStencilExtraction && parser.extractStencils())
    return true;
  else if (!isStencilExtraction && parser.parseMain())
    return true;
  return false;
}

static bool isXmlVisioDocument(librevenge::RVNGInputStream *input)
{
  xmlTextReaderPtr reader = 0;
  try
  {
    input->seek(0, librevenge::RVNG_SEEK_SET);
    reader = libvisio::xmlReaderForStream(input, 0, 0, XML_PARSE_NOBLANKS|XML_PARSE_NOENT|XML_PARSE_NONET|XML_PARSE_RECOVER);
    if (!reader)
      return false;
    int ret = xmlTextReaderRead(reader);
    while (ret == 1 && 1 != xmlTextReaderNodeType(reader))
      ret = xmlTextReaderRead(reader);
    if (ret != 1)
    {
      xmlFreeTextReader(reader);
      return false;
    }
    const xmlChar *name = xmlTextReaderConstName(reader);
    if (!name)
    {
      xmlFreeTextReader(reader);
      return false;
    }
    if (!xmlStrEqual(name, BAD_CAST("VisioDocument")))
    {
      xmlFreeTextReader(reader);
      return false;
    }

    // Checking the two possible namespaces of VDX documents. This may be a bit strict
    // and filter out some of third party VDX documents. If that happens, commenting out
    // this block could be an option.
    const xmlChar *nsname = xmlTextReaderConstNamespaceUri(reader);
    if (!nsname)
    {
      xmlFreeTextReader(reader);
      return false;
    }
    if (!xmlStrEqual(nsname, BAD_CAST("urn:schemas-microsoft-com:office:visio"))
        && !xmlStrEqual(nsname, BAD_CAST("http://schemas.microsoft.com/visio/2003/core")))
    {
      xmlFreeTextReader(reader);
      return false;
    }

    xmlFreeTextReader(reader);
    return true;
  }
  catch (...)
  {
    if (reader)
      xmlFreeTextReader(reader);
    return false;
  }
}

static bool parseXmlVisioDocument(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter, bool isStencilExtraction)
{
  VSD_DEBUG_MSG(("Parsing Visio DrawingML Document\n"));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  libvisio::VDXParser parser(input, painter);
  if (isStencilExtraction && parser.extractStencils())
    return true;
  else if (!isStencilExtraction && parser.parseMain())
    return true;
  return false;
}

} // anonymous namespace


/**
Analyzes the content of an input stream to see if it can be parsed
\param input The input stream
\return A value that indicates whether the content from the input
stream is a Visio Document that libvisio able to parse
*/
VSDAPI bool libvisio::VisioDocument::isSupported(librevenge::RVNGInputStream *input)
{
  if (isBinaryVisioDocument(input))
    return true;
  if (isOpcVisioDocument(input))
    return true;
  if (isXmlVisioDocument(input))
    return true;
  return false;
}

/**
Parses the input stream content. It will make callbacks to the functions provided by a
librevenge::RVNGDrawingInterface class implementation when needed. This is often commonly called the
'main parsing routine'.
\param input The input stream
\param painter A WPGPainterInterface implementation
\return A value that indicates whether the parsing was successful
*/
VSDAPI bool libvisio::VisioDocument::parse(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter)
{
  if (isBinaryVisioDocument(input))
  {
    if (parseBinaryVisioDocument(input, painter, false))
      return true;
    return false;
  }
  if (isOpcVisioDocument(input))
  {
    if (parseOpcVisioDocument(input, painter, false))
      return true;
    return false;
  }
  if (isXmlVisioDocument(input))
  {
    if (parseXmlVisioDocument(input, painter, false))
      return true;
    return false;
  }
  return false;
}

/**
Parses the input stream content and extracts stencil pages, one stencil page per output page.
It will make callbacks to the functions provided by a librevenge::RVNGDrawingInterface class implementation
when needed.
\param input The input stream
\param painter A WPGPainterInterface implementation
\return A value that indicates whether the parsing was successful
*/
VSDAPI bool libvisio::VisioDocument::parseStencils(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter)
{
  if (isBinaryVisioDocument(input))
  {
    if (parseBinaryVisioDocument(input, painter, true))
      return true;
    return false;
  }
  if (isOpcVisioDocument(input))
  {
    if (parseOpcVisioDocument(input, painter, true))
      return true;
    return false;
  }
  if (isXmlVisioDocument(input))
  {
    if (parseXmlVisioDocument(input, painter, true))
      return true;
    return false;
  }
  return false;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
