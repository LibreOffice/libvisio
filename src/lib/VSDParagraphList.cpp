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

#include "VSDCollector.h"
#include "VSDParagraphList.h"

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
            const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
            const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
            const boost::optional<unsigned> &flags) : VSDParagraphListElement(id, level),
    m_style(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align, flags) {}
  ~VSDParaIX() {}
  void handle(VSDCollector *collector) const;
  VSDParagraphListElement *clone();
  unsigned getCharCount() const
  {
    return m_style.charCount;
  }
  void setCharCount(unsigned charCount)
  {
    m_style.charCount = charCount;
  }

  VSDOptionalParaStyle m_style;
};
} // namespace libvisio


void libvisio::VSDParaIX::handle(VSDCollector *collector) const
{
  collector->collectParaIX(m_id, m_level, m_style.charCount, m_style.indFirst, m_style.indLeft, m_style.indRight,
                           m_style.spLine, m_style.spBefore, m_style.spAfter, m_style.align, m_style.flags);
}

libvisio::VSDParagraphListElement *libvisio::VSDParaIX::clone()
{
  return new VSDParaIX(m_id, m_level, m_style.charCount, m_style.indFirst, m_style.indLeft, m_style.indRight,
                       m_style.spLine, m_style.spBefore, m_style.spAfter, m_style.align, m_style.flags);
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
  std::map<unsigned, VSDParagraphListElement *>::const_iterator iter = paraList.m_elements.begin();
  for (; iter != paraList.m_elements.end(); ++iter)
    m_elements[iter->first] = iter->second->clone();
}

libvisio::VSDParagraphList &libvisio::VSDParagraphList::operator=(const libvisio::VSDParagraphList &paraList)
{
  clear();
  std::map<unsigned, VSDParagraphListElement *>::const_iterator iter = paraList.m_elements.begin();
  for (; iter != paraList.m_elements.end(); ++iter)
    m_elements[iter->first] = iter->second->clone();
  m_elementsOrder = paraList.m_elementsOrder;
  return *this;
}

libvisio::VSDParagraphList::~VSDParagraphList()
{
  clear();
}

void libvisio::VSDParagraphList::addParaIX(unsigned id, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                                           const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                                           const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                                           const boost::optional<unsigned> &flags)
{
  VSDParaIX *tmpElement = dynamic_cast<VSDParaIX *>(m_elements[id]);
  if (!tmpElement)
  {
    std::map<unsigned, VSDParagraphListElement *>::iterator iter = m_elements.find(id);
    if (m_elements.end() != iter)
    {
      if (iter->second)
        delete iter->second;
      m_elements.erase(iter);
    }

    m_elements[id] = new VSDParaIX(id, level, charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align, flags);
  }
  else
    tmpElement->m_style.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align, flags));
}

void libvisio::VSDParagraphList::addParaIX(unsigned id, unsigned level, const VSDOptionalParaStyle &style)
{
  addParaIX(id, level, style.charCount, style.indFirst, style.indLeft, style.indRight, style.spLine,
            style.spBefore, style.spAfter, style.align, style.flags);
}

unsigned libvisio::VSDParagraphList::getCharCount(unsigned id) const
{
  std::map<unsigned, VSDParagraphListElement *>::const_iterator iter = m_elements.find(id);
  if (iter != m_elements.end() && iter->second)
    return iter->second->getCharCount();
  else
    return MINUS_ONE;
}

void libvisio::VSDParagraphList::setCharCount(unsigned id, unsigned charCount)
{
  std::map<unsigned, VSDParagraphListElement *>::iterator iter = m_elements.find(id);
  if (iter != m_elements.end() && iter->second)
    iter->second->setCharCount(charCount);
}

void libvisio::VSDParagraphList::resetCharCount()
{
  for (std::map<unsigned, VSDParagraphListElement *>::iterator iter = m_elements.begin();
       iter != m_elements.end(); ++iter)
    iter->second->setCharCount(0);
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
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDParagraphList::handle(VSDCollector *collector) const
{
  if (empty())
    return;
  std::map<unsigned, VSDParagraphListElement *>::const_iterator iter;
  if (!m_elementsOrder.empty())
  {
    for (unsigned i = 0; i < m_elementsOrder.size(); i++)
    {
      iter = m_elements.find(m_elementsOrder[i]);
      if (iter != m_elements.end() && (0 == i || iter->second->getCharCount()))
        iter->second->handle(collector);
    }
  }
  else
  {
    for (iter = m_elements.begin(); iter != m_elements.end(); ++iter)
      if (m_elements.begin() == iter || iter->second->getCharCount())
        iter->second->handle(collector);
  }
}

void libvisio::VSDParagraphList::clear()
{
  for (std::map<unsigned, VSDParagraphListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
