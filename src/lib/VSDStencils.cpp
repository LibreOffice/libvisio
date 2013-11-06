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

#include "VSDStencils.h"
#include "libvisio_utils.h"

libvisio::VSDShape::VSDShape()
  : m_geometries(), m_shapeList(), m_fields(), m_foreign(0), m_parent(0), m_masterPage(MINUS_ONE),
    m_masterShape(MINUS_ONE), m_shapeId(MINUS_ONE), m_lineStyleId(MINUS_ONE), m_fillStyleId(MINUS_ONE),
    m_textStyleId(MINUS_ONE), m_lineStyle(), m_fillStyle(), m_textBlockStyle(), m_charStyle(), m_charList(),
    m_paraStyle(), m_paraList(), m_text(), m_names(), m_textFormat(libvisio::VSD_TEXT_UTF16),
    m_nurbsData(), m_polylineData(), m_xform(), m_txtxform(0), m_misc()
{
}

libvisio::VSDShape::VSDShape(const libvisio::VSDShape &shape)
  : m_geometries(shape.m_geometries), m_shapeList(shape.m_shapeList), m_fields(shape.m_fields),
    m_foreign(shape.m_foreign ? new ForeignData(*(shape.m_foreign)) : 0), m_parent(shape.m_parent),
    m_masterPage(shape.m_masterPage), m_masterShape(shape.m_masterShape), m_shapeId(shape.m_shapeId),
    m_lineStyleId(shape.m_lineStyleId), m_fillStyleId(shape.m_fillStyleId), m_textStyleId(shape.m_textStyleId),
    m_lineStyle(shape.m_lineStyle), m_fillStyle(shape.m_fillStyle), m_textBlockStyle(shape.m_textBlockStyle),
    m_charStyle(shape.m_charStyle), m_charList(shape.m_charList), m_paraStyle(shape.m_paraStyle),
    m_paraList(shape.m_paraList), m_text(shape.m_text), m_names(shape.m_names),
    m_textFormat(shape.m_textFormat), m_nurbsData(shape.m_nurbsData), m_polylineData(shape.m_polylineData),
    m_xform(shape.m_xform), m_txtxform(shape.m_txtxform ? new XForm(*(shape.m_txtxform)) : 0), m_misc(shape.m_misc)
{
}

libvisio::VSDShape::~VSDShape()
{
  clear();
}

libvisio::VSDShape &libvisio::VSDShape::operator=(const libvisio::VSDShape &shape)
{
  if (this != &shape)
  {
    m_geometries = shape.m_geometries;
    m_shapeList = shape.m_shapeList;
    m_fields = shape.m_fields;
    if (m_foreign)
      delete m_foreign;
    m_foreign = shape.m_foreign ? new ForeignData(*(shape.m_foreign)) : 0;
    m_parent = shape.m_parent;
    m_masterPage = shape.m_masterPage;
    m_masterShape = shape.m_masterShape;
    m_shapeId = shape.m_shapeId;
    m_lineStyleId = shape.m_lineStyleId;
    m_fillStyleId = shape.m_fillStyleId;
    m_textStyleId = shape.m_textStyleId;
    m_lineStyle = shape.m_lineStyle;
    m_fillStyle = shape.m_fillStyle;
    m_textBlockStyle = shape.m_textBlockStyle;
    m_charStyle = shape.m_charStyle;
    m_charList = shape.m_charList;
    m_paraStyle = shape.m_paraStyle;
    m_paraList = shape.m_paraList;
    m_text = shape.m_text;
    m_names = shape.m_names;
    m_textFormat = shape.m_textFormat;
    m_nurbsData = shape.m_nurbsData;
    m_polylineData = shape.m_polylineData;
    m_xform = shape.m_xform;
    if (m_txtxform)
      delete m_txtxform;
    m_txtxform = shape.m_txtxform ? new XForm(*(shape.m_txtxform)) : 0;
    m_misc = shape.m_misc;
  }
  return *this;
}

