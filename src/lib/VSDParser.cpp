/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDParser.h"

#include <librevenge-stream/librevenge-stream.h>
#include <locale.h>
#include <cassert>
#include <sstream>
#include <string>
#include <cmath>
#include <set>
#include "libvisio_utils.h"
#include "VSDInternalStream.h"
#include "VSDDocumentStructure.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"
#include "VSDMetaData.h"

libvisio::VSDParser::VSDParser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter, librevenge::RVNGInputStream *container)
  : m_input(input), m_painter(painter), m_container(container), m_header(), m_collector(nullptr), m_shapeList(), m_currentLevel(0),
    m_stencils(), m_currentStencil(nullptr), m_shape(), m_isStencilStarted(false), m_isInStyles(false),
    m_currentShapeLevel(0), m_currentShapeID(MINUS_ONE), m_currentLayerListLevel(0), m_extractStencils(false), m_colours(),
    m_isBackgroundPage(false), m_isShapeStarted(false), m_shadowOffsetX(0.0), m_shadowOffsetY(0.0),
    m_currentGeometryList(nullptr), m_currentGeomListCount(0), m_fonts(), m_names(), m_namesMapMap(),
    m_currentPageName(), m_currentTabSet()
{}

libvisio::VSDParser::~VSDParser()
{
}

void libvisio::VSDParser::_nameFromId(VSDName &name, unsigned id, unsigned level)
{
  name = VSDName();
  std::map<unsigned, std::map<unsigned, VSDName> >::const_iterator iter1 = m_namesMapMap.find(level);
  if (iter1 != m_namesMapMap.end())
  {
    auto iter = iter1->second.find(id);
    if (iter != iter1->second.end())
      name = iter->second;
  }
}

bool libvisio::VSDParser::getChunkHeader(librevenge::RVNGInputStream *input)
{
  unsigned char tmpChar = 0;
  while (!input->isEnd() && !tmpChar)
    tmpChar = readU8(input);

  if (input->isEnd())
    return false;
  else
    input->seek(-1, librevenge::RVNG_SEEK_CUR);

  m_header.chunkType = readU32(input);
  m_header.id = readU32(input);
  m_header.list = readU32(input);

  // Certain chunk types seem to always have a trailer
  m_header.trailer = 0;
  if (m_header.list != 0 || m_header.chunkType == 0x71 || m_header.chunkType == 0x70 ||
      m_header.chunkType == 0x6b || m_header.chunkType == 0x6a || m_header.chunkType == 0x69 ||
      m_header.chunkType == 0x66 || m_header.chunkType == 0x65 || m_header.chunkType == 0x2c)
    m_header.trailer += 8; // 8 byte trailer

  m_header.dataLength = readU32(input);
  m_header.level = readU16(input);
  m_header.unknown = readU8(input);

  unsigned trailerChunks [14] = {0x64, 0x65, 0x66, 0x69, 0x6a, 0x6b, 0x6f, 0x71,
                                 0x92, 0xa9, 0xb4, 0xb6, 0xb9, 0xc7
                                };
  // Add word separator under certain circumstances for v11
  // Below are known conditions, may be more or a simpler pattern
  if (m_header.list != 0 || (m_header.level == 2 && m_header.unknown == 0x55) ||
      (m_header.level == 2 && m_header.unknown == 0x54 && m_header.chunkType == 0xaa)
      || (m_header.level == 3 && m_header.unknown != 0x50 && m_header.unknown != 0x54))
  {
    m_header.trailer += 4;
  }

  for (unsigned int trailerChunk : trailerChunks)
  {
    if (m_header.chunkType == trailerChunk && m_header.trailer != 12 && m_header.trailer != 4)
    {
      m_header.trailer += 4;
      break;
    }
  }

  // Some chunks never have a trailer
  if (m_header.chunkType == 0x1f || m_header.chunkType == 0xc9 ||
      m_header.chunkType == 0x2d || m_header.chunkType == 0xd1)
  {
    m_header.trailer = 0;
  }
  return true;
}

bool libvisio::VSDParser::parseMain()
{
  if (!m_input)
  {
    return false;
  }
  // Seek to trailer stream pointer
  m_input->seek(0x24, librevenge::RVNG_SEEK_SET);

  Pointer trailerPointer;
  readPointer(m_input, trailerPointer);
  bool compressed = ((trailerPointer.Format & 2) == 2);
  unsigned shift = 0;
  if (compressed)
    shift = 4;

  m_input->seek(trailerPointer.Offset, librevenge::RVNG_SEEK_SET);
  VSDInternalStream trailerStream(m_input, trailerPointer.Length, compressed);

  std::vector<std::map<unsigned, XForm> > groupXFormsSequence;
  std::vector<std::map<unsigned, unsigned> > groupMembershipsSequence;
  std::vector<std::list<unsigned> > documentPageShapeOrders;

  VSDStylesCollector stylesCollector(groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders);
  m_collector = &stylesCollector;
  VSD_DEBUG_MSG(("VSDParser::parseMain 1st pass\n"));
  if (!parseDocument(&trailerStream, shift))
    return false;

  _handleLevelChange(0);

  VSDStyles styles = stylesCollector.getStyleSheets();

  VSDContentCollector contentCollector(m_painter, groupXFormsSequence, groupMembershipsSequence, documentPageShapeOrders, styles, m_stencils);
  m_collector = &contentCollector;
  if (m_container)
    parseMetaData();

  VSD_DEBUG_MSG(("VSDParser::parseMain 2nd pass\n"));
  if (!parseDocument(&trailerStream, shift))
    return false;

  return true;
}

void libvisio::VSDParser::parseMetaData() try
{
  if (!m_container)
    return;
  m_container->seek(0, librevenge::RVNG_SEEK_SET);
  if (!m_container->isStructured())
    return;
  VSDMetaData metaData;

  const RVNGInputStreamPtr_t sumaryInfo(m_container->getSubStreamByName("\x05SummaryInformation"));
  if (bool(sumaryInfo))
    metaData.parse(sumaryInfo.get());

  const RVNGInputStreamPtr_t docSumaryInfo(m_container->getSubStreamByName("\005DocumentSummaryInformation"));
  if (bool(docSumaryInfo))
    metaData.parse(docSumaryInfo.get());

  m_container->seek(0, librevenge::RVNG_SEEK_SET);
  metaData.parseTimes(m_container);
  m_collector->collectMetaData(metaData.getMetaData());
}
catch (...)
{
  // Ignore any exceptions in metadata. They are not important enough to stop parsing.
}

bool libvisio::VSDParser::parseDocument(librevenge::RVNGInputStream *input, unsigned shift)
{
  std::set<unsigned> visited;
  try
  {
    handleStreams(input, VSD_TRAILER_STREAM, shift, 0, visited);
    assert(visited.empty());
    return true;
  }
  catch (...)
  {
    assert(visited.empty());
    return false;
  }
}

bool libvisio::VSDParser::extractStencils()
{
  m_extractStencils = true;
  return parseMain();
}

void libvisio::VSDParser::readPointer(librevenge::RVNGInputStream *input, Pointer &ptr)
{
  ptr.Type = readU32(input);
  input->seek(4, librevenge::RVNG_SEEK_CUR); // Skip dword
  ptr.Offset = readU32(input);
  ptr.Length = readU32(input);
  ptr.Format = readU16(input);
}

void libvisio::VSDParser::readPointerInfo(librevenge::RVNGInputStream *input, unsigned /* ptrType */, unsigned shift, unsigned &listSize, int &pointerCount)
{
  VSD_DEBUG_MSG(("VSDParser::readPointerInfo\n"));
  input->seek(shift, librevenge::RVNG_SEEK_SET);
  unsigned offset = readU32(input);
  input->seek(offset+shift-4, librevenge::RVNG_SEEK_SET);
  listSize = readU32(input);
  pointerCount = readS32(input);
  input->seek(4, librevenge::RVNG_SEEK_CUR);
}

void libvisio::VSDParser::handleStreams(librevenge::RVNGInputStream *input, unsigned ptrType, unsigned shift, unsigned level, std::set<unsigned> &visited)
{
  VSD_DEBUG_MSG(("VSDParser::HandleStreams\n"));
  std::vector<unsigned> pointerOrder;
  std::map<unsigned, libvisio::Pointer> PtrList;
  std::map<unsigned, libvisio::Pointer> FontFaces;
  std::map<unsigned, libvisio::Pointer> NameList;
  std::map<unsigned, libvisio::Pointer> NameIDX;

  try
  {
    // Parse out pointers to streams
    unsigned listSize = 0;
    int pointerCount = 0;
    readPointerInfo(input, ptrType, shift, listSize, pointerCount);
    for (int i = 0; i < pointerCount; i++)
    {
      Pointer ptr;
      readPointer(input, ptr);
      if (ptr.Type == 0)
        continue;

      if (ptr.Type == VSD_FONTFACES)
        FontFaces[i] = ptr;
      else if (ptr.Type == VSD_NAME_LIST2)
        NameList[i] = ptr;
      else if (ptr.Type == VSD_NAMEIDX || ptr.Type == VSD_NAMEIDX123)
        NameIDX[i] = ptr;
      else if (ptr.Type)
        PtrList[i] = ptr;
    }
    if (listSize <= 1)
      listSize = 0;
    while (listSize--)
      pointerOrder.push_back(readU32(input));
  }
  catch (const EndOfStreamException &)
  {
    pointerOrder.clear();
    PtrList.clear();
    FontFaces.clear();
    NameList.clear();
  }

  std::map<unsigned, libvisio::Pointer>::iterator iter;
  for (iter = NameList.begin(); iter != NameList.end(); ++iter)
    handleStream(iter->second, iter->first, level+1, visited);

  for (iter = NameIDX.begin(); iter != NameIDX.end(); ++iter)
    handleStream(iter->second, iter->first, level+1, visited);

  for (iter = FontFaces.begin(); iter != FontFaces.end(); ++iter)
    handleStream(iter->second, iter->first, level+1, visited);

  if (!pointerOrder.empty())
  {
    for (unsigned int j : pointerOrder)
    {
      iter = PtrList.find(j);
      if (iter != PtrList.end())
      {
        handleStream(iter->second, iter->first, level+1, visited);
        PtrList.erase(iter);
      }
    }
  }
  for (iter = PtrList.begin(); iter != PtrList.end(); ++iter)
    handleStream(iter->second, iter->first, level+1, visited);

}

