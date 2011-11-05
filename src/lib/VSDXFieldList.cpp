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

#include "VSDXCollector.h"
#include "VSDXFieldList.h"

namespace libvisio
{

class VSDXFieldListElement
{
public:
  VSDXFieldListElement() {}
  virtual ~VSDXFieldListElement() {}
  virtual void handle(VSDXCollector *collector) = 0;
  virtual VSDXFieldListElement *clone() = 0;
};

class VSDXTextField : public VSDXFieldListElement
{
public:
  VSDXTextField(unsigned id, unsigned level) : m_id(id), m_level(level) {}
  ~VSDXTextField() {}
  void handle(VSDXCollector *collector);
  VSDXFieldListElement *clone();
private:
  unsigned m_id, m_level;
};

class VSDXNumericField : public VSDXFieldListElement
{
public:
  VSDXNumericField(unsigned id, unsigned level) : m_id(id), m_level(level) {}
  ~VSDXNumericField() {}
  void handle(VSDXCollector *collector);
  VSDXFieldListElement *clone();
private:
  unsigned m_id, m_level;
};

class VSDXGenericField : public VSDXFieldListElement
{
public:
  VSDXGenericField(unsigned id, unsigned level) : m_id(id), m_level(level) {}
  ~VSDXGenericField() {}
  void handle(VSDXCollector *collector);
  VSDXFieldListElement *clone();
private:
  unsigned m_id, m_level;
};

} // namespace libvisio



void libvisio::VSDXTextField::handle(VSDXCollector *collector)
{
}

libvisio::VSDXFieldListElement *libvisio::VSDXTextField::clone()
{
  return new VSDXTextField(m_id, m_level);
}


void libvisio::VSDXNumericField::handle(VSDXCollector *collector)
{
}

libvisio::VSDXFieldListElement *libvisio::VSDXNumericField::clone()
{
  return new VSDXTextField(m_id, m_level);
}


void libvisio::VSDXGenericField::handle(VSDXCollector *collector)
{
}

libvisio::VSDXFieldListElement *libvisio::VSDXGenericField::clone()
{
  return new VSDXGenericField(m_id, m_level);
}


libvisio::VSDXFieldList::VSDXFieldList() :
  m_elements(),
  m_elementsOrder()
{
}

libvisio::VSDXFieldList::VSDXFieldList(const libvisio::VSDXFieldList &fieldList) :
  m_elements(),
  m_elementsOrder(fieldList.m_elementsOrder)
{
  std::map<unsigned, VSDXFieldListElement *>::const_iterator iter = fieldList.m_elements.begin();
  for (; iter != fieldList.m_elements.end(); iter++)
    m_elements[iter->first] = iter->second->clone();
}

libvisio::VSDXFieldList &libvisio::VSDXFieldList::operator=(const libvisio::VSDXFieldList &fieldList)
{
  clear();
  std::map<unsigned, VSDXFieldListElement *>::const_iterator iter = fieldList.m_elements.begin();
  for (; iter != fieldList.m_elements.end(); iter++)
    m_elements[iter->first] = iter->second->clone();
  m_elementsOrder = fieldList.m_elementsOrder;
  return *this;
}

libvisio::VSDXFieldList::~VSDXFieldList()
{
  clear();
}

void libvisio::VSDXFieldList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDXFieldList::handle(VSDXCollector *collector)
{
  if (empty())
    return;
  std::map<unsigned, VSDXFieldListElement *>::iterator iter;
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

void libvisio::VSDXFieldList::clear()
{
  for (std::map<unsigned, VSDXFieldListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}

libvisio::VSDXFieldListElement *libvisio::VSDXFieldList::getElement(unsigned index)
{
  std::map<unsigned, VSDXFieldListElement *>::iterator iter = m_elements.find(index);
  if (iter != m_elements.end())
    return iter->second;
  else
    return 0;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
