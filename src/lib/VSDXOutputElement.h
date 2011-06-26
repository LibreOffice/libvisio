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

#ifndef __VSDXOUTPUTELEMENT_H__
#define __VSDXOUTPUTELEMENT_H__

#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>

namespace libvisio {

class VSDXOutputElement
{
public:
  VSDXOutputElement() {}
  virtual ~VSDXOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter) = 0;
};


class VSDXStyleOutputElement : public VSDXOutputElement
{
public:
  VSDXStyleOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec);
  virtual ~VSDXStyleOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
private:
  WPXPropertyList m_propList;
  WPXPropertyListVector m_propListVec;
};


class VSDXEllipseOutputElement : public VSDXOutputElement
{
public:
  VSDXEllipseOutputElement(const WPXPropertyList &propList);
  virtual ~VSDXEllipseOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
private:
  WPXPropertyList m_propList;
};


class VSDXPathOutputElement : public VSDXOutputElement
{
public:
  VSDXPathOutputElement(const WPXPropertyListVector &propListVec);
  virtual ~VSDXPathOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
private:
  WPXPropertyListVector m_propListVec;
};


class VSDXGraphicObjectOutputElement : public VSDXOutputElement
{
public:
  VSDXGraphicObjectOutputElement(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData);
  virtual ~VSDXGraphicObjectOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
private:
  WPXPropertyList m_propList;
  WPXBinaryData m_binaryData;
};

} // namespace libvisio

#endif // __VSDXOUTPUTELEMENT_H__
