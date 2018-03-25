/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDOutputElementList.h"

#include "libvisio_utils.h"

namespace libvisio
{

namespace
{

static void separateSpacesAndInsertText(librevenge::RVNGDrawingInterface *iface, const librevenge::RVNGString &text)
{
  if (!iface)
    return;
  if (text.empty())
  {
    iface->insertText(text);
    return;
  }
  librevenge::RVNGString tmpText;
  int numConsecutiveSpaces = 0;
  librevenge::RVNGString::Iter i(text);
  for (i.rewind(); i.next();)
  {
    if (*(i()) == ' ')
      numConsecutiveSpaces++;
    else
      numConsecutiveSpaces = 0;

    if (numConsecutiveSpaces > 1)
    {
      if (!tmpText.empty())
      {
        iface->insertText(tmpText);
        tmpText.clear();
      }

      if (iface)
        iface->insertSpace();
    }
    else
    {
      tmpText.append(i());
    }
  }
  iface->insertText(tmpText);
}

} // anonymous namespace

class VSDOutputElement
{
public:
  VSDOutputElement() {}
  virtual ~VSDOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter) = 0;
  virtual VSDOutputElement *clone() = 0;
};


class VSDStyleOutputElement : public VSDOutputElement
{
public:
  VSDStyleOutputElement(const librevenge::RVNGPropertyList &propList);
  ~VSDStyleOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDStyleOutputElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDPathOutputElement : public VSDOutputElement
{
public:
  VSDPathOutputElement(const librevenge::RVNGPropertyList &propList);
  ~VSDPathOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDPathOutputElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDGraphicObjectOutputElement : public VSDOutputElement
{
public:
  VSDGraphicObjectOutputElement(const librevenge::RVNGPropertyList &propList);
  ~VSDGraphicObjectOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDGraphicObjectOutputElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDStartTextObjectOutputElement : public VSDOutputElement
{
public:
  VSDStartTextObjectOutputElement(const librevenge::RVNGPropertyList &propList);
  ~VSDStartTextObjectOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDStartTextObjectOutputElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDOpenParagraphOutputElement : public VSDOutputElement
{
public:
  VSDOpenParagraphOutputElement(const librevenge::RVNGPropertyList &propList);
  ~VSDOpenParagraphOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDOpenParagraphOutputElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDStartLayerOutputElement : public VSDOutputElement
{
public:
  VSDStartLayerOutputElement(const librevenge::RVNGPropertyList &propList);
  ~VSDStartLayerOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDStartLayerOutputElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDEndLayerOutputElement : public VSDOutputElement
{
public:
  VSDEndLayerOutputElement();
  ~VSDEndLayerOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDEndLayerOutputElement();
  }
};


class VSDOpenSpanOutputElement : public VSDOutputElement
{
public:
  VSDOpenSpanOutputElement(const librevenge::RVNGPropertyList &propList);
  ~VSDOpenSpanOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDOpenSpanOutputElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDInsertTextOutputElement : public VSDOutputElement
{
public:
  VSDInsertTextOutputElement(const librevenge::RVNGString &text);
  ~VSDInsertTextOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDInsertTextOutputElement(m_text);
  }
private:
  librevenge::RVNGString m_text;
};


class VSDInsertLineBreakOutputElement : public VSDOutputElement
{
public:
  VSDInsertLineBreakOutputElement();
  ~VSDInsertLineBreakOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDInsertLineBreakOutputElement();
  }
};


class VSDInsertTabOutputElement : public VSDOutputElement
{
public:
  VSDInsertTabOutputElement();
  ~VSDInsertTabOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDInsertTabOutputElement();
  }
};


class VSDCloseSpanOutputElement : public VSDOutputElement
{
public:
  VSDCloseSpanOutputElement();
  ~VSDCloseSpanOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDCloseSpanOutputElement();
  }
};


class VSDCloseParagraphOutputElement : public VSDOutputElement
{
public:
  VSDCloseParagraphOutputElement();
  ~VSDCloseParagraphOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDCloseParagraphOutputElement();
  }
};


class VSDEndTextObjectOutputElement : public VSDOutputElement
{
public:
  VSDEndTextObjectOutputElement();
  ~VSDEndTextObjectOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDEndTextObjectOutputElement();
  }
};