void libvisio::VSDParser::handleStream(const Pointer &ptr, unsigned idx, unsigned level, std::set<unsigned> &visited)
{
  VSD_DEBUG_MSG(("VSDParser::HandleStream %u type 0x%x\n", idx, ptr.Type));
  m_header.level = level;
  m_header.id = idx;
  m_header.chunkType = ptr.Type;
  _handleLevelChange(level);
  VSDStencil tmpStencil;
  bool compressed = ((ptr.Format & 2) == 2);
  m_input->seek(ptr.Offset, librevenge::RVNG_SEEK_SET);
  VSDInternalStream tmpInput(m_input, ptr.Length, compressed);
  m_header.dataLength = tmpInput.getSize();
  unsigned shift = compressed ? 4 : 0;
  switch (ptr.Type)
  {
  case VSD_STYLES:
    m_isInStyles = true;
    break;
  case VSD_PAGES:
    if (m_extractStencils)
      return;
    break;
  case VSD_PAGE:
    if (m_extractStencils)
      return;
    if (!(ptr.Format&0x1))
      m_isBackgroundPage = true;
    else
      m_isBackgroundPage = false;
    _nameFromId(m_currentPageName, idx, level+1);
    m_collector->startPage(idx);
    break;
  case VSD_STENCILS:
    if (m_extractStencils)
      break;
    if (m_stencils.count())
      return;
    m_isStencilStarted = true;
    break;
  case VSD_STENCIL_PAGE:
    if (m_extractStencils)
    {
      m_isBackgroundPage = false;
      _nameFromId(m_currentPageName, idx, level+1);
      m_collector->startPage(idx);
    }
    else
      m_currentStencil = &tmpStencil;
    break;
  case VSD_SHAPE_GROUP:
  case VSD_SHAPE_SHAPE:
  case VSD_SHAPE_FOREIGN:
    m_currentShapeID = idx;
    break;
  case VSD_OLE_LIST:
    if (!m_shape.m_foreign)
      m_shape.m_foreign = make_unique<ForeignData>();
    m_shape.m_foreign->dataId = idx;
    break;
  default:
    break;
  }

  if ((ptr.Format >> 4) == 0x4 || (ptr.Format >> 4) == 0x5 || (ptr.Format >> 4) == 0x0)
  {
    handleBlob(&tmpInput, shift, level+1);
    if ((ptr.Format >> 4) == 0x5 && ptr.Type != VSD_COLORS)
    {
      const auto it = visited.insert(ptr.Offset);
      if (it.second)
      {
        try
        {
          handleStreams(&tmpInput, ptr.Type, shift, level+1, visited);
        }
        catch (...)
        {
          visited.erase(it.first);
          throw;
        }
        visited.erase(it.first);
      }
    }
  }
  else if ((ptr.Format >> 4) == 0xd || (ptr.Format >> 4) == 0xc || (ptr.Format >> 4) == 0x8)
    handleChunks(&tmpInput, level+1);

  switch (ptr.Type)
  {
  case VSD_STYLES:
    _handleLevelChange(0);
    m_isInStyles = false;
    break;
  case VSD_PAGE:
    _handleLevelChange(0);
    m_collector->endPage();
    break;
  case VSD_PAGES:
    _handleLevelChange(0);
    m_collector->endPages();
    break;
  case VSD_STENCILS:
    _handleLevelChange(0);
    if (m_extractStencils)
      m_collector->endPages();
    else
      m_isStencilStarted = false;
    break;
  case VSD_STENCIL_PAGE:
    _handleLevelChange(0);
    if (m_extractStencils)
      m_collector->endPage();
    else if (m_currentStencil)
    {
      m_stencils.addStencil(idx, *m_currentStencil);
      m_currentStencil = nullptr;
    }
    break;
  case VSD_SHAPE_GROUP:
  case VSD_SHAPE_SHAPE:
  case VSD_SHAPE_FOREIGN:
    if (m_isStencilStarted)
    {
      _handleLevelChange(0);
      if (m_currentStencil)
        m_currentStencil->addStencilShape(m_shape.m_shapeId, m_shape);
    }
    break;
  default:
    break;
  }

}

void libvisio::VSDParser::handleBlob(librevenge::RVNGInputStream *input, unsigned shift, unsigned level)
{
  try
  {
    m_header.level = level;
    input->seek(shift, librevenge::RVNG_SEEK_SET);
    m_header.dataLength -= shift;
    _handleLevelChange(m_header.level);
    handleChunk(input);
  }
  catch (EndOfStreamException &)
  {
    VSD_DEBUG_MSG(("VSDParser::handleBlob - catching EndOfStreamException\n"));
  }
}

void libvisio::VSDParser::handleChunks(librevenge::RVNGInputStream *input, unsigned level)
{
  long endPos = 0;

  while (!input->isEnd())
  {
    if (!getChunkHeader(input))
      return;
    m_header.level += level;
    endPos = m_header.dataLength+m_header.trailer+input->tell();

    _handleLevelChange(m_header.level);
    VSD_DEBUG_MSG(("VSDParser::handleChunks - parsing chunk type 0x%x\n", m_header.chunkType));
    handleChunk(input);
    input->seek(endPos, librevenge::RVNG_SEEK_SET);
  }
}

void libvisio::VSDParser::handleChunk(librevenge::RVNGInputStream *input)
{
  switch (m_header.chunkType)
  {
  case VSD_SHAPE_GROUP:
  case VSD_SHAPE_SHAPE:
  case VSD_SHAPE_FOREIGN:
    readShape(input);
    break;
  case VSD_XFORM_DATA:
    readXFormData(input);
    break;
  case VSD_XFORM_1D:
    readXForm1D(input);
    break;
  case VSD_TEXT_XFORM:
    readTxtXForm(input);
    break;
  case VSD_SHAPE_LIST:
    readShapeList(input);
    break;
  case VSD_SHAPE_ID:
    readShapeId(input);
    break;
  case VSD_LINE:
    readLine(input);
    break;
  case VSD_PROP_LIST:
    readPropList(input);
    break;
  case VSD_FILL_AND_SHADOW:
    readFillAndShadow(input);
    break;
  case VSD_GEOM_LIST:
    readGeomList(input);
    break;
  case VSD_GEOMETRY:
    readGeometry(input);
    break;
  case VSD_MOVE_TO:
    readMoveTo(input);
    break;
  case VSD_LINE_TO:
    readLineTo(input);
    break;
  case VSD_ARC_TO:
    readArcTo(input);
    break;
  case VSD_ELLIPSE:
    readEllipse(input);
    break;
  case VSD_ELLIPTICAL_ARC_TO:
    readEllipticalArcTo(input);
    break;
  case VSD_NURBS_TO:
    readNURBSTo(input);
    break;
  case VSD_POLYLINE_TO:
    readPolylineTo(input);
    break;
  case VSD_INFINITE_LINE:
    readInfiniteLine(input);
    break;
  case VSD_SHAPE_DATA:
    readShapeData(input);
    break;
  case VSD_FOREIGN_DATA_TYPE:
    readForeignDataType(input);
    break;
  case VSD_FOREIGN_DATA:
    readForeignData(input);
    break;
  case VSD_OLE_LIST:
    readOLEList(input);
    break;
  case VSD_OLE_DATA:
    readOLEData(input);
    break;
  case VSD_NAMEIDX:
    readNameIDX(input);
    break;
  case VSD_NAMEIDX123:
    readNameIDX123(input);
    break;
  case VSD_PAGE_PROPS:
    readPageProps(input);
    break;
  case VSD_CHAR_LIST:
    readCharList(input);
    break;
  case VSD_PARA_LIST:
    readParaList(input);
    break;
  case VSD_TEXT:
    readText(input);
    break;
  case VSD_CHAR_IX:
    readCharIX(input);
    break;
  case VSD_PARA_IX:
    readParaIX(input);
    break;
  case VSD_TEXT_BLOCK:
    readTextBlock(input);
    break;
  case VSD_FONT_IX: // ver 6 only
    readFontIX(input);
    break;
  case VSD_PAGE:
    readPage(input);
    break;
  case VSD_STENCIL_PAGE:
    if (m_extractStencils)
      readPage(input);
    break;
  case VSD_SPLINE_START:
    readSplineStart(input);
    break;
  case VSD_SPLINE_KNOT:
    readSplineKnot(input);
    break;
  case VSD_NAME_LIST:
    readNameList(input);
    break;
  case VSD_NAME_LIST2:
    readNameList2(input);
    break;
  case VSD_NAME:
    readName(input);
    break;
  case VSD_NAME2:
    readName2(input);
    break;
  case VSD_FIELD_LIST:
    readFieldList(input);
    break;
  case VSD_TEXT_FIELD:
    readTextField(input);
    break;
  case VSD_STYLE_SHEET:
    readStyleSheet(input);
    break;
  case VSD_PAGE_SHEET:
    readPageSheet(input);
    break;
  case VSD_COLORS:
    readColours(input);
    break;
  case VSD_FONTFACE: // substreams of FONTAFACES stream, ver 11 only
    readFont(input);
    break;
  case VSD_MISC:
    readMisc(input);
    break;
  case VSD_LAYER_LIST:
    readLayerList(input);
    break;
  case VSD_LAYER:
    readLayer(input);
    break;
  case VSD_LAYER_MEMBERSHIP:
    readLayerMem(input);
    break;
  case VSD_TABS_DATA_LIST:
    readTabsDataList(input);
    break;
  case VSD_TABS_DATA_1:
  case VSD_TABS_DATA_2:
  case VSD_TABS_DATA_3:
    readTabsData(input);
    break;
  default:
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
  }
}

