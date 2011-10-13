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

#include <vector>
#include <map>
#include "VSDXStylesCollector.h"

libvisio::VSDXStylesCollector::VSDXStylesCollector(
  std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
  std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
  std::vector<std::list<unsigned> > &documentPageShapeOrders
) :
  m_currentLevel(0), m_isShapeStarted(false),
  m_shadowOffsetX(0.0), m_shadowOffsetY(0.0),
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

void libvisio::VSDXStylesCollector::collectEllipticalArcTo(unsigned /* id */, unsigned level, double /* x3 */, double /* y3 */,
                                                           double /* x2 */, double /* y2 */, double /* angle */, double /* ecc */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectForeignData(unsigned /* id */, unsigned level, const WPXBinaryData & /* binaryData */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectEllipse(unsigned /* id */, unsigned level, double /* cx */, double /* cy */,
                                                   double /* xleft */, double /* yleft */, double /* xtop */, double /* ytop */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectLine(unsigned /* id */, unsigned level, double /* strokeWidth */, Colour /* c */, unsigned /* linePattern */, unsigned /* lineCap */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectFillAndShadow(unsigned /* id */, unsigned level, unsigned /* colourIndexFG */, unsigned /* colourIndexBG */,
                                                         unsigned /* fillPattern */, unsigned /* fillFGTransparency */, unsigned /* fillBGTransparency */,
                                                         unsigned /* shadowPattern */, Colour /* shfgc */, double /* shadowOffsetX */, double /* shadowOffsetY */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectFillAndShadow(unsigned /* id */, unsigned level, unsigned /* colourIndexFG */, unsigned /* colourIndexBG */,
                                                         unsigned /* fillPattern */, unsigned /* fillFGTransparency */, unsigned /* fillBGTransparency */,
                                                         unsigned /* shadowPattern */, Colour /* shfgc */)
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

void libvisio::VSDXStylesCollector::collectGeometry(unsigned /* id */, unsigned level, unsigned char /* geomFlags */)
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

void libvisio::VSDXStylesCollector::collectNURBSTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */,
                                                   unsigned char /* xType */, unsigned char /* yType */, unsigned /* degree */,
                                                   std::vector<std::pair<double, double> > /* controlPoints */,
                                                   std::vector<double> /* knotVector */, std::vector<double> /* weights */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectNURBSTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */, double /* knot */,
                                                   double /* knotPrev */, double /* weight */, double /* weightPrev */, unsigned /* dataID */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectPolylineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */,
                                                      unsigned char /* xType */, unsigned char /* yType */,
                                                      std::vector<std::pair<double, double> > & /* points */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectPolylineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */, unsigned /* dataID */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectSplineStart(unsigned /* id */, unsigned level, double /* x */, double /* y */,
                                                       double /* secondKnot */, double /* firstKnot */, double /* lastKnot */, unsigned /* degree */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectSplineKnot(unsigned /* id */, unsigned level, double /* x */, double /* y */, double /* knot */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectSplineEnd()
{
}

void libvisio::VSDXStylesCollector::collectShapeData(unsigned /* id */, unsigned level, unsigned char /* xType */, unsigned char /* yType */,
                                                     unsigned /* degree */, double /*lastKnot*/, std::vector<std::pair<double, double> > /* controlPoints */,
                                                     std::vector<double> /* knotVector */, std::vector<double> /* weights */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectShapeData(unsigned /* id */, unsigned level, unsigned char /* xType */, unsigned char /* yType */,
                                                     std::vector<std::pair<double, double> > /* points */)
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

void libvisio::VSDXStylesCollector::collectPageProps(unsigned /* id */, unsigned level, double /* pageWidth */, double /* pageHeight */,
                                                     double /* shadowOffsetX */, double /* shadowOffsetY */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectPage(unsigned /* id */, unsigned level, unsigned /* backgroundPageID */, unsigned /* currentPageID */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectShape(unsigned id, unsigned level, unsigned /*masterPage*/, unsigned /*masterShape*/,
                                                 unsigned /* lineStyle */, unsigned /* fillStyle */, unsigned /* textStyle */)
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

void libvisio::VSDXStylesCollector::collectCharFormat(unsigned /*id*/ , unsigned level, unsigned /*charCount*/, unsigned short /*fontID*/, Colour /*fontColour*/,
                                                      unsigned /*langId*/, double /*fontSize*/, bool /*bold*/, bool /*italic*/, bool /*underline*/,
                                                      bool /* doubleunderline */, bool /* strikeout */, bool /* doublestrikeout */, bool /* allcaps */,
                                                      bool /* initcaps */, bool /* smallcaps */, bool /* superscript */, bool /* subscript */, WPXString /*fontFace*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectTextBlock(unsigned /* id */, unsigned level, double /* leftMargin */, double /* rightMargin */,
                                                     double /* topMargin */, double /* bottomMargin */,  unsigned char /* verticalAlign */,
                                                     unsigned char /* textBkgndColour */, unsigned char /* textBkgndTransparency */,
                                                     double /* defaultTabStop */,  unsigned char /* textDirection */)
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

void libvisio::VSDXStylesCollector::collectLineStyle(unsigned /* id */, unsigned level, double strokeWidth, Colour c, unsigned char linePattern, unsigned char lineCap)
{
  if (!m_lineStyle)
    m_lineStyle = new VSDXLineStyle();

  m_lineStyle->width = strokeWidth;
  m_lineStyle->colour = c;
  m_lineStyle->pattern = linePattern;
  m_lineStyle->cap = lineCap;

  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectFillStyle(unsigned /*id*/, unsigned level, unsigned char colourIndexFG, unsigned char colourIndexBG,
                                                     unsigned char fillPattern, unsigned char fillFGTransparency, unsigned char fillBGTransparency,
                                                     unsigned char shadowPattern, Colour shfgc, double shadowOffsetX, double shadowOffsetY)
{
  if (!m_fillStyle)
    m_fillStyle = new VSDXFillStyle();

  m_fillStyle->fgColourId = colourIndexFG;
  m_fillStyle->bgColourId = colourIndexBG;
  m_fillStyle->pattern = fillPattern;
  m_fillStyle->fgTransparency = fillFGTransparency;
  m_fillStyle->bgTransparency = fillBGTransparency;

  m_fillStyle->shadowPattern = shadowPattern;
  m_fillStyle->shadowFgColour = shfgc;
  m_fillStyle->shadowOffsetX = shadowOffsetX;
  m_fillStyle->shadowOffsetY = shadowOffsetY;

  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectCharIXStyle(unsigned /*id*/ , unsigned level, unsigned charCount, unsigned short fontID, Colour fontColour, unsigned langID, double fontSize,
                                                       bool bold, bool italic, bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
                                                       bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, WPXString fontFace)
{
  _handleLevelChange(level);
  if (m_textStyle == 0) m_textStyle = new VSDXTextStyle();
  CharFormat f(charCount, fontID, fontColour, langID, fontSize, bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
               allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
  m_textStyle->format = f;
}

void libvisio::VSDXStylesCollector::collectTextBlockStyle(unsigned /* id */, unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin,
                                                          unsigned char verticalAlign, unsigned char textBkgndColour, unsigned char textBkgndTransparency,
                                                          double defaultTabStop,  unsigned char textDirection)
{
  if (!m_textStyle)
    m_textStyle = new VSDXTextStyle();

  TextBlockFormat f(leftMargin, rightMargin, topMargin, bottomMargin, verticalAlign, textBkgndColour, textBkgndTransparency, defaultTabStop, textDirection);
  m_textStyle->txtBlockFormat = f;

  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectFillStyle(unsigned id, unsigned level, unsigned char colourIndexFG, unsigned char colourIndexBG, unsigned char fillPattern, unsigned char fillFGTransparency, unsigned char fillBGTransparency, unsigned char shadowPattern, Colour shfgc)
{
  collectFillStyle(id, level, colourIndexFG, colourIndexBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc, m_shadowOffsetX, m_shadowOffsetY);
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

      if (m_lineStyle)
        delete m_lineStyle;
      if (m_fillStyle)
        delete m_fillStyle;
      if (m_textStyle)
        delete m_textStyle;
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
