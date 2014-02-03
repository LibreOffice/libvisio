/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* libvisio
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */

#include <string>
#include <libwpd-stream/libwpd-stream.h>
#include <libvisio/libvisio.h>
#include "libvisio_utils.h"
#include "VDXParser.h"
#include "VSDSVGGenerator.h"
#include "VSDParser.h"
#include "VSDXParser.h"
#include "VSD5Parser.h"
#include "VSD6Parser.h"
#include "VSDXMLHelper.h"
#include "VSDZipStream.h"

namespace
{

#define VISIO_MAGIC_LENGTH 21

static bool checkVisioMagic(WPXInputStream *input)
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
    input->seek(startPosition, WPX_SEEK_SET);
    return returnValue;
  }
  catch (...)
  {
    input->seek(startPosition, WPX_SEEK_SET);
    return false;
  }
}

static bool isBinaryVisioDocument(WPXInputStream *input)
{
  WPXInputStream *docStream = 0;
  try
  {
    input->seek(0, WPX_SEEK_SET);
    if (input->isOLEStream())
    {
      input->seek(0, WPX_SEEK_SET);
      docStream = input->getDocumentOLEStream("VisioDocument");
    }
    if (!docStream)
      docStream = input;

    docStream->seek(0, WPX_SEEK_SET);
    unsigned char version = 0;
    if (checkVisioMagic(docStream))
    {
      docStream->seek(0x1A, WPX_SEEK_SET);
      version = libvisio::readU8(docStream);
    }
    input->seek(0, WPX_SEEK_SET);
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

static bool parseBinaryVisioDocument(WPXInputStream *input, libwpg::WPGPaintInterface *painter, bool isStencilExtraction)
{
  VSD_DEBUG_MSG(("Parsing Binary Visio Document\n"));
  input->seek(0, WPX_SEEK_SET);
  WPXInputStream *docStream = 0;
  if (input->isOLEStream())
    docStream = input->getDocumentOLEStream("VisioDocument");
  if (!docStream)
    docStream = input;

  docStream->seek(0x1A, WPX_SEEK_SET);

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

static bool isOpcVisioDocument(WPXInputStream *input)
{
  WPXInputStream *tmpInput = 0;
  try
  {
    input->seek(0, WPX_SEEK_SET);
    libvisio::VSDZipStream zinput(input);
    // Kidnapping the OLE document API and extending it to support zip files.
    if (!zinput.isOLEStream())
      return false;

    tmpInput = zinput.getDocumentOLEStream("_rels/.rels");
    if (!tmpInput)
      return false;

    libvisio::VSDXRelationships rootRels(tmpInput);
    delete tmpInput;

    // Check whether the relationship points to a Visio document stream
    const libvisio::VSDXRelationship *rel = rootRels.getRelationshipByType("http://schemas.microsoft.com/visio/2010/relationships/document");
    if (!rel)
      return false;

    // check whether the pointed Visio document stream exists in the document
    tmpInput = zinput.getDocumentOLEStream(rel->getTarget().c_str());
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

static bool parseOpcVisioDocument(WPXInputStream *input, libwpg::WPGPaintInterface *painter, bool isStencilExtraction)
{
  VSD_DEBUG_MSG(("Parsing Visio Document based on Open Packaging Convention\n"));
  input->seek(0, WPX_SEEK_SET);
  libvisio::VSDXParser parser(input, painter);
  if (isStencilExtraction && parser.extractStencils())
    return true;
  else if (!isStencilExtraction && parser.parseMain())
    return true;
  return false;
}

static bool isXmlVisioDocument(WPXInputStream *input)
{
  xmlTextReaderPtr reader = 0;
  try
  {
    input->seek(0, WPX_SEEK_SET);
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

static bool parseXmlVisioDocument(WPXInputStream *input, libwpg::WPGPaintInterface *painter, bool isStencilExtraction)
{
  VSD_DEBUG_MSG(("Parsing Visio DrawingML Document\n"));
  input->seek(0, WPX_SEEK_SET);
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
bool libvisio::VisioDocument::isSupported(WPXInputStream *input)
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
WPGPaintInterface class implementation when needed. This is often commonly called the
'main parsing routine'.
\param input The input stream
\param painter A WPGPainterInterface implementation
\return A value that indicates whether the parsing was successful
*/
bool libvisio::VisioDocument::parse(::WPXInputStream *input, libwpg::WPGPaintInterface *painter)
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
It will make callbacks to the functions provided by a WPGPaintInterface class implementation
when needed.
\param input The input stream
\param painter A WPGPainterInterface implementation
\return A value that indicates whether the parsing was successful
*/
bool libvisio::VisioDocument::parseStencils(::WPXInputStream *input, libwpg::WPGPaintInterface *painter)
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


/**
Parses the input stream content and generates a valid Scalable Vector Graphics
Provided as a convenience function for applications that support SVG internally.
\param input The input stream
\param output The output string whose content is the resulting SVG
\return A value that indicates whether the SVG generation was successful.
*/
bool libvisio::VisioDocument::generateSVG(::WPXInputStream *input, libvisio::VSDStringVector &output)
{
  libvisio::VSDSVGGenerator generator(output);
  bool result = libvisio::VisioDocument::parse(input, &generator);
  return result;
}

/**
Parses the input stream content and extracts stencil pages. It generates a valid
Scalable Vector Graphics document per stencil.
Provided as a convenience function for applications that support SVG internally.
\param input The input stream
\param output The output string whose content is the resulting SVG
\return A value that indicates whether the SVG generation was successful.
*/
bool libvisio::VisioDocument::generateSVGStencils(::WPXInputStream *input, libvisio::VSDStringVector &output)
{
  libvisio::VSDSVGGenerator generator(output);
  bool result = libvisio::VisioDocument::parseStencils(input, &generator);
  return result;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