void libvisio::VSDParser::_flushShape()
{
  if (!m_isShapeStarted)
    return;

  m_collector->collectShape(m_shape.m_shapeId, m_currentShapeLevel, m_shape.m_parent, m_shape.m_masterPage, m_shape.m_masterShape, m_shape.m_lineStyleId, m_shape.m_fillStyleId, m_shape.m_textStyleId);

  m_collector->collectShapesOrder(0, m_currentShapeLevel+2, m_shape.m_shapeList.getShapesOrder());

  m_collector->collectXFormData(m_currentShapeLevel+2, m_shape.m_xform);

  m_collector->collectLayerMem(m_currentShapeLevel+2, m_shape.m_layerMem);

  m_collector->collectMisc(m_currentShapeLevel+2, m_shape.m_misc);

  if (m_shape.m_txtxform)
    m_collector->collectTxtXForm(m_currentShapeLevel+2, *(m_shape.m_txtxform));

  m_collector->collectLine(m_currentShapeLevel+2, m_shape.m_lineStyle.width, m_shape.m_lineStyle.colour, m_shape.m_lineStyle.pattern,
                           m_shape.m_lineStyle.startMarker, m_shape.m_lineStyle.endMarker, m_shape.m_lineStyle.cap, m_shape.m_lineStyle.rounding,
                           m_shape.m_lineStyle.qsLineColour, m_shape.m_lineStyle.qsLineMatrix);

  m_collector->collectFillAndShadow(m_currentShapeLevel+2, m_shape.m_fillStyle.fgColour, m_shape.m_fillStyle.bgColour, m_shape.m_fillStyle.pattern,
                                    m_shape.m_fillStyle.fgTransparency, m_shape.m_fillStyle.bgTransparency, m_shape.m_fillStyle.shadowPattern,
                                    m_shape.m_fillStyle.shadowFgColour, m_shape.m_fillStyle.shadowOffsetX, m_shape.m_fillStyle.shadowOffsetY,
                                    m_shape.m_fillStyle.qsFillColour, m_shape.m_fillStyle.qsShadowColour, m_shape.m_fillStyle.qsFillMatrix);

  m_collector->collectTextBlock(m_currentShapeLevel+2, m_shape.m_textBlockStyle.leftMargin, m_shape.m_textBlockStyle.rightMargin,
                                m_shape.m_textBlockStyle.topMargin, m_shape.m_textBlockStyle.bottomMargin, m_shape.m_textBlockStyle.verticalAlign,
                                m_shape.m_textBlockStyle.isTextBkgndFilled, m_shape.m_textBlockStyle.textBkgndColour,
                                m_shape.m_textBlockStyle.defaultTabStop, m_shape.m_textBlockStyle.textDirection);

  if (m_shape.m_foreign)
    m_collector->collectForeignDataType(m_currentShapeLevel+2, m_shape.m_foreign->type, m_shape.m_foreign->format,
                                        m_shape.m_foreign->offsetX, m_shape.m_foreign->offsetY, m_shape.m_foreign->width, m_shape.m_foreign->height);

  for (std::map<unsigned, NURBSData>::const_iterator iterNurbs = m_shape.m_nurbsData.begin(); iterNurbs != m_shape.m_nurbsData.end(); ++iterNurbs)
    m_collector->collectShapeData(iterNurbs->first, m_currentShapeLevel+2, iterNurbs->second.xType, iterNurbs->second.yType,
                                  iterNurbs->second.degree, iterNurbs->second.lastKnot, iterNurbs->second.points,
                                  iterNurbs->second.knots, iterNurbs->second.weights);

  for (std::map<unsigned, PolylineData>::const_iterator iterPoly = m_shape.m_polylineData.begin(); iterPoly != m_shape.m_polylineData.end(); ++iterPoly)
    m_collector->collectShapeData(iterPoly->first, m_currentShapeLevel+2, iterPoly->second.xType, iterPoly->second.yType, iterPoly->second.points);

  for (std::map<unsigned, VSDName>::const_iterator iterName = m_shape.m_names.begin(); iterName != m_shape.m_names.end(); ++iterName)
    m_collector->collectName(iterName->first, m_currentShapeLevel+2, iterName->second.m_data, iterName->second.m_format);

  if (m_shape.m_foreign && m_shape.m_foreign->data.size())
    m_collector->collectForeignData(m_currentShapeLevel+1, m_shape.m_foreign->data);

  m_collector->collectTabsDataList(m_currentShapeLevel+1, m_shape.m_tabSets);

  if (!m_shape.m_fields.empty())
    m_shape.m_fields.handle(m_collector);

  if (m_shape.m_text.size())
    m_collector->collectText(m_currentShapeLevel+1, m_shape.m_text, m_shape.m_textFormat);


  for (std::map<unsigned, VSDGeometryList>::const_iterator iterGeom = m_shape.m_geometries.begin(); iterGeom != m_shape.m_geometries.end(); ++iterGeom)
    iterGeom->second.handle(m_collector);

  m_collector->collectDefaultCharStyle(m_shape.m_charStyle.charCount, m_shape.m_charStyle.font, m_shape.m_charStyle.colour,
                                       m_shape.m_charStyle.size, m_shape.m_charStyle.bold, m_shape.m_charStyle.italic, m_shape.m_charStyle.underline,
                                       m_shape.m_charStyle.doubleunderline, m_shape.m_charStyle.strikeout, m_shape.m_charStyle.doublestrikeout,
                                       m_shape.m_charStyle.allcaps, m_shape.m_charStyle.initcaps, m_shape.m_charStyle.smallcaps,
                                       m_shape.m_charStyle.superscript, m_shape.m_charStyle.subscript, m_shape.m_charStyle.scaleWidth);

  m_shape.m_charList.handle(m_collector);

  m_collector->collectDefaultParaStyle(m_shape.m_paraStyle.charCount, m_shape.m_paraStyle.indFirst, m_shape.m_paraStyle.indLeft,
                                       m_shape.m_paraStyle.indRight, m_shape.m_paraStyle.spLine, m_shape.m_paraStyle.spBefore,
                                       m_shape.m_paraStyle.spAfter, m_shape.m_paraStyle.align, m_shape.m_paraStyle.bullet,
                                       m_shape.m_paraStyle.bulletStr, m_shape.m_paraStyle.bulletFont, m_shape.m_paraStyle.bulletFontSize,
                                       m_shape.m_paraStyle.textPosAfterBullet, m_shape.m_paraStyle.flags);

  m_shape.m_paraList.handle(m_collector);
}

void libvisio::VSDParser::_handleLevelChange(unsigned level)
{
  if (level == m_currentLevel)
    return;
  if (level <= m_currentShapeLevel+1)
  {
    if (!m_shape.m_geometries.empty() && m_currentGeometryList && m_currentGeometryList->empty())
    {
      m_shape.m_geometries.erase(--m_currentGeomListCount);
      m_currentGeometryList = nullptr;
    }
    m_collector->collectShapesOrder(0, m_currentShapeLevel+2, m_shapeList.getShapesOrder());
    m_shapeList.clear();

  }
  if (level <= m_currentShapeLevel)
  {
    if (!m_isStencilStarted)
    {
      _flushShape();
      m_shape.clear();
      m_currentGeometryList = nullptr;
    }
    m_isShapeStarted = false;
    m_currentShapeLevel = 0;
  }
  m_currentLevel = level;
}

// --- READERS ---

void libvisio::VSDParser::readEllipticalArcTo(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x3 = readDouble(input); // End x
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y3 = readDouble(input); // End y
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x2 = readDouble(input); // Mid x
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y2 = readDouble(input); // Mid y
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double angle = readDouble(input); // Angle
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double ecc = readDouble(input); // Eccentricity

  if (m_currentGeometryList)
    m_currentGeometryList->addEllipticalArcTo(m_header.id, m_header.level, x3, y3, x2, y2, angle, ecc);
}


void libvisio::VSDParser::readForeignData(librevenge::RVNGInputStream *input)
{
  unsigned long tmpBytesRead = 0;
  const unsigned char *buffer = input->read(m_header.dataLength, tmpBytesRead);
  if (m_header.dataLength != tmpBytesRead)
    return;
  librevenge::RVNGBinaryData binaryData(buffer, tmpBytesRead);

  if (!m_shape.m_foreign)
    m_shape.m_foreign = make_unique<ForeignData>();
  m_shape.m_foreign->dataId = m_header.id;
  m_shape.m_foreign->data = binaryData;
}

void libvisio::VSDParser::readOLEList(librevenge::RVNGInputStream * /* input */)
{
}

void libvisio::VSDParser::readOLEData(librevenge::RVNGInputStream *input)
{
  unsigned long tmpBytesRead = 0;
  const unsigned char *buffer = input->read(m_header.dataLength, tmpBytesRead);
  if (m_header.dataLength != tmpBytesRead)
    return;
  librevenge::RVNGBinaryData oleData(buffer, tmpBytesRead);

  if (!m_shape.m_foreign)
    m_shape.m_foreign = make_unique<ForeignData>();
  // Append data instead of setting it - allows multi-stream OLE objects
  m_shape.m_foreign->data.append(oleData);

}

