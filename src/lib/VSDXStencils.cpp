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

libvisio::VSDXStencilShape::VSDXStencilShape()
{
}

libvisio::VSDXStencilShape::VSDXStencilShape(const libvisio::VSDXStencilShape &shape)
{
}

libvisio::VSDXStencilShape::~VSDXStencilShape()
{
}

libvisio::VSDXStencilShape &libvisio::VSDXStencilShape::operator=(const libvisio::VSDXStencilShape &shape)
{
  return *this;
}




libvisio::VSDXStencil::VSDXStencil()
{
}

libvisio::VSDXStencil::VSDXStencil(const libvisio::VSDXStencil &stencil)
{
}

libvisio::VSDXStencil::~VSDXStencil()
{
}

libvisio::VSDXStencil &libvisio::VSDXStencil::operator=(const libvisio::VSDXStencil &stencil)
{
  return *this;
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
