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
  m_currentShapeLevel(0)
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

void libvisio::VSDStylesCollector::collectForeignData(unsigned level, const WPXBinaryData & /* binaryData */)
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

void libvisio::VSDStylesCollector::collectLine(unsigned level, const boost::optional<double> & /* strokeWidth */,
    const boost::optional<Colour> & /* c */, const boost::optional<unsigned char> & /* linePattern */,
    const boost::optional<unsigned char> & /* startMarker */, const boost::optional<unsigned char> & /* endMarker */,
    const boost::optional<unsigned char> &/* lineCap */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectFillAndShadow(unsigned level, const boost::optional<Colour> & /* colourFG */, const boost::optional<Colour> & /* colourBG */,
    const boost::optional<unsigned char> & /* fillPattern */, const boost::optional<double> & /* fillFGTransparency */,
    const boost::optional<double> & /* fillBGTransparency */, const boost::optional<unsigned char> & /* shadowPattern */, const boost::optional<Colour> & /* shfgc */,
    const boost::optional<double> & /* shadowOffsetX */, const boost::optional<double> &/* shadowOffsetY */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectFillAndShadow(unsigned level, const boost::optional<Colour> & /* colourFG */, const boost::optional<Colour> & /* colourBG */,
    const boost::optional<unsigned char> & /* fillPattern */, const boost::optional<double> & /* fillFGTransparency */,
    const boost::optional<double> & /* fillBGTransparency */, const boost::optional<unsigned char> & /* shadowPattern */, const boost::optional<Colour> & /* shfgc */)
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
    unsigned char /* xType */, unsigned char /* yType */, unsigned /* degree */, const std::vector<std::pair<double, double> > & /* ctrlPts */,
    const std::vector<double> & /* kntVec */, const std::vector<double> & /* weights */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectNURBSTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */, double /* knot */,
    double /* knotPrev */, double /* weight */, double /* weightPrev */, unsigned /* dataID */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectNURBSTo(unsigned /* id */, unsigned level, double /* x2 */, double /* y2 */, double /* knot */,
    double /* knotPrev */, double /* weight */, double /* weightPrev */, const NURBSData & /* data */)
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

void libvisio::VSDStylesCollector::collectPolylineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */, const PolylineData & /* data */)
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

void libvisio::VSDStylesCollector::collectRelCubBezTo(unsigned /* id */, unsigned level, double /* x */, double /* y */, double /* a */, double /* b */, double /* c */, double /* d */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectRelEllipticalArcTo(unsigned /* id */, unsigned level, double /* x */, double /* y */, double /* a */, double /* b */, double /* c */, double /* d */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectRelLineTo(unsigned /* id */, unsigned level, double /* x */, double /* y */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectRelMoveTo(unsigned /* id */, unsigned level, double /* x */, double /* y */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectRelQuadBezTo(unsigned /* id */, unsigned level, double /* x */, double /* y */, double /* a */, double /* b */)
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

void libvisio::VSDStylesCollector::collectShapesOrder(unsigned /* id */, unsigned level, const std::vector<unsigned> &shapeIds)
{
  _handleLevelChange(level);
  m_shapeList.clear();
  for (unsigned i = 0; i < shapeIds.size(); ++i)
    m_shapeList.push_back(shapeIds[i]);
  _flushShapeList();
}

void libvisio::VSDStylesCollector::collectForeignDataType(unsigned level, unsigned /* foreignType */, unsigned /* foreignFormat */,
    double /* offsetX */, double /* offsetY */, double /* width */, double /* height */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectPageProps(unsigned /* id */, unsigned level, double /* pageWidth */, double /* pageHeight */,
    double /* shadowOffsetX */, double /* shadowOffsetY */, double /* scale */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectPage(unsigned /* id */, unsigned level, unsigned /* backgroundPageID */, bool /* isBackgroundPage */, const VSDName & /* pageName */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectShape(unsigned id, unsigned level, unsigned parent, unsigned /*masterPage*/, unsigned /*masterShape*/,
    unsigned /* lineStyle */, unsigned /* fillStyle */, unsigned /* textStyle */)
{
  _handleLevelChange(level);
  m_currentShapeLevel = level;
  m_currentShapeId = id;
  m_isShapeStarted = true;
  if (parent && parent != MINUS_ONE)
    m_groupMemberships[m_currentShapeId] = parent;
}

void libvisio::VSDStylesCollector::collectMisc(unsigned level, const VSDMisc & /* misc */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectUnhandledChunk(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectText(unsigned level, const ::WPXBinaryData & /*textStream*/, TextFormat /*format*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectParaIX(unsigned /* id */, unsigned level, unsigned /* charCount */,
    const boost::optional<double> & /* indFirst */, const boost::optional<double> & /* indLeft */, const boost::optional<double> & /* indRight */,
    const boost::optional<double> & /* spLine */, const boost::optional<double> & /* spBefore */, const boost::optional<double> & /* spAfter */,
    const boost::optional<unsigned char> & /* align */, const boost::optional<unsigned> & /* flags */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectDefaultParaStyle(unsigned /* charCount */, const boost::optional<double> & /* indFirst */,
    const boost::optional<double> & /* indLeft */, const boost::optional<double> & /* indRight */, const boost::optional<double> & /* spLine */,
    const boost::optional<double> & /* spBefore */, const boost::optional<double> & /* spAfter */,
    const boost::optional<unsigned char> & /* align */, const boost::optional<unsigned> & /* flags */)
{
}

void libvisio::VSDStylesCollector::collectCharIX(unsigned /* id */, unsigned level, unsigned /* charCount */,
    const boost::optional<VSDName> & /* font */, const boost::optional<Colour> & /* fontColour */, const boost::optional<double> & /* fontSize */,
    const boost::optional<bool> & /* bold */, const boost::optional<bool> & /* italic */, const boost::optional<bool> & /* underline */,
    const boost::optional<bool> & /* doubleunderline */, const boost::optional<bool> & /* strikeout */, const boost::optional<bool> & /* doublestrikeout */,
    const boost::optional<bool> & /* allcaps */, const boost::optional<bool> & /* initcaps */, const boost::optional<bool> & /* smallcaps */,
    const boost::optional<bool> & /* superscript */, const boost::optional<bool> & /* subscript */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectDefaultCharStyle(unsigned /* charCount */,
    const boost::optional<VSDName> & /* font */, const boost::optional<Colour> & /* fontColour */, const boost::optional<double> & /* fontSize */,
    const boost::optional<bool> & /* bold */, const boost::optional<bool> & /* italic */, const boost::optional<bool> & /* underline */,
    const boost::optional<bool> & /* doubleunderline */, const boost::optional<bool> & /* strikeout */, const boost::optional<bool> & /* doublestrikeout */,
    const boost::optional<bool> & /* allcaps */, const boost::optional<bool> & /* initcaps */, const boost::optional<bool> & /* smallcaps */,
    const boost::optional<bool> & /* superscript */, const boost::optional<bool> & /* subscript */)
{
}

void libvisio::VSDStylesCollector::collectTextBlock(unsigned level, const boost::optional<double> & /* leftMargin */,
    const boost::optional<double> & /* rightMargin */, const boost::optional<double> & /* topMargin */, const boost::optional<double> & /* bottomMargin */,
    const boost::optional<unsigned char> & /* verticalAlign */, const boost::optional<bool> & /* isBgFilled */, const boost::optional<Colour> & /* bgColour */,
    const boost::optional<double> & /* defaultTabStop */, const boost::optional<unsigned char> & /* textDirection */)
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

void libvisio::VSDStylesCollector::collectStyleSheet(unsigned /* id */, unsigned level, unsigned /* parentLineStyle */, unsigned /* parentFillStyle */, unsigned /* parentTextStyle */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectLineStyle(unsigned level, const boost::optional<double> & /* strokeWidth */, const boost::optional<Colour> & /* c */,
    const boost::optional<unsigned char> & /* linePattern */, const boost::optional<unsigned char> & /* startMarker */, const boost::optional<unsigned char> & /* endMarker */,
    const boost::optional<unsigned char> & /* lineCap */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectFillStyle(unsigned level, const boost::optional<Colour> & /* colourFG */, const boost::optional<Colour> & /* colourBG */,
    const boost::optional<unsigned char> & /* fillPattern */, const boost::optional<double> & /* fillFGTransparency */, const boost::optional<double> & /* fillBGTransparency */,
    const boost::optional<unsigned char> & /* shadowPattern */, const boost::optional<Colour> & /* shfgc */, const boost::optional<double> & /* shadowOffsetX */,
    const boost::optional<double> & /* shadowOffsetY */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectFillStyle(unsigned level, const boost::optional<Colour> & /* colourFG */, const boost::optional<Colour> & /* colourBG */,
    const boost::optional<unsigned char> & /* fillPattern */, const boost::optional<double> & /* fillFGTransparency */, const boost::optional<double> & /* fillBGTransparency */,
    const boost::optional<unsigned char> & /* shadowPattern */, const boost::optional<Colour> & /* shfgc */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectCharIXStyle(unsigned /* id */, unsigned level, unsigned /* charCount */, const boost::optional<VSDName> & /* font */,
    const boost::optional<Colour> & /* fontColour */, const boost::optional<double> & /* fontSize */, const boost::optional<bool> & /* bold */, const boost::optional<bool> & /* italic */,
    const boost::optional<bool> & /* underline */, const boost::optional<bool> & /* doubleunderline */, const boost::optional<bool> & /* strikeout */,
    const boost::optional<bool> & /* doublestrikeout */, const boost::optional<bool> & /* allcaps */, const boost::optional<bool> & /* initcaps */, const boost::optional<bool> & /* smallcaps */,
    const boost::optional<bool> & /* superscript */, const boost::optional<bool> & /* subscript */)
{
  _handleLevelChange(level);
}

void libvisio::VSDStylesCollector::collectParaIXStyle(unsigned /* id */, unsigned level, unsigned /* charCount */, const boost::optional<double> & /* indFirst */,
    const boost::optional<double> & /* indLeft */, const boost::optional<double> & /* indRight */, const boost::optional<double> & /* spLine */, const boost::optional<double> & /* spBefore */,
    const boost::optional<double> & /* spAfter */, const boost::optional<unsigned char> & /* align */, const boost::optional<unsigned> & /* flags */)
{
  _handleLevelChange(level);
}


void libvisio::VSDStylesCollector::collectTextBlockStyle(unsigned level, const boost::optional<double> & /* leftMargin */, const boost::optional<double> & /* rightMargin */,
    const boost::optional<double> & /* topMargin */, const boost::optional<double> & /* bottomMargin */, const boost::optional<unsigned char> & /* verticalAlign */,
    const boost::optional<bool> & /* isBgFilled */, const boost::optional<Colour> & /* bgColour */, const boost::optional<double> & /* defaultTabStop */,
    const boost::optional<unsigned char> & /* textDirection */)
{
  _handleLevelChange(level);
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
  if (level <= m_currentShapeLevel)
    m_isShapeStarted = false;

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