void libvisio::VSDParser::readTabsData(librevenge::RVNGInputStream *input)
{
  m_shape.m_tabSets[m_header.id].m_numChars = getUInt(input);
  unsigned char numStops = readU8(input);
  m_shape.m_tabSets[m_header.id].m_tabStops.clear();
  for (unsigned char i = 0; i < numStops; ++i)
  {
    input->seek(1, librevenge::RVNG_SEEK_CUR);
    m_shape.m_tabSets[m_header.id].m_tabStops[i].m_position = readDouble(input);
    m_shape.m_tabSets[m_header.id].m_tabStops[i].m_alignment = readU8(input);
    m_shape.m_tabSets[m_header.id].m_tabStops[i].m_leader = readU8(input);
  }
}

void libvisio::VSDParser::readNameIDX(librevenge::RVNGInputStream *input)
{
  std::map<unsigned, VSDName> names;
  unsigned recordCount = readU32(input);
  if (recordCount > getRemainingLength(input) / 13)
    recordCount = getRemainingLength(input) / 13;
  for (unsigned i = 0; i < recordCount; ++i)
  {
    unsigned nameId = readU32(input);
    if (nameId != readU32(input))
    {
      VSD_DEBUG_MSG(("VSDParser::readNameIDX --> mismatch of first two dwords\n"));
    }
    unsigned elementId = readU32(input);
    input->seek(1, librevenge::RVNG_SEEK_CUR);
    std::map<unsigned, VSDName>::const_iterator iter = m_names.find(nameId);
    if (iter != m_names.end())
      names[elementId] = iter->second;
  }
  m_namesMapMap[m_header.level] = names;
}

void libvisio::VSDParser::readNameIDX123(librevenge::RVNGInputStream *input)
{
  std::map<unsigned, VSDName> names;
  long endPosition = input->tell() + m_header.dataLength;
  while (!input->isEnd() && input->tell() < endPosition)
  {
    unsigned nameId = getUInt(input);
    unsigned elementId = getUInt(input);
    std::map<unsigned, VSDName>::const_iterator iter = m_names.find(nameId);
    if (iter != m_names.end())
      names[elementId] = iter->second;
  }
  m_namesMapMap[m_header.level] = names;

}

void libvisio::VSDParser::readEllipse(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double cx = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double cy = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double xleft = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double yleft = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double xtop = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double ytop = readDouble(input);

  if (m_currentGeometryList)
    m_currentGeometryList->addEllipse(m_header.id, m_header.level, cx, cy, xleft, yleft, xtop, ytop);
}

