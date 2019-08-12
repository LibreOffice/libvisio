/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libvisio/libvisio.h>

#include <algorithm>
#include <memory>
#include <string>

#include <librevenge/librevenge.h>
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

static bool checkVisioMagic(librevenge::RVNGInputStream *input)
{
  const unsigned char magic[] =
  {
    0x56, 0x69, 0x73, 0x69, 0x6f, 0x20, 0x28, 0x54, 0x4d, 0x29,
    0x20, 0x44, 0x72, 0x61, 0x77, 0x69, 0x6e, 0x67, 0x0d, 0x0a,
    0x0
  };
  auto startPosition = (int)input->tell();
  unsigned long numBytesRead = 0;
  const unsigned char *buffer = input->read(VSD_NUM_ELEMENTS(magic), numBytesRead);
  const bool returnValue = VSD_NUM_ELEMENTS(magic) == numBytesRead
                           && std::equal(magic, magic + VSD_NUM_ELEMENTS(magic), buffer);
  input->seek(startPosition, librevenge::RVNG_SEEK_SET);
  return returnValue;
}

static bool isBinaryVisioDocument(librevenge::RVNGInputStream *input) try
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
catch (...)
{
  return false;
}

static bool parseBinaryVisioDocument(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter, bool isStencilExtraction) try
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
catch (...)
{
  return false;
}

static bool isOpcVisioDocument(librevenge::RVNGInputStream *input) try
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
catch (...)
{
  return false;
}

static bool parseOpcVisioDocument(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter, bool isStencilExtraction) try
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
catch (...)
{
  return false;
}

static bool isXmlVisioDocument(librevenge::RVNGInputStream *input) try
{
  input->seek(0, librevenge::RVNG_SEEK_SET);
  auto reader = libvisio::xmlReaderForStream(input);
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

static bool parseXmlVisioDocument(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter, bool isStencilExtraction) try
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
catch (...)
{
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
