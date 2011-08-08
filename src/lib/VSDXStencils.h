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

#ifndef __VSDXSTENCILS_H__
#define __VSDXSTENCILS_H__

#include <map>
#include "VSDXStyles.h"

namespace libvisio {

class VSDXStencilShape
{
  public:
    VSDXStencilShape();
    VSDXStencilShape(const VSDXStencilShape &shape);
	~VSDXStencilShape();
	VSDXStencilShape &operator=(const VSDXStencilShape &shape);
  private:
};

class VSDXStencil
{
  public:
    VSDXStencil();
    VSDXStencil(const VSDXStencil &stencil);
    ~VSDXStencil();
    VSDXStencil &operator=(const VSDXStencil &stencil);
  private:
};

class VSDXStencils
{
  public:
    VSDXStencils();
    ~VSDXStencils();
    void addStencil(unsigned idx, const VSDXStencil &stencil);
    const VSDXStencil *getStencil(unsigned idx) const;
  private:
    std::map<unsigned, VSDXStencil> m_stencils;
};


} // namespace libvisio

#endif // __VSDXSTENCILS_H__
