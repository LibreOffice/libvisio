/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string>

#include <librevenge/librevenge.h>
#include <libvisio/libvisio.h>
#include "libvisio_utils.h"
#include "libvisio_xml.h"
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
  std::shared_ptr<librevenge::RVNGInputStream> docStream;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (input->isStructured())
  {
    input->seek(0, librevenge::RVNG_SEEK_SET);
    docStream.reset(input->getSubStreamByName("VisioDocument"));
  }
  if (!docStream)
    docStream.reset(input, libvisio::VSDDummyDeleter());

  docStream->seek(0, librevenge::RVNG_SEEK_SET);
  unsigned char version = 0;
  if (checkVisioMagic(docStream.get()))
  {
    docStream->seek(0x1A, librevenge::RVNG_SEEK_SET);
    version = libvisio::readU8(docStream.get());
  }
  input->seek(0, librevenge::RVNG_SEEK_SET);

  VSD_DEBUG_MSG(("VisioDocument: version %i\n", version));

  // Versions 2k (6) and 2k3 (11)
  return ((version >= 1 && version <= 6) || version == 11);
}

static bool parseBinaryVisioDocument(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter, bool isStencilExtraction)
{
  VSD_DEBUG_MSG(("Parsing Binary Visio Document\n"));
  input->seek(0, librevenge::RVNG_SEEK_SET);
  std::shared_ptr<librevenge::RVNGInputStream> docStream;
  if (input->isStructured())
    docStream.reset(input->getSubStreamByName("VisioDocument"));
  if (!docStream)
    docStream.reset(input, libvisio::VSDDummyDeleter());

  docStream->seek(0x1A, librevenge::RVNG_SEEK_SET);

  std::unique_ptr<libvisio::VSDParser> parser;

  unsigned char version = libvisio::readU8(docStream.get());
  switch (version)
  {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    parser.reset(new libvisio::VSD5Parser(docStream.get(), painter));
    break;
  case 6:
    parser.reset(new libvisio::VSD6Parser(docStream.get(), painter));
    break;
  case 11:
    parser.reset(new libvisio::VSDParser(docStream.get(), painter, input));
    break;
  default:
    break;
  }

  if (isStencilExtraction)
    return parser->extractStencils();
  else
    return parser->parseMain();
}

static bool isOpcVisioDocument(librevenge::RVNGInputStream *input)
{
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (!input->isStructured())
    return false;

  std::unique_ptr<librevenge::RVNGInputStream> tmpInput(input->getSubStreamByName("_rels/.rels"));
  if (!tmpInput)
    return false;

  libvisio::VSDXRelationships rootRels(tmpInput.get());

  // Check whether the relationship points to a Visio document stream
  const libvisio::VSDXRelationship *rel = rootRels.getRelationshipByType("http://schemas.microsoft.com/visio/2010/relationships/document");
  if (!rel)
    return false;

  // check whether the pointed Visio document stream exists in the document
  tmpInput.reset(input->getSubStreamByName(rel->getTarget().c_str()));
  return bool(tmpInput);
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
  try
  {
    input->seek(0, librevenge::RVNG_SEEK_SET);
    const std::shared_ptr<xmlTextReader> reader(
      libvisio::xmlReaderForStream(input, 0, 0, XML_PARSE_NOBLANKS|XML_PARSE_NOENT|XML_PARSE_NONET|XML_PARSE_RECOVER),
      xmlFreeTextReader);
    if (!reader)
      return false;
    int ret = xmlTextReaderRead(reader.get());
    while (ret == 1 && 1 != xmlTextReaderNodeType(reader.get()))
      ret = xmlTextReaderRead(reader.get());
    if (ret != 1)
    {
      return false;
    }
    const xmlChar *name = xmlTextReaderConstName(reader.get());
    if (!name)
    {
      return false;
    }
    if (!xmlStrEqual(name, BAD_CAST("VisioDocument")))
    {
      return false;
    }
    return true;
  }
  catch (...)
  {
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
  if (!input)
    return false;

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
  if (!input || !painter)
    return false;

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
  if (!input || !painter)
    return false;

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
