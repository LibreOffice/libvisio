/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02111-1301 USA
 */

#include <vector>
#include <map>
#include "VSDXStylesCollector.h"

libvisio::VSDXStylesCollector::VSDXStylesCollector(
  std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
  std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
  std::vector<std::list<unsigned> > &documentPageShapeOrders
) :
  m_currentLevel(0), m_isShapeStarted(false), 
  m_currentShapeId(0), m_groupXForms(), m_groupMemberships(),
  m_groupXFormsSequence(groupXFormsSequence),
  m_groupMembershipsSequence(groupMembershipsSequence), m_pageShapeOrder(),
  m_documentPageShapeOrders(documentPageShapeOrders),
  m_shapeList(), m_currentStyleSheet(0), m_styles(),
  m_lineStyle(), m_fillStyle(), m_textStyle(), 
  m_isStyleStarted(false)
{
  m_groupXFormsSequence.clear();
  m_groupMembershipsSequence.clear();
  m_documentPageShapeOrders.clear();
}

void libvisio::VSDXStylesCollector::collectEllipticalArcTo(unsigned /* id */, unsigned level, double /* x3 */, double /* y3 */, double /* x2 */, double /* y2 */, double /* angle */, double /* ecc */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectForeignData(unsigned /* id */, unsigned level, const WPXBinaryData & /* binaryData */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectEllipse(unsigned /* id */, unsigned level, double /* cx */, double /* cy */, double /* xleft */, double /* yleft */, double /* xtop */, double /* ytop */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectLine(unsigned /* id */, unsigned level, double /* strokeWidth */, Colour /* c */, unsigned /* linePattern */, unsigned /* lineCap */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectFillAndShadow(unsigned /* id */, unsigned level, unsigned /* colourIndexFG */, unsigned /* colourIndexBG */, unsigned /* fillPattern */, unsigned /* fillFGTransparency */, unsigned /* fillBGTransparency */, unsigned /* shadowPattern */, Colour /* shfgc */, double /* shadowOffsetX */, double /* shadowOffsetY */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectFillAndShadow(unsigned /* id */, unsigned level, unsigned /* colourIndexFG */, unsigned /* colourIndexBG */, unsigned /* fillPattern */, unsigned /* fillFGTransparency */, unsigned /* fillBGTransparency */, unsigned /* shadowPattern */, Colour /* shfgc */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectGeomList(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectCharList(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectGeometry(unsigned /* id */, unsigned level, unsigned /* geomFlags */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectMoveTo(unsigned /* id */, unsigned level, double /* x */, double /* y */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectLineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectArcTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */, double /* bow */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectNURBSTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */, unsigned /* xType */, unsigned /* yType */, unsigned /* degree */, std::vector<std::pair<double, double> > /* controlPoints */, std::vector<double> /* knotVector */, std::vector<double> /* weights */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectNURBSTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */, double /* knot */, double /* knotPrev */, double /* weight */, double /* weightPrev */, unsigned /* dataID */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectPolylineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */, unsigned /* xType */, unsigned /* yType */, std::vector<std::pair<double, double> > & /* points */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectPolylineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */, unsigned /* dataID */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectShapeData(unsigned /* id */, unsigned level, unsigned /* xType */, unsigned /* yType */, unsigned /* degree */, double /*lastKnot*/, std::vector<std::pair<double, double> > /* controlPoints */, std::vector<double> /* knotVector */, std::vector<double> /* weights */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectShapeData(unsigned /* id */, unsigned level, unsigned /* xType */, unsigned /* yType */, std::vector<std::pair<double, double> > /* points */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectXFormData(unsigned /* id */, unsigned level, const XForm &xform)
{
  _handleLevelChange(level);
  if (m_isShapeStarted)
    m_groupXForms[m_currentShapeId] = xform;
}

void libvisio::VSDXStylesCollector::collectTxtXForm(unsigned /* id */, unsigned level, const XForm & /* txtxform */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectShapeId(unsigned /* id */, unsigned level, unsigned shapeId)
{
  _handleLevelChange(level);
  if (m_isShapeStarted)
    m_groupMemberships[shapeId] = m_currentShapeId;
  m_shapeList.push_back(shapeId);
}

void libvisio::VSDXStylesCollector::collectShapeList(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectForeignDataType(unsigned /* id */, unsigned level, unsigned /* foreignType */, unsigned /* foreignFormat */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectPageProps(unsigned /* id */, unsigned level, double /* pageWidth */, double /* pageHeight */, double /* shadowOffsetX */, double /* shadowOffsetY */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectShape(unsigned id, unsigned level, unsigned lineStyle, unsigned fillStyle, unsigned textStyle)
{
  _handleLevelChange(level);
  m_currentShapeId = id;
  m_isShapeStarted = true;
}

void libvisio::VSDXStylesCollector::collectUnhandledChunk(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectColours(const std::vector<Colour> & /* colours */)
{
}

void libvisio::VSDXStylesCollector::collectFont(unsigned short /* fontID */, const std::vector<unsigned char> & /* textStream */, TextFormat /* format */)
{
}

void libvisio::VSDXStylesCollector::collectText(unsigned /*id*/, unsigned level, const std::vector<unsigned char> & /*textStream*/, TextFormat /*format*/)
{
 _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectCharFormat(unsigned /*id*/ , unsigned level, unsigned /*charCount*/, unsigned short /* fontID */, Colour /* fontColour */, unsigned /*langId*/, double /*fontSize*/, bool /*bold*/, bool /*italic*/, bool /*underline*/, WPXString /*fontFace*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectStyleSheet(unsigned id, unsigned level, unsigned lineStyleParent, unsigned fillStyleParent, unsigned textStyleParent)
{
  _handleLevelChange(level);
  m_lineStyle = 0;
  m_fillStyle = 0;
  m_textStyle = 0;
  m_currentStyleSheet = id;
  m_styles.addLineStyleMaster(m_currentStyleSheet, lineStyleParent);
  m_styles.addFillStyleMaster(m_currentStyleSheet, fillStyleParent);
  m_styles.addTextStyleMaster(m_currentStyleSheet, textStyleParent);
  m_isStyleStarted = true;
}

void libvisio::VSDXStylesCollector::startPage()
{
  m_groupXForms.clear();
  m_groupMemberships.clear();
  m_pageShapeOrder.clear();
  m_groupShapeOrder.clear();
}

void libvisio::VSDXStylesCollector::endPage()
{
  _handleLevelChange(0);
  m_groupXFormsSequence.push_back(m_groupXForms);
  m_groupMembershipsSequence.push_back(m_groupMemberships);
  while (m_groupShapeOrder.size())
  {
    for (std::list<unsigned>::iterator j = m_pageShapeOrder.begin(); j != m_pageShapeOrder.end();)
    {
      std::map<unsigned, std::list<unsigned> >::iterator iter = m_groupShapeOrder.find(*j);
      if (m_groupShapeOrder.end() != iter)
      {
        j++;
        m_pageShapeOrder.splice(j, iter->second, iter->second.begin(), iter->second.end());
        m_groupShapeOrder.erase(iter);
      }
      else
        j++;
    }
  }
  m_documentPageShapeOrders.push_back(m_pageShapeOrder);
}

void libvisio::VSDXStylesCollector::_handleLevelChange(unsigned level)
{
  if (m_currentLevel == level)
    return;
  if (level < 3)
    _flushShapeList();
  if (level < 2)
  {
    m_isShapeStarted = false;
    if (m_isStyleStarted)
    {
      m_isStyleStarted = false;
      m_styles.addLineStyle(m_currentStyleSheet, m_lineStyle);
      m_styles.addFillStyle(m_currentStyleSheet, m_fillStyle);
      m_styles.addTextStyle(m_currentStyleSheet, m_textStyle);

      if (m_lineStyle) delete m_lineStyle;
      if (m_fillStyle) delete m_fillStyle;
      if (m_textStyle) delete m_textStyle;
    }
  }

  m_currentLevel = level;
}

void libvisio::VSDXStylesCollector::_flushShapeList()
{
  if (!m_shapeList.size())
    return;

  if (m_isShapeStarted)
    m_groupShapeOrder[m_currentShapeId] = m_shapeList;
  else
    m_pageShapeOrder = m_shapeList;

  m_shapeList.clear();
}
