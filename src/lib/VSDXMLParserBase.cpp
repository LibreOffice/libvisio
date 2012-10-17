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

#include <string.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlstring.h>
#include <libwpd-stream/libwpd-stream.h>
#include <boost/algorithm/string.hpp>
#include "VSDXMLParserBase.h"
#include "libvisio_utils.h"
#include "VSDContentCollector.h"
#include "VSDStylesCollector.h"
#include "VSDZipStream.h"
#include "VSDXMLHelper.h"
#include "VSDXMLTokenMap.h"


libvisio::VSDXMLParserBase::VSDXMLParserBase()
  : m_collector(), m_stencils(), m_currentStencil(0), m_shape(),
    m_isStencilStarted(false), m_currentStencilID((unsigned)-1), m_extractStencils(false), m_isInStyles(false),
    m_currentLevel(0), m_currentShapeLevel(0), m_colours(), m_charList(new VSDCharacterList()), m_charListVector(),
    m_fieldList(), m_geomList(new VSDGeometryList()), m_geomListVector(), m_paraList(new VSDParagraphList()),
    m_paraListVector(), m_shapeList(), m_currentBinaryData(), m_shapeStack(), m_shapeLevelStack(), m_isShapeStarted(false)
{
}

libvisio::VSDXMLParserBase::~VSDXMLParserBase()
{
  if (m_geomList)
  {
    m_geomList->clear();
    delete m_geomList;
  }
  if (m_charList)
  {
    m_charList->clear();
    delete m_charList;
  }
  if (m_paraList)
  {
    m_paraList->clear();
    delete m_paraList;
  }
  if (m_currentStencil)
    delete m_currentStencil;
}

// Common functions

