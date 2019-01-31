/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDParagraphList.h"

#include "VSDCollector.h"

namespace libvisio
{

class VSDParagraphListElement
{
public:
  VSDParagraphListElement(unsigned id, unsigned level) : m_id(id), m_level(level) {}
  virtual ~VSDParagraphListElement() {}
  virtual void handle(VSDCollector *collector) const = 0;
  virtual VSDParagraphListElement *clone() = 0;
  virtual unsigned getCharCount() const = 0;
  virtual void setCharCount(unsigned charCount) = 0;

  unsigned m_id, m_level;
};

class VSDParaIX : public VSDParagraphListElement
{
public:
  VSDParaIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
            const boost::optional<double> &indLeft, const boost::optional<double> &indRight,
            const boost::optional<double> &spLine, const boost::optional<double> &spBefore,
            const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
            const boost::optional<unsigned char> &bullet, const boost::optional<VSDName> &bulletStr,
            const boost::optional<VSDName> &bulletFont, const boost::optional<double> &bulletFontSize,
            const boost::optional<double> &textPosAfterBullet, const boost::optional<unsigned> &flags) :
    VSDParagraphListElement(id, level),
    m_style(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter,
            align, bullet, bulletStr, bulletFont, bulletFontSize, textPosAfterBullet, flags) {}
  ~VSDParaIX() override {}
  void handle(VSDCollector *collector) const override;
  VSDParagraphListElement *clone() override;
  unsigned getCharCount() const override
  {
    return m_style.charCount;
  }
  void setCharCount(unsigned charCount) override
  {
    m_style.charCount = charCount;
  }

  VSDOptionalParaStyle m_style;
};
} // namespace libvisio


void libvisio::VSDParaIX::handle(VSDCollector *collector) const
{
  collector->collectParaIX(m_id, m_level, m_style.charCount, m_style.indFirst, m_style.indLeft,
                           m_style.indRight, m_style.spLine, m_style.spBefore, m_style.spAfter,
                           m_style.align, m_style.bullet, m_style.bulletStr, m_style.bulletFont,
                           m_style.bulletFontSize, m_style.textPosAfterBullet, m_style.flags);
}

libvisio::VSDParagraphListElement *libvisio::VSDParaIX::clone()
{
  return new VSDParaIX(m_id, m_level, m_style.charCount, m_style.indFirst, m_style.indLeft,
                       m_style.indRight, m_style.spLine, m_style.spBefore, m_style.spAfter,
                       m_style.align, m_style.bullet, m_style.bulletStr, m_style.bulletFont,
                       m_style.bulletFontSize, m_style.textPosAfterBullet, m_style.flags);
}


libvisio::VSDParagraphList::VSDParagraphList() :
  m_elements(),
  m_elementsOrder()
{
}

libvisio::VSDParagraphList::VSDParagraphList(const libvisio::VSDParagraphList &paraList) :
  m_elements(),
  m_elementsOrder(paraList.m_elementsOrder)
{
  for (auto iter = paraList.m_elements.begin(); iter != paraList.m_elements.end(); ++iter)
    m_elements[iter->first] = clone(iter->second);
}

libvisio::VSDParagraphList &libvisio::VSDParagraphList::operator=(const libvisio::VSDParagraphList &paraList)
{
  if (this != &paraList)
  {
    clear();
    for (auto iter = paraList.m_elements.begin(); iter != paraList.m_elements.end(); ++iter)
      m_elements[iter->first] = clone(iter->second);
    m_elementsOrder = paraList.m_elementsOrder;
  }
  return *this;
}

libvisio::VSDParagraphList::~VSDParagraphList()
{
}

void libvisio::VSDParagraphList::addParaIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                                           const boost::optional<double> &indLeft, const boost::optional<double> &indRight,
                                           const boost::optional<double> &spLine, const boost::optional<double> &spBefore,
                                           const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                                           const boost::optional<unsigned char> &bullet, const boost::optional<VSDName> &bulletStr,
                                           const boost::optional<VSDName> &bulletFont, const boost::optional<double> &bulletFontSize,
                                           const boost::optional<double> &textPosAfterBullet, const boost::optional<unsigned> &flags)
{
  auto *tmpElement = dynamic_cast<VSDParaIX *>(m_elements[id].get());
  if (!tmpElement)
    m_elements[id] = make_unique<VSDParaIX>(id, level, charCount, indFirst, indLeft, indRight, spLine, spBefore,
                                            spAfter, align, bullet, bulletStr, bulletFont, bulletFontSize,
                                            textPosAfterBullet, flags);
  else
    tmpElement->m_style.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore,
                                                      spAfter, align, bullet, bulletStr, bulletFont, bulletFontSize,
                                                      textPosAfterBullet, flags));
}

void libvisio::VSDParagraphList::addParaIX(unsigned id, unsigned level, const VSDOptionalParaStyle &style)
{
  addParaIX(id, level, style.charCount, style.indFirst, style.indLeft, style.indRight,
            style.spLine, style.spBefore, style.spAfter, style.align,
            style.bullet, style.bulletStr, style.bulletFont, style.bulletFontSize,
            style.textPosAfterBullet, style.flags);
}

unsigned libvisio::VSDParagraphList::getCharCount(unsigned id) const
{
  auto iter = m_elements.find(id);
  if (iter != m_elements.end() && iter->second)
    return iter->second->getCharCount();
  else
    return MINUS_ONE;
}

void libvisio::VSDParagraphList::setCharCount(unsigned id, unsigned charCount)
{
  auto iter = m_elements.find(id);
  if (iter != m_elements.end() && iter->second)
    iter->second->setCharCount(charCount);
}

void libvisio::VSDParagraphList::resetCharCount()
{
  for (auto &element : m_elements)
    element.second->setCharCount(0);
}

unsigned libvisio::VSDParagraphList::getLevel() const
{
  if (m_elements.empty() || !m_elements.begin()->second)
    return 0;
  return m_elements.begin()->second->m_level;
}

void libvisio::VSDParagraphList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned int i : elementsOrder)
    m_elementsOrder.push_back(i);
}

void libvisio::VSDParagraphList::handle(VSDCollector *collector) const
{
  if (empty())
    return;
  if (!m_elementsOrder.empty())
  {
    for (size_t i = 0; i < m_elementsOrder.size(); i++)
    {
      auto iter = m_elements.find(m_elementsOrder[i]);
      if (iter != m_elements.end() && (0 == i || iter->second->getCharCount()))
        iter->second->handle(collector);
    }
  }
  else
  {
    for (auto iter = m_elements.begin(); iter != m_elements.end(); ++iter)
      if (m_elements.begin() == iter || iter->second->getCharCount())
        iter->second->handle(collector);
  }
}

void libvisio::VSDParagraphList::clear()
{
  m_elements.clear();
  m_elementsOrder.clear();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
