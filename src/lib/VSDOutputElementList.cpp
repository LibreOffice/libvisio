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
  virtual void draw(RVNGDrawingInterface *painter) = 0;
  virtual VSDOutputElement *clone() = 0;
};


class VSDStyleOutputElement : public VSDOutputElement
{
public:
  VSDStyleOutputElement(const RVNGPropertyList &propList, const RVNGPropertyListVector &propListVec);
  virtual ~VSDStyleOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDStyleOutputElement(m_propList, m_propListVec);
  }
private:
  RVNGPropertyList m_propList;
  RVNGPropertyListVector m_propListVec;
};


class VSDPathOutputElement : public VSDOutputElement
{
public:
  VSDPathOutputElement(const RVNGPropertyListVector &propListVec);
  virtual ~VSDPathOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDPathOutputElement(m_propListVec);
  }
private:
  RVNGPropertyListVector m_propListVec;
};


class VSDGraphicObjectOutputElement : public VSDOutputElement
{
public:
  VSDGraphicObjectOutputElement(const RVNGPropertyList &propList, const ::RVNGBinaryData &binaryData);
  virtual ~VSDGraphicObjectOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDGraphicObjectOutputElement(m_propList, m_binaryData);
  }
private:
  RVNGPropertyList m_propList;
  RVNGBinaryData m_binaryData;
};


class VSDStartTextObjectOutputElement : public VSDOutputElement
{
public:
  VSDStartTextObjectOutputElement(const RVNGPropertyList &propList, const RVNGPropertyListVector &propListVec);
  virtual ~VSDStartTextObjectOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDStartTextObjectOutputElement(m_propList, m_propListVec);
  }
private:
  RVNGPropertyList m_propList;
  RVNGPropertyListVector m_propListVec;
};


class VSDOpenParagraphOutputElement : public VSDOutputElement
{
public:
  VSDOpenParagraphOutputElement(const RVNGPropertyList &propList, const RVNGPropertyListVector &propListVec);
  virtual ~VSDOpenParagraphOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDOpenParagraphOutputElement(m_propList, m_propListVec);
  }
private:
  RVNGPropertyList m_propList;
  RVNGPropertyListVector m_propListVec;
};


class VSDStartLayerOutputElement : public VSDOutputElement
{
public:
  VSDStartLayerOutputElement(const RVNGPropertyList &propList);
  virtual ~VSDStartLayerOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDStartLayerOutputElement(m_propList);
  }
private:
  RVNGPropertyList m_propList;
};


class VSDEndLayerOutputElement : public VSDOutputElement
{
public:
  VSDEndLayerOutputElement();
  virtual ~VSDEndLayerOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDEndLayerOutputElement();
  }
};


class VSDOpenSpanOutputElement : public VSDOutputElement
{
public:
  VSDOpenSpanOutputElement(const RVNGPropertyList &propList);
  virtual ~VSDOpenSpanOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDOpenSpanOutputElement(m_propList);
  }
private:
  RVNGPropertyList m_propList;
};


class VSDInsertTextOutputElement : public VSDOutputElement
{
public:
  VSDInsertTextOutputElement(const RVNGString &text);
  virtual ~VSDInsertTextOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDInsertTextOutputElement(m_text);
  }
private:
  RVNGString m_text;
};


class VSDCloseSpanOutputElement : public VSDOutputElement
{
public:
  VSDCloseSpanOutputElement();
  virtual ~VSDCloseSpanOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDCloseSpanOutputElement();
  }
};


class VSDCloseParagraphOutputElement : public VSDOutputElement
{
public:
  VSDCloseParagraphOutputElement();
  virtual ~VSDCloseParagraphOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDCloseParagraphOutputElement();
  }
};


class VSDEndTextObjectOutputElement : public VSDOutputElement
{
public:
  VSDEndTextObjectOutputElement();
  virtual ~VSDEndTextObjectOutputElement() {}
  virtual void draw(RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDEndTextObjectOutputElement();
  }
};

} // namespace libvisio

libvisio::VSDStyleOutputElement::VSDStyleOutputElement(const RVNGPropertyList &propList, const RVNGPropertyListVector &propListVec) :
  m_propList(propList), m_propListVec(propListVec) {}

void libvisio::VSDStyleOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->setStyle(m_propList, m_propListVec);
}


libvisio::VSDPathOutputElement::VSDPathOutputElement(const RVNGPropertyListVector &propListVec) :
  m_propListVec(propListVec) {}

void libvisio::VSDPathOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->drawPath(m_propListVec);
}