void libvisio::VSDXMLParserBase::readEllipticalArcTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readEllipse(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readGeomList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readGeometry(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readMoveTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readLineTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readArcTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readNURBSTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readPolylineTo(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readInfiniteLine(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readShapeData(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readShapeId(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readShapeList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readShape(xmlTextReaderPtr reader)
{
  m_isShapeStarted = true;
  m_currentShapeLevel = getElementDepth(reader);

  xmlChar *idString = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  xmlChar *masterPageString = xmlTextReaderGetAttribute(reader, BAD_CAST("MasterPage"));
  if (!masterPageString)
    masterPageString = xmlTextReaderGetAttribute(reader, BAD_CAST("Master"));
  xmlChar *masterShapeString = xmlTextReaderGetAttribute(reader, BAD_CAST("MasterShape"));
  xmlChar *lineStyleString = xmlTextReaderGetAttribute(reader, BAD_CAST("LineStyle"));
  xmlChar *fillStyleString = xmlTextReaderGetAttribute(reader, BAD_CAST("FillStyle"));
  xmlChar *textStyleString = xmlTextReaderGetAttribute(reader, BAD_CAST("TextStyle"));

  unsigned id = (unsigned)(idString ? xmlStringToLong(idString) : -1);
  unsigned masterPage = (unsigned)-1;
  if (!m_shapeStack.empty())
    masterPage = m_shapeStack.top().m_masterPage;
  if (masterPageString)
    masterPage = xmlStringToLong(masterPageString);
  unsigned masterShape =  (unsigned)(masterShapeString ? xmlStringToLong(masterShapeString) : -1);
  unsigned lineStyle =  (unsigned)(lineStyleString ? xmlStringToLong(lineStyleString) : -1);
  unsigned fillStyle =  (unsigned)(fillStyleString ? xmlStringToLong(fillStyleString) : -1);
  unsigned textStyle =  (unsigned)(textStyleString ? xmlStringToLong(textStyleString) : -1);
  if (idString)
    xmlFree(idString);
  if (masterPageString)
    xmlFree(masterPageString);
  if (masterShapeString)
    xmlFree(masterShapeString);
  if (lineStyleString)
    xmlFree(lineStyleString);
  if (fillStyleString)
    xmlFree(fillStyleString);
  if (textStyleString)
    xmlFree(textStyleString);

  m_shape.clear();
  const VSDStencil *tmpStencil = m_stencils.getStencil(masterPage);
  if (tmpStencil)
  {
    if ((unsigned)-1 == masterShape)
      masterShape = tmpStencil->m_firstShapeId;
    const VSDShape *tmpShape = tmpStencil->getStencilShape(masterShape);
    if (tmpShape)
    {
      if (tmpShape->m_foreign)
        m_shape.m_foreign = new ForeignData(*(tmpShape->m_foreign));
      m_shape.m_text = tmpShape->m_text;
      m_shape.m_textFormat = tmpShape->m_textFormat;
      if (tmpShape->m_lineStyle)
        m_shape.m_lineStyle = new VSDLineStyle(*(tmpShape->m_lineStyle));
      if (tmpShape->m_fillStyle)
        m_shape.m_fillStyle = new VSDFillStyle(*(tmpShape->m_fillStyle));
      m_shape.m_xform = tmpShape->m_xform;
      if (tmpShape->m_txtxform)
        m_shape.m_txtxform = new XForm(*(tmpShape->m_txtxform));
    }
  }

  if (!m_shapeStack.empty())
    m_shapeStack.top().m_shapeList.addShapeId(id);
  else
    m_shapeList.addShapeId(id);

  m_shape.m_lineStyleId = lineStyle;
  m_shape.m_fillStyleId = fillStyle;
  m_shape.m_textStyleId = textStyle;
  m_shape.m_parent = m_shapeStack.empty() ? (unsigned)-1 : m_shapeStack.top().m_shapeId;
  m_shape.m_masterPage = masterPage;
  m_shape.m_masterShape = masterShape;
  m_shape.m_shapeId = id;
}

void libvisio::VSDXMLParserBase::readColours(xmlTextReaderPtr reader)
{
  m_colours.clear();

  int ret = 1;
  int tokenId = -1;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (-1 == tokenId)
    {
      VSD_DEBUG_MSG(("VSDXMLParserBase::readColours: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);

    if (XML_COLORENTRY == tokenId)
    {
      xmlChar *ix = xmlTextReaderGetAttribute(reader, BAD_CAST("IX"));
      xmlChar *rgb = xmlTextReaderGetAttribute(reader, BAD_CAST("RGB"));
      if (ix && rgb)
      {
        unsigned idx = (unsigned)xmlStringToLong(ix);
        Colour rgbColour = xmlStringToColour(rgb);
        if (idx)
          m_colours[idx] = rgbColour;
      }
      xmlFree(rgb);
      xmlFree(ix);

    }
  }
  while ((XML_COLORS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libvisio::VSDXMLParserBase::readCharList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readParaList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readPage(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  xmlChar *bgndPage = xmlTextReaderGetAttribute(reader, BAD_CAST("BackPage"));
  xmlChar *background = xmlTextReaderGetAttribute(reader, BAD_CAST("Background"));
  if (id)
  {
    unsigned nId = (unsigned)xmlStringToLong(id);
    unsigned backgroundPageID =  (unsigned)(bgndPage ? xmlStringToLong(bgndPage) : -1);
    bool isBackgroundPage = background ? xmlStringToBool(background) : false;
    m_collector->startPage(nId);
    m_collector->collectPage(nId, (unsigned)getElementDepth(reader), backgroundPageID, isBackgroundPage);
  }
  if (id)
    xmlFree(id);
  if (bgndPage)
    xmlFree(bgndPage);
  if (background)
    xmlFree(background);
}

void libvisio::VSDXMLParserBase::readText(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readCharIX(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readParaIX(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readNameList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readName(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readFieldList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readTextField(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readStyleSheet(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  xmlChar *lineStyle = xmlTextReaderGetAttribute(reader, BAD_CAST("LineStyle"));
  xmlChar *fillStyle = xmlTextReaderGetAttribute(reader, BAD_CAST("FillStyle"));
  xmlChar *textStyle = xmlTextReaderGetAttribute(reader, BAD_CAST("TextStyle"));
  if (id)
  {
    unsigned nId = (unsigned)xmlStringToLong(id);
    unsigned nLineStyle =  (unsigned)(lineStyle ? xmlStringToLong(lineStyle) : -1);
    unsigned nFillStyle = (unsigned)(fillStyle ? xmlStringToLong(fillStyle) : -1);
    unsigned nTextStyle = (unsigned)(textStyle ? xmlStringToLong(textStyle) : -1);
    m_collector->collectStyleSheet(nId, (unsigned)getElementDepth(reader), nLineStyle, nFillStyle, nTextStyle);
  }
  if (id)
    xmlFree(id);
  if (lineStyle)
    xmlFree(lineStyle);
  if (fillStyle)
    xmlFree(fillStyle);
  if (textStyle)
    xmlFree(textStyle);
}

void libvisio::VSDXMLParserBase::readPageSheet(xmlTextReaderPtr reader)
{
  m_currentShapeLevel = (unsigned)getElementDepth(reader);
  m_collector->collectPageSheet(0, m_currentShapeLevel);
}

void libvisio::VSDXMLParserBase::readSplineStart(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readSplineKnot(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readStencil(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("ID"));
  if (id)
  {
    unsigned nId = (unsigned)xmlStringToLong(id);
    m_currentStencilID = nId;
    xmlFree(id);
  }
  else
    m_currentStencilID = (unsigned)-1;
  if (m_currentStencil)
    delete m_currentStencil;
  m_currentStencil = new VSDStencil();
}

void libvisio::VSDXMLParserBase::readStencilShape(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readOLEList(xmlTextReaderPtr /* reader */)
{
}

void libvisio::VSDXMLParserBase::readOLEData(xmlTextReaderPtr /* reader */)
{
}

int libvisio::VSDXMLParserBase::readExtendedColourData(Colour &value, long &idx, xmlTextReaderPtr reader)
{
  int ret = 1;
  idx = -1;
  try
  {
    ret = readLongData(idx, reader);
  }
  catch (const XmlParserException &)
  {
    idx = -1;
    ret = readColourData(value, reader);
  }
  if (idx > 0)
  {
    std::map<unsigned, Colour>::const_iterator iter = m_colours.find((unsigned)idx);
    if (iter != m_colours.end())
      value = iter->second;
    else
      idx = -1;
  }
  return ret;
}

void libvisio::VSDXMLParserBase::readForeignData(xmlTextReaderPtr reader)
{
  VSD_DEBUG_MSG(("VSDXParser::readForeignData\n"));
  if (!m_shape.m_foreign)
    m_shape.m_foreign = new ForeignData();

  xmlChar *foreignTypeString = xmlTextReaderGetAttribute(reader, BAD_CAST("ForeignType"));
  if (foreignTypeString)
  {
    if (xmlStrEqual(foreignTypeString, BAD_CAST("Bitmap")))
      m_shape.m_foreign->type = 1;
    else if (xmlStrEqual(foreignTypeString, BAD_CAST("Object")))
      m_shape.m_foreign->type = 2;
    else if (xmlStrEqual(foreignTypeString, BAD_CAST("EnhMetaFile")))
      m_shape.m_foreign->type = 4;
    xmlFree(foreignTypeString);
  }
  xmlChar *foreignFormatString = xmlTextReaderGetAttribute(reader, BAD_CAST("CompressionType"));
  if (foreignFormatString)
  {
    if (xmlStrEqual(foreignFormatString, BAD_CAST("JPEG")))
      m_shape.m_foreign->format = 1;
    else if (xmlStrEqual(foreignFormatString, BAD_CAST("GIF")))
      m_shape.m_foreign->format = 2;
    else if (xmlStrEqual(foreignFormatString, BAD_CAST("TIFF")))
      m_shape.m_foreign->format = 3;
    else if (xmlStrEqual(foreignFormatString, BAD_CAST("PNG")))
      m_shape.m_foreign->format = 4;
    else
      m_shape.m_foreign->format = 0;
    xmlFree(foreignFormatString);
  }
  else
    m_shape.m_foreign->format = 255;

  getBinaryData(reader);
}

int libvisio::VSDXMLParserBase::readExtendedColourData(Colour &value, xmlTextReaderPtr reader)
{
  long idx = -1;
  return readExtendedColourData(value, idx, reader);
}

void libvisio::VSDXMLParserBase::_flushShape()
{
  if (!m_isShapeStarted)
    return;

  m_collector->collectShape(m_shape.m_shapeId, m_currentShapeLevel, m_shape.m_parent, m_shape.m_masterPage, m_shape.m_masterShape, m_shape.m_lineStyleId, m_shape.m_fillStyleId, m_shape.m_textStyleId);

  m_collector->collectShapesOrder(0, m_currentShapeLevel+2, m_shape.m_shapeList.getShapesOrder());

  m_collector->collectXFormData(m_currentShapeLevel+2, m_shape.m_xform);

  if (m_shape.m_txtxform)
    m_collector->collectTxtXForm(m_currentShapeLevel+2, *(m_shape.m_txtxform));
  if (m_shape.m_lineStyle)
    m_collector->collectLine(m_currentShapeLevel+2, m_shape.m_lineStyle->width, m_shape.m_lineStyle->colour, m_shape.m_lineStyle->pattern,
                             m_shape.m_lineStyle->startMarker, m_shape.m_lineStyle->endMarker, m_shape.m_lineStyle->cap);
  if (m_shape.m_fillStyle)
    m_collector->collectFillAndShadow(m_currentShapeLevel+2, m_shape.m_fillStyle->fgColour, m_shape.m_fillStyle->bgColour, m_shape.m_fillStyle->pattern,
                                      m_shape.m_fillStyle->fgTransparency, m_shape.m_fillStyle->bgTransparency, m_shape.m_fillStyle->shadowPattern,
                                      m_shape.m_fillStyle->shadowFgColour, m_shape.m_fillStyle->shadowOffsetX, m_shape.m_fillStyle->shadowOffsetY);
  if (m_shape.m_textBlockStyle)
    m_collector->collectTextBlock(m_currentShapeLevel+2, m_shape.m_textBlockStyle->leftMargin, m_shape.m_textBlockStyle->rightMargin,
                                  m_shape.m_textBlockStyle->topMargin, m_shape.m_textBlockStyle->bottomMargin, m_shape.m_textBlockStyle->verticalAlign,
                                  m_shape.m_textBlockStyle->isTextBkgndFilled, m_shape.m_textBlockStyle->textBkgndColour,
                                  m_shape.m_textBlockStyle->defaultTabStop, m_shape.m_textBlockStyle->textDirection);

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

  if (m_shape.m_foreign)
    m_collector->collectForeignData(m_currentShapeLevel+1, m_shape.m_foreign->data);

  if (!m_shape.m_fields.empty())
    m_shape.m_fields.handle(m_collector);

  if (m_shape.m_text.size())
    m_collector->collectText(m_currentShapeLevel+1, m_shape.m_text, m_shape.m_textFormat);


  for (std::map<unsigned, VSDGeometryList>::const_iterator iterGeom = m_shape.m_geometries.begin(); iterGeom != m_shape.m_geometries.end(); ++iterGeom)
    iterGeom->second.handle(m_collector);

  for (std::vector<VSDCharacterList>::const_iterator iterChar = m_shape.m_charListVector.begin(); iterChar != m_shape.m_charListVector.end(); ++iterChar)
    iterChar->handle(m_collector);

  for (std::vector<VSDParagraphList>::const_iterator iterPara = m_shape.m_paraListVector.begin(); iterPara != m_shape.m_paraListVector.end(); ++iterPara)
    iterPara->handle(m_collector);
}

void libvisio::VSDXMLParserBase::_handleLevelChange(unsigned level)
{
  m_currentLevel = level;
  m_collector->collectUnhandledChunk(0, m_currentLevel);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
