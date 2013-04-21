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

#ifndef __VSDPARSER_H__
#define __VSDPARSER_H__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <stack>
#include <map>
#include <libwpd/libwpd.h>
#include <libwpd-stream/libwpd-stream.h>
#include <libwpg/libwpg.h>
#include "VSDTypes.h"
#include "VSDGeometryList.h"
#include "VSDFieldList.h"
#include "VSDCharacterList.h"
#include "VSDParagraphList.h"
#include "VSDShapeList.h"
#include "VSDStencils.h"

namespace libvisio
{

class VSDCollector;

struct Pointer
{
  Pointer()
    : Type(0), Offset(0), Length(0), Format(0), ListSize(0) {}
  Pointer(const Pointer &ptr)
    : Type(ptr.Type), Offset(ptr.Offset), Length(ptr.Length), Format(ptr.Format), ListSize(ptr.ListSize) {}
  unsigned Type;
  unsigned Offset;
  unsigned Length;
  unsigned short Format;
  unsigned ListSize;
};

class VSDParser
{
public:
  explicit VSDParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter);
  virtual ~VSDParser();
  bool parseMain();
  bool extractStencils();

protected:
  // reader functions
  void readEllipticalArcTo(WPXInputStream *input);
  void readForeignData(WPXInputStream *input);
  void readEllipse(WPXInputStream *input);
  virtual void readLine(WPXInputStream *input);
  virtual void readFillAndShadow(WPXInputStream *input);
  virtual void readGeomList(WPXInputStream *input);
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
  virtual void readShapeList(WPXInputStream *input);
  void readForeignDataType(WPXInputStream *input);
  void readPageProps(WPXInputStream *input);
  virtual void readShape(WPXInputStream *input);
  void readColours(WPXInputStream *input);
  void readFont(WPXInputStream *input);
  void readFontIX(WPXInputStream *input);
  virtual void readCharList(WPXInputStream *input);
  virtual void readParaList(WPXInputStream *input);
  virtual void readPropList(WPXInputStream *input);
  virtual void readPage(WPXInputStream *input);
  virtual void readText(WPXInputStream *input);
  virtual void readCharIX(WPXInputStream *input);
  virtual void readParaIX(WPXInputStream *input);
  virtual void readTextBlock(WPXInputStream *input);

  void readNameList(WPXInputStream *input);
  virtual void readName(WPXInputStream *input);

  virtual void readNameList2(WPXInputStream *input);
  virtual void readName2(WPXInputStream *input);

  virtual void readFieldList(WPXInputStream *input);
  virtual void readTextField(WPXInputStream *input);

  virtual void readStyleSheet(WPXInputStream *input);
  void readPageSheet(WPXInputStream *input);

  void readSplineStart(WPXInputStream *input);
  void readSplineKnot(WPXInputStream *input);

  void readStencilShape(WPXInputStream *input);

  void readOLEList(WPXInputStream *input);
  void readOLEData(WPXInputStream *input);

  virtual void readNameIDX(WPXInputStream *input);
  virtual void readNameIDX123(WPXInputStream *input);

  void readMisc(WPXInputStream *input);

  // parser of one pass
  bool parseDocument(WPXInputStream *input, unsigned shift);

  // Stream handlers
  void handleStreams(WPXInputStream *input, unsigned ptrType, unsigned shift, unsigned level);
  void handleStream(const Pointer &ptr, unsigned idx, unsigned level);
  void handleChunks(WPXInputStream *input, unsigned level);
  void handleChunk(WPXInputStream *input);
  void handleBlob(WPXInputStream *input, unsigned shift, unsigned level);

  virtual void readPointer(WPXInputStream *input, Pointer &ptr);
  virtual void readPointerInfo(WPXInputStream *input, unsigned ptrType, unsigned shift, unsigned &listSize, int &pointerCount);
  virtual bool getChunkHeader(WPXInputStream *input);
  void _handleLevelChange(unsigned level);
  Colour _colourFromIndex(unsigned idx);
  void _flushShape();
  void _nameFromId(VSDName &name, unsigned id, unsigned level);

  virtual unsigned getUInt(WPXInputStream *input);
  virtual int getInt(WPXInputStream *input);

  WPXInputStream *m_input;
  libwpg::WPGPaintInterface *m_painter;
  ChunkHeader m_header;
  VSDCollector *m_collector;
  VSDShapeList m_shapeList;
  unsigned m_currentLevel;

  VSDStencils m_stencils;
  VSDStencil *m_currentStencil;
  VSDShape m_shape;
  bool m_isStencilStarted;
  bool m_isInStyles;
  unsigned m_currentShapeLevel;
  unsigned m_currentShapeID;

  bool m_extractStencils;
  std::vector<Colour> m_colours;

  bool m_isBackgroundPage;
  bool m_isShapeStarted;

  double m_shadowOffsetX;
  double m_shadowOffsetY;

  VSDGeometryList *m_currentGeometryList;
  unsigned m_currentGeomListCount;

  std::map<unsigned, VSDName> m_fonts;
  std::map<unsigned, VSDName> m_names;
  std::map<unsigned, std::map<unsigned, VSDName> > m_namesMapMap;
  VSDName m_currentPageName;

private:
  VSDParser();
  VSDParser(const VSDParser &);
  VSDParser &operator=(const VSDParser &);

};

} // namespace libvisio

#endif // __VSDPARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
