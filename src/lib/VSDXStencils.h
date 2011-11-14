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

#ifndef __VSDXSTENCILS_H__
#define __VSDXSTENCILS_H__

#include <map>
#include <vector>
#include "VSDXStyles.h"
#include "VSDXGeometryList.h"
#include "VSDXFieldList.h"
#include "VSDXTypes.h"

namespace libvisio
{

class VSDXStencilShape
{
public:
  VSDXStencilShape();
  VSDXStencilShape(const VSDXStencilShape &shape);
  ~VSDXStencilShape();
  VSDXStencilShape &operator=(const VSDXStencilShape &shape);

  std::vector<VSDXGeometryList> m_geometries;
  VSDXFieldList m_fields;
  ForeignData *m_foreign;
  unsigned m_lineStyleId, m_fillStyleId, m_textStyleId;
  VSDXLineStyle *m_lineStyle;
  VSDXFillStyle *m_fillStyle;
  VSDXTextBlockStyle *m_textBlockStyle;
  VSDXCharStyle *m_charStyle;
  VSDXParaStyle *m_paraStyle;
  WPXBinaryData m_text;
  std::vector< VSDXName > m_names;
  TextFormat m_textFormat;
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
  unsigned count() const
  {
    return m_stencils.size();
  }
private:
  std::map<unsigned, VSDXStencil> m_stencils;
};


} // namespace libvisio

#endif // __VSDXSTENCILS_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
