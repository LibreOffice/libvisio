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

#include "VSDXOutputElementList.h"

namespace libvisio
{

class VSDXOutputElement
{
public:
  VSDXOutputElement() {}
  virtual ~VSDXOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter) = 0;
  virtual VSDXOutputElement *clone() = 0;
};


class VSDXStyleOutputElement : public VSDXOutputElement
{
public:
  VSDXStyleOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec);
  virtual ~VSDXStyleOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXStyleOutputElement(m_propList, m_propListVec);
  }
private:
  WPXPropertyList m_propList;
  WPXPropertyListVector m_propListVec;
};


class VSDXPathOutputElement : public VSDXOutputElement
{
public:
  VSDXPathOutputElement(const WPXPropertyListVector &propListVec);
  virtual ~VSDXPathOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXPathOutputElement(m_propListVec);
  }
private:
  WPXPropertyListVector m_propListVec;
};


class VSDXGraphicObjectOutputElement : public VSDXOutputElement
{
public:
  VSDXGraphicObjectOutputElement(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData);
  virtual ~VSDXGraphicObjectOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXGraphicObjectOutputElement(m_propList, m_binaryData);
  }
private:
  WPXPropertyList m_propList;
  WPXBinaryData m_binaryData;
};


class VSDXStartTextObjectOutputElement : public VSDXOutputElement
{
public:
  VSDXStartTextObjectOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec);
  virtual ~VSDXStartTextObjectOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXStartTextObjectOutputElement(m_propList, m_propListVec);
  }
private:
  WPXPropertyList m_propList;
  WPXPropertyListVector m_propListVec;
};


class VSDXStartTextLineOutputElement : public VSDXOutputElement
{
public:
  VSDXStartTextLineOutputElement(const WPXPropertyList &propList);
  virtual ~VSDXStartTextLineOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXStartTextLineOutputElement(m_propList);
  }
private:
  WPXPropertyList m_propList;
};


class VSDXStartLayerOutputElement : public VSDXOutputElement
{
public:
  VSDXStartLayerOutputElement(const WPXPropertyList &propList);
  virtual ~VSDXStartLayerOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXStartLayerOutputElement(m_propList);
  }
private:
  WPXPropertyList m_propList;
};


class VSDXEndLayerOutputElement : public VSDXOutputElement
{
public:
  VSDXEndLayerOutputElement();
  virtual ~VSDXEndLayerOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXEndLayerOutputElement();
  }
};


class VSDXStartTextSpanOutputElement : public VSDXOutputElement
{
public:
  VSDXStartTextSpanOutputElement(const WPXPropertyList &propList);
  virtual ~VSDXStartTextSpanOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXStartTextSpanOutputElement(m_propList);
  }
private:
  WPXPropertyList m_propList;
};


class VSDXInsertTextOutputElement : public VSDXOutputElement
{
public:
  VSDXInsertTextOutputElement(const WPXString &text);
  virtual ~VSDXInsertTextOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXInsertTextOutputElement(m_text);
  }
private:
  WPXString m_text;
};


class VSDXEndTextSpanOutputElement : public VSDXOutputElement
{
public:
  VSDXEndTextSpanOutputElement();
  virtual ~VSDXEndTextSpanOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXEndTextSpanOutputElement();
  }
};


class VSDXEndTextLineOutputElement : public VSDXOutputElement
{
public:
  VSDXEndTextLineOutputElement();
  virtual ~VSDXEndTextLineOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXEndTextLineOutputElement();
  }
};


class VSDXEndTextObjectOutputElement : public VSDXOutputElement
{
public:
  VSDXEndTextObjectOutputElement();
  virtual ~VSDXEndTextObjectOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDXOutputElement *clone()
  {
    return new VSDXEndTextObjectOutputElement();
  }
};

} // namespace libvisio

libvisio::VSDXStyleOutputElement::VSDXStyleOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec) :
  m_propList(propList), m_propListVec(propListVec) {}

void libvisio::VSDXStyleOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->setStyle(m_propList, m_propListVec);
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


libvisio::VSDXStartTextObjectOutputElement::VSDXStartTextObjectOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec) :
  m_propList(propList), m_propListVec(propListVec) {}

void libvisio::VSDXStartTextObjectOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->startTextObject(m_propList, m_propListVec);
}

