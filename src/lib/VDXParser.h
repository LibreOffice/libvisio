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

#include <map>
#include <string>
#include <libwpd-stream/libwpd-stream.h>
#include <libwpg/libwpg.h>
#include "VSDXMLHelper.h"
#include "VSDStencils.h"

namespace libvisio
{

class VSDCollector;

class VDXParser
{
public:
  explicit VDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter);
  virtual ~VDXParser();
  bool parseMain();
  bool extractStencils();

private:
  VDXParser();
  VDXParser(const VDXParser &);
  VDXParser &operator=(const VDXParser &);

  // Helper functions

  int readLongData(long &value, xmlTextReaderPtr reader);
  int readDoubleData(double &value, xmlTextReaderPtr reader);

  // Functions to read teh DiagramML document structure

  bool processXmlDocument(WPXInputStream *input);
  void processXmlNode(xmlTextReaderPtr reader);

  // Functions reading the DiagramML document content

  void readEllipticalArcTo(xmlTextReaderPtr reader);
  void readForeignData(xmlTextReaderPtr reader);
  void readEllipse(xmlTextReaderPtr reader);
  void readLine(xmlTextReaderPtr reader);
  void readFillAndShadow(xmlTextReaderPtr reader);
  void readGeomList(xmlTextReaderPtr reader);
  void readGeometry(xmlTextReaderPtr reader);
  void readMoveTo(xmlTextReaderPtr reader);
  void readLineTo(xmlTextReaderPtr reader);
  void readArcTo(xmlTextReaderPtr reader);
  void readNURBSTo(xmlTextReaderPtr reader);
  void readPolylineTo(xmlTextReaderPtr reader);
  void readInfiniteLine(xmlTextReaderPtr reader);
  void readShapeData(xmlTextReaderPtr reader);
  void readXFormData(xmlTextReaderPtr reader);
  void readTxtXForm(xmlTextReaderPtr reader);
  void readShapeId(xmlTextReaderPtr reader);
  void readShapeList(xmlTextReaderPtr reader);
  void readForeignDataType(xmlTextReaderPtr reader);
  void readPageProps(xmlTextReaderPtr reader);
  void readShape(xmlTextReaderPtr reader);
  void readColours(xmlTextReaderPtr reader);
  void readFonts(xmlTextReaderPtr reader);
  void readCharList(xmlTextReaderPtr reader);
  void readParaList(xmlTextReaderPtr reader);
  void readPage(xmlTextReaderPtr reader);
  void readText(xmlTextReaderPtr reader);
  void readCharIX(xmlTextReaderPtr reader);
  void readParaIX(xmlTextReaderPtr reader);
  void readTextBlock(xmlTextReaderPtr reader);

  void readNameList(xmlTextReaderPtr reader);
  void readName(xmlTextReaderPtr reader);

  void readFieldList(xmlTextReaderPtr reader);
  void readTextField(xmlTextReaderPtr reader);

  void readStyleSheet(xmlTextReaderPtr reader);
  void readPageSheet(xmlTextReaderPtr reader);

  void readSplineStart(xmlTextReaderPtr reader);
  void readSplineKnot(xmlTextReaderPtr reader);

  void readStencil(xmlTextReaderPtr reader);
  void readStencilShape(xmlTextReaderPtr reader);

  void readOLEList(xmlTextReaderPtr reader);
  void readOLEData(xmlTextReaderPtr reader);

  // Private data

  WPXInputStream *m_input;
  libwpg::WPGPaintInterface *m_painter;
  VSDCollector *m_collector;
  VSDStencils m_stencils;
  VSDStencilShape m_stencilShape;
  bool m_isStencilStarted;

  bool m_extractStencils;
  bool m_isInStyles;
};

} // namespace libvisio

#endif // __VDXPARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
