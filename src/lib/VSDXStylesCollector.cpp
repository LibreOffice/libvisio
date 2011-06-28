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
  std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence
) :
  m_currentShapeId(0), m_groupXForms(), m_groupMemberships(),
  m_groupXFormsSequence(groupXFormsSequence),
  m_groupMembershipsSequence(groupMembershipsSequence)
{
  m_groupXFormsSequence.clear();
  m_groupMembershipsSequence.clear();
}

void libvisio::VSDXStylesCollector::collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectEllipse(unsigned id, unsigned level, double cx, double cy, double aa, double dd)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectLine(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectGeomList(unsigned id, unsigned level, const std::vector<unsigned> &geometryOrder)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectGeometry(unsigned id, unsigned level, unsigned geomFlags)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectMoveTo(unsigned id, unsigned level, double x, double y)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectLineTo(unsigned id, unsigned level, double x, double y)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectXFormData(unsigned id, unsigned level, const XForm &xform)
{
  _handleLevelChange(level);
  if (m_isShapeStarted)
    m_groupXForms[m_currentShapeId] = xform;
}

void libvisio::VSDXStylesCollector::collectShapeID(unsigned id, unsigned level, unsigned shapeId)
{
  _handleLevelChange(level);
  if (m_isShapeStarted)
    m_groupMemberships[shapeId] = m_currentShapeId;
}

void libvisio::VSDXStylesCollector::collectShapeList(unsigned id, unsigned level, std::vector<unsigned int> /*shapeList*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectForeignDataType(unsigned id, unsigned level, unsigned foreignType, unsigned foreignFormat)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectShape(unsigned id, unsigned level)
{
  _handleLevelChange(level);
  m_currentShapeId = id;
  m_isShapeStarted = true;
}

void libvisio::VSDXStylesCollector::collectUnhandledChunk(unsigned id, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDXStylesCollector::collectColours(const std::vector<Colour> &colours)
{
}

void libvisio::VSDXStylesCollector::startPage()
{
}

void libvisio::VSDXStylesCollector::endPage()
{
  m_groupXFormsSequence.push_back(m_groupXForms);
  m_groupXForms.clear();
  m_groupMembershipsSequence.push_back(m_groupMemberships);
  m_groupMemberships.clear();
}

void libvisio::VSDXStylesCollector::_handleLevelChange(unsigned level)
{
  if (m_currentLevel == level)
    return;
  if (level < 2)
    m_isShapeStarted = false;

  m_currentLevel = level;
}
