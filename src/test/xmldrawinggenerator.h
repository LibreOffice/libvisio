/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __XMLDRAWINGGENERATOR_H__
#define __XMLDRAWINGGENERATOR_H__

#include <librevenge/librevenge.h>
#include <libxml/xmlwriter.h>

namespace libvisio
{

class XmlDrawingGenerator : public librevenge::RVNGDrawingInterface
{
  // disable copying
  XmlDrawingGenerator(const XmlDrawingGenerator &other);
  XmlDrawingGenerator &operator=(const XmlDrawingGenerator &other);

  xmlTextWriterPtr m_writer;

public:
  XmlDrawingGenerator(xmlTextWriterPtr writer);

  ~XmlDrawingGenerator();

  void startDocument(const librevenge::RVNGPropertyList &propList);
  void endDocument();
  void setDocumentMetaData(const librevenge::RVNGPropertyList &propList);
  void defineEmbeddedFont(const librevenge::RVNGPropertyList &propList);
  void startPage(const librevenge::RVNGPropertyList &propList);
  void endPage();
  void startMasterPage(const librevenge::RVNGPropertyList &propList);
  void endMasterPage();
  void startLayer(const librevenge::RVNGPropertyList &propList);
  void endLayer();
  void startEmbeddedGraphics(const librevenge::RVNGPropertyList &propList);
  void endEmbeddedGraphics();

  void openGroup(const librevenge::RVNGPropertyList &propList);
  void closeGroup();

  void setStyle(const librevenge::RVNGPropertyList &propList);

  void drawRectangle(const librevenge::RVNGPropertyList &propList);
  void drawEllipse(const librevenge::RVNGPropertyList &propList);
  void drawPolyline(const librevenge::RVNGPropertyList &propList);
  void drawPolygon(const librevenge::RVNGPropertyList &propList);
  void drawPath(const librevenge::RVNGPropertyList &propList);
  void drawGraphicObject(const librevenge::RVNGPropertyList &propList);
  void drawConnector(const librevenge::RVNGPropertyList &propList);
  void startTextObject(const librevenge::RVNGPropertyList &propList);
  void endTextObject();

  void startTableObject(const librevenge::RVNGPropertyList &propList);
  void openTableRow(const librevenge::RVNGPropertyList &propList);
  void closeTableRow();
  void openTableCell(const librevenge::RVNGPropertyList &propList);
  void closeTableCell();
  void insertCoveredTableCell(const librevenge::RVNGPropertyList &propList);
  void endTableObject();

  void openOrderedListLevel(const librevenge::RVNGPropertyList &propList);
  void closeOrderedListLevel();
  void openUnorderedListLevel(const librevenge::RVNGPropertyList &propList);
  void closeUnorderedListLevel();
  void openListElement(const librevenge::RVNGPropertyList &propList);
  void closeListElement();

  void defineParagraphStyle(const librevenge::RVNGPropertyList &propList);
  void openParagraph(const librevenge::RVNGPropertyList &propList);
  void closeParagraph();

  void defineCharacterStyle(const librevenge::RVNGPropertyList &propList);
  void openSpan(const librevenge::RVNGPropertyList &propList);
  void closeSpan();

  void openLink(const librevenge::RVNGPropertyList &propList);
  void closeLink();

  void insertTab();
  void insertSpace();
  void insertText(const librevenge::RVNGString &text);
  void insertLineBreak();
  void insertField(const librevenge::RVNGPropertyList &propList);
};

} // namespace libvisio

#endif // __XMLDRAWINGGENERATOR_H__

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
