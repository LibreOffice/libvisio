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

#include "VSDXStencils.h"
#include "libvisio_utils.h"

libvisio::VSDXStencilShape::VSDXStencilShape()
  : m_geometries(), m_foreign(0), m_lineStyleID(0xffffffff), m_fillStyleID(0xffffffff), m_textStyleID(0xffffffff),
    m_lineStyle(0), m_fillStyle(0), m_textStyle(0), m_text(), m_nurbsData(), m_polylineData()
{
}

libvisio::VSDXStencilShape::VSDXStencilShape(const libvisio::VSDXStencilShape &shape)
  : m_geometries(shape.m_geometries), m_foreign(shape.m_foreign ? new ForeignData(*(shape.m_foreign)) : 0),
    m_lineStyleID(shape.m_lineStyleID), m_fillStyleID(shape.m_fillStyleID), m_textStyleID(shape.m_textStyleID),
    m_lineStyle(shape.m_lineStyle ? new VSDXLineStyle(*(shape.m_lineStyle)) : 0),
    m_fillStyle(shape.m_fillStyle ? new VSDXFillStyle(*(shape.m_fillStyle)) : 0),
    m_textStyle(shape.m_textStyle ? new VSDXTextStyle(*(shape.m_textStyle)) : 0),
    m_text(shape.m_text), m_nurbsData(shape.m_nurbsData), m_polylineData(shape.m_polylineData) {}

libvisio::VSDXStencilShape::~VSDXStencilShape()
{
  if (m_foreign)
    delete m_foreign;
  if (m_lineStyle)
    delete m_lineStyle;
  if (m_fillStyle)
    delete m_fillStyle;
  if (m_textStyle)
    delete m_textStyle;
}

libvisio::VSDXStencilShape &libvisio::VSDXStencilShape::operator=(const libvisio::VSDXStencilShape &shape)
{
  m_geometries = shape.m_geometries;
  if (m_foreign)
    delete m_foreign;
  m_foreign = shape.m_foreign ? new ForeignData(*(shape.m_foreign)) : 0;
  m_lineStyleID = shape.m_lineStyleID;
  m_fillStyleID = shape.m_fillStyleID;
  m_textStyleID = shape.m_textStyleID;
  if (m_lineStyle)
    delete m_lineStyle;
  m_lineStyle = shape.m_lineStyle ? new VSDXLineStyle(*(shape.m_lineStyle)) : 0;
  if (m_fillStyle)
    delete m_fillStyle;
  m_fillStyle = shape.m_fillStyle ? new VSDXFillStyle(*(shape.m_fillStyle)) : 0;
  if (m_textStyle)
    delete m_textStyle;
  m_textStyle = shape.m_textStyle ? new VSDXTextStyle(*(shape.m_textStyle)) : 0;
  m_text = shape.m_text;
  m_nurbsData = shape.m_nurbsData;
  m_polylineData = shape.m_polylineData;
  return *this;
}


libvisio::VSDXStencil::VSDXStencil()
  : m_shapes(), m_shadowOffsetX(0.0), m_shadowOffsetY(0.0)
{
}

libvisio::VSDXStencil::VSDXStencil(const libvisio::VSDXStencil &stencil)
  : m_shapes(stencil.m_shapes), m_shadowOffsetX(stencil.m_shadowOffsetX), m_shadowOffsetY(stencil.m_shadowOffsetY)
{
}

libvisio::VSDXStencil::~VSDXStencil()
{
}

libvisio::VSDXStencil &libvisio::VSDXStencil::operator=(const libvisio::VSDXStencil &stencil)
{
  m_shapes = stencil.m_shapes;
  m_shadowOffsetX = stencil.m_shadowOffsetX;
  m_shadowOffsetY = stencil.m_shadowOffsetY;
  return *this;
}

void libvisio::VSDXStencil::addStencilShape(unsigned id, const VSDXStencilShape &shape)
{
  m_shapes[id] = shape;
}

const libvisio::VSDXStencilShape *libvisio::VSDXStencil::getStencilShape(unsigned id) const
{
  std::map<unsigned, VSDXStencilShape>::const_iterator iter = m_shapes.find(id);
  if (iter != m_shapes.end())
    return &(iter->second);
  else
    return 0;  
}



libvisio::VSDXStencils::VSDXStencils() :
  m_stencils()
{
}

libvisio::VSDXStencils::~VSDXStencils()
{
}

void libvisio::VSDXStencils::addStencil(unsigned idx, const libvisio::VSDXStencil &stencil)
{
  m_stencils[idx] = stencil;
}

const libvisio::VSDXStencil *libvisio::VSDXStencils::getStencil(unsigned idx) const
{
  std::map<unsigned, VSDXStencil>::const_iterator iter = m_stencils.find(idx);
  if (iter != m_stencils.end())
    return &(iter->second);
  else
    return 0;
}
