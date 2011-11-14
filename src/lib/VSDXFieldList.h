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

#ifndef __VSDXFIELDLIST_H__
#define __VSDXFIELDLIST_H__

#include <vector>
#include <map>
#include <time.h>
#include <libwpd/libwpd.h>
#include "VSDXDocumentStructure.h"
#include "VSDXTypes.h"

namespace libvisio
{

class VSDXCollector;

class VSDXFieldListElement
{
public:
  VSDXFieldListElement() {}
  virtual ~VSDXFieldListElement() {}
  virtual void handle(VSDXCollector *collector) = 0;
  virtual VSDXFieldListElement *clone() = 0;
  virtual WPXString getString(const std::vector<WPXString>&)
  {
    return WPXString();
  };
  virtual void setNameId(int) = 0;
  virtual void setFormat(int) = 0;
  virtual void setValue(double) = 0;
};

class VSDXTextField : public VSDXFieldListElement
{
public:
  VSDXTextField(unsigned id, unsigned level, int nameId)
    : m_id(id),
      m_level(level),
      m_nameId(nameId) {}
  ~VSDXTextField() {}
  void handle(VSDXCollector *collector);
  VSDXFieldListElement *clone();
  WPXString getString(const std::vector<WPXString> &strVec)
  {
    if (m_nameId < 0 || (unsigned)m_nameId >= strVec.size())
      return WPXString();
    else
      return strVec[m_nameId];
  }
  void setNameId(int nameId)
  {
    m_nameId = nameId;
  }
  void setFormat(int format) {}
  void setValue(double) {}
private:
  unsigned m_id, m_level;
  int m_nameId;
};

#define MAX_BUFFER 1024

class VSDXNumericField : public VSDXFieldListElement
{
public:
  VSDXNumericField(unsigned id, unsigned level, unsigned short format, double number)
    : m_id(id),
      m_level(level),
      m_format(format),
      m_number(number) {}
  ~VSDXNumericField() {}
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
        WPXString result;
        WPXProperty *pProp = WPXPropertyFactory::newDoubleProp(m_number);
        if (pProp)
        {
          result = pProp->getStr();
          delete pProp;
        }
        return result;
      }
    }
  }

  void setNameId(int) {}
  void setFormat(int format)
  {
    m_format = format;
  }
  void setValue(double number)
  {
    m_number = number;
  }
private:
  unsigned m_id, m_level;
  unsigned short m_format;
  double m_number;
};

class VSDXDatetimeField : public VSDXFieldListElement
{
public:
  VSDXDatetimeField(unsigned id, unsigned level, unsigned format, double timeValue)
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
  void setNameId(int) {}
  void setFormat(int format)
  {
    m_format = format;
  }
  void setValue(double number)
  {
    m_timeValue = number;
  }
private:
  unsigned m_id, m_level;
  int m_format;
  double m_timeValue;
};

class VSDXFieldList
{
public:
  VSDXFieldList();
  VSDXFieldList(const VSDXFieldList &fieldList);
  ~VSDXFieldList();
  VSDXFieldList &operator=(const VSDXFieldList &fieldList);
  void setElementsOrder(const std::vector<unsigned> &m_elementsOrder);
  void addFieldList(unsigned id, unsigned level);
  void addTextField(unsigned id, unsigned level, int nameId);
  void addNumericField(unsigned id, unsigned level, unsigned short format, double number);
  void addDatetimeField(unsigned id, unsigned level, int format, double timeValue);
  void addClonedField(unsigned id);
  void handle(VSDXCollector *collector);
  void clear();
  unsigned long size() const
  {
    return (unsigned long)m_elements.size();
  }
  bool empty() const
  {
    return (!m_elements.size());
  }
  VSDXFieldListElement *getElement(unsigned index);
private:
  std::map<unsigned, VSDXFieldListElement *> m_elements;
  std::vector<unsigned> m_elementsOrder;
  unsigned m_id, m_level;
};

} // namespace libvisio

#endif // __VSDXFIELDLIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
