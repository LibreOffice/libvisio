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
  VSDParagraphListElement() {}
  virtual ~VSDParagraphListElement() {}
  virtual void handle(VSDCollector *collector) = 0;
  virtual VSDParagraphListElement *clone() = 0;
};

class VSDParaIX : public VSDParagraphListElement
{
public:
  VSDParaIX(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
            double spLine, double spBefore, double spAfter, unsigned char align, unsigned flags) :
    m_id(id), m_level(level), m_charCount(charCount), m_indFirst(indFirst), m_indLeft(indLeft), m_indRight(indRight),
    m_spLine(spLine), m_spBefore(spBefore), m_spAfter(spAfter), m_align(align), m_flags(flags) {}
  ~VSDParaIX() {}
  void handle(VSDCollector *collector);
  VSDParagraphListElement *clone();
private:
  unsigned m_id, m_level;
  unsigned m_charCount;
  double m_indFirst;
  double m_indLeft;
  double m_indRight;
  double m_spLine;
  double m_spBefore;
  double m_spAfter;
  unsigned char m_align;
  unsigned m_flags;
};
} // namespace libvisio


void libvisio::VSDParaIX::handle(VSDCollector *collector)
{
  collector->collectVSDParaStyle(m_id, m_level, m_charCount, m_indFirst, m_indLeft, m_indRight,
                                 m_spLine, m_spBefore, m_spAfter, m_align, m_flags);
}

libvisio::VSDParagraphListElement *libvisio::VSDParaIX::clone()
{
  return new VSDParaIX(m_id, m_level, m_charCount, m_indFirst, m_indLeft, m_indRight,
                       m_spLine, m_spBefore, m_spAfter, m_align, m_flags);
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

void libvisio::VSDParagraphList::addParaIX(unsigned id, unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
    double spLine, double spBefore, double spAfter, unsigned char align, unsigned flags)
{
  m_elements[id] = new VSDParaIX(id, level, charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align, flags);
}

void libvisio::VSDParagraphList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDParagraphList::handle(VSDCollector *collector)
{
  if (empty())
    return;
  std::map<unsigned, VSDParagraphListElement *>::iterator iter;
  if (!m_elementsOrder.empty())
  {
    for (unsigned i = 0; i < m_elementsOrder.size(); i++)
    {
      iter = m_elements.find(m_elementsOrder[i]);
      if (iter != m_elements.end())
        iter->second->handle(collector);
    }
  }
  else
  {
    for (iter = m_elements.begin(); iter != m_elements.end(); ++iter)
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