libvisio::VSDGraphicObjectOutputElement::VSDGraphicObjectOutputElement(const RVNGPropertyList &propList, const ::RVNGBinaryData &binaryData) :
  m_propList(propList), m_binaryData(binaryData) {}

void libvisio::VSDGraphicObjectOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->drawGraphicObject(m_propList, m_binaryData);
}


libvisio::VSDStartTextObjectOutputElement::VSDStartTextObjectOutputElement(const RVNGPropertyList &propList, const RVNGPropertyListVector &propListVec) :
  m_propList(propList), m_propListVec(propListVec) {}

void libvisio::VSDStartTextObjectOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->startTextObject(m_propList, m_propListVec);
}

libvisio::VSDOpenSpanOutputElement::VSDOpenSpanOutputElement(const RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDOpenSpanOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->openSpan(m_propList);
}


libvisio::VSDStartLayerOutputElement::VSDStartLayerOutputElement(const RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDStartLayerOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->startLayer(m_propList);
}


libvisio::VSDEndLayerOutputElement::VSDEndLayerOutputElement() {}

void libvisio::VSDEndLayerOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->endLayer();
}


libvisio::VSDOpenParagraphOutputElement::VSDOpenParagraphOutputElement(const RVNGPropertyList &propList, const RVNGPropertyListVector &propListVec) :
  m_propList(propList), m_propListVec(propListVec) {}

void libvisio::VSDOpenParagraphOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->openParagraph(m_propList, m_propListVec);
}


libvisio::VSDInsertTextOutputElement::VSDInsertTextOutputElement(const RVNGString &text) :
  m_text(text) {}

void libvisio::VSDInsertTextOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->insertText(m_text);
}

libvisio::VSDCloseSpanOutputElement::VSDCloseSpanOutputElement() {}

void libvisio::VSDCloseSpanOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->closeSpan();
}


libvisio::VSDCloseParagraphOutputElement::VSDCloseParagraphOutputElement() {}

void libvisio::VSDCloseParagraphOutputElement::draw(RVNGDrawingInterface *painter)
{
  if (painter)
    painter->closeParagraph();
}


libvisio::VSDEndTextObjectOutputElement::VSDEndTextObjectOutputElement() {}

void libvisio::VSDEndTextObjectOutputElement::draw(RVNGDrawingInterface *painter)
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

void libvisio::VSDOutputElementList::draw(RVNGDrawingInterface *painter) const
{
  for (std::vector<VSDOutputElement *>::const_iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    (*iter)->draw(painter);
}

void libvisio::VSDOutputElementList::addStyle(const RVNGPropertyList &propList, const RVNGPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDStyleOutputElement(propList, propListVec));
}

void libvisio::VSDOutputElementList::addPath(const RVNGPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDPathOutputElement(propListVec));
}

void libvisio::VSDOutputElementList::addGraphicObject(const RVNGPropertyList &propList, const ::RVNGBinaryData &binaryData)
{
  m_elements.push_back(new VSDGraphicObjectOutputElement(propList, binaryData));
}

void libvisio::VSDOutputElementList::addStartTextObject(const RVNGPropertyList &propList, const RVNGPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDStartTextObjectOutputElement(propList, propListVec));
}

void libvisio::VSDOutputElementList::addOpenParagraph(const RVNGPropertyList &propList, const RVNGPropertyListVector &propListVec)
{
  m_elements.push_back(new VSDOpenParagraphOutputElement(propList, propListVec));
}

void libvisio::VSDOutputElementList::addOpenSpan(const RVNGPropertyList &propList)
{
  m_elements.push_back(new VSDOpenSpanOutputElement(propList));
}

void libvisio::VSDOutputElementList::addInsertText(const RVNGString &text)
{
  m_elements.push_back(new VSDInsertTextOutputElement(text));
}

void libvisio::VSDOutputElementList::addCloseSpan()
{
  m_elements.push_back(new VSDCloseSpanOutputElement());
}

void libvisio::VSDOutputElementList::addCloseParagraph()
{
  m_elements.push_back(new VSDCloseParagraphOutputElement());
}

void libvisio::VSDOutputElementList::addEndTextObject()
{
  m_elements.push_back(new VSDEndTextObjectOutputElement());
}

void libvisio::VSDOutputElementList::addStartLayer(const RVNGPropertyList &propList)
{
  m_elements.push_back(new VSDStartLayerOutputElement(propList));
}

void libvisio::VSDOutputElementList::addEndLayer()
{
  m_elements.push_back(new VSDEndLayerOutputElement());
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
