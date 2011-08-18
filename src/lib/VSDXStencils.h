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
#include <vector>
#include "VSDXStyles.h"
#include "VSDXGeometryList.h"
#include "VSDXTypes.h"

namespace libvisio {

class VSDXStencilShape
{
  public:
    VSDXStencilShape();
    VSDXStencilShape(const VSDXStencilShape &shape);
    ~VSDXStencilShape();
    VSDXStencilShape &operator=(const VSDXStencilShape &shape);

    std::vector<VSDXGeometryList> m_geometries;
    ForeignData * m_foreign;
    unsigned m_lineStyleID, m_fillStyleID;
    VSDXLineStyle * m_lineStyle;
    VSDXFillStyle * m_fillStyle;
    WPXString m_text;
    std::vector<CharFormat> m_charFormats;
    std::map<unsigned, NURBSData> m_nurbsData;
    std::map<unsigned, PolylineData> m_polylineData;
};

class VSDXStencil
{
  public:
    VSDXStencil();
    VSDXStencil(const VSDXStencil &stencil);
    ~VSDXStencil();
    VSDXStencil &operator=(const VSDXStencil &stencil);
    void addStencilShape(unsigned id, const VSDXStencilShape &shape);
    const VSDXStencilShape *getStencilShape(unsigned id) const;
    std::map<unsigned, VSDXStencilShape> m_shapes;
    double m_shadowOffsetX;
    double m_shadowOffsetY;
};

class VSDXStencils
{
  public:
    VSDXStencils();
    ~VSDXStencils();
    void addStencil(unsigned idx, const VSDXStencil &stencil);
    const VSDXStencil *getStencil(unsigned idx) const;
    unsigned count() const { return m_stencils.size(); }
  private:
    std::map<unsigned, VSDXStencil> m_stencils;
};


} // namespace libvisio

#endif // __VSDXSTENCILS_H__
