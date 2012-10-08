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

#include <vector>
#include <map>
#include "VSDStylesCollector.h"

libvisio::VSDStylesCollector::VSDStylesCollector(
  std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
  std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
  std::vector<std::list<unsigned> > &documentPageShapeOrders
) :
  m_currentLevel(0), m_isShapeStarted(false),
  m_shadowOffsetX(0.0), m_shadowOffsetY(0.0),
  m_currentShapeId(0), m_groupXForms(), m_groupMemberships(),
  m_groupXFormsSequence(groupXFormsSequence),
  m_groupMembershipsSequence(groupMembershipsSequence),
  m_pageShapeOrder(), m_documentPageShapeOrders(documentPageShapeOrders),
  m_groupShapeOrder(), m_shapeList(), m_currentStyleSheet(0), m_styles(),
  m_lineStyle(0), m_fillStyle(0), m_textBlockStyle(0), m_charStyle(0), m_paraStyle(0),
  m_lineStyleMaster((unsigned)-1), m_fillStyleMaster((unsigned)-1), m_textStyleMaster((unsigned)-1),
  m_isStyleStarted(false), m_currentShapeLevel(0)
{
  m_groupXFormsSequence.clear();
  m_groupMembershipsSequence.clear();
  m_documentPageShapeOrders.clear();
}

