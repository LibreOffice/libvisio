/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmldrawinggenerator.h"

#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace libvisio
{

XmlDrawingGenerator::XmlDrawingGenerator(xmlTextWriterPtr writer)
  : RVNGDrawingInterface(),
    m_writer(writer)
{
}

XmlDrawingGenerator::~XmlDrawingGenerator()
{
}

void XmlDrawingGenerator::startDocument(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("document"));

  xmlTextWriterWriteAttribute(m_writer, BAD_CAST("xmlns:dc"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:meta:1.0"));
  xmlTextWriterWriteAttribute(m_writer, BAD_CAST("xmlns:draw"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"));
  xmlTextWriterWriteAttribute(m_writer, BAD_CAST("xmlns:fo"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"));
  xmlTextWriterWriteAttribute(m_writer, BAD_CAST("xmlns:librevenge"), BAD_CAST("urn:x-documentliberation:xmlns:librevenge:0.0"));
  xmlTextWriterWriteAttribute(m_writer, BAD_CAST("xmlns:office"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:office:1.0"));
  xmlTextWriterWriteAttribute(m_writer, BAD_CAST("xmlns:style"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:style:1.0"));
  xmlTextWriterWriteAttribute(m_writer, BAD_CAST("xmlns:svg"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"));
  xmlTextWriterWriteAttribute(m_writer, BAD_CAST("xmlns:meta"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:meta:1.0"));

  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::endDocument()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::setDocumentMetaData(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("setDocumentMetaData"));
  librevenge::RVNGPropertyList::Iter i(propList);
  std::vector< std::pair<std::string, std::string> > userDefined;
  for (i.rewind(); i.next();)
  {
    if (strncmp(i.key(), "meta:user-defined:", 18))
      xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
    else
      userDefined.push_back(std::make_pair(i.key() + 18, i()->getStr().cstr()));
  }

  if (!userDefined.empty())
  {
    for (size_t idx = 0; idx < userDefined.size(); ++idx)
    {
      xmlTextWriterStartElement(m_writer, BAD_CAST("user-defined"));
      xmlTextWriterWriteAttribute(m_writer, BAD_CAST("name"), BAD_CAST(userDefined[idx].first.c_str()));
      xmlTextWriterWriteString(m_writer, BAD_CAST(userDefined[idx].second.c_str()));
      xmlTextWriterEndElement(m_writer);
    }
  }
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::defineEmbeddedFont(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("defineEmbeddedFont"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::startPage(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("page"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::endPage()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::startMasterPage(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("masterPage"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::endMasterPage()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::startLayer(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("layer"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::endLayer()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::startEmbeddedGraphics(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("embeddedGraphics"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::endEmbeddedGraphics()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::openGroup(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("group"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::closeGroup()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::setStyle(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("setStyle"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::drawRectangle(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("drawRectangle"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::drawEllipse(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("drawEllipse"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::drawPolyline(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("drawPolyline"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::drawPolygon(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("drawPolygon"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::drawPath(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("drawPath"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::drawGraphicObject(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("drawGraphicObject"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::drawConnector(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("drawConnector"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::startTextObject(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("textObject"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::endTextObject()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::openOrderedListLevel(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("orderedListLevel"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::closeOrderedListLevel()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::openUnorderedListLevel(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("unorderedListLevel"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::closeUnorderedListLevel()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::openListElement(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("listElement"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::closeListElement()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::defineParagraphStyle(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("defineParagraphStyle"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::openParagraph(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("paragraph"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::closeParagraph()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::defineCharacterStyle(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("defineCharacterStyle"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::openSpan(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("span"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::closeSpan()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::openLink(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("link"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::closeLink()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::insertTab()
{
  xmlTextWriterWriteElement(m_writer, BAD_CAST("insertTab"), 0);
}

void XmlDrawingGenerator::insertSpace()
{
  xmlTextWriterWriteElement(m_writer, BAD_CAST("insertSpace"), 0);
}

void XmlDrawingGenerator::insertText(const librevenge::RVNGString &str)
{
  xmlTextWriterWriteElement(m_writer, BAD_CAST("insertText"), BAD_CAST(str.cstr()));
}

void XmlDrawingGenerator::insertLineBreak()
{
  xmlTextWriterWriteElement(m_writer, BAD_CAST("insertLineBreak"), 0);
}

void XmlDrawingGenerator::insertField(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("insertField"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::startTableObject(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("tableObject"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::openTableRow(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("tableRow"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::closeTableRow()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::openTableCell(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("tableCell"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
}

void XmlDrawingGenerator::closeTableCell()
{
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::insertCoveredTableCell(const librevenge::RVNGPropertyList &propList)
{
  xmlTextWriterStartElement(m_writer, BAD_CAST("insertCoveredTableCell"));
  librevenge::RVNGPropertyList::Iter i(propList);
  for (i.rewind(); i.next();)
    xmlTextWriterWriteFormatAttribute(m_writer, BAD_CAST(i.key()), "%s", i()->getStr().cstr());
  xmlTextWriterEndElement(m_writer);
}

void XmlDrawingGenerator::endTableObject()
{
  xmlTextWriterEndElement(m_writer);
}

} // namespace libvisio

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
