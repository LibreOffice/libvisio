/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDOUTPUTELEMENTLIST_H__
#define __VSDOUTPUTELEMENTLIST_H__

#include <map>
#include <list>
#include <vector>
#include <librevenge/librevenge.h>

namespace libvisio
{

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
  virtual ~VSDStyleOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
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
  virtual ~VSDPathOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
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
  virtual ~VSDGraphicObjectOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
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
  virtual ~VSDStartTextObjectOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDStartTextObjectOutputElement(m_propList);
  }

  librevenge::RVNGPropertyList &GetPropertyList()
  {
    return m_propList;
  }

private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDOpenParagraphOutputElement : public VSDOutputElement
{
public:
  VSDOpenParagraphOutputElement(const librevenge::RVNGPropertyList &propList);
  virtual ~VSDOpenParagraphOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDOpenParagraphOutputElement(m_propList);
  }

  librevenge::RVNGPropertyList &GetPropertyList()
  {
    return m_propList;
  }
  const librevenge::RVNGPropertyList &GetPropertyList() const
  {
    return m_propList;
  }

private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDStartLayerOutputElement : public VSDOutputElement
{
public:
  VSDStartLayerOutputElement(const librevenge::RVNGPropertyList &propList);
  virtual ~VSDStartLayerOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
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
  virtual ~VSDEndLayerOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDEndLayerOutputElement();
  }
};


class VSDOpenSpanOutputElement : public VSDOutputElement
{
public:
  VSDOpenSpanOutputElement(const librevenge::RVNGPropertyList &propList);
  virtual ~VSDOpenSpanOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDOpenSpanOutputElement(m_propList);
  }

  librevenge::RVNGPropertyList &GetPropertyList()
  {
    return m_propList;
  }
  const librevenge::RVNGPropertyList &GetPropertyList() const
  {
    return m_propList;
  }

private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDInsertTextOutputElement : public VSDOutputElement
{
public:
  VSDInsertTextOutputElement(const librevenge::RVNGString &text);
  virtual ~VSDInsertTextOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDInsertTextOutputElement(m_text);
  }

  const librevenge::RVNGString &GetText() const
  {
    return m_text;
  }

private:
  librevenge::RVNGString m_text;
};


class VSDInsertLineBreakOutputElement : public VSDOutputElement
{
public:
  VSDInsertLineBreakOutputElement();
  virtual ~VSDInsertLineBreakOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDInsertLineBreakOutputElement();
  }
};


class VSDInsertTabOutputElement : public VSDOutputElement
{
public:
  VSDInsertTabOutputElement();
  virtual ~VSDInsertTabOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDInsertTabOutputElement();
  }
};


class VSDCloseSpanOutputElement : public VSDOutputElement
{
public:
  VSDCloseSpanOutputElement();
  virtual ~VSDCloseSpanOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
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
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
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
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDEndTextObjectOutputElement();
  }
};


class VSDOpenListElementOutputElement : public VSDOutputElement
{
public:
  VSDOpenListElementOutputElement(const librevenge::RVNGPropertyList &propList);
  virtual ~VSDOpenListElementOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDOpenListElementOutputElement(m_propList);
  }

  librevenge::RVNGPropertyList &GetPropertyList()
  {
    return m_propList;
  }
  const librevenge::RVNGPropertyList &GetPropertyList() const
  {
    return m_propList;
  }

private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDCloseListElementOutputElement : public VSDOutputElement
{
public:
  VSDCloseListElementOutputElement();
  virtual ~VSDCloseListElementOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDCloseListElementOutputElement();
  }

};


class VSDOpenUnorderedListLevelOutputElement : public VSDOutputElement
{
public:
  VSDOpenUnorderedListLevelOutputElement(const librevenge::RVNGPropertyList &propList);
  virtual ~VSDOpenUnorderedListLevelOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDOpenUnorderedListLevelOutputElement(m_propList);
  }

  librevenge::RVNGPropertyList &GetPropertyList()
  {
    return m_propList;
  }
  const librevenge::RVNGPropertyList &GetPropertyList() const
  {
    return m_propList;
  }

private:
  librevenge::RVNGPropertyList m_propList;
};


class VSDCloseUnorderedListLevelOutputElement : public VSDOutputElement
{
public:
  VSDCloseUnorderedListLevelOutputElement();
  virtual ~VSDCloseUnorderedListLevelOutputElement() {}
  virtual void draw(librevenge::RVNGDrawingInterface *painter);
  virtual VSDOutputElement *clone()
  {
    return new VSDCloseUnorderedListLevelOutputElement();
  }
};


class VSDOutputElementList
{
public:
  VSDOutputElementList();
  VSDOutputElementList(const VSDOutputElementList &elementList);
  VSDOutputElementList &operator=(const VSDOutputElementList &elementList);
  virtual ~VSDOutputElementList();
  void append(const VSDOutputElementList &elementList);
  void draw(librevenge::RVNGDrawingInterface *painter) const;
  void addStyle(const librevenge::RVNGPropertyList &propList);
  void addPath(const librevenge::RVNGPropertyList &propList);
  void addGraphicObject(const librevenge::RVNGPropertyList &propList);
  void addStartTextObject(const librevenge::RVNGPropertyList &propList);
  void addEndTextObject();
  void addOpenUnorderedListLevel(const librevenge::RVNGPropertyList &propList);
  void addCloseUnorderedListLevel();
  void addOpenListElement(const librevenge::RVNGPropertyList &propList);
  void addCloseListElement();
  void addOpenParagraph(const librevenge::RVNGPropertyList &propList);
  void addCloseParagraph();
  void addOpenSpan(const librevenge::RVNGPropertyList &propList);
  void addCloseSpan();
  void addInsertText(const librevenge::RVNGString &text);
  void addInsertLineBreak();
  void addInsertTab();
  void addStartLayer(const librevenge::RVNGPropertyList &propList);
  void addEndLayer();
  bool empty() const
  {
    return m_elements.empty();
  }
private:
  std::vector<VSDOutputElement *> m_elements;
};


} // namespace libvisio

#endif // __VSDOUTPUTELEMENTLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