class VSDOpenListElementOutputElement : public VSDOutputElement
{
public:
  VSDOpenListElementOutputElement(const librevenge::RVNGPropertyList &propList);
  ~VSDOpenListElementOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDOpenListElementOutputElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDCloseListElementOutputElement : public VSDOutputElement
{
public:
  VSDCloseListElementOutputElement();
  ~VSDCloseListElementOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDCloseListElementOutputElement();
  }
};


class VSDOpenUnorderedListLevelOutputElement : public VSDOutputElement
{
public:
  VSDOpenUnorderedListLevelOutputElement(const librevenge::RVNGPropertyList &propList);
  ~VSDOpenUnorderedListLevelOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDOpenUnorderedListLevelOutputElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDCloseUnorderedListLevelOutputElement : public VSDOutputElement
{
public:
  VSDCloseUnorderedListLevelOutputElement();
  ~VSDCloseUnorderedListLevelOutputElement() override {}
  void draw(librevenge::RVNGDrawingInterface *painter) override;
  VSDOutputElement *clone() override
  {
    return new VSDCloseUnorderedListLevelOutputElement();
  }
};


} // namespace libvisio

libvisio::VSDStyleOutputElement::VSDStyleOutputElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDStyleOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->setStyle(m_propList);
}


libvisio::VSDPathOutputElement::VSDPathOutputElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDPathOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->drawPath(m_propList);
}


libvisio::VSDGraphicObjectOutputElement::VSDGraphicObjectOutputElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDGraphicObjectOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->drawGraphicObject(m_propList);
}


libvisio::VSDStartTextObjectOutputElement::VSDStartTextObjectOutputElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDStartTextObjectOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->startTextObject(m_propList);
}

libvisio::VSDOpenSpanOutputElement::VSDOpenSpanOutputElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDOpenSpanOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->openSpan(m_propList);
}


libvisio::VSDStartLayerOutputElement::VSDStartLayerOutputElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDStartLayerOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->startLayer(m_propList);
}


libvisio::VSDEndLayerOutputElement::VSDEndLayerOutputElement() {}

void libvisio::VSDEndLayerOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->endLayer();
}


libvisio::VSDOpenParagraphOutputElement::VSDOpenParagraphOutputElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDOpenParagraphOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->openParagraph(m_propList);
}


libvisio::VSDInsertTextOutputElement::VSDInsertTextOutputElement(const librevenge::RVNGString &text) :
  m_text(text) {}

void libvisio::VSDInsertTextOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    separateSpacesAndInsertText(painter, m_text);
}

libvisio::VSDInsertLineBreakOutputElement::VSDInsertLineBreakOutputElement() {}

void libvisio::VSDInsertLineBreakOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->insertLineBreak();
}

libvisio::VSDInsertTabOutputElement::VSDInsertTabOutputElement() {}

void libvisio::VSDInsertTabOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->insertTab();
}

libvisio::VSDCloseSpanOutputElement::VSDCloseSpanOutputElement() {}

void libvisio::VSDCloseSpanOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->closeSpan();
}


libvisio::VSDCloseParagraphOutputElement::VSDCloseParagraphOutputElement() {}

void libvisio::VSDCloseParagraphOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->closeParagraph();
}


libvisio::VSDEndTextObjectOutputElement::VSDEndTextObjectOutputElement() {}

void libvisio::VSDEndTextObjectOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->endTextObject();
}


libvisio::VSDOpenListElementOutputElement::VSDOpenListElementOutputElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDOpenListElementOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->openListElement(m_propList);
}


libvisio::VSDCloseListElementOutputElement::VSDCloseListElementOutputElement() {}

void libvisio::VSDCloseListElementOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->closeListElement();
}


libvisio::VSDOpenUnorderedListLevelOutputElement::VSDOpenUnorderedListLevelOutputElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libvisio::VSDOpenUnorderedListLevelOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->openUnorderedListLevel(m_propList);
}


