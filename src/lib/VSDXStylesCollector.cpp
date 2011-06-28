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

#include "VSDXStylesCollector.h"

libvisio::VSDXStylesCollector::VSDXStylesCollector()
{
}

void libvisio::VSDXStylesCollector::collectEllipticalArcTo(unsigned id, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc)
{
}

void libvisio::VSDXStylesCollector::collectForeignData(unsigned id, unsigned level, const WPXBinaryData &binaryData)
{
}

void libvisio::VSDXStylesCollector::collectEllipse(unsigned id, unsigned level, double cx, double cy, double aa, double dd)
{
}

void libvisio::VSDXStylesCollector::collectLine(unsigned id, unsigned level, double strokeWidth, Colour c, unsigned linePattern)
{
}

void libvisio::VSDXStylesCollector::collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern)
{
}

void libvisio::VSDXStylesCollector::collectGeomList(unsigned id, unsigned level, const std::vector<unsigned> &geometryOrder)
{
}

void libvisio::VSDXStylesCollector::collectGeometry(unsigned id, unsigned level, unsigned geomFlags)
{
}

void libvisio::VSDXStylesCollector::collectMoveTo(unsigned id, unsigned level, double x, double y)
{
}

void libvisio::VSDXStylesCollector::collectLineTo(unsigned id, unsigned level, double x, double y)
{
}

void libvisio::VSDXStylesCollector::collectArcTo(unsigned id, unsigned level, double x2, double y2, double bow)
{
}

void libvisio::VSDXStylesCollector::collectXFormData(unsigned id, unsigned level, const XForm &xform)
{
}

void libvisio::VSDXStylesCollector::collectShapeID(unsigned id, unsigned level, unsigned shapeId)
{
}

void libvisio::VSDXStylesCollector::collectForeignDataType(unsigned id, unsigned level, unsigned foreignType, unsigned foreignFormat)
{
}

void libvisio::VSDXStylesCollector::collectPageProps(unsigned id, unsigned level, double pageWidth, double pageHeight)
{
}

void libvisio::VSDXStylesCollector::collectShape(unsigned id, unsigned level)
{
}

void libvisio::VSDXStylesCollector::collectUnhandledChunk(unsigned id, unsigned level)
{
}

void libvisio::VSDXStylesCollector::collectColours(const std::vector<Colour> &colours)
{
}

void libvisio::VSDXStylesCollector::startPage()
{
}

void libvisio::VSDXStylesCollector::endPage()
{
}