void libvisio::VSDParser::readLine(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double strokeWidth = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  Colour c;
  c.r = readU8(input);
  c.g = readU8(input);
  c.b = readU8(input);
  c.a = readU8(input);
  unsigned char linePattern = readU8(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double rounding = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  unsigned char startMarker = readU8(input);
  unsigned char endMarker = readU8(input);
  unsigned char lineCap = readU8(input);

  if (m_isInStyles)
    m_collector->collectLineStyle(m_header.level, strokeWidth, c, linePattern, startMarker, endMarker, lineCap, rounding, -1, -1);
  else
    m_shape.m_lineStyle.override(VSDOptionalLineStyle(strokeWidth, c, linePattern, startMarker, endMarker, lineCap, rounding, -1, -1));
}

void libvisio::VSDParser::readTextBlock(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double leftMargin = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double rightMargin = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double topMargin = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double bottomMargin = readDouble(input);
  unsigned char verticalAlign = readU8(input);
  const unsigned char bgColourIdx = readU8(input);
  const bool isBgFilled = bgColourIdx != 0 && bgColourIdx != 0xff;
  Colour c;
  c.r = readU8(input);
  c.g = readU8(input);
  c.b = readU8(input);
  c.a = readU8(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double defaultTabStop = readDouble(input);
  input->seek(12, librevenge::RVNG_SEEK_CUR);
  unsigned char textDirection = readU8(input);

  if (m_isInStyles)
    m_collector->collectTextBlockStyle(m_header.level, leftMargin, rightMargin, topMargin, bottomMargin,
                                       verticalAlign, isBgFilled, c, defaultTabStop, textDirection);
  else
    m_shape.m_textBlockStyle.override(VSDOptionalTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin,
                                                                verticalAlign, isBgFilled, c, defaultTabStop, textDirection));
}

void libvisio::VSDParser::readGeomList(librevenge::RVNGInputStream *input)
{
  if (!m_shape.m_geometries.empty() && m_currentGeometryList && m_currentGeometryList->empty())
    m_shape.m_geometries.erase(--m_currentGeomListCount);
  // Since this is a map, this will default construct an element and then
  // the m_currentGeometryList pointer takes its address and we will work
  // on it over that pointer.
  m_currentGeometryList = &m_shape.m_geometries[m_currentGeomListCount++];

  if (m_header.trailer)
  {
    uint32_t subHeaderLength = readU32(input);
    uint32_t childrenListLength = readU32(input);
    input->seek(subHeaderLength, librevenge::RVNG_SEEK_CUR);
    std::vector<unsigned> geometryOrder;
    if (childrenListLength > getRemainingLength(input))
      childrenListLength = getRemainingLength(input);
    geometryOrder.reserve(childrenListLength / sizeof(uint32_t));
    for (size_t i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
      geometryOrder.push_back(readU32(input));

    if (m_currentGeometryList)
      m_currentGeometryList->setElementsOrder(geometryOrder);
  }

  // We want the collectors to still get the level information
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);
}

void libvisio::VSDParser::readCharList(librevenge::RVNGInputStream *input)
{
  // We want the collectors to still get the level information
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);

  if (m_header.trailer)
  {
    uint32_t subHeaderLength = readU32(input);
    uint32_t childrenListLength = readU32(input);
    input->seek(subHeaderLength, librevenge::RVNG_SEEK_CUR);
    if (childrenListLength > getRemainingLength(input))
      childrenListLength = getRemainingLength(input);
    std::vector<unsigned> characterOrder;
    characterOrder.reserve(childrenListLength / sizeof(uint32_t));
    for (size_t i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
      characterOrder.push_back(readU32(input));

    m_shape.m_charList.setElementsOrder(characterOrder);
  }
}

void libvisio::VSDParser::readParaList(librevenge::RVNGInputStream *input)
{
  // We want the collectors to still get the level information
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);

  if (m_header.trailer)
  {
    uint32_t subHeaderLength = readU32(input);
    uint32_t childrenListLength = readU32(input);
    input->seek(subHeaderLength, librevenge::RVNG_SEEK_CUR);
    if (childrenListLength > getRemainingLength(input))
      childrenListLength = getRemainingLength(input);
    std::vector<unsigned> paragraphOrder;
    paragraphOrder.reserve(childrenListLength / sizeof(uint32_t));
    for (size_t i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
      paragraphOrder.push_back(readU32(input));

    m_shape.m_paraList.setElementsOrder(paragraphOrder);
  }
}

void libvisio::VSDParser::readPropList(librevenge::RVNGInputStream * /* input */)
{
}

void libvisio::VSDParser::readTabsDataList(librevenge::RVNGInputStream *input)
{
  // We want the collectors to still get the level information
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);

  if (m_header.trailer)
  {
    uint32_t subHeaderLength = readU32(input);
    uint32_t childrenListLength = readU32(input);
    input->seek(subHeaderLength, librevenge::RVNG_SEEK_CUR);
    if (childrenListLength > getRemainingLength(input))
      childrenListLength = getRemainingLength(input);
    std::vector<unsigned> tabsOrder;
    tabsOrder.reserve(childrenListLength / sizeof(uint32_t));
    for (size_t i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
      tabsOrder.push_back(readU32(input));
  }
}

void libvisio::VSDParser::readLayerList(librevenge::RVNGInputStream *input)
{
  // We want the collectors to still get the level information
  if (!m_isStencilStarted)
    m_collector->collectUnhandledChunk(m_header.id, m_header.level);

  if (m_header.trailer)
  {
    uint32_t subHeaderLength = readU32(input);
    uint32_t childrenListLength = readU32(input);
    input->seek(subHeaderLength, librevenge::RVNG_SEEK_CUR);
    if (childrenListLength > getRemainingLength(input))
      childrenListLength = getRemainingLength(input);
    std::vector<unsigned> layerOrder;
    layerOrder.reserve(childrenListLength / sizeof(uint32_t));
    for (size_t i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
      layerOrder.push_back(readU32(input));
  }
}

void libvisio::VSDParser::readLayer(librevenge::RVNGInputStream *input)
{
  libvisio::VSDLayer layer;
  input->seek(8, librevenge::RVNG_SEEK_CUR);
  unsigned char colourId = readU8(input);
  if (colourId == 0xff)
    input->seek(4, librevenge::RVNG_SEEK_CUR);
  else
  {
    libvisio::Colour colour;
    colour.r = readU8(input);
    colour.g = readU8(input);
    colour.b = readU8(input);
    colour.a = readU8(input);
    layer.m_colour = colour;
  }
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  layer.m_visible = !!readU8(input);
  layer.m_printable = !!readU8(input);

  m_collector->collectLayer(m_header.id, m_header.level, layer);
}

void libvisio::VSDParser::readLayerMem(librevenge::RVNGInputStream *input)
{
  input->seek(13, librevenge::RVNG_SEEK_CUR);
  unsigned textLength = readU8(input);

  librevenge::RVNGBinaryData  textStream;
  unsigned long numBytesRead = 0;
  const unsigned char *tmpBuffer = input->read(textLength*2, numBytesRead);
  if (numBytesRead)
  {
    textStream.append(tmpBuffer, numBytesRead);
    m_shape.m_layerMem.m_data = textStream;
    m_shape.m_layerMem.m_format = libvisio::VSD_TEXT_UTF16;
  }

}

void libvisio::VSDParser::readPage(librevenge::RVNGInputStream *input)
{
  input->seek(8, librevenge::RVNG_SEEK_CUR); //sub header length and children list length
  unsigned backgroundPageID = readU32(input);
  m_collector->collectPage(m_header.id, m_header.level, backgroundPageID, m_isBackgroundPage, m_currentPageName);
}

void libvisio::VSDParser::readGeometry(librevenge::RVNGInputStream *input)
{
  unsigned char geomFlags = readU8(input);
  bool noFill = (!!(geomFlags & 1));
  bool noLine = (!!(geomFlags & 2));
  bool noShow = (!!(geomFlags & 4));

  if (m_currentGeometryList)
    m_currentGeometryList->addGeometry(m_header.id, m_header.level, noFill, noLine, noShow);
}

void libvisio::VSDParser::readMoveTo(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y = readDouble(input);

  if (m_currentGeometryList)
    m_currentGeometryList->addMoveTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDParser::readLineTo(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y = readDouble(input);

  if (m_currentGeometryList)
    m_currentGeometryList->addLineTo(m_header.id, m_header.level, x, y);
}

void libvisio::VSDParser::readArcTo(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x2 = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y2 = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double bow = readDouble(input);

  if (m_currentGeometryList)
    m_currentGeometryList->addArcTo(m_header.id, m_header.level, x2, y2, bow);
}

void libvisio::VSDParser::readXFormData(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform.pinX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform.pinY = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform.width = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform.height = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform.pinLocX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform.pinLocY = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform.angle = readDouble(input);
  m_shape.m_xform.flipX = !!readU8(input);
  m_shape.m_xform.flipY = !!readU8(input);
}

void libvisio::VSDParser::readXForm1D(librevenge::RVNGInputStream *input)
{
  if (!m_shape.m_xform1d)
    m_shape.m_xform1d = make_unique<XForm1D>();
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform1d->beginX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform1d->beginY = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform1d->endX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_xform1d->endY = readDouble(input);
}

void libvisio::VSDParser::readTxtXForm(librevenge::RVNGInputStream *input)
{
  m_shape.m_txtxform = make_unique<XForm>();
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_txtxform->pinX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_txtxform->pinY = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_txtxform->width = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_txtxform->height = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_txtxform->pinLocX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_txtxform->pinLocY = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shape.m_txtxform->angle = readDouble(input);
}

void libvisio::VSDParser::readShapeId(librevenge::RVNGInputStream *input)
{
  if (!m_isShapeStarted)
    m_shapeList.addShapeId(m_header.id, getUInt(input));
  else
    m_shape.m_shapeList.addShapeId(m_header.id, getUInt(input));
}

void libvisio::VSDParser::readShapeList(librevenge::RVNGInputStream *input)
{
  // We want the collectors to still get the level information
  m_collector->collectUnhandledChunk(m_header.id, m_header.level);

  if (m_header.trailer)
  {
    uint32_t subHeaderLength = readU32(input);
    uint32_t childrenListLength = readU32(input);
    input->seek(subHeaderLength, librevenge::RVNG_SEEK_CUR);
    if (childrenListLength > getRemainingLength(input))
      childrenListLength = getRemainingLength(input);
    std::vector<unsigned> shapeOrder;
    shapeOrder.reserve(childrenListLength / sizeof(uint32_t));
    for (size_t i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
      shapeOrder.push_back(readU32(input));

    if (!m_isShapeStarted)
      m_shapeList.setElementsOrder(shapeOrder);
    else
      m_shape.m_shapeList.setElementsOrder(shapeOrder);
  }
}

void libvisio::VSDParser::readForeignDataType(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double imgOffsetX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double imgOffsetY = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double imgWidth = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double imgHeight = readDouble(input);
  unsigned foreignType = readU16(input);
  unsigned foreignMapMode = readU16(input);
  if (foreignMapMode == 0x8)
    foreignType = 0x4;
  input->seek(0x9, librevenge::RVNG_SEEK_CUR);
  unsigned foreignFormat = readU32(input);

  if (!m_shape.m_foreign)
    m_shape.m_foreign = make_unique<ForeignData>();
  m_shape.m_foreign->typeId = m_header.id;
  m_shape.m_foreign->type = foreignType;
  m_shape.m_foreign->format = foreignFormat;
  m_shape.m_foreign->offsetX = imgOffsetX;
  m_shape.m_foreign->offsetY = imgOffsetY;
  m_shape.m_foreign->width = imgWidth;
  m_shape.m_foreign->height = imgHeight;
}

void libvisio::VSDParser::readPageProps(librevenge::RVNGInputStream *input)
{
  // Skip bytes representing unit to *display* (value is always inches)
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  const double pageWidth = std::max<double>(readDouble(input), 0);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  const double pageHeight = std::max<double>(readDouble(input), 0);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shadowOffsetX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_shadowOffsetY = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  const double numerator = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double denominator = readDouble(input);
  if (VSD_ALMOST_ZERO(denominator))
    denominator = 1;

  const double scale = std::abs(numerator / denominator);

  if (m_isStencilStarted && m_currentStencil)
  {
    m_currentStencil->m_shadowOffsetX = m_shadowOffsetX;
    m_currentStencil->m_shadowOffsetY = m_shadowOffsetY;
  }
  m_collector->collectPageProps(m_header.id, m_header.level, pageWidth, pageHeight, m_shadowOffsetX, m_shadowOffsetY, scale);
}

void libvisio::VSDParser::readShape(librevenge::RVNGInputStream *input)
{
  m_currentGeomListCount = 0;
  m_isShapeStarted = true;
  m_shapeList.clear();
  if (m_header.id != MINUS_ONE)
    m_currentShapeID = m_header.id;
  m_currentShapeLevel = m_header.level;
  unsigned parent = 0;
  auto masterPage = MINUS_ONE;
  auto masterShape = MINUS_ONE;
  auto lineStyle = MINUS_ONE;
  auto fillStyle = MINUS_ONE;
  auto textStyle = MINUS_ONE;

  try
  {
    input->seek(10, librevenge::RVNG_SEEK_CUR);
    parent = readU32(input);
    input->seek(4, librevenge::RVNG_SEEK_CUR);
    masterPage = readU32(input);
    input->seek(4, librevenge::RVNG_SEEK_CUR);
    masterShape = readU32(input);
    input->seek(0x4, librevenge::RVNG_SEEK_CUR);
    fillStyle = readU32(input);
    input->seek(4, librevenge::RVNG_SEEK_CUR);
    lineStyle = readU32(input);
    input->seek(4, librevenge::RVNG_SEEK_CUR);
    textStyle = readU32(input);
  }
  catch (const EndOfStreamException &)
  {
  }

  m_shape.clear();
  m_currentGeometryList = nullptr;
  const VSDShape *tmpShape = m_stencils.getStencilShape(masterPage, masterShape);
  if (tmpShape)
  {
    if (tmpShape->m_foreign)
      m_shape.m_foreign = make_unique<ForeignData>(*(tmpShape->m_foreign));
    m_shape.m_xform = tmpShape->m_xform;
    if (tmpShape->m_txtxform)
      m_shape.m_txtxform = make_unique<XForm>(*(tmpShape->m_txtxform));
    m_shape.m_tabSets = tmpShape->m_tabSets;
    m_shape.m_text = tmpShape->m_text;
    m_shape.m_textFormat = tmpShape->m_textFormat;
    m_shape.m_misc = tmpShape->m_misc;
  }

  m_shape.m_lineStyleId = lineStyle;
  m_shape.m_fillStyleId = fillStyle;
  m_shape.m_textStyleId = textStyle;
  m_shape.m_parent = parent;
  m_shape.m_masterPage = masterPage;
  m_shape.m_masterShape = masterShape;
  m_shape.m_shapeId = m_currentShapeID;
  m_currentShapeID = MINUS_ONE;
}

void libvisio::VSDParser::readNURBSTo(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y = readDouble(input);
  double knot = readDouble(input); // Second last knot
  double weight = readDouble(input); // Last weight
  double knotPrev = readDouble(input); // First knot
  double weightPrev = readDouble(input); // First weight

  // Detect whether to use Shape Data block
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  unsigned char useData = readU8(input);
  if (useData == 0x8a)
  {
    input->seek(3, librevenge::RVNG_SEEK_CUR);
    unsigned dataId = readU32(input);

    if (m_currentGeometryList)
      m_currentGeometryList->addNURBSTo(m_header.id, m_header.level, x, y, knot, knotPrev, weight, weightPrev, dataId);
    return;
  }

  std::vector<double> knotVector;
  knotVector.push_back(knotPrev);
  std::vector<std::pair<double, double> > controlPoints;
  std::vector<double> weights;
  weights.push_back(weightPrev);

  input->seek(9, librevenge::RVNG_SEEK_CUR); // Seek to blocks at offset 0x50 (80)
  unsigned long chunkBytesRead = 0x50;

  // Find formula block referring to cell E (cell 6)
  unsigned cellRef = 0;
  unsigned length = 0;
  unsigned long inputPos = input->tell();
  while (cellRef != 6 && !input->isEnd() &&
         m_header.dataLength - chunkBytesRead > 4)
  {
    length = readU32(input);
    input->seek(1, librevenge::RVNG_SEEK_CUR);
    cellRef = readU8(input);
    if (cellRef < 6)
      input->seek(length - 6, librevenge::RVNG_SEEK_CUR);
    chunkBytesRead += input->tell() - inputPos;
    inputPos = input->tell();
  }

  if (input->isEnd())
    return;

  // Only read formula if block is found
  if (cellRef == 6)
  {
    unsigned char xType = 1;
    unsigned char yType = 1;
    unsigned degree = 3;
    // Indicates whether it's a "simple" NURBS block with a static format
    // or a complex block where parameters each have a type
    unsigned char paramType = readU8(input);
    unsigned char valueType = 0;

    double lastKnot = 0;
    unsigned repetitions = 0;

    // Read formula's static first four parameters
    if (paramType == 0x8a)
    {
      lastKnot = readDouble(input);
      degree = readU16(input);
      xType = readU8(input);
      yType = readU8(input);
      repetitions = readU32(input);
    }
    else
    {
      valueType = paramType;
      if (valueType == 0x20)
        lastKnot = readDouble(input);
      else
        lastKnot = readU16(input);

      input->seek(1, librevenge::RVNG_SEEK_CUR);
      degree = readU16(input);
      input->seek(1, librevenge::RVNG_SEEK_CUR);
      xType = readU16(input);
      input->seek(1, librevenge::RVNG_SEEK_CUR);
      yType = readU16(input);
    }

    // Read sequences of (x, y, knot, weight) until finished
    unsigned long bytesRead = input->tell() - inputPos;
    unsigned char flag = 0;
    if (paramType != 0x8a) flag = readU8(input);
    while ((paramType == 0x8a ? repetitions > 0 : flag != 0x81) && bytesRead < length)
    {
      inputPos = input->tell();
      double knot_ = 0;
      double weight_ = 0;
      double controlX = 0;
      double controlY = 0;

      if (paramType == 0x8a) // Parameters have static format
      {
        controlX = readDouble(input);
        controlY = readDouble(input);
        knot_ = readDouble(input);
        weight_ = readDouble(input);
      }
      else // Parameters have types
      {
        valueType = flag;
        if (valueType == 0x20)
          controlX = readDouble(input);
        else
          controlX = readU16(input);

        valueType = readU8(input);
        if (valueType == 0x20)
          controlY = readDouble(input);
        else
          controlY = readU16(input);

        valueType = readU8(input);
        if (valueType == 0x20)
          knot_ = readDouble(input);
        else if (valueType == 0x62)
          knot_ = readU16(input);

        valueType = readU8(input);
        if (valueType == 0x20)
          weight_ = readDouble(input);
        else if (valueType == 0x62)
          weight_ = readU16(input);
      }
      controlPoints.push_back(std::pair<double, double>(controlX, controlY));
      knotVector.push_back(knot_);
      weights.push_back(weight_);

      if (paramType != 0x8a) flag = readU8(input);
      else repetitions--;
      bytesRead += input->tell() - inputPos;
    }
    knotVector.push_back(knot);
    knotVector.push_back(lastKnot);
    weights.push_back(weight);

    if (m_currentGeometryList)
      m_currentGeometryList->addNURBSTo(m_header.id, m_header.level, x, y, xType,
                                        yType, degree, controlPoints, knotVector, weights);
  }
  else // No formula found, use line
  {
    if (m_currentGeometryList)
      m_currentGeometryList->addLineTo(m_header.id, m_header.level, x,  y);
  }
}

void libvisio::VSDParser::readPolylineTo(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y = readDouble(input);

  // Detect whether to use Shape Data block
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  unsigned useData = readU8(input);
  if (useData == 0x8b)
  {
    input->seek(3, librevenge::RVNG_SEEK_CUR);
    unsigned dataId = readU32(input);

    if (m_currentGeometryList)
      m_currentGeometryList->addPolylineTo(m_header.id, m_header.level, x, y, dataId);
    return;
  }

  // Blocks start at 0x30
  input->seek(0x9, librevenge::RVNG_SEEK_CUR);
  unsigned long chunkBytesRead = 0x30;

  // Find formula block referring to cell A (cell 2)
  unsigned cellRef = 0;
  unsigned length = 0;
  unsigned long inputPos = input->tell();
  while (cellRef != 2 && !input->isEnd() &&
         m_header.dataLength - chunkBytesRead > 4)
  {
    length = readU32(input);
    if (!length)
      break;
    input->seek(1, librevenge::RVNG_SEEK_CUR);
    cellRef = readU8(input);
    if (cellRef < 2)
      input->seek(length - 6, librevenge::RVNG_SEEK_CUR);
    chunkBytesRead += input->tell() - inputPos;
    inputPos = input->tell();
  }

  if (input->isEnd())
    return;

  // Default to local co-ordinates if unspecified
  std::vector<std::pair<double, double> > points;

  // Only formula if block is found
  if (cellRef == 2)
  {
    unsigned long blockBytesRead = 0;
    inputPos = input->tell();
    blockBytesRead += 6;

    // Parse static first two parameters to function
    input->seek(1, librevenge::RVNG_SEEK_CUR);
    unsigned char xType = readU16(input);
    input->seek(1, librevenge::RVNG_SEEK_CUR);
    unsigned char yType = readU16(input);

    // Parse pairs of x,y co-ordinates
    unsigned flag = readU8(input);
    unsigned valueType = 0; // Holds parameter type indicator
    blockBytesRead += input->tell() - inputPos;
    while (flag != 0x81 && blockBytesRead < length)
    {
      inputPos = input->tell();
      double x2 = 0;
      double y2 = 0;

      valueType = flag;
      if (valueType == 0x20)
        x2 = readDouble(input);
      else
        x2 = readU16(input);

      valueType = readU8(input);
      if (valueType == 0x20)
        y2 = readDouble(input);
      else
        y2 = readU16(input);

      points.push_back(std::pair<double, double>(x2, y2));
      flag = readU8(input);
      blockBytesRead += input->tell() - inputPos;
    }

    if (m_currentGeometryList)
      m_currentGeometryList->addPolylineTo(m_header.id, m_header.level, x, y, xType,
                                           yType, points);
  }
  else
  {
    if (m_currentGeometryList)
      m_currentGeometryList->addLineTo(m_header.id, m_header.level, x, y);
  }
}

void libvisio::VSDParser::readInfiniteLine(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x1 = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y1 = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x2 = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y2 = readDouble(input);
  if (m_currentGeometryList)
    m_currentGeometryList->addInfiniteLine(m_header.id, m_header.level, x1, y1, x2, y2);
}

void libvisio::VSDParser::readShapeData(librevenge::RVNGInputStream *input)
{
  unsigned char dataType = readU8(input);

  input->seek(15, librevenge::RVNG_SEEK_CUR);
  // Polyline data
  if (dataType == 0x80)
  {
    std::vector<std::pair<double, double> > points;
    unsigned char xType = readU8(input);
    unsigned char yType = readU8(input);
    unsigned pointCount = readU32(input);
    if (pointCount > getRemainingLength(input) / 16)
      pointCount = getRemainingLength(input) / 16;

    for (unsigned i = 0; i < pointCount; i++)
    {
      double x = readDouble(input);
      double y = readDouble(input);
      points.push_back(std::pair<double, double>(x, y));
    }

    PolylineData data;
    data.xType = xType;
    data.yType = yType;
    data.points = points;
    m_shape.m_polylineData[m_header.id] = data;
  }

  // NURBS data
  else if (dataType == 0x82)
  {
    double lastKnot = readDouble(input);

    unsigned degree = readU16(input);
    unsigned char xType = readU8(input);
    unsigned char yType = readU8(input);
    unsigned pointCount = readU32(input);
    if (pointCount > getRemainingLength(input) / 32)
      pointCount = getRemainingLength(input) / 32;

    std::vector<double> knotVector;
    std::vector<std::pair<double, double> > controlPoints;
    std::vector<double> weights;

    for (unsigned i = 0; i < pointCount; i++)
    {
      double controlX = readDouble(input);
      double controlY = readDouble(input);
      double knot = readDouble(input);
      double weight = readDouble(input);

      knotVector.push_back(knot);
      weights.push_back(weight);
      controlPoints.push_back(std::pair<double, double>(controlX, controlY));
    }

    NURBSData data;
    data.lastKnot = lastKnot;
    data.degree = degree;
    data.xType = xType;
    data.yType = yType;
    data.knots = knotVector;
    data.weights = weights;
    data.points = controlPoints;
    m_shape.m_nurbsData[m_header.id] = data;
  }
}

void libvisio::VSDParser::readSplineStart(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y = readDouble(input);
  double secondKnot = readDouble(input);
  double firstKnot = readDouble(input);
  double lastKnot = readDouble(input);
  unsigned degree = readU8(input);

  if (m_currentGeometryList)
    m_currentGeometryList->addSplineStart(m_header.id, m_header.level, x, y, secondKnot, firstKnot, lastKnot, degree);
}

void libvisio::VSDParser::readSplineKnot(librevenge::RVNGInputStream *input)
{
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double x = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double y = readDouble(input);
  double knot = readDouble(input);

  if (m_currentGeometryList)
    m_currentGeometryList->addSplineKnot(m_header.id, m_header.level, x, y, knot);
}

void libvisio::VSDParser::readNameList(librevenge::RVNGInputStream * /* input */)
{
  m_shape.m_names.clear();
}

void libvisio::VSDParser::readNameList2(librevenge::RVNGInputStream * /* input */)
{
  m_names.clear();
}

void libvisio::VSDParser::readFieldList(librevenge::RVNGInputStream *input)
{
  if (m_header.trailer)
  {
    uint32_t subHeaderLength = readU32(input);
    uint32_t childrenListLength = readU32(input);
    input->seek(subHeaderLength, librevenge::RVNG_SEEK_CUR);
    if (childrenListLength > getRemainingLength(input))
      childrenListLength = getRemainingLength(input);
    std::vector<unsigned> fieldOrder;
    fieldOrder.reserve(childrenListLength / sizeof(uint32_t));
    for (size_t i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
      fieldOrder.push_back(readU32(input));

    m_shape.m_fields.setElementsOrder(fieldOrder);
    m_shape.m_fields.addFieldList(m_header.id, m_header.level);
  }
}

void libvisio::VSDParser::readColours(librevenge::RVNGInputStream *input)
{
  input->seek(2, librevenge::RVNG_SEEK_CUR);
  unsigned numColours = readU8(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  m_colours.clear();

  for (unsigned i = 0; i < numColours; i++)
  {
    Colour tmpColour;
    tmpColour.r = readU8(input);
    tmpColour.g = readU8(input);
    tmpColour.b = readU8(input);
    tmpColour.a = readU8(input);

    m_colours.push_back(tmpColour);
  }
}

void libvisio::VSDParser::readFont(librevenge::RVNGInputStream *input)
{
  input->seek(4, librevenge::RVNG_SEEK_CUR);
  librevenge::RVNGBinaryData textStream;

  for (unsigned i = 0; i < 32; i++)
  {
    unsigned char curchar = readU8(input);
    unsigned char nextchar = readU8(input);
    if (curchar == 0 && nextchar == 0)
      break;
    textStream.append(curchar);
    textStream.append(nextchar);
  }
  m_fonts[m_header.id] = VSDName(textStream, libvisio::VSD_TEXT_UTF16);
}

void libvisio::VSDParser::readFontIX(librevenge::RVNGInputStream *input)
{
  long tmpAdjust = input->tell();
  input->seek(2, librevenge::RVNG_SEEK_CUR);
  auto codePage = (unsigned char)(getUInt(input) & 0xff);
  tmpAdjust -= input->tell();

  std::string fontName;

  for (long i = 0; i < (long)(m_header.dataLength + tmpAdjust); i++)
  {
    auto curchar = (char)readU8(input);
    if (curchar == 0)
      break;
    fontName.append(1, curchar);
  }

  if (!codePage)
  {
    // Try to parse the font name for codePage
    size_t length = fontName.length();
    size_t found = std::string::npos;

    if (length > 3 && (found=fontName.find(" CE", length - 3)) != std::string::npos)
      codePage = 0xee;
    else if (length > 9 && (found=fontName.rfind(" Cyrillic", length - 9)) != std::string::npos)
      codePage = 0xcc;
    else if (length > 4 && (found=fontName.rfind(" Cyr", length - 4)) != std::string::npos)
      codePage = 0xcc;
    else if (length > 4 && (found=fontName.rfind(" CYR", length - 4)) != std::string::npos)
      codePage = 0xcc;
    else if (length > 7 && (found=fontName.rfind(" Baltic", length - 7)) != std::string::npos)
      codePage = 0xba;
    else if (length > 6 && (found=fontName.rfind(" Greek", length - 6)) != std::string::npos)
      codePage = 0xa1;
    else if (length > 4 && (found=fontName.rfind(" Tur", length - 4)) != std::string::npos)
      codePage = 0xa2;
    else if (length > 4 && (found=fontName.rfind(" TUR", length - 4)) != std::string::npos)
      codePage = 0xa2;
    else if (length > 7 && (found=fontName.rfind(" Hebrew", length - 7)) != std::string::npos)
      codePage = 0xb1;
    else if (length > 7 && (found=fontName.rfind(" Arabic", length - 7)) != std::string::npos)
      codePage = 0xb2;
    else if (length > 5 && (found=fontName.rfind(" Thai", length - 5)) != std::string::npos)
      codePage = 0xde;
    else if (length >= 4 && (found=fontName.find("GOST", 0, 4)) != std::string::npos)
    {
      codePage = 0xcc;
      found = std::string::npos;
    }

    if (found != std::string::npos)
    {
      fontName.erase(found, std::string::npos);
    }
  }

  TextFormat format = libvisio::VSD_TEXT_ANSI;
  switch (codePage)
  {
  case 0: // ANSI
    format = libvisio::VSD_TEXT_ANSI;
    break;
  case 0x02:
    format = libvisio::VSD_TEXT_SYMBOL;
    break;
  case 0xa1: // GREEK
    format = libvisio::VSD_TEXT_GREEK;
    break;
  case 0xa2: // TURKISH
    format = libvisio::VSD_TEXT_TURKISH;
    break;
  case 0xa3: // VIETNAMESE
    format = libvisio::VSD_TEXT_VIETNAMESE;
    break;
  case 0xb1: // HEBREW
    format = libvisio::VSD_TEXT_HEBREW;
    break;
  case 0xb2: // ARABIC
    format = libvisio::VSD_TEXT_ARABIC;
    break;
  case 0xba: // BALTIC
    format = libvisio::VSD_TEXT_BALTIC;
    break;
  case 0xcc: // RUSSIAN
    format = libvisio::VSD_TEXT_RUSSIAN;
    break;
  case 0xde: // THAI
    format = libvisio::VSD_TEXT_THAI;
    break;
  case 0xee: // CENTRAL EUROPE
    format = libvisio::VSD_TEXT_CENTRAL_EUROPE;
    break;
  case 0x80: // SHIFTJIS
    format = libvisio::VSD_TEXT_JAPANESE;
    break;
  case 0x81: // HANGUL
    format = libvisio::VSD_TEXT_KOREAN;
    break;
  case 0x86: // GB2312
    format = libvisio::VSD_TEXT_CHINESE_SIMPLIFIED;
    break;
  case 0x88: // CHINESEBIG5
    format = libvisio::VSD_TEXT_CHINESE_TRADITIONAL;
    break;
  default:
    break;
  }

  librevenge::RVNGBinaryData textStream((const unsigned char *)fontName.c_str(), fontName.length());
  m_fonts[m_header.id] = VSDName(textStream, format);
}

/* StyleSheet readers */

void libvisio::VSDParser::readStyleSheet(librevenge::RVNGInputStream *input)
{
  input->seek(0x22, librevenge::RVNG_SEEK_CUR);
  unsigned lineStyle = readU32(input);
  input->seek(4, librevenge::RVNG_SEEK_CUR);
  unsigned fillStyle = readU32(input);
  input->seek(4, librevenge::RVNG_SEEK_CUR);
  unsigned textStyle = readU32(input);

  m_collector->collectStyleSheet(m_header.id, m_header.level, lineStyle, fillStyle, textStyle);
}

void libvisio::VSDParser::readPageSheet(librevenge::RVNGInputStream * /* input */)
{
  m_currentShapeLevel = m_header.level;
  m_collector->collectPageSheet(m_header.id, m_header.level);
}

void libvisio::VSDParser::readText(librevenge::RVNGInputStream *input)
{
  input->seek(8, librevenge::RVNG_SEEK_CUR);
  librevenge::RVNGBinaryData textStream;

  // Read up to end of chunk in byte pairs (except from last 2 bytes)
  unsigned long numBytesRead = 0;
  const unsigned char *tmpBuffer = input->read(m_header.dataLength - 8, numBytesRead);
  if (numBytesRead)
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil text\n"));
    }
    textStream.append(tmpBuffer, numBytesRead);
    m_shape.m_text = textStream;
  }
  else
    m_shape.m_text.clear();
  m_shape.m_textFormat = libvisio::VSD_TEXT_UTF16;
}

void libvisio::VSDParser::readCharIX(librevenge::RVNGInputStream *input)
{
  VSDFont fontFace;
  unsigned charCount = readU32(input);
  unsigned fontID = readU16(input);
  VSDName font;
  std::map<unsigned, VSDName>::const_iterator iter = m_fonts.find(fontID);
  if (iter != m_fonts.end())
    font = iter->second;
  input->seek(1, librevenge::RVNG_SEEK_CUR);  // Color ID
  Colour fontColour;            // Font Colour
  fontColour.r = readU8(input);
  fontColour.g = readU8(input);
  fontColour.b = readU8(input);
  fontColour.a = readU8(input);

  bool bold(false);
  bool italic(false);
  bool underline(false);
  bool doubleunderline(false);
  bool strikeout(false);
  bool doublestrikeout(false);
  bool allcaps(false);
  bool initcaps(false);
  bool smallcaps(false);
  bool superscript(false);
  bool subscript(false);
  unsigned char fontMod = readU8(input);
  if (fontMod & 1) bold = true;
  if (fontMod & 2) italic = true;
  if (fontMod & 4) underline = true;
  if (fontMod & 8) smallcaps = true;
  fontMod = readU8(input);
  if (fontMod & 1) allcaps = true;
  if (fontMod & 2) initcaps = true;
  fontMod = readU8(input);
  if (fontMod & 1) superscript = true;
  if (fontMod & 2) subscript = true;

  double scaleWidth = (double)(readU16(input)) / 10000.0;
  input->seek(2, librevenge::RVNG_SEEK_CUR);
  double fontSize = readDouble(input);

  fontMod = readU8(input);
  if (fontMod & 1) doubleunderline = true;
  if (fontMod & 4) strikeout = true;
  if (fontMod & 0x20) doublestrikeout = true;

  if (m_isInStyles)
    m_collector->collectCharIXStyle(m_header.id, m_header.level, charCount, font, fontColour, fontSize,
                                    bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                    allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil character style\n"));
    }

    m_shape.m_charStyle.override(VSDOptionalCharStyle(charCount, font, fontColour, fontSize,
                                                      bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                                      allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth));
    m_shape.m_charList.addCharIX(m_header.id, m_header.level, charCount, font, fontColour, fontSize,
                                 bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                 allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth);
  }
}

void libvisio::VSDParser::readParaIX(librevenge::RVNGInputStream *input)
{
  long startPosition = input->tell();
  unsigned charCount = readU32(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double indFirst = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double indLeft = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double indRight = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double spLine = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double spBefore = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double spAfter = readDouble(input);
  unsigned char align = readU8(input);
  unsigned char bullet = readU8(input);
  input->seek(4, librevenge::RVNG_SEEK_CUR);
  unsigned fontID = readU16(input);
  VSDName bulletFont;
  if (fontID)
  {
    std::map<unsigned, VSDName>::const_iterator iter = m_fonts.find(fontID);
    if (iter != m_fonts.end())
      bulletFont = iter->second;
  }
  input->seek(2, librevenge::RVNG_SEEK_CUR);
  double bulletFontSize = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR);
  double textPosAfterBullet = readDouble(input);
  unsigned flags = readU32(input);
  input->seek(34, librevenge::RVNG_SEEK_CUR);
  long remainingData = m_header.dataLength - input->tell() + startPosition;
  unsigned blockLength = 0;
  VSDName bulletStr;

  while (remainingData >= 4 && (blockLength = readU32(input)))
  {
    long blockEnd = blockLength-4 + input->tell();
    unsigned char blockType = readU8(input);
    unsigned char blockIdx = readU8(input);
    if (blockType == 2 && blockIdx == 8)
    {
      input->seek(1, librevenge::RVNG_SEEK_CUR);
      unsigned long numBytes = 2*readU8(input);
      unsigned long numBytesRead = 0;
      const unsigned char *tmpBuffer = input->read(numBytes, numBytesRead);
      if (tmpBuffer && numBytesRead)
      {
        librevenge::RVNGBinaryData tmpBulletString(tmpBuffer, numBytesRead);
        bulletStr = VSDName(tmpBulletString, libvisio::VSD_TEXT_UTF16);
      }
    }
    else if (blockType == 2 && blockIdx == 3)
    {
    };
    input->seek(blockEnd, librevenge::RVNG_SEEK_SET);
    remainingData -= blockLength;
  }


  if (m_isInStyles)
    m_collector->collectParaIXStyle(m_header.id, m_header.level, charCount, indFirst, indLeft, indRight,
                                    spLine, spBefore, spAfter, align, bullet, bulletStr, bulletFont,
                                    bulletFontSize, textPosAfterBullet, flags);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil paragraph style\n"));
    }

    m_shape.m_paraStyle.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight,
                                                      spLine, spBefore, spAfter, align, bullet,
                                                      bulletStr, bulletFont, bulletFontSize,
                                                      textPosAfterBullet, flags));
    m_shape.m_paraList.addParaIX(m_header.id, m_header.level, charCount, indFirst,
                                 indLeft, indRight, spLine, spBefore, spAfter, align,
                                 bullet, bulletStr, bulletFont, bulletFontSize,
                                 textPosAfterBullet, flags);
  }
}


