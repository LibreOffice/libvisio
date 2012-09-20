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

#include <libwpd-stream/libwpd-stream.h>
#include <string>
#include "libvisio.h"
#include "libvisio_utils.h"
#include "VSDSVGGenerator.h"
#include "VSDParser.h"
#include "VSDXParser.h"
#include "VSD6Parser.h"
#include "VSD11Parser.h"
#include "VSDZipStream.h"

namespace
{

static bool isBinaryVisioDocument(WPXInputStream *input)
{
  WPXInputStream *tmpDocStream = 0;
  try
  {
    input->seek(0, WPX_SEEK_SET);
    if (!input->isOLEStream())
      return false;
    tmpDocStream = input->getDocumentOLEStream("VisioDocument");
    if (!tmpDocStream)
      return false;

    tmpDocStream->seek(0x1A, WPX_SEEK_SET);

    unsigned char version = libvisio::readU8(tmpDocStream);
    delete tmpDocStream;

    VSD_DEBUG_MSG(("VisioDocument: version %i\n", version));

    // Versions 2k (6) and 2k3 (11)
    if (version == 6 || version == 11)
    {
      return true;
    }
  }
  catch (...)
  {
    if (tmpDocStream)
      delete tmpDocStream;
    return false;
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
    input->seek(0, WPX_SEEK_SET);
    if (!input->isOLEStream())
      return false;

    WPXInputStream *docStream = input->getDocumentOLEStream("VisioDocument");

    if (!docStream)
      return false;

    docStream->seek(0x1A, WPX_SEEK_SET);

    unsigned char version = readU8(docStream);
    VSDParser *parser = 0;
    switch(version)
    {
    case 6:
      parser = new VSD6Parser(docStream, painter);
      break;
    case 11:
      parser = new VSD11Parser(docStream, painter);
      break;
    default:
      break;
    }

    if (parser)
      parser->parseMain();
    else
    {
      delete docStream;
      return false;
    }

    delete parser;
    delete docStream;

    return true;
  }
  else if (isOpcVisioDocument(input))
  {
    input->seek(0, WPX_SEEK_SET);
    VSDXParser parser(input, painter);
    if (parser.parseMain())
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
    input->seek(0, WPX_SEEK_SET);
    if (!input->isOLEStream())
      return false;

    WPXInputStream *docStream = input->getDocumentOLEStream("VisioDocument");

    if (!docStream)
      return false;

    docStream->seek(0x1A, WPX_SEEK_SET);

    unsigned char version = readU8(docStream);
    VSDParser *parser;
    switch(version)
    {
    case 6:
      parser = new VSD6Parser(docStream, painter);
      break;
    case 11:
      parser = new VSD11Parser(docStream, painter);
      break;
    default:
      return false;
    }

    if (parser)
      parser->extractStencils();
    else
    {
      delete docStream;
      return false;
    }

    delete parser;
    delete docStream;

    return true;
  }
  else if (isOpcVisioDocument(input))
  {
    input->seek(0, WPX_SEEK_SET);
    VSDXParser parser(input, painter);
    if (parser.extractStencils())
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
