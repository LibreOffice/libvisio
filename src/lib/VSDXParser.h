/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDXPARSER_H__
#define __VSDXPARSER_H__

#include <librevenge/librevenge.h>
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
  explicit VSDXParser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter);
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

  bool parseDocument(librevenge::RVNGInputStream *input, const char *name);
  bool parseMasters(librevenge::RVNGInputStream *input, const char *name);
  bool parseMaster(librevenge::RVNGInputStream *input, const char *name);
  bool parsePages(librevenge::RVNGInputStream *input, const char *name);
  bool parsePage(librevenge::RVNGInputStream *input, const char *name);
  bool parseTheme(librevenge::RVNGInputStream *input, const char *name);
  void processXmlDocument(librevenge::RVNGInputStream *input, VSDXRelationships &rels);
  void processXmlNode(xmlTextReaderPtr reader);

  // Functions reading the Visio 2013 OPC document content

  void extractBinaryData(librevenge::RVNGInputStream *input, const char *name);

  void readPageSheetProperties(xmlTextReaderPtr reader);

  void readStyleProperties(xmlTextReaderPtr reader);

  void readShapeProperties(xmlTextReaderPtr reader);

  void getBinaryData(xmlTextReaderPtr reader);

  void readParagraph(xmlTextReaderPtr reader);
  void readCharacter(xmlTextReaderPtr reader);
  void readFonts(xmlTextReaderPtr reader);

  // Private data

  librevenge::RVNGInputStream *m_input;
  librevenge::RVNGDrawingInterface *m_painter;
  int m_currentDepth;
  VSDXRelationships *m_rels;
  VSDXTheme m_currentTheme;
};

} // namespace libvisio

#endif // __VSDXPARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
