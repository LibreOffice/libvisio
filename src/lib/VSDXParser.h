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
 * Copyright (C) 2012 Fridrich Strba <fridrich.strba@bluewin.ch>
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

#ifndef __VSDXPARSER_H__
#define __VSDXPARSER_H__

#include <libwpd-stream/libwpd-stream.h>
#include <libwpg/libwpg.h>
#include "VSDXTheme.h"
#include "VSDXMLParserBase.h"

namespace libvisio
{

class VSDCollector;

class VSDXParser : public VSDXMLParserBase
{
  using VSDXMLParserBase::readExtendedColourData;
  using VSDXMLParserBase::readDoubleData;
  using VSDXMLParserBase::readBoolData;
  using VSDXMLParserBase::readLongData;

public:
  explicit VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter);
  virtual ~VSDXParser();
  bool parseMain();
  bool extractStencils();

private:
  VSDXParser();
  VSDXParser(const VSDXParser &);
  VSDXParser &operator=(const VSDXParser &);

  // Helper functions

  xmlChar *readStringData(xmlTextReaderPtr reader);

  int getElementToken(xmlTextReaderPtr reader);
  int getElementDepth(xmlTextReaderPtr reader);

  int skipSection(xmlTextReaderPtr reader);

  // Functions parsing the Visio 2013 OPC document structure

  bool parseDocument(WPXInputStream *input, const char *name);
  bool parseMasters(WPXInputStream *input, const char *name);
  bool parseMaster(WPXInputStream *input, const char *name);
  bool parsePages(WPXInputStream *input, const char *name);
  bool parsePage(WPXInputStream *input, const char *name);
  bool parseTheme(WPXInputStream *input, const char *name);
  void processXmlDocument(WPXInputStream *input, VSDXRelationships &rels);
  void processXmlNode(xmlTextReaderPtr reader);

  // Functions reading the Visio 2013 OPC document content

  void extractBinaryData(WPXInputStream *input, const char *name);

  void readPageSheetProperties(xmlTextReaderPtr reader);

  void readStyleProperties(xmlTextReaderPtr reader);

  void readShapeProperties(xmlTextReaderPtr reader);

  void getBinaryData(xmlTextReaderPtr reader);

  void readParagraph(xmlTextReaderPtr reader);
  void readCharacter(xmlTextReaderPtr reader);
  void readFonts(xmlTextReaderPtr reader);

  // Private data

  WPXInputStream *m_input;
  libwpg::WPGPaintInterface *m_painter;
  int m_currentDepth;
  VSDXRelationships *m_rels;
  VSDXTheme m_currentTheme;
};

} // namespace libvisio

#endif // __VSDXPARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
