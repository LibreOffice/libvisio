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

#ifndef __VSDXPARSER_H__
#define __VSDXPARSER_H__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <libwpd/libwpd.h>
#include <libwpd-stream/libwpd-stream.h>
#include <libwpg/libwpg.h>
#include "VSDXTypes.h"
#include "VSDXGeometryList.h"
#include "VSDXFieldList.h"
#include "VSDXNameList.h"
#include "VSDXCharacterList.h"
#include "VSDXParagraphList.h"
#include "VSDXShapeList.h"
#include "VSDXStencils.h"

namespace libvisio
{

class VSDXCollector;

class VSDXParser
{
public:
  explicit VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter);
  virtual ~VSDXParser();
  bool parseMain();

protected:
  // reader functions
  void readEllipticalArcTo(WPXInputStream *input);
  void readForeignData(WPXInputStream *input);
  void readEllipse(WPXInputStream *input);
  void readLine(WPXInputStream *input);
  virtual void readFillAndShadow(WPXInputStream *input) = 0;
  void readGeomList(WPXInputStream *input);
  void readGeometry(WPXInputStream *input);
  void readMoveTo(WPXInputStream *input);
  void readLineTo(WPXInputStream *input);
  void readArcTo(WPXInputStream *input);
  void readNURBSTo(WPXInputStream *input);
  void readPolylineTo(WPXInputStream *input);
  void readInfiniteLine(WPXInputStream *input);
  void readShapeData(WPXInputStream *input);
  void readXFormData(WPXInputStream *input);
  void readTxtXForm(WPXInputStream *input);
  void readShapeId(WPXInputStream *input);
  void readShapeList(WPXInputStream *input);
  void readForeignDataType(WPXInputStream *input);
  void readPageProps(WPXInputStream *input);
  void readShape(WPXInputStream *input);
  void readColours(WPXInputStream *input);
  void readFont(WPXInputStream *input, unsigned id);
  void readFontIX(WPXInputStream *input);
  void readCharList(WPXInputStream *input);
  void readParaList(WPXInputStream *input);
  void readPage(WPXInputStream *input);
  virtual void readText(WPXInputStream *input) = 0;
  virtual void readCharIX(WPXInputStream *input) = 0;
  void readParaIX(WPXInputStream *input);
  void readTextBlock(WPXInputStream *input);

  void readNameList(WPXInputStream *input);
  virtual void readName(WPXInputStream *input) = 0;

  void readFieldList(WPXInputStream *input);
  void readTextField(WPXInputStream *input);

  void readStyleSheet(WPXInputStream *input);

  void readSplineStart(WPXInputStream *input);
  void readSplineKnot(WPXInputStream *input);

  void readStencilShape(WPXInputStream *input);

  // parser of one pass
  bool parseDocument(WPXInputStream *input);

  // Stream handlers
  void handlePages(WPXInputStream *input, unsigned shift);
  void handlePage(WPXInputStream *input);
  void handleStyles(WPXInputStream *input);
  void handleStencils(WPXInputStream *input, unsigned shift);
  void handleStencilPage(WPXInputStream *input, unsigned shift);
  void handleStencilForeign(WPXInputStream *input, unsigned shift);
  void handleStencilShape(WPXInputStream *input);

  virtual bool getChunkHeader(WPXInputStream *input) = 0;
  void _handleLevelChange(unsigned level);

  WPXInputStream *m_input;
  libwpg::WPGPaintInterface *m_painter;
  ChunkHeader m_header;
  VSDXCollector *m_collector;
  VSDXGeometryList *m_geomList;
  std::vector<VSDXGeometryList *> m_geomListVector;
  VSDXFieldList m_fieldList;
  VSDXNameList m_nameList;
  VSDXCharacterList *m_charList;
  VSDXParagraphList *m_paraList;
  std::vector<VSDXCharacterList *> m_charListVector;
  std::vector<VSDXParagraphList *> m_paraListVector;
  VSDXShapeList m_shapeList;
  unsigned m_currentLevel;

  VSDXStencils m_stencils;
  VSDXStencil *m_currentStencil;
  VSDXStencilShape m_stencilShape;
  bool m_isStencilStarted;
  bool m_isInStyles;
  unsigned m_currentPageID;

private:
  VSDXParser();
  VSDXParser(const VSDXParser &);
  VSDXParser &operator=(const VSDXParser &);

};

struct Pointer
{
  unsigned Type;
  unsigned Offset;
  unsigned Length;
  unsigned short Format;
};

} // namespace libvisio

#endif // __VSDXPARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
