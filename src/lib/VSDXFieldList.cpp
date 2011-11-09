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

#include <libwpd/libwpd.h>
#include "VSDXCollector.h"
#include "VSDXFieldList.h"

namespace libvisio
{

class VSDXTextField : public VSDXFieldListElement
{
public:
  VSDXTextField(unsigned id, unsigned level, unsigned nameId)
    : m_id(id),
      m_level(level),
      m_nameId(nameId) {}
  ~VSDXTextField() {}
  void handle(VSDXCollector *collector);
  VSDXFieldListElement *clone();
  WPXString getString(const std::vector<WPXString> &strVec)
  {
    if (m_nameId >= strVec.size())
      return WPXString();
    else
      return strVec[m_nameId];
  }
private:
  unsigned m_id, m_level, m_nameId;
};

class VSDXNumericField : public VSDXFieldListElement
{
public:
  VSDXNumericField(unsigned id, unsigned level, unsigned format, double number)
    : m_id(id),
      m_level(level),
      m_format(format),
      m_number(number) {}
  ~VSDXNumericField() {}
  void handle(VSDXCollector *collector);
  VSDXFieldListElement *clone();
  WPXString getString(const std::vector<WPXString> &)
  {
    WPXString result;
    WPXProperty *pProp = WPXPropertyFactory::newDoubleProp(m_number);
    if (pProp)
    {
      result = pProp->getStr();
      delete pProp;
    }
    return result;
  }
private:
  unsigned m_id, m_level, m_format;
  double m_number;
};

#define MAX_BUFFER 1024

class VSDXDatetimeField : public VSDXFieldListElement
{
public:
  VSDXDatetimeField(unsigned id, unsigned level, unsigned format, unsigned long timeValue)
    : m_id(id),
      m_level(level),
      m_format(format),
      m_timeValue(timeValue) {}
  ~VSDXDatetimeField() {}
  void handle(VSDXCollector *collector);
  VSDXFieldListElement *clone();
  static WPXString datetimeToString(const char *format, double datetime)
  {
    WPXString result;
    char buffer[MAX_BUFFER];
    time_t timer = (time_t)(86400 * datetime - 2209161600.0);
    strftime(&buffer[0], MAX_BUFFER-1, format, gmtime(&timer));
    result.append(&buffer[0]);
    return result;
  }
  WPXString getString(const std::vector<WPXString> &)
  {
    return datetimeToString("%x %X", m_timeValue);
  }
private:
  unsigned m_id, m_level, m_format;
  unsigned long m_timeValue;
};

class VSDXEmptyField : public VSDXFieldListElement
{
public:
  VSDXEmptyField(unsigned id, unsigned level)
    : m_id(id),
      m_level(level) {}
  ~VSDXEmptyField() {}
  void handle(VSDXCollector *collector);
  VSDXFieldListElement *clone();
private:
  unsigned m_id, m_level;
};

} // namespace libvisio



void libvisio::VSDXTextField::handle(VSDXCollector *collector)
{
  collector->collectTextField(m_id, m_level, m_nameId);
}

libvisio::VSDXFieldListElement *libvisio::VSDXTextField::clone()
{
  return new VSDXTextField(m_id, m_level, m_nameId);
}


void libvisio::VSDXNumericField::handle(VSDXCollector *collector)
{
  collector->collectNumericField(m_id, m_level, m_format, m_number);
}

libvisio::VSDXFieldListElement *libvisio::VSDXNumericField::clone()
{
  return new VSDXNumericField(m_id, m_level, m_format, m_number);
}


void libvisio::VSDXDatetimeField::handle(VSDXCollector *collector)
{
  collector->collectDatetimeField(m_id, m_level, m_format, m_timeValue);
}

libvisio::VSDXFieldListElement *libvisio::VSDXDatetimeField::clone()
{
  return new VSDXDatetimeField(m_id, m_level, m_format, m_timeValue);
}


void libvisio::VSDXEmptyField::handle(VSDXCollector *collector)
{
  collector->collectEmptyField(m_id, m_level);
}

libvisio::VSDXFieldListElement *libvisio::VSDXEmptyField::clone()
{
  return new VSDXEmptyField(m_id, m_level);
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

void libvisio::VSDXFieldList::toVector(std::vector<libvisio::VSDXFieldListElement *> &vec) const
{
  if (!vec.empty())
    vec.clear();
  if (empty())
    return;
  std::map<unsigned, VSDXFieldListElement *>::const_iterator iter;
  if (m_elementsOrder.size())
  {
    for (unsigned i = 0; i < m_elementsOrder.size(); i++)
    {
      iter = m_elements.find(m_elementsOrder[i]);
      if (iter != m_elements.end())
        vec.push_back(iter->second);
    }
  }
  else
  {
    for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
      vec.push_back(iter->second);
  }
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

void libvisio::VSDXFieldList::addTextField(unsigned id, unsigned level, unsigned nameId)
{
  m_elements[id] = new VSDXTextField(id, level, nameId);
}

void libvisio::VSDXFieldList::addNumericField(unsigned id, unsigned level, unsigned format, double number)
{
  m_elements[id] = new VSDXNumericField(id, level, format, number);
}

void libvisio::VSDXFieldList::addDatetimeField(unsigned id, unsigned level, unsigned format, unsigned long timeValue)
{
  m_elements[id] = new VSDXDatetimeField(id, level, format, timeValue);
}

void libvisio::VSDXFieldList::addEmptyField(unsigned id, unsigned level)
{
  m_elements[id] = new VSDXEmptyField(id, level);
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