void libvisio::VSDStylesCollector::collectEllipticalArcTo(unsigned /* id */, unsigned level, double /* x3 */, double /* y3 */,
    double /* x2 */, double /* y2 */, double /* angle */, double /* ecc */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectForeignData(unsigned /* id */, unsigned level, const WPXBinaryData & /* binaryData */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectOLEData(unsigned /* id */, unsigned level, const WPXBinaryData & /* oleData */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectEllipse(unsigned /* id */, unsigned level, double /* cx */, double /* cy */,
    double /* xleft */, double /* yleft */, double /* xtop */, double /* ytop */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectLine(unsigned level, double /* strokeWidth */, const Colour & /* c */,
    unsigned /* linePattern */, unsigned char /*startMarker*/, unsigned char /*endMarker*/, unsigned /* lineCap */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectFillAndShadow(unsigned level, const Colour & /* colourIndexFG */,
    const Colour & /* colourIndexBG */, unsigned /* fillPattern */, double /* fillFGTransparency */, double /* fillBGTransparency */,
    unsigned /* shadowPattern */, const Colour & /* shfgc */, double /* shadowOffsetX */, double /* shadowOffsetY */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectFillAndShadow(unsigned level, const Colour & /* colourIndexFG */,
    const Colour & /* colourIndexBG */, unsigned /* fillPattern */, double /* fillFGTransparency */, double /* fillBGTransparency */,
    unsigned /* shadowPattern */, const Colour & /* shfgc */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectGeometry(unsigned /* id */, unsigned level, bool /* noFill */, bool /* noLine */, bool /* noShow */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectMoveTo(unsigned /* id */, unsigned level, double /* x */, double /* y */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectLineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectArcTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */, double /* bow */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectNURBSTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */,
    unsigned char /* xType */, unsigned char /* yType */, unsigned /* degree */,
    std::vector<std::pair<double, double> > /* controlPoints */,
    std::vector<double> /* knotVector */, std::vector<double> /* weights */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectNURBSTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */, double /* knot */,
    double /* knotPrev */, double /* weight */, double /* weightPrev */, unsigned /* dataID */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectPolylineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */,
    unsigned char /* xType */, unsigned char /* yType */,
    const std::vector<std::pair<double, double> > & /* points */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectPolylineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */, unsigned /* dataID */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectSplineStart(unsigned /* id */, unsigned level, double /* x */, double /* y */,
    double /* secondKnot */, double /* firstKnot */, double /* lastKnot */, unsigned /* degree */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectSplineKnot(unsigned /* id */, unsigned level, double /* x */, double /* y */, double /* knot */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectSplineEnd()
{
}

void libvisio::VSDStylesCollector::collectInfiniteLine(unsigned /* id */, unsigned level, double /* x1 */, double /* y1 */, double /* x2 */, double /* y2 */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectShapeData(unsigned /* id */, unsigned level, unsigned char /* xType */, unsigned char /* yType */,
    unsigned /* degree */, double /*lastKnot*/, std::vector<std::pair<double, double> > /* controlPoints */,
    std::vector<double> /* knotVector */, std::vector<double> /* weights */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectShapeData(unsigned /* id */, unsigned level, unsigned char /* xType */, unsigned char /* yType */,
    std::vector<std::pair<double, double> > /* points */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectXFormData(unsigned level, const XForm &xform)
{
  _handleLevelChange(level);
  if (m_isShapeStarted)
    m_groupXForms[m_currentShapeId] = xform;
}

void libvisio::VSDStylesCollector::collectTxtXForm(unsigned level, const XForm & /* txtxform */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectShapeId(unsigned /* id */, unsigned level, unsigned shapeId)
{
  _handleLevelChange(level);
  if (m_isShapeStarted)
    m_groupMemberships[shapeId] = m_currentShapeId;
  m_shapeList.push_back(shapeId);
}

void libvisio::VSDStylesCollector::collectForeignDataType(unsigned /* id */, unsigned level, unsigned /* foreignType */, unsigned /* foreignFormat */,
    double /* offsetX */, double /* offsetY */, double /* width */, double /* height */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectPageProps(unsigned /* id */, unsigned level, double /* pageWidth */, double /* pageHeight */,
    double /* shadowOffsetX */, double /* shadowOffsetY */, double /* scale */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectPage(unsigned /* id */, unsigned level, unsigned /* backgroundPageID */, bool /* isBackgroundPage */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectShape(unsigned id, unsigned level, unsigned /*masterPage*/, unsigned /*masterShape*/,
    unsigned /* lineStyle */, unsigned /* fillStyle */, unsigned /* textStyle */)
{
  _handleLevelChange(level);
  m_currentShapeLevel = level;
  m_currentShapeId = id;
  m_isShapeStarted = true;
}

void libvisio::VSDStylesCollector::collectUnhandledChunk(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectFont(unsigned short /* fontID */, const ::WPXBinaryData & /* textStream */, TextFormat /* format */)
{
}

void libvisio::VSDStylesCollector::collectText(unsigned /*id*/, unsigned level, const ::WPXBinaryData & /*textStream*/, TextFormat /*format*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectVSDParaStyle(unsigned /* id */ , unsigned level, unsigned /* charCount */, double /* indFirst */, double /* indLeft */,
    double /* indRight */, double /* spLine */, double /* spBefore */, double /* spAfter */, unsigned char /* align */, unsigned /* flags */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectVSDCharStyle(unsigned /*id*/ , unsigned level, unsigned /*charCount*/, unsigned short /*fontID*/,
    Colour /*fontColour*/, double /*fontSize*/, bool /*bold*/, bool /*italic*/, bool /*underline*/,
    bool /* doubleunderline */, bool /* strikeout */, bool /* doublestrikeout */, bool /* allcaps */,
    bool /* initcaps */, bool /* smallcaps */, bool /* superscript */, bool /* subscript */, VSDFont /*fontFace*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectTextBlock(unsigned level, double /* leftMargin */, double /* rightMargin */,
    double /* topMargin */, double /* bottomMargin */,  unsigned char /* verticalAlign */, bool /* isBgFilled */,
    const Colour & /* colour */, double /* defaultTabStop */,  unsigned char /* textDirection */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectName(unsigned /*id*/, unsigned level, const ::WPXBinaryData & /*name*/, TextFormat /*format*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectPageSheet(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
  m_currentShapeLevel = level;
}

void libvisio::VSDStylesCollector::collectStyleSheet(unsigned id, unsigned level, unsigned lineStyleParent, unsigned fillStyleParent, unsigned textStyleParent)
{
  _handleLevelChange(level);
  // reusing the shape level for style sheet to avoid another variable
  m_currentShapeLevel = level;
  m_lineStyle = 0;
  m_fillStyle = 0;
  m_textBlockStyle = 0;
  m_charStyle = 0;
  m_paraStyle = 0;
  m_currentStyleSheet = id;
  m_styles.addLineStyleMaster(m_currentStyleSheet, lineStyleParent);
  m_styles.addFillStyleMaster(m_currentStyleSheet, fillStyleParent);
  m_styles.addTextStyleMaster(m_currentStyleSheet, textStyleParent);
  m_isStyleStarted = true;
}

void libvisio::VSDStylesCollector::collectLineStyle(unsigned level, double strokeWidth, const Colour &c, unsigned char linePattern,
    unsigned char startMarker, unsigned char endMarker, unsigned char lineCap)
{
  if (!m_lineStyle)
    m_lineStyle = new VSDLineStyle();

  m_lineStyle->width = strokeWidth;
  m_lineStyle->colour = c;
  m_lineStyle->pattern = linePattern;
  m_lineStyle->startMarker = startMarker;
  m_lineStyle->endMarker = endMarker;
  m_lineStyle->cap = lineCap;

  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectFillStyle(unsigned level, const Colour &colourFG, const Colour &colourBG,
    unsigned char fillPattern, double fillFGTransparency, double fillBGTransparency, unsigned char shadowPattern, const Colour &shfgc,
    double shadowOffsetX, double shadowOffsetY)
{
  if (!m_fillStyle)
    m_fillStyle = new VSDFillStyle();

  m_fillStyle->fgColour = colourFG;
  m_fillStyle->bgColour = colourBG;
  m_fillStyle->pattern = fillPattern;
  m_fillStyle->fgTransparency = fillFGTransparency;
  m_fillStyle->bgTransparency = fillBGTransparency;

  m_fillStyle->shadowPattern = shadowPattern;
  m_fillStyle->shadowFgColour = shfgc;
  m_fillStyle->shadowOffsetX = shadowOffsetX;
  m_fillStyle->shadowOffsetY = shadowOffsetY;

  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectFillStyle(unsigned level, const Colour &colourFG, const Colour &colourBG,
    unsigned char fillPattern, double fillFGTransparency, double fillBGTransparency, unsigned char shadowPattern, const Colour &shfgc)
{
  collectFillStyle(level, colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc, m_shadowOffsetX, m_shadowOffsetY);
}

void libvisio::VSDStylesCollector::collectParaIXStyle(unsigned /*id*/, unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
    double spLine, double spBefore, double spAfter, unsigned char align, unsigned flags)
{
  _handleLevelChange(level);
  if (!m_paraStyle)
    m_paraStyle = new VSDParaStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align, flags);
}


void libvisio::VSDStylesCollector::collectCharIXStyle(unsigned /*id*/ , unsigned level, unsigned charCount, unsigned short fontID,
    Colour fontColour, double fontSize, bool bold, bool italic, bool underline, bool doubleunderline,
    bool strikeout, bool doublestrikeout, bool allcaps, bool initcaps, bool smallcaps,
    bool superscript, bool subscript, VSDFont fontFace)
{
  _handleLevelChange(level);
  if (!m_charStyle)
    m_charStyle = new VSDCharStyle(charCount, fontID, fontColour, fontSize, bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                   allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
}

void libvisio::VSDStylesCollector::collectTextBlockStyle(unsigned level, double leftMargin, double rightMargin, double topMargin, double bottomMargin,
    unsigned char verticalAlign, bool isBgFilled, const Colour &bgColour, double defaultTabStop, unsigned char textDirection)
{
  _handleLevelChange(level);
  if (!m_textBlockStyle)
    m_textBlockStyle = new VSDTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin, verticalAlign, isBgFilled, bgColour, defaultTabStop, textDirection);
}

void libvisio::VSDStylesCollector::collectFieldList(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectTextField(unsigned /* id */, unsigned level, int /* nameId */, int /* formatStringId */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectNumericField(unsigned /* id */, unsigned level, unsigned short /* format */, double /* number */, int /* formatStringId */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::startPage(unsigned /* pageId */)
{
  m_groupXForms.clear();
  m_groupMemberships.clear();
  m_pageShapeOrder.clear();
  m_groupShapeOrder.clear();
}

void libvisio::VSDStylesCollector::endPage()
{
  _handleLevelChange(0);
  m_groupXFormsSequence.push_back(m_groupXForms);
  m_groupMembershipsSequence.push_back(m_groupMemberships);

  while (!m_groupShapeOrder.empty())
  {
    for (std::list<unsigned>::iterator j = m_pageShapeOrder.begin(); j != m_pageShapeOrder.end();)
    {
      std::map<unsigned, std::list<unsigned> >::iterator iter = m_groupShapeOrder.find(*j++);
      if (m_groupShapeOrder.end() != iter)
      {
        m_pageShapeOrder.splice(j, iter->second, iter->second.begin(), iter->second.end());
        m_groupShapeOrder.erase(iter);
      }
    }
  }
  m_documentPageShapeOrders.push_back(m_pageShapeOrder);
}

void libvisio::VSDStylesCollector::_handleLevelChange(unsigned level)
{
  if (m_currentLevel == level)
    return;
  if (level <= m_currentShapeLevel+1)
    _flushShapeList();
  if (level <= m_currentShapeLevel)
  {
    m_isShapeStarted = false;
    if (m_isStyleStarted)
    {
      m_isStyleStarted = false;
      m_styles.addLineStyle(m_currentStyleSheet, m_lineStyle);
      m_styles.addFillStyle(m_currentStyleSheet, m_fillStyle);
      m_styles.addTextBlockStyle(m_currentStyleSheet, m_textBlockStyle);
      m_styles.addCharStyle(m_currentStyleSheet, m_charStyle);
      m_styles.addParaStyle(m_currentStyleSheet, m_paraStyle);

      if (m_lineStyle)
        delete m_lineStyle;
      m_lineStyle = 0;

      if (m_fillStyle)
        delete m_fillStyle;
      m_fillStyle = 0;

      if (m_textBlockStyle)
        delete m_textBlockStyle;
      m_textBlockStyle = 0;

      if (m_charStyle)
        delete m_charStyle;
      m_charStyle = 0;

      if (m_paraStyle)
        delete m_paraStyle;
      m_paraStyle = 0;
    }
  }

  m_currentLevel = level;
}

void libvisio::VSDStylesCollector::_flushShapeList()
{
  if (m_shapeList.empty())
    return;

  if (m_isShapeStarted)
    m_groupShapeOrder[m_currentShapeId] = m_shapeList;
  else
    m_pageShapeOrder = m_shapeList;

  m_shapeList.clear();
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