libvisio::VSDXStartTextSpanOutputElement::VSDXStartTextSpanOutputElement(const WPXPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDXStartTextSpanOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->startTextSpan(m_propList);
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


libvisio::VSDXStartTextLineOutputElement::VSDXStartTextLineOutputElement(const WPXPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDXStartTextLineOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->startTextLine(m_propList);
}


libvisio::VSDXInsertTextOutputElement::VSDXInsertTextOutputElement(const WPXString &text) :
  m_text(text) {}

void libvisio::VSDXInsertTextOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->insertText(m_text);
}

libvisio::VSDXEndTextSpanOutputElement::VSDXEndTextSpanOutputElement() {}

void libvisio::VSDXEndTextSpanOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->endTextSpan();
}


libvisio::VSDXEndTextLineOutputElement::VSDXEndTextLineOutputElement() {}

void libvisio::VSDXEndTextLineOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->endTextLine();
}


libvisio::VSDXEndTextObjectOutputElement::VSDXEndTextObjectOutputElement() {}

void libvisio::VSDXEndTextObjectOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->endTextObject();
}


libvisio::VSDXOutputElementList::VSDXOutputElementList()
  : m_elements()
{
}

libvisio::VSDXOutputElementList::VSDXOutputElementList(const libvisio::VSDXOutputElementList &elementList)
  : m_elements()
{
  std::vector<libvisio::VSDXOutputElement *>::const_iterator iter;
  for (iter = elementList.m_elements.begin(); iter != elementList.m_elements.end(); ++iter)
    m_elements.push_back((*iter)->clone());
}

libvisio::VSDXOutputElementList &libvisio::VSDXOutputElementList::operator=(const libvisio::VSDXOutputElementList &elementList)
{
  for (std::vector<VSDXOutputElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete (*iter);

  m_elements.clear();

  for (std::vector<VSDXOutputElement *>::const_iterator cstiter = elementList.m_elements.begin(); cstiter != elementList.m_elements.end(); ++cstiter)
    m_elements.push_back((*cstiter)->clone());

  return *this;
}

void libvisio::VSDXOutputElementList::append(const libvisio::VSDXOutputElementList &elementList)
{
  for (std::vector<VSDXOutputElement *>::const_iterator cstiter = elementList.m_elements.begin(); cstiter != elementList.m_elements.end(); ++cstiter)
    m_elements.push_back((*cstiter)->clone());
}

libvisio::VSDXOutputElementList::~VSDXOutputElementList()
{
  for (std::vector<VSDXOutputElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete (*iter);
  m_elements.clear();
}

void libvisio::VSDXOutputElementList::draw(libwpg::WPGPaintInterface *painter) const
{
  for (std::vector<VSDXOutputElement *>::const_iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    (*iter)->draw(painter);
}

void libvisio::VSDXOutputElementList::addStyle(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDXStyleOutputElement(propList, propListVec));
}

void libvisio::VSDXOutputElementList::addPath(const WPXPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDXPathOutputElement(propListVec));
}

void libvisio::VSDXOutputElementList::addGraphicObject(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData)
{
  m_elements.push_back(new VSDXGraphicObjectOutputElement(propList, binaryData));
}

void libvisio::VSDXOutputElementList::addStartTextObject(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDXStartTextObjectOutputElement(propList, propListVec));
}

void libvisio::VSDXOutputElementList::addStartTextLine(const WPXPropertyList &propList)
{
  m_elements.push_back(new VSDXStartTextLineOutputElement(propList));
}

void libvisio::VSDXOutputElementList::addStartTextSpan(const WPXPropertyList &propList)
{
  m_elements.push_back(new VSDXStartTextSpanOutputElement(propList));
}

void libvisio::VSDXOutputElementList::addInsertText(const WPXString &text)
{
  m_elements.push_back(new VSDXInsertTextOutputElement(text));
}

void libvisio::VSDXOutputElementList::addEndTextSpan()
{
  m_elements.push_back(new VSDXEndTextSpanOutputElement());
}

void libvisio::VSDXOutputElementList::addEndTextLine()
{
  m_elements.push_back(new VSDXEndTextLineOutputElement());
}

void libvisio::VSDXOutputElementList::addEndTextObject()
{
  m_elements.push_back(new VSDXEndTextObjectOutputElement());
}

void libvisio::VSDXOutputElementList::addStartLayer(const WPXPropertyList &propList)
{
  m_elements.push_back(new VSDXStartLayerOutputElement(propList));
}

void libvisio::VSDXOutputElementList::addEndLayer()
{
  m_elements.push_back(new VSDXEndLayerOutputElement());
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
