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

#include "VSDXOutputElementList.h"

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


class VSDXStartLayerOutputElement : public VSDXOutputElement
{
public:
  VSDXStartLayerOutputElement(const WPXPropertyList &propList);
  virtual ~VSDXStartLayerOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
private:
  WPXPropertyList m_propList;
};


class VSDXEndLayerOutputElement : public VSDXOutputElement
{
public:
  VSDXEndLayerOutputElement();
  virtual ~VSDXEndLayerOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
};

} // namespace libvisio

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


libvisio::VSDXStartLayerOutputElement::VSDXStartLayerOutputElement(const WPXPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDXStartLayerOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->startLayer(m_propList);
}


libvisio::VSDXEndLayerOutputElement::VSDXEndLayerOutputElement() {}

void libvisio::VSDXEndLayerOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->endLayer();
}


libvisio::VSDXOutputElementList::~VSDXOutputElementList()
{
  for (std::vector<VSDXOutputElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    delete (*iter);
  m_elements.clear();
}

void libvisio::VSDXOutputElementList::draw(libwpg::WPGPaintInterface *painter)
{
  for (std::vector<VSDXOutputElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    (*iter)->draw(painter);
}

void libvisio::VSDXOutputElementList::addStyle(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDXStyleOutputElement(propList, propListVec));
}

void libvisio::VSDXOutputElementList::addEllipse(const WPXPropertyList &propList)
{
  m_elements.push_back(new VSDXEllipseOutputElement(propList));
}

void libvisio::VSDXOutputElementList::addPath(const WPXPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDXPathOutputElement(propListVec));
}

void libvisio::VSDXOutputElementList::addGraphicObject(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData)
{
  m_elements.push_back(new VSDXGraphicObjectOutputElement(propList, binaryData));
}

void libvisio::VSDXOutputElementList::addStartLayer(const WPXPropertyList &propList)
{
  m_elements.push_back(new VSDXStartLayerOutputElement(propList));
}

void libvisio::VSDXOutputElementList::addEndLayer()
{
  m_elements.push_back(new VSDXEndLayerOutputElement());
}

void libvisio::VSDXOutputElementList::clear()
{
  for (std::vector<VSDXOutputElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    delete (*iter);
  m_elements.clear();
}
