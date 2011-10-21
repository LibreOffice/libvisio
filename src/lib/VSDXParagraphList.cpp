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

#include "VSDXCollector.h"
#include "VSDXParagraphList.h"

namespace libvisio {

class VSDXParagraphListElement
{
public:
  VSDXParagraphListElement() {}
  virtual ~VSDXParagraphListElement() {}
  virtual void handle(VSDXCollector *collector) = 0;
  virtual VSDXParagraphListElement *clone() = 0;
};

class VSDXParaIX : public VSDXParagraphListElement
{
public:
  VSDXParaIX(unsigned id , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                 double spLine, double spBefore, double spAfter, unsigned char align) :
    m_id(id), m_level(level), m_charCount(charCount), m_indFirst(indFirst), m_indLeft(indLeft), m_indRight(indRight),
    m_spLine(spLine), m_spBefore(spBefore), m_spAfter(spAfter), m_align(align) {}
  ~VSDXParaIX() {} 
  void handle(VSDXCollector *collector);
  VSDXParagraphListElement *clone();
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
};
} // namespace libvisio


void libvisio::VSDXParaIX::handle(VSDXCollector *collector)
{
  collector->collectVSDXParaStyle(m_id, m_level, m_charCount, m_indFirst, m_indLeft, m_indRight,
                               m_spLine, m_spBefore, m_spAfter, m_align);
}

libvisio::VSDXParagraphListElement *libvisio::VSDXParaIX::clone()
{
  return new VSDXParaIX(m_id, m_level, m_charCount, m_indFirst, m_indLeft, m_indRight,
                        m_spLine, m_spBefore, m_spAfter, m_align);
}


libvisio::VSDXParagraphList::VSDXParagraphList() :
  m_elements(),
  m_elementsOrder()
{
}

libvisio::VSDXParagraphList::VSDXParagraphList(const libvisio::VSDXParagraphList &paraList) :
  m_elements(),
  m_elementsOrder(paraList.m_elementsOrder)
{
  std::map<unsigned, VSDXParagraphListElement *>::const_iterator iter = paraList.m_elements.begin();
  for (; iter != paraList.m_elements.end(); iter++)
      m_elements[iter->first] = iter->second->clone();
}

libvisio::VSDXParagraphList &libvisio::VSDXParagraphList::operator=(const libvisio::VSDXParagraphList &paraList)
{
  clear();
  std::map<unsigned, VSDXParagraphListElement *>::const_iterator iter = paraList.m_elements.begin();
  for (; iter != paraList.m_elements.end(); iter++)
      m_elements[iter->first] = iter->second->clone();
  m_elementsOrder = paraList.m_elementsOrder;
  return *this;
}

libvisio::VSDXParagraphList::~VSDXParagraphList()
{
  clear();
}

void libvisio::VSDXParagraphList::addParaIX(unsigned id, unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
                                            double spLine, double spBefore, double spAfter, unsigned char align)
{
  m_elements[id] = new VSDXParaIX(id, level, charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align);
}

void libvisio::VSDXParagraphList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDXParagraphList::handle(VSDXCollector *collector)
{
  if (empty())
    return;
  std::map<unsigned, VSDXParagraphListElement *>::iterator iter;
  if (m_elementsOrder.size())
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
    for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
      iter->second->handle(collector);
  }
}

void libvisio::VSDXParagraphList::clear()
{
  for (std::map<unsigned, VSDXParagraphListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}

libvisio::VSDXParagraphListElement *libvisio::VSDXParagraphList::getElement(unsigned index)
{
  std::map<unsigned, VSDXParagraphListElement *>::iterator iter = m_elements.find(index);
  if (iter != m_elements.end())
    return iter->second;
  else
    return 0;
}
