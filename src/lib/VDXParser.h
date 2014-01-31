/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
