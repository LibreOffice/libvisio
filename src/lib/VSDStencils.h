/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDSTENCILS_H__
#define __VSDSTENCILS_H__

#include <map>
#include <memory>
#include <vector>
#include "VSDStyles.h"
#include "VSDGeometryList.h"
#include "VSDFieldList.h"
#include "VSDTypes.h"
#include "VSDParagraphList.h"
#include "VSDCharacterList.h"
#include "VSDShapeList.h"

namespace libvisio
{

class VSDShape
{
public:
  VSDShape();
  VSDShape(const VSDShape &shape);
  ~VSDShape();
  VSDShape &operator=(const VSDShape &shape);
  void clear();

  std::map<unsigned, VSDGeometryList> m_geometries;
  VSDShapeList m_shapeList;
  VSDFieldList m_fields;
  std::unique_ptr<ForeignData> m_foreign;
  unsigned m_parent, m_masterPage, m_masterShape, m_shapeId;
  unsigned m_lineStyleId, m_fillStyleId, m_textStyleId;
  VSDOptionalLineStyle m_lineStyle;
  VSDOptionalFillStyle m_fillStyle;
  VSDOptionalTextBlockStyle m_textBlockStyle;
  VSDOptionalCharStyle m_charStyle;
  VSDCharacterList m_charList;
  VSDOptionalParaStyle m_paraStyle;
  VSDParagraphList m_paraList;
  std::map<unsigned, VSDTabSet> m_tabSets;
  librevenge::RVNGBinaryData m_text;
  std::map<unsigned, VSDName> m_names;
  TextFormat m_textFormat;
  std::map<unsigned, NURBSData> m_nurbsData;
  std::map<unsigned, PolylineData> m_polylineData;
  XForm m_xform;
  std::unique_ptr<XForm> m_txtxform;
  std::unique_ptr<XForm1D> m_xform1d;
  VSDMisc m_misc;
  VSDName m_layerMem;
};

class VSDStencil
{
public:
  VSDStencil();
  VSDStencil(const VSDStencil &stencil) = default;
  ~VSDStencil();
  VSDStencil &operator=(const VSDStencil &stencil) = default;
  void addStencilShape(unsigned id, const VSDShape &shape);
  void setFirstShape(unsigned id);
  const VSDShape *getStencilShape(unsigned id) const;
  std::map<unsigned, VSDShape> m_shapes;
  double m_shadowOffsetX;
  double m_shadowOffsetY;
  unsigned m_firstShapeId;
};

class VSDStencils
{
public:
  VSDStencils();
  ~VSDStencils();
  void addStencil(unsigned idx, const VSDStencil &stencil);
  const VSDStencil *getStencil(unsigned idx) const;
  const VSDShape *getStencilShape(unsigned pageId, unsigned shapeId) const;
  unsigned count() const
  {
    return m_stencils.size();
  }
private:
  std::map<unsigned, VSDStencil> m_stencils;
};


} // namespace libvisio

#endif // __VSDSTENCILS_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
