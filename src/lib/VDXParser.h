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
  using VSDXMLParserBase::readStringData;

public:
  explicit VDXParser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter);
  ~VDXParser() override;
  bool parseMain() override;
  bool extractStencils() override;

private:
  VDXParser();
  VDXParser(const VDXParser &);
  VDXParser &operator=(const VDXParser &);

  // Helper functions

  xmlChar *readStringData(xmlTextReaderPtr reader) override;

  int getElementToken(xmlTextReaderPtr reader) override;
  int getElementDepth(xmlTextReaderPtr reader) override;

  // Functions to read the DatadiagramML document structure

  bool processXmlDocument(librevenge::RVNGInputStream *input);
  void processXmlNode(xmlTextReaderPtr reader);

  // Functions reading the DiagramML document content

  void readLine(xmlTextReaderPtr reader);
  void readFillAndShadow(xmlTextReaderPtr reader);
  void readXFormData(xmlTextReaderPtr reader);
  void readMisc(xmlTextReaderPtr reader);
  void readTxtXForm(xmlTextReaderPtr reader);
  void readXForm1D(xmlTextReaderPtr reader);
  void readPageProps(xmlTextReaderPtr reader);
  void readFonts(xmlTextReaderPtr reader);
  void readTextBlock(xmlTextReaderPtr reader);
  void readForeignInfo(xmlTextReaderPtr reader);
  void readLayerMem(xmlTextReaderPtr reader);
  void readTabs(xmlTextReaderPtr reader);
  void readTab(xmlTextReaderPtr reader);

  void getBinaryData(xmlTextReaderPtr reader) override;

  // Private data

  librevenge::RVNGInputStream *m_input;
  librevenge::RVNGDrawingInterface *m_painter;
};

} // namespace libvisio

#endif // __VDXPARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