libvisio::VSDCloseUnorderedListLevelOutputElement::VSDCloseUnorderedListLevelOutputElement() {}

void libvisio::VSDCloseUnorderedListLevelOutputElement::draw(librevenge::RVNGDrawingInterface *painter)
{
  if (painter)
    painter->closeUnorderedListLevel();
}


libvisio::VSDOutputElementList::VSDOutputElementList()
  : m_elements()
{
}

libvisio::VSDOutputElementList::VSDOutputElementList(const libvisio::VSDOutputElementList &elementList)
  : m_elements()
{
  for (const auto &elem : elementList.m_elements)
    m_elements.push_back(clone(elem));
}

libvisio::VSDOutputElementList &libvisio::VSDOutputElementList::operator=(const libvisio::VSDOutputElementList &elementList)
{
  if (&elementList != this)
  {
    m_elements.clear();

    for (const auto &elem : elementList.m_elements)
      m_elements.push_back(clone(elem));
  }

  return *this;
}

void libvisio::VSDOutputElementList::append(const libvisio::VSDOutputElementList &elementList)
{
  for (const auto &elem : elementList.m_elements)
    m_elements.push_back(clone(elem));
}

libvisio::VSDOutputElementList::~VSDOutputElementList()
{
}

void libvisio::VSDOutputElementList::draw(librevenge::RVNGDrawingInterface *painter) const
{
  for (const auto &elem : m_elements)
    elem->draw(painter);
}

void libvisio::VSDOutputElementList::addStyle(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(make_unique<VSDStyleOutputElement>(propList));
}

void libvisio::VSDOutputElementList::addPath(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(make_unique<VSDPathOutputElement>(propList));
}

void libvisio::VSDOutputElementList::addGraphicObject(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(make_unique<VSDGraphicObjectOutputElement>(propList));
}

void libvisio::VSDOutputElementList::addStartTextObject(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(make_unique<VSDStartTextObjectOutputElement>(propList));
}

void libvisio::VSDOutputElementList::addOpenParagraph(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(make_unique<VSDOpenParagraphOutputElement>(propList));
}

void libvisio::VSDOutputElementList::addOpenSpan(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(make_unique<VSDOpenSpanOutputElement>(propList));
}

void libvisio::VSDOutputElementList::addInsertText(const librevenge::RVNGString &text)
{
  m_elements.push_back(make_unique<VSDInsertTextOutputElement>(text));
}

void libvisio::VSDOutputElementList::addInsertLineBreak()
{
  m_elements.push_back(make_unique<VSDInsertLineBreakOutputElement>());
}

void libvisio::VSDOutputElementList::addInsertTab()
{
  m_elements.push_back(make_unique<VSDInsertTabOutputElement>());
}

void libvisio::VSDOutputElementList::addCloseSpan()
{
  m_elements.push_back(make_unique<VSDCloseSpanOutputElement>());
}

void libvisio::VSDOutputElementList::addCloseParagraph()
{
  m_elements.push_back(make_unique<VSDCloseParagraphOutputElement>());
}

void libvisio::VSDOutputElementList::addEndTextObject()
{
  m_elements.push_back(make_unique<VSDEndTextObjectOutputElement>());
}

void libvisio::VSDOutputElementList::addStartLayer(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(make_unique<VSDStartLayerOutputElement>(propList));
}

void libvisio::VSDOutputElementList::addEndLayer()
{
  m_elements.push_back(make_unique<VSDEndLayerOutputElement>());
}

void libvisio::VSDOutputElementList::addOpenListElement(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(make_unique<VSDOpenListElementOutputElement>(propList));
}

void libvisio::VSDOutputElementList::addOpenUnorderedListLevel(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(make_unique<VSDOpenUnorderedListLevelOutputElement>(propList));
}

void libvisio::VSDOutputElementList::addCloseListElement()
{
  m_elements.push_back(make_unique<VSDCloseListElementOutputElement>());
}

void libvisio::VSDOutputElementList::addCloseUnorderedListLevel()
{
  m_elements.push_back(make_unique<VSDCloseUnorderedListLevelOutputElement>());
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
