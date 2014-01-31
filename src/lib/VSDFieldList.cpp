/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <time.h>
#include "VSDCollector.h"
#include "VSDFieldList.h"

void libvisio::VSDTextField::handle(VSDCollector *collector) const
{
  collector->collectTextField(m_id, m_level, m_nameId, m_formatStringId);
}

libvisio::VSDFieldListElement *libvisio::VSDTextField::clone()
{
  return new VSDTextField(m_id, m_level, m_nameId, m_formatStringId);
}

librevenge::RVNGString libvisio::VSDTextField::getString(const std::map<unsigned, librevenge::RVNGString> &strVec)
{
  std::map<unsigned, librevenge::RVNGString>::const_iterator iter = strVec.find(m_nameId);
  if (iter != strVec.end())
    return iter->second;
  else
    return librevenge::RVNGString();
}

void libvisio::VSDTextField::setNameId(int nameId)
{
  m_nameId = nameId;
}


void libvisio::VSDNumericField::handle(VSDCollector *collector) const
{
  collector->collectNumericField(m_id, m_level, m_format, m_number, m_formatStringId);
}

libvisio::VSDFieldListElement *libvisio::VSDNumericField::clone()
{
  return new VSDNumericField(m_id, m_level, m_format, m_number, m_formatStringId);
}

#define MAX_BUFFER 1024

librevenge::RVNGString libvisio::VSDNumericField::datetimeToString(const char *format, double datetime)
{
  librevenge::RVNGString result;
  char buffer[MAX_BUFFER];
  time_t timer = (time_t)(86400 * datetime - 2209161600.0);
  const struct tm *const time = gmtime(&timer);
  if (time)
  {
    strftime(&buffer[0], MAX_BUFFER-1, format, time);
    result.append(&buffer[0]);
  }
  return result;
}

librevenge::RVNGString libvisio::VSDNumericField::getString(const std::map<unsigned, librevenge::RVNGString> &)
{
  if (m_format == 0xffff)
    return librevenge::RVNGString();
  switch (m_format)
  {
  case VSD_FIELD_FORMAT_DateMDYY:
  case VSD_FIELD_FORMAT_DateMMDDYY:
  case VSD_FIELD_FORMAT_DateMmmDYYYY:
  case VSD_FIELD_FORMAT_DateMmmmDYYYY:
  case VSD_FIELD_FORMAT_DateDMYY:
  case VSD_FIELD_FORMAT_DateDDMMYY:
  case VSD_FIELD_FORMAT_DateDMMMYYYY:
  case VSD_FIELD_FORMAT_DateDMMMMYYYY:
  case VSD_FIELD_FORMAT_Dateyyyymd:
  case VSD_FIELD_FORMAT_Dateyymmdd:
  case VSD_FIELD_FORMAT_DateTWNfYYYYMMDDD_C:
  case VSD_FIELD_FORMAT_DateTWNsYYYYMMDDD_C:
  case VSD_FIELD_FORMAT_DateTWNfyyyymmddww_C:
  case VSD_FIELD_FORMAT_DateTWNfyyyymmdd_C:
  case VSD_FIELD_FORMAT_Dategggemdww_J:
  case VSD_FIELD_FORMAT_Dateyyyymdww_J:
  case VSD_FIELD_FORMAT_Dategggemd_J:
  case VSD_FIELD_FORMAT_Dateyyyymd_J:
  case VSD_FIELD_FORMAT_DateYYYYMMMDDDWWW_C:
  case VSD_FIELD_FORMAT_DateYYYYMMMDDD_C:
  case VSD_FIELD_FORMAT_DategeMMMMddddww_K:
  case VSD_FIELD_FORMAT_Dateyyyymdww_K:
  case VSD_FIELD_FORMAT_DategeMMMMddd_K:
  case VSD_FIELD_FORMAT_Dateyyyymd_K:
  case VSD_FIELD_FORMAT_Dateyyyy_m_d:
  case VSD_FIELD_FORMAT_Dateyy_mm_dd:
  case VSD_FIELD_FORMAT_Dateyyyymd_S:
  case VSD_FIELD_FORMAT_Dateyyyymmdd_S:
  case VSD_FIELD_FORMAT_Datewwyyyymmdd_S:
  case VSD_FIELD_FORMAT_Datewwyyyymd_S:
  case VSD_FIELD_FORMAT_MsoDateShort:
  case VSD_FIELD_FORMAT_MsoDateLongDay:
  case VSD_FIELD_FORMAT_MsoDateLong:
  case VSD_FIELD_FORMAT_MsoDateShortAlt:
  case VSD_FIELD_FORMAT_MsoDateISO:
  case VSD_FIELD_FORMAT_MsoDateShortMon:
  case VSD_FIELD_FORMAT_MsoDateShortSlash:
  case VSD_FIELD_FORMAT_MsoDateShortAbb:
  case VSD_FIELD_FORMAT_MsoDateEnglish:
  case VSD_FIELD_FORMAT_MsoDateMonthYr:
  case VSD_FIELD_FORMAT_MsoDateMon_Yr:
    return datetimeToString("%x", m_number);
  case VSD_FIELD_FORMAT_TimeGen:
  case VSD_FIELD_FORMAT_TimeHMM:
  case VSD_FIELD_FORMAT_TimeHHMM:
  case VSD_FIELD_FORMAT_TimeHMM24:
  case VSD_FIELD_FORMAT_TimeHHMM24:
  case VSD_FIELD_FORMAT_TimeHMMAMPM:
  case VSD_FIELD_FORMAT_TimeHHMMAMPM:
  case VSD_FIELD_FORMAT_TimeAMPMhmm_J:
  case VSD_FIELD_FORMAT_TimeAMPMhmm_C:
  case VSD_FIELD_FORMAT_TimeAMPMhmm_K:
  case VSD_FIELD_FORMAT_TimeAMPM_hmm_J:
  case VSD_FIELD_FORMAT_Timehmm_J:
  case VSD_FIELD_FORMAT_TimeAMPM_hmm_C:
  case VSD_FIELD_FORMAT_Timehmm_C:
  case VSD_FIELD_FORMAT_TimeAMPM_hmm_K:
  case VSD_FIELD_FORMAT_Timehmm_K:
  case VSD_FIELD_FORMAT_TimeHMMAMPM_E:
  case VSD_FIELD_FORMAT_TimeHHMMAMPM_E:
  case VSD_FIELD_FORMAT_TimeAMPMhmm_S:
  case VSD_FIELD_FORMAT_TimeAMPMhhmm_S:
  case VSD_FIELD_FORMAT_MsoTimePM:
  case VSD_FIELD_FORMAT_MsoTimeSecPM:
  case VSD_FIELD_FORMAT_MsoTime24:
  case VSD_FIELD_FORMAT_MsoTimeSec24:
    return datetimeToString("%X", m_number);
  case VSD_FIELD_FORMAT_MsoTimeDatePM:
  case VSD_FIELD_FORMAT_MsoTimeDateSecPM:
    return datetimeToString("%x %X", m_number);
  default:
  {
    librevenge::RVNGString result;
    librevenge::RVNGProperty *pProp = librevenge::RVNGPropertyFactory::newDoubleProp(m_number);
    if (pProp)
    {
      result = pProp->getStr();
      delete pProp;
    }
    return result;
  }
  }
}