void libvisio::VSDParser::readFillAndShadow(librevenge::RVNGInputStream *input)
{
  unsigned char colourFGIndex = readU8(input);
  Colour colourFG;
  colourFG.r = readU8(input);
  colourFG.g = readU8(input);
  colourFG.b = readU8(input);
  colourFG.a = readU8(input);
  unsigned char colourBGIndex = readU8(input);
  Colour colourBG;
  colourBG.r = readU8(input);
  colourBG.g = readU8(input);
  colourBG.b = readU8(input);
  colourBG.a = readU8(input);
  if (!colourFG && !colourBG)
  {
    colourFG = _colourFromIndex(colourFGIndex);
    colourBG = _colourFromIndex(colourBGIndex);
  }
  double fillFGTransparency = (double)colourFG.a / 255.0;
  double fillBGTransparency = (double)colourBG.a / 255.0;

  unsigned char fillPattern = readU8(input);

  unsigned char shadowFGIndex = readU8(input);
  Colour shadowFG;
  shadowFG.r = readU8(input);
  shadowFG.g = readU8(input);
  shadowFG.b = readU8(input);
  shadowFG.a = readU8(input);
  unsigned char shadowBGIndex = readU8(input);
  Colour shadowBG;
  shadowBG.r = readU8(input);
  shadowBG.g = readU8(input);
  shadowBG.b = readU8(input);
  shadowBG.a = readU8(input);
  if (!shadowFG && !shadowBG)
  {
    shadowFG = _colourFromIndex(shadowFGIndex);
    shadowBG = _colourFromIndex(shadowBGIndex);
  }

  unsigned char shadowPattern = readU8(input);

// only version 11 after that point
  input->seek(2, librevenge::RVNG_SEEK_CUR); // Shadow Type and Value format byte
  double shadowOffsetX = readDouble(input);
  input->seek(1, librevenge::RVNG_SEEK_CUR); // Value format byte
  double shadowOffsetY = readDouble(input);



  if (m_isInStyles)
    m_collector->collectFillStyle(m_header.level, colourFG, colourBG, fillPattern,
                                  fillFGTransparency, fillBGTransparency, shadowPattern, shadowFG,
                                  shadowOffsetX, shadowOffsetY, -1, -1, -1);
  else
  {
    if (m_isStencilStarted)
    {
      VSD_DEBUG_MSG(("Found stencil fill\n"));
    }
    m_shape.m_fillStyle.override(VSDOptionalFillStyle(colourFG, colourBG, fillPattern, fillFGTransparency,
                                                      fillBGTransparency, shadowFG, shadowPattern,
                                                      shadowOffsetX, shadowOffsetY, -1, -1, -1));
  }
}

