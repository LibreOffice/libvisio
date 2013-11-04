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

#include <stdio.h>
#include <string.h>

#include <librevenge-stream/librevenge-stream.h>
#include <librevenge/librevenge.h>
#include <libvisio/libvisio.h>

class TextPainter : public RVNGDrawingInterface
{
public:
  TextPainter();

  void startDocument(const ::RVNGPropertyList & /*propList*/) {}
  void endDocument() {}
  void setDocumentMetaData(const RVNGPropertyList & /*propList*/) {}
  void startPage(const ::RVNGPropertyList &) {}
  void endPage() {}
  void startLayer(const ::RVNGPropertyList &) {}
  void endLayer() {}
  void startEmbeddedGraphics(const ::RVNGPropertyList &) {}
  void endEmbeddedGraphics() {}

  void setStyle(const ::RVNGPropertyList &, const ::RVNGPropertyListVector &) {}

  void drawRectangle(const ::RVNGPropertyList &) {}
  void drawEllipse(const ::RVNGPropertyList &) {}
  void drawPolyline(const ::RVNGPropertyListVector &) {}
  void drawPolygon(const ::RVNGPropertyListVector &) {}
  void drawPath(const ::RVNGPropertyListVector &) {}
  void drawGraphicObject(const ::RVNGPropertyList &, const ::RVNGBinaryData &) {}
  void startTextObject(const ::RVNGPropertyList &, const ::RVNGPropertyListVector &) {}
  void endTextObject() {}


  void openOrderedListLevel(const RVNGPropertyList & /*propList*/) {}
  void closeOrderedListLevel() {}

  void openUnorderedListLevel(const RVNGPropertyList & /*propList*/) {}
  void closeUnorderedListLevel() {}

  void openListElement(const RVNGPropertyList & /*propList*/, const RVNGPropertyListVector & /* tabStops */) {}
  void closeListElement() {}

  void openParagraph(const RVNGPropertyList & /*propList*/, const RVNGPropertyListVector & /* tabStops */) {}
  void closeParagraph();

  void openSpan(const RVNGPropertyList & /*propList*/) {}
  void closeSpan() {}

  void insertTab() {}
  void insertSpace() {}
  void insertText(const RVNGString &text);
  void insertLineBreak() {}
  void insertField(const RVNGString & /* type */, const RVNGPropertyList & /*propList*/) {}

};

TextPainter::TextPainter(): RVNGDrawingInterface()
{
}

void TextPainter::insertText(const ::RVNGString &str)
{
  printf("%s", str.cstr());
}

void TextPainter::closeParagraph()
{
  printf("\n");
}

namespace
{

int printUsage()
{
  printf("Usage: vss2text [OPTION] <Visio Stencils File>\n");
  printf("\n");
  printf("Options:\n");
  printf("--help                Shows this help message\n");
  return -1;
}

} // anonymous namespace

int main(int argc, char *argv[])
{
  if (argc < 2)
    return printUsage();

  char *file = 0;

  for (int i = 1; i < argc; i++)
  {
    if (!file && strncmp(argv[i], "--", 2))
      file = argv[i];
    else
      return printUsage();
  }

  if (!file)
    return printUsage();

  RVNGFileStream input(file);

  if (!libvisio::VisioDocument::isSupported(&input))
  {
    fprintf(stderr, "ERROR: Unsupported file format (unsupported version) or file is encrypted!\n");
    return 1;
  }

  TextPainter painter;
  if (!libvisio::VisioDocument::parseStencils(&input, &painter))
  {
    fprintf(stderr, "ERROR: Parsing of document failed!\n");
    return 1;
  }

  return 0;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