void libvisio::VSDNumericField::setFormat(unsigned short format)
{
  m_format = format;
}
void libvisio::VSDNumericField::setValue(double number)
{
  m_number = number;
}


libvisio::VSDFieldList::VSDFieldList() :
  m_elements(),
  m_elementsOrder(),
  m_id(0),
  m_level(0)
{
}

libvisio::VSDFieldList::VSDFieldList(const libvisio::VSDFieldList &fieldList) :
  m_elements(),
  m_elementsOrder(fieldList.m_elementsOrder),
  m_id(fieldList.m_id),
  m_level(fieldList.m_level)
{
  std::map<unsigned, VSDFieldListElement *>::const_iterator iter = fieldList.m_elements.begin();
  for (; iter != fieldList.m_elements.end(); ++iter)
    m_elements[iter->first] = iter->second->clone();
}

libvisio::VSDFieldList &libvisio::VSDFieldList::operator=(const libvisio::VSDFieldList &fieldList)
{
  if (this != &fieldList)
  {
    clear();
    std::map<unsigned, VSDFieldListElement *>::const_iterator iter = fieldList.m_elements.begin();
    for (; iter != fieldList.m_elements.end(); ++iter)
      m_elements[iter->first] = iter->second->clone();
    m_elementsOrder = fieldList.m_elementsOrder;
    m_id = fieldList.m_id;
    m_level = fieldList.m_level;
  }
  return *this;
}

libvisio::VSDFieldList::~VSDFieldList()
{
  clear();
}

void libvisio::VSDFieldList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned i = 0; i<elementsOrder.size(); i++)
    m_elementsOrder.push_back(elementsOrder[i]);
}

void libvisio::VSDFieldList::addFieldList(unsigned id, unsigned level)
{
  m_id = id;
  m_level = level;
}

void libvisio::VSDFieldList::addTextField(unsigned id, unsigned level, int nameId, int formatStringId)
{
  m_elements[id] = new VSDTextField(id, level, nameId, formatStringId);
}

void libvisio::VSDFieldList::addNumericField(unsigned id, unsigned level, unsigned short format, double number, int formatStringId)
{
  m_elements[id] = new VSDNumericField(id, level, format, number, formatStringId);
}

void libvisio::VSDFieldList::handle(VSDCollector *collector) const
{
  if (empty())
    return;

  collector->collectFieldList(m_id, m_level);
  std::map<unsigned, VSDFieldListElement *>::const_iterator iter;
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

void libvisio::VSDFieldList::clear()
{
  for (std::map<unsigned, VSDFieldListElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete iter->second;
  m_elements.clear();
  m_elementsOrder.clear();
}

libvisio::VSDFieldListElement *libvisio::VSDFieldList::getElement(unsigned index)
{
  if (m_elementsOrder.size() > index)
    index = m_elementsOrder[index];

  std::map<unsigned, VSDFieldListElement *>::const_iterator iter = m_elements.find(index);
  if (iter != m_elements.end())
    return iter->second;
  else
    return 0;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
