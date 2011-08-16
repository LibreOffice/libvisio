/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
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

#include "VSDXStencils.h"
#include "libvisio_utils.h"

libvisio::VSDXStencilShape::VSDXStencilShape()
  : m_geometries(), m_foreign(0), m_lineStyleID(0xffffffff), m_fillStyleID(0xffffffff), m_lineStyle(0), m_fillStyle(0), m_text()
{
}

libvisio::VSDXStencilShape::VSDXStencilShape(const libvisio::VSDXStencilShape &shape)
  : m_geometries(shape.m_geometries), m_foreign(shape.m_foreign ? new ForeignData(*(shape.m_foreign)) : 0), m_lineStyleID(shape.m_lineStyleID),
    m_fillStyleID(shape.m_fillStyleID), m_lineStyle(shape.m_lineStyle ? new VSDXLineStyle(*(shape.m_lineStyle)) : 0),
    m_fillStyle(shape.m_fillStyle ? new VSDXFillStyle(*(shape.m_fillStyle)) : 0), m_text(shape.m_text),
    m_nurbsData(shape.m_nurbsData), m_polylineData(shape.m_polylineData) {}

libvisio::VSDXStencilShape::~VSDXStencilShape()
{
  if (m_foreign)
    delete m_foreign;
  if (m_lineStyle)
    delete m_lineStyle;
  if (m_fillStyle)
    delete m_fillStyle;
}

libvisio::VSDXStencilShape &libvisio::VSDXStencilShape::operator=(const libvisio::VSDXStencilShape &shape)
{
  m_geometries = shape.m_geometries;
  m_foreign = shape.m_foreign ? new ForeignData(*(shape.m_foreign)) : 0;
  m_lineStyleID = shape.m_lineStyleID;
  m_fillStyleID = shape.m_fillStyleID;
  m_lineStyle = shape.m_lineStyle ? new VSDXLineStyle(*(shape.m_lineStyle)) : 0;
  m_fillStyle = shape.m_fillStyle ? new VSDXFillStyle(*(shape.m_fillStyle)) : 0;
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



libvisio::VSDXStencils::VSDXStencils()
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