void libvisio::VSDParser::readName(librevenge::RVNGInputStream *input)
{
  unsigned long numBytesRead = 0;
  const unsigned char *tmpBuffer = input->read(m_header.dataLength, numBytesRead);
  if (numBytesRead)
  {
    librevenge::RVNGBinaryData name(tmpBuffer, numBytesRead);
    m_shape.m_names[m_header.id] = VSDName(name, libvisio::VSD_TEXT_UTF16);
  }
}

void libvisio::VSDParser::readName2(librevenge::RVNGInputStream *input)
{
  unsigned short unicharacter = 0;
  librevenge::RVNGBinaryData name;
  input->seek(4, librevenge::RVNG_SEEK_CUR); // skip a dword that seems to be always 1
  while ((unicharacter = readU16(input)))
  {
    name.append(unicharacter & 0xff);
    name.append((unicharacter & 0xff00) >> 8);
  }
  name.append(unicharacter & 0xff);
  name.append((unicharacter & 0xff00) >> 8);
  m_names[m_header.id] = VSDName(name, libvisio::VSD_TEXT_UTF16);
}

void libvisio::VSDParser::readTextField(librevenge::RVNGInputStream *input)
{
  unsigned long initialPosition = input->tell();
  input->seek(7, librevenge::RVNG_SEEK_CUR);
  unsigned char cellType = readU8(input);
  if (cellType == CELL_TYPE_StringWithoutUnit)
  {
    int nameId = readS32(input);
    input->seek(6, librevenge::RVNG_SEEK_CUR);
    int formatStringId = readS32(input);
    m_shape.m_fields.addTextField(m_header.id, m_header.level, nameId, formatStringId);
  }
  else
  {
    double numericValue = readDouble(input);
    input->seek(2, librevenge::RVNG_SEEK_CUR);
    int formatStringId = readS32(input);

    unsigned blockIdx = 0;
    unsigned length = 0;
    unsigned short formatNumber = 0;
    input->seek(initialPosition+0x36, librevenge::RVNG_SEEK_SET);
    while (blockIdx != 2 && !input->isEnd() && (unsigned long) input->tell() < (unsigned long)(initialPosition+m_header.dataLength+m_header.trailer))
    {
      unsigned long inputPos = input->tell();
      length = readU32(input);
      if (!length)
        break;
      input->seek(1, librevenge::RVNG_SEEK_CUR);
      blockIdx = readU8(input);
      if (blockIdx != 2)
        input->seek(inputPos + length, librevenge::RVNG_SEEK_SET);
      else
      {
        input->seek(1, librevenge::RVNG_SEEK_CUR);
        formatNumber = readU16(input);
        if (0x80 != readU8(input))
        {
          input->seek(inputPos + length, librevenge::RVNG_SEEK_SET);
          blockIdx = 0;
        }
        else
        {
          if (0xc2 != readU8(input))
          {
            input->seek(inputPos + length, librevenge::RVNG_SEEK_SET);
            blockIdx = 0;
          }
          else
            break;
        }
      }
    }

    if (input->isEnd())
      return;

    if (blockIdx != 2)
    {
      if (cellType == CELL_TYPE_Date)
        formatNumber = VSD_FIELD_FORMAT_MsoDateShort;
      else
        formatNumber = VSD_FIELD_FORMAT_Unknown;
    }

    m_shape.m_fields.addNumericField(m_header.id, m_header.level, formatNumber, cellType, numericValue, formatStringId);
  }
}

