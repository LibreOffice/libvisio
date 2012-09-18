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

#include "VSDOutputElementList.h"

namespace libvisio
{

class VSDOutputElement
{
public:
  VSDOutputElement() {}
  virtual ~VSDOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter) = 0;
  virtual VSDOutputElement *clone() = 0;
};


class VSDStyleOutputElement : public VSDOutputElement
{
public:
  VSDStyleOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec);
  virtual ~VSDStyleOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDStyleOutputElement(m_propList, m_propListVec);
  }
private:
  WPXPropertyList m_propList;
  WPXPropertyListVector m_propListVec;
};


class VSDPathOutputElement : public VSDOutputElement
{
public:
  VSDPathOutputElement(const WPXPropertyListVector &propListVec);
  virtual ~VSDPathOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDPathOutputElement(m_propListVec);
  }
private:
  WPXPropertyListVector m_propListVec;
};


class VSDGraphicObjectOutputElement : public VSDOutputElement
{
public:
  VSDGraphicObjectOutputElement(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData);
  virtual ~VSDGraphicObjectOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDGraphicObjectOutputElement(m_propList, m_binaryData);
  }
private:
  WPXPropertyList m_propList;
  WPXBinaryData m_binaryData;
};


class VSDStartTextObjectOutputElement : public VSDOutputElement
{
public:
  VSDStartTextObjectOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec);
  virtual ~VSDStartTextObjectOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDStartTextObjectOutputElement(m_propList, m_propListVec);
  }
private:
  WPXPropertyList m_propList;
  WPXPropertyListVector m_propListVec;
};


class VSDStartTextLineOutputElement : public VSDOutputElement
{
public:
  VSDStartTextLineOutputElement(const WPXPropertyList &propList);
  virtual ~VSDStartTextLineOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDStartTextLineOutputElement(m_propList);
  }
private:
  WPXPropertyList m_propList;
};


class VSDStartLayerOutputElement : public VSDOutputElement
{
public:
  VSDStartLayerOutputElement(const WPXPropertyList &propList);
  virtual ~VSDStartLayerOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDStartLayerOutputElement(m_propList);
  }
private:
  WPXPropertyList m_propList;
};


class VSDEndLayerOutputElement : public VSDOutputElement
{
public:
  VSDEndLayerOutputElement();
  virtual ~VSDEndLayerOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDEndLayerOutputElement();
  }
};


class VSDStartTextSpanOutputElement : public VSDOutputElement
{
public:
  VSDStartTextSpanOutputElement(const WPXPropertyList &propList);
  virtual ~VSDStartTextSpanOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDStartTextSpanOutputElement(m_propList);
  }
private:
  WPXPropertyList m_propList;
};


class VSDInsertTextOutputElement : public VSDOutputElement
{
public:
  VSDInsertTextOutputElement(const WPXString &text);
  virtual ~VSDInsertTextOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDInsertTextOutputElement(m_text);
  }
private:
  WPXString m_text;
};


class VSDEndTextSpanOutputElement : public VSDOutputElement
{
public:
  VSDEndTextSpanOutputElement();
  virtual ~VSDEndTextSpanOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDEndTextSpanOutputElement();
  }
};


class VSDEndTextLineOutputElement : public VSDOutputElement
{
public:
  VSDEndTextLineOutputElement();
  virtual ~VSDEndTextLineOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDEndTextLineOutputElement();
  }
};


class VSDEndTextObjectOutputElement : public VSDOutputElement
{
public:
  VSDEndTextObjectOutputElement();
  virtual ~VSDEndTextObjectOutputElement() {}
  virtual void draw(libwpg::WPGPaintInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDEndTextObjectOutputElement();
  }
};

} // namespace libvisio

libvisio::VSDStyleOutputElement::VSDStyleOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec) :
  m_propList(propList), m_propListVec(propListVec) {}

void libvisio::VSDStyleOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->setStyle(m_propList, m_propListVec);
}


libvisio::VSDPathOutputElement::VSDPathOutputElement(const WPXPropertyListVector &propListVec) :
  m_propListVec(propListVec) {}

void libvisio::VSDPathOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->drawPath(m_propListVec);
}


libvisio::VSDGraphicObjectOutputElement::VSDGraphicObjectOutputElement(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData) :
  m_propList(propList), m_binaryData(binaryData) {}

void libvisio::VSDGraphicObjectOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->drawGraphicObject(m_propList, m_binaryData);
}


libvisio::VSDStartTextObjectOutputElement::VSDStartTextObjectOutputElement(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec) :
  m_propList(propList), m_propListVec(propListVec) {}

void libvisio::VSDStartTextObjectOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->startTextObject(m_propList, m_propListVec);
}

libvisio::VSDStartTextSpanOutputElement::VSDStartTextSpanOutputElement(const WPXPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDStartTextSpanOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->startTextSpan(m_propList);
}


