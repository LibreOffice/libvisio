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

#include "libvisio.h"
#include <libwpd-stream/libwpd-stream.h>
#include <libwpd/libwpd.h>

class RawPainter : public libwpg::WPGPaintInterface
{
public:
  RawPainter();

  void startGraphics(const ::WPXPropertyList &propList);
  void endGraphics();
  void startLayer(const ::WPXPropertyList &propList);
  void endLayer();
  void startEmbeddedGraphics(const ::WPXPropertyList &propList);
  void endEmbeddedGraphics();

  void setStyle(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &gradient);

  void drawRectangle(const ::WPXPropertyList &propList);
  void drawEllipse(const ::WPXPropertyList &propList);
  void drawPolyline(const ::WPXPropertyListVector &vertices);
  void drawPolygon(const ::WPXPropertyListVector &vertices);
  void drawPath(const ::WPXPropertyListVector &path);
  void drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData &binaryData);
  void startTextObject(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &path);
  void endTextObject();
  void startTextLine(const ::WPXPropertyList &propList);
  void endTextLine();
  void startTextSpan(const ::WPXPropertyList &propList);
  void endTextSpan();
  void insertText(const ::WPXString &str);
};

WPXString getPropString(const WPXPropertyList &propList)
{
  WPXString propString;
  WPXPropertyList::Iter i(propList);
  if (!i.last())
  {
    propString.append(i.key());
    propString.append(": ");
    propString.append(i()->getStr().cstr());
    for (; i.next(); )
    {
      propString.append(", ");
      propString.append(i.key());
      propString.append(": ");
      propString.append(i()->getStr().cstr());
    }
  }

  return propString;
}

WPXString getPropString(const WPXPropertyListVector &itemList)
{
  WPXString propString;

  propString.append("(");
  WPXPropertyListVector::Iter i(itemList);

  if (!i.last())
  {
    propString.append("(");
    propString.append(getPropString(i()));
    propString.append(")");

    for (; i.next();)
    {
      propString.append(", (");
      propString.append(getPropString(i()));
      propString.append(")");
    }

  }
  propString.append(")");

  return propString;
}

RawPainter::RawPainter(): libwpg::WPGPaintInterface()
{
}

void RawPainter::startGraphics(const ::WPXPropertyList &propList)
{
  printf("RawPainter::startGraphics(%s)\n", getPropString(propList).cstr());
}

void RawPainter::endGraphics()
{
  printf("RawPainter::endGraphics\n");
}

void RawPainter::startLayer(const ::WPXPropertyList &propList)
{
  printf("RawPainter::startLayer (%s)\n", getPropString(propList).cstr());
}

void RawPainter::endLayer()
{
  printf("RawPainter::endLayer\n");
}

void RawPainter::startEmbeddedGraphics(const ::WPXPropertyList &propList)
{
  printf("RawPainter::startEmbeddedGraphics (%s)\n", getPropString(propList).cstr());
}

void RawPainter::endEmbeddedGraphics()
{
  printf("RawPainter::endEmbeddedGraphics \n");
}

void RawPainter::setStyle(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &gradient)
{
  printf("RawPainter::setStyle(%s, gradient: (%s))\n", getPropString(propList).cstr(), getPropString(gradient).cstr());
}

void RawPainter::drawRectangle(const ::WPXPropertyList &propList)
{
  printf("RawPainter::drawRectangle (%s)\n", getPropString(propList).cstr());
}

void RawPainter::drawEllipse(const ::WPXPropertyList &propList)
{
  printf("RawPainter::drawEllipse (%s)\n", getPropString(propList).cstr());
}

void RawPainter::drawPolyline(const ::WPXPropertyListVector &vertices)
{
  printf("RawPainter::drawPolyline (%s)\n", getPropString(vertices).cstr());
}

void RawPainter::drawPolygon(const ::WPXPropertyListVector &vertices)
{
  printf("RawPainter::drawPolygon (%s)\n", getPropString(vertices).cstr());
}

void RawPainter::drawPath(const ::WPXPropertyListVector &path)
{
  printf("RawPainter::drawPath (%s)\n", getPropString(path).cstr());
}

void RawPainter::drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData & /*binaryData*/)
{
  printf("RawPainter::drawGraphicObject (%s)\n", getPropString(propList).cstr());
}

void RawPainter::startTextObject(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &path)
{
  printf("RawPainter::startTextObject (%s, path: (%s))\n", getPropString(propList).cstr(), getPropString(path).cstr());
}

void RawPainter::endTextObject()
{
  printf("RawPainter::endTextObject\n");
}

void RawPainter::startTextLine(const ::WPXPropertyList &propList)
{
  printf("RawPainter::startTextLine (%s)\n", getPropString(propList).cstr());
}

void RawPainter::endTextLine()
{
  printf("RawPainter::endTextLine\n");
}

void RawPainter::startTextSpan(const ::WPXPropertyList &propList)
{
  printf("RawPainter::startTextSpan (%s)\n", getPropString(propList).cstr());
}

void RawPainter::endTextSpan()
{
  printf("RawPainter::endTextSpan\n");
}

void RawPainter::insertText(const ::WPXString &str)
{
  printf("RawPainter::insertText (%s)\n", str.cstr());
}


namespace
{

int printUsage()
{
  printf("Usage: vsd2raw [OPTION] <Visio Stencils File>\n");
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

  WPXFileStream input(file);

  if (!libvisio::VisioDocument::isSupported(&input))
  {
    fprintf(stderr, "ERROR: Unsupported file format (unsupported version) or file is encrypted!\n");
    return 1;
  }

  RawPainter painter;
  if (!libvisio::VisioDocument::parseStencils(&input, &painter))
  {
    fprintf(stderr, "ERROR: Parsing of document failed!\n");
    return 1;
  }

  return 0;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
