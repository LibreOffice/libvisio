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

#include "VSDXOutputElement.h"

libvisio::VSDXStyleOutputElement::VSDXStyleOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec) :
  m_propList(propList), m_propListVec(propListVec) {}

void libvisio::VSDXStyleOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->setStyle(m_propList, m_propListVec);
}


libvisio::VSDXEllipseOutputElement::VSDXEllipseOutputElement(const WPXPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDXEllipseOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->drawEllipse(m_propList);
}


libvisio::VSDXPathOutputElement::VSDXPathOutputElement(const WPXPropertyListVector &propListVec) :
  m_propListVec(propListVec) {}

void libvisio::VSDXPathOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->drawPath(m_propListVec);
}


libvisio::VSDXGraphicObjectOutputElement::VSDXGraphicObjectOutputElement(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData) :
  m_propList(propList), m_binaryData(binaryData) {}

void libvisio::VSDXGraphicObjectOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->drawGraphicObject(m_propList, m_binaryData);
}