libvisio::VSDStartLayerOutputElement::VSDStartLayerOutputElement(const WPXPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDStartLayerOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->startLayer(m_propList);
}


libvisio::VSDEndLayerOutputElement::VSDEndLayerOutputElement() {}

void libvisio::VSDEndLayerOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->endLayer();
}


libvisio::VSDStartTextLineOutputElement::VSDStartTextLineOutputElement(const WPXPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDStartTextLineOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->startTextLine(m_propList);
}


libvisio::VSDInsertTextOutputElement::VSDInsertTextOutputElement(const WPXString &text) :
  m_text(text) {}

void libvisio::VSDInsertTextOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->insertText(m_text);
}

libvisio::VSDEndTextSpanOutputElement::VSDEndTextSpanOutputElement() {}

void libvisio::VSDEndTextSpanOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->endTextSpan();
}


libvisio::VSDEndTextLineOutputElement::VSDEndTextLineOutputElement() {}

void libvisio::VSDEndTextLineOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->endTextLine();
}


libvisio::VSDEndTextObjectOutputElement::VSDEndTextObjectOutputElement() {}

void libvisio::VSDEndTextObjectOutputElement::draw(libwpg::WPGPaintInterface *painter)
{
  if (painter)
    painter->endTextObject();
}


libvisio::VSDOutputElementList::VSDOutputElementList()
  : m_elements()
{
}

libvisio::VSDOutputElementList::VSDOutputElementList(const libvisio::VSDOutputElementList &elementList)
  : m_elements()
{
  std::vector<libvisio::VSDOutputElement *>::const_iterator iter;
  for (iter = elementList.m_elements.begin(); iter != elementList.m_elements.end(); ++iter)
    m_elements.push_back((*iter)->clone());
}

libvisio::VSDOutputElementList &libvisio::VSDOutputElementList::operator=(const libvisio::VSDOutputElementList &elementList)
{
  for (std::vector<VSDOutputElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete (*iter);

  m_elements.clear();

  for (std::vector<VSDOutputElement *>::const_iterator cstiter = elementList.m_elements.begin(); cstiter != elementList.m_elements.end(); ++cstiter)
    m_elements.push_back((*cstiter)->clone());

  return *this;
}

void libvisio::VSDOutputElementList::append(const libvisio::VSDOutputElementList &elementList)
{
  for (std::vector<VSDOutputElement *>::const_iterator cstiter = elementList.m_elements.begin(); cstiter != elementList.m_elements.end(); ++cstiter)
    m_elements.push_back((*cstiter)->clone());
}

libvisio::VSDOutputElementList::~VSDOutputElementList()
{
  for (std::vector<VSDOutputElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete (*iter);
  m_elements.clear();
}

void libvisio::VSDOutputElementList::draw(libwpg::WPGPaintInterface *painter) const
{
  for (std::vector<VSDOutputElement *>::const_iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    (*iter)->draw(painter);
}

void libvisio::VSDOutputElementList::addStyle(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDStyleOutputElement(propList, propListVec));
}

void libvisio::VSDOutputElementList::addPath(const WPXPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDPathOutputElement(propListVec));
}

void libvisio::VSDOutputElementList::addGraphicObject(const WPXPropertyList &propList, const ::WPXBinaryData &binaryData)
{
  m_elements.push_back(new VSDGraphicObjectOutputElement(propList, binaryData));
}

void libvisio::VSDOutputElementList::addStartTextObject(const WPXPropertyList &propList, const WPXPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDStartTextObjectOutputElement(propList, propListVec));
}

void libvisio::VSDOutputElementList::addStartTextLine(const WPXPropertyList &propList)
{
  m_elements.push_back(new VSDStartTextLineOutputElement(propList));
}

void libvisio::VSDOutputElementList::addStartTextSpan(const WPXPropertyList &propList)
{
  m_elements.push_back(new VSDStartTextSpanOutputElement(propList));
}

void libvisio::VSDOutputElementList::addInsertText(const WPXString &text)
{
  m_elements.push_back(new VSDInsertTextOutputElement(text));
}

void libvisio::VSDOutputElementList::addEndTextSpan()
{
  m_elements.push_back(new VSDEndTextSpanOutputElement());
}

void libvisio::VSDOutputElementList::addEndTextLine()
{
  m_elements.push_back(new VSDEndTextLineOutputElement());
}

void libvisio::VSDOutputElementList::addEndTextObject()
{
  m_elements.push_back(new VSDEndTextObjectOutputElement());
}

void libvisio::VSDOutputElementList::addStartLayer(const WPXPropertyList &propList)
{
  m_elements.push_back(new VSDStartLayerOutputElement(propList));
}

void libvisio::VSDOutputElementList::addEndLayer()
{
  m_elements.push_back(new VSDEndLayerOutputElement());
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
