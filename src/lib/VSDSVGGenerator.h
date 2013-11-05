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

#ifndef __VISIOSVGGENERATOR_H__
#define __VISIOSVGGENERATOR_H__

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <librevenge/librevenge.h>
#include <libvisio/libvisio.h>

namespace libvisio
{
struct VSDSVGGeneratorPrivate;

class VSDSVGGenerator : public librevenge::RVNGDrawingInterface
{
public:
  VSDSVGGenerator(librevenge::RVNGStringVector &vec, const librevenge::RVNGString &nmspace="svg");
  ~VSDSVGGenerator();

  void startDocument(const librevenge::RVNGPropertyList & /*propList*/) {}
  void endDocument() {}
  void setDocumentMetaData(const librevenge::RVNGPropertyList & /*propList*/) {}
  void startPage(const librevenge::RVNGPropertyList &propList);
  void endPage();
  void startLayer(const librevenge::RVNGPropertyList &propList);
  void endLayer();
  void startEmbeddedGraphics(const librevenge::RVNGPropertyList & /*propList*/) {}
  void endEmbeddedGraphics() {}

  void setStyle(const librevenge::RVNGPropertyList &propList, const librevenge::RVNGPropertyListVector &gradient);

  void drawRectangle(const librevenge::RVNGPropertyList &propList);
  void drawEllipse(const librevenge::RVNGPropertyList &propList);
  void drawPolyline(const librevenge::RVNGPropertyListVector &vertices);
  void drawPolygon(const librevenge::RVNGPropertyListVector &vertices);
  void drawPath(const librevenge::RVNGPropertyListVector &path);
  void drawGraphicObject(const librevenge::RVNGPropertyList &propList, const librevenge::RVNGBinaryData &binaryData);
  void startTextObject(const librevenge::RVNGPropertyList &propList, const librevenge::RVNGPropertyListVector &path);
  void endTextObject();

  void openOrderedListLevel(const librevenge::RVNGPropertyList & /*propList*/) {}
  void closeOrderedListLevel() {}

  void openUnorderedListLevel(const librevenge::RVNGPropertyList & /*propList*/) {}
  void closeUnorderedListLevel() {}

  void openListElement(const librevenge::RVNGPropertyList & /*propList*/, const librevenge::RVNGPropertyListVector & /* tabStops */) {}
  void closeListElement() {}

  void openParagraph(const librevenge::RVNGPropertyList & /*propList*/, const librevenge::RVNGPropertyListVector & /* tabStops */) {}
  void closeParagraph() {}

  void openSpan(const librevenge::RVNGPropertyList &propList);
  void closeSpan();

  void insertTab() {}
  void insertSpace() {}
  void insertText(const librevenge::RVNGString &text);
  void insertLineBreak() {}
  void insertField(const librevenge::RVNGString & /* type */, const librevenge::RVNGPropertyList & /*propList*/) {}

private:
  VSDSVGGenerator(const VSDSVGGenerator &);
  VSDSVGGenerator &operator=(const VSDSVGGenerator &);
  VSDSVGGeneratorPrivate *m_pImpl;
};

} // namespace libvisio

#endif // __VISIOSVGGENERATOR_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
