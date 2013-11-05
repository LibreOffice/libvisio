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
#include <librevenge/librevenge.h>
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
  explicit VSDParser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter);
  virtual ~VSDParser();
  bool parseMain();
  bool extractStencils();

protected:
  // reader functions
  void readEllipticalArcTo(librevenge::RVNGInputStream *input);
  void readForeignData(librevenge::RVNGInputStream *input);
  void readEllipse(librevenge::RVNGInputStream *input);
  virtual void readLine(librevenge::RVNGInputStream *input);
  virtual void readFillAndShadow(librevenge::RVNGInputStream *input);
  virtual void readGeomList(librevenge::RVNGInputStream *input);
  void readGeometry(librevenge::RVNGInputStream *input);
  void readMoveTo(librevenge::RVNGInputStream *input);
  void readLineTo(librevenge::RVNGInputStream *input);
  void readArcTo(librevenge::RVNGInputStream *input);
  void readNURBSTo(librevenge::RVNGInputStream *input);
  void readPolylineTo(librevenge::RVNGInputStream *input);
  void readInfiniteLine(librevenge::RVNGInputStream *input);
  void readShapeData(librevenge::RVNGInputStream *input);
  void readXFormData(librevenge::RVNGInputStream *input);
  void readTxtXForm(librevenge::RVNGInputStream *input);
  void readShapeId(librevenge::RVNGInputStream *input);
  virtual void readShapeList(librevenge::RVNGInputStream *input);
  void readForeignDataType(librevenge::RVNGInputStream *input);
  void readPageProps(librevenge::RVNGInputStream *input);
  virtual void readShape(librevenge::RVNGInputStream *input);
  void readColours(librevenge::RVNGInputStream *input);
  void readFont(librevenge::RVNGInputStream *input);
  void readFontIX(librevenge::RVNGInputStream *input);
  virtual void readCharList(librevenge::RVNGInputStream *input);
  virtual void readParaList(librevenge::RVNGInputStream *input);
  virtual void readPropList(librevenge::RVNGInputStream *input);
  virtual void readPage(librevenge::RVNGInputStream *input);
  virtual void readText(librevenge::RVNGInputStream *input);
  virtual void readCharIX(librevenge::RVNGInputStream *input);
  virtual void readParaIX(librevenge::RVNGInputStream *input);
  virtual void readTextBlock(librevenge::RVNGInputStream *input);

  void readNameList(librevenge::RVNGInputStream *input);
  virtual void readName(librevenge::RVNGInputStream *input);

  virtual void readNameList2(librevenge::RVNGInputStream *input);
  virtual void readName2(librevenge::RVNGInputStream *input);

  virtual void readFieldList(librevenge::RVNGInputStream *input);
  virtual void readTextField(librevenge::RVNGInputStream *input);

  virtual void readStyleSheet(librevenge::RVNGInputStream *input);
  void readPageSheet(librevenge::RVNGInputStream *input);

  void readSplineStart(librevenge::RVNGInputStream *input);
  void readSplineKnot(librevenge::RVNGInputStream *input);

  void readStencilShape(librevenge::RVNGInputStream *input);

  void readOLEList(librevenge::RVNGInputStream *input);
  void readOLEData(librevenge::RVNGInputStream *input);

  virtual void readNameIDX(librevenge::RVNGInputStream *input);
  virtual void readNameIDX123(librevenge::RVNGInputStream *input);

  void readMisc(librevenge::RVNGInputStream *input);

  // parser of one pass
  bool parseDocument(librevenge::RVNGInputStream *input, unsigned shift);

  // Stream handlers
  void handleStreams(librevenge::RVNGInputStream *input, unsigned ptrType, unsigned shift, unsigned level);
  void handleStream(const Pointer &ptr, unsigned idx, unsigned level);
  void handleChunks(librevenge::RVNGInputStream *input, unsigned level);
  void handleChunk(librevenge::RVNGInputStream *input);
  void handleBlob(librevenge::RVNGInputStream *input, unsigned shift, unsigned level);

  virtual void readPointer(librevenge::RVNGInputStream *input, Pointer &ptr);
  virtual void readPointerInfo(librevenge::RVNGInputStream *input, unsigned ptrType, unsigned shift, unsigned &listSize, int &pointerCount);
  virtual bool getChunkHeader(librevenge::RVNGInputStream *input);
  void _handleLevelChange(unsigned level);
  Colour _colourFromIndex(unsigned idx);
  void _flushShape();
  void _nameFromId(VSDName &name, unsigned id, unsigned level);

  virtual unsigned getUInt(librevenge::RVNGInputStream *input);
  virtual int getInt(librevenge::RVNGInputStream *input);

  librevenge::RVNGInputStream *m_input;
  librevenge::RVNGDrawingInterface *m_painter;
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