void libvisio::VSDShape::clear()
{
  if (m_foreign)
    delete m_foreign;
  m_foreign = 0;
  if (m_txtxform)
    delete m_txtxform;
  m_txtxform = 0;

  m_geometries.clear();
  m_shapeList.clear();
  m_fields.clear();
  m_lineStyle = VSDOptionalLineStyle();
  m_fillStyle = VSDOptionalFillStyle();
  m_textBlockStyle = VSDOptionalTextBlockStyle();
  m_charStyle = VSDOptionalCharStyle();
  m_charList.clear();
  m_paraStyle = VSDOptionalParaStyle();
  m_paraList.clear();
  m_text.clear();
  m_names.clear();
  m_nurbsData.clear();
  m_polylineData.clear();
  m_xform = XForm();
  m_parent = 0;
  m_masterPage = MINUS_ONE;
  m_masterShape = MINUS_ONE;
  m_shapeId = MINUS_ONE;
  m_lineStyleId = MINUS_ONE;
  m_fillStyleId = MINUS_ONE;
  m_textStyleId = MINUS_ONE;
  m_textFormat = libvisio::VSD_TEXT_UTF16;
  m_misc = VSDMisc();
}

libvisio::VSDStencil::VSDStencil()
  : m_shapes(), m_shadowOffsetX(0.0), m_shadowOffsetY(0.0), m_firstShapeId(MINUS_ONE)
{
}

libvisio::VSDStencil::VSDStencil(const libvisio::VSDStencil &stencil)
  : m_shapes(stencil.m_shapes), m_shadowOffsetX(stencil.m_shadowOffsetX),
    m_shadowOffsetY(stencil.m_shadowOffsetY), m_firstShapeId(stencil.m_firstShapeId)
{
}

libvisio::VSDStencil::~VSDStencil()
{
}

libvisio::VSDStencil &libvisio::VSDStencil::operator=(const libvisio::VSDStencil &stencil)
{
  if (this != &stencil)
  {
    m_shapes = stencil.m_shapes;
    m_shadowOffsetX = stencil.m_shadowOffsetX;
    m_shadowOffsetY = stencil.m_shadowOffsetY;
    m_firstShapeId = stencil.m_firstShapeId;
  }
  return *this;
}

void libvisio::VSDStencil::addStencilShape(unsigned id, const VSDShape &shape)
{
  m_shapes[id] = shape;
}

void libvisio::VSDStencil::setFirstShape(unsigned id)
{
  if (m_firstShapeId == MINUS_ONE)
    m_firstShapeId = id;
}

const libvisio::VSDShape *libvisio::VSDStencil::getStencilShape(unsigned id) const
{
  std::map<unsigned, VSDShape>::const_iterator iter = m_shapes.find(id);
  if (iter != m_shapes.end())
    return &(iter->second);
  else
    return 0;
}



libvisio::VSDStencils::VSDStencils() :
  m_stencils()
{
}

libvisio::VSDStencils::~VSDStencils()
{
}

void libvisio::VSDStencils::addStencil(unsigned idx, const libvisio::VSDStencil &stencil)
{
  m_stencils[idx] = stencil;
}

const libvisio::VSDStencil *libvisio::VSDStencils::getStencil(unsigned idx) const
{
  std::map<unsigned, VSDStencil>::const_iterator iter = m_stencils.find(idx);
  if (iter != m_stencils.end())
    return &(iter->second);
  else
    return 0;
}

const libvisio::VSDShape *libvisio::VSDStencils::getStencilShape(unsigned pageId, unsigned shapeId) const
{
  if (MINUS_ONE == pageId)
    return 0;
  const libvisio::VSDStencil *tmpStencil = getStencil(pageId);
  if (!tmpStencil)
    return 0;
  if (MINUS_ONE == shapeId)
    shapeId = tmpStencil->m_firstShapeId;
  return tmpStencil->getStencilShape(shapeId);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
