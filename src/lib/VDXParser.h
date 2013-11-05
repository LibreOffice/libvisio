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

#ifndef __VDXPARSER_H__
#define __VDXPARSER_H__

#include <librevenge/librevenge.h>
#include "VSDXMLParserBase.h"

namespace libvisio
{

class VSDCollector;

class VDXParser : public VSDXMLParserBase
{
  using VSDXMLParserBase::readExtendedColourData;
  using VSDXMLParserBase::readDoubleData;
  using VSDXMLParserBase::readBoolData;
  using VSDXMLParserBase::readLongData;

public:
  explicit VDXParser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter);
  virtual ~VDXParser();
  bool parseMain();
  bool extractStencils();

private:
  VDXParser();
  VDXParser(const VDXParser &);
  VDXParser &operator=(const VDXParser &);

  // Helper functions

  xmlChar *readStringData(xmlTextReaderPtr reader);

  int getElementToken(xmlTextReaderPtr reader);
  int getElementDepth(xmlTextReaderPtr reader);

  // Functions to read the DatadiagramML document structure

  bool processXmlDocument(librevenge::RVNGInputStream *input);
  void processXmlNode(xmlTextReaderPtr reader);

  // Functions reading the DiagramML document content

  void readLine(xmlTextReaderPtr reader);
  void readFillAndShadow(xmlTextReaderPtr reader);
  void readXFormData(xmlTextReaderPtr reader);
  void readMisc(xmlTextReaderPtr reader);
  void readTxtXForm(xmlTextReaderPtr reader);
  void readPageProps(xmlTextReaderPtr reader);
  void readFonts(xmlTextReaderPtr reader);
  void readTextBlock(xmlTextReaderPtr reader);
  void readForeignInfo(xmlTextReaderPtr reader);

  void getBinaryData(xmlTextReaderPtr reader);

  // Private data

  librevenge::RVNGInputStream *m_input;
  librevenge::RVNGDrawingInterface *m_painter;
};

} // namespace libvisio

#endif // __VDXPARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