void libvisio::VSDParser::readMisc(librevenge::RVNGInputStream *input)
{
  unsigned long initialPosition = input->tell();
  unsigned char flags = readU8(input);
  if (flags & 0x20)
    m_shape.m_misc.m_hideText = true;
  else
    m_shape.m_misc.m_hideText = false;

  input->seek(initialPosition+45, librevenge::RVNG_SEEK_SET);
  while (!input->isEnd() && (unsigned long) input->tell() < (unsigned long)(initialPosition+m_header.dataLength+m_header.trailer))
  {
    unsigned long inputPos = input->tell();
    unsigned length = readU32(input);
    if (!length)
      break;
    unsigned blockType = readU8(input);
    input->seek(1, librevenge::RVNG_SEEK_CUR);
    if (blockType == 2)
    {
      if (0x74 == readU8(input))
      {
        if (0x6000004e == readU32(input))
        {
          unsigned shapeId = readU32(input);
          if (0x7a == readU8(input))
          {
            if (0x40000073 == readU32(input))
            {
              if (!m_shape.m_xform1d)
                m_shape.m_xform1d = make_unique<XForm1D>();
              if (m_shape.m_xform1d->beginId == MINUS_ONE)
                m_shape.m_xform1d->beginId = shapeId;
              else if (m_shape.m_xform1d->endId == MINUS_ONE)
                m_shape.m_xform1d->endId = shapeId;
            }
          }
        }
      }
    }
    input->seek(inputPos + length, librevenge::RVNG_SEEK_SET);
  }
}

libvisio::Colour libvisio::VSDParser::_colourFromIndex(unsigned idx)
{
  if (idx < m_colours.size())
    return m_colours[idx];
  return libvisio::Colour();
}

unsigned libvisio::VSDParser::getUInt(librevenge::RVNGInputStream *input)
{
  return readU32(input);
}

int libvisio::VSDParser::getInt(librevenge::RVNGInputStream *input)
{
  return readS32(input);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
