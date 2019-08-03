/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDFieldList.h"

#include <time.h>
#include <cmath>
#include "VSDCollector.h"
#include "libvisio_utils.h"

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
  //TODO VSD_FIELD_FORMAT_StrNormal  37
  //TODO VSD_FIELD_FORMAT_StrLower  38
  //TODO VSD_FIELD_FORMAT_StrUpper  39
  auto iter = strVec.find(m_nameId);
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
  collector->collectNumericField(m_id, m_level, m_format, m_cell_type, m_number, m_formatStringId);
}

libvisio::VSDFieldListElement *libvisio::VSDNumericField::clone()
{
  return new VSDNumericField(m_id, m_level, m_format, m_cell_type, m_number, m_formatStringId);
}

#define MAX_BUFFER 1024

librevenge::RVNGString libvisio::VSDNumericField::datetimeToString(const char *format, double datetime)
{
  librevenge::RVNGString result;
  char buffer[MAX_BUFFER];
  auto timer = (time_t)(86400 * datetime - 2209161600.0);
  const struct tm *const time = gmtime(&timer);
  if (time)
  {
    strftime(&buffer[0], MAX_BUFFER-1, format, time);
    result.append(&buffer[0]);
  }
  return result;
}

// This method is copied from:
// https://sourceforge.net/p/libwpd/librevenge/ci/master/tree/src/lib/RVNGProperty.cpp#l35
// to avoid ABI breakage. If upstream file was modified, please update method accordingly.
static librevenge::RVNGString doubleToString(const double value, const char *format, const char *postfix)
{
  librevenge::RVNGString tempString;
  if (value < 0.0001 && value > -0.0001)
    tempString.sprintf(format, 0.0, postfix);
  else
    tempString.sprintf(format, value, postfix);
#ifndef __ANDROID__
  std::string decimalPoint(localeconv()->decimal_point);
#else
  std::string decimalPoint(".");
#endif
  if ((decimalPoint.size() == 0) || (decimalPoint == "."))
    return tempString;
  std::string stringValue(tempString.cstr());
  if (!stringValue.empty())
  {
    std::string::size_type pos;
    while ((pos = stringValue.find(decimalPoint)) != std::string::npos)
      stringValue.replace(pos,decimalPoint.size(),".");
  }
  return librevenge::RVNGString(stringValue.c_str());
}

double convertNumber(const unsigned short cellType, const double number)
{
  switch (cellType)
  {
  case CELL_TYPE_Percent:
    return number*100.0;
  // All elapsed times are stored in days
  case CELL_TYPE_ElapsedDay:
    return number;
  case CELL_TYPE_ElapsedWeek:
    return number/7.0;
  case CELL_TYPE_ElapsedHour:
    return number*24.0;
  case CELL_TYPE_ElapsedMin:
    return number*24.0*60.0;
  case CELL_TYPE_ElapsedSec:
    return number*24.0*60.0*60.0;

  // All lenght values are stored in Inches
  case CELL_TYPE_Inches:
    return number;
  case CELL_TYPE_Points:
    return number*72.0;
  case CELL_TYPE_Picas:
    return number*6.0;
  case CELL_TYPE_Didots:
    return number*67.75;

  case CELL_TYPE_Ciceros:
    return number*5.644444444444;
  case CELL_TYPE_Feet:
    return number*0.0833333333;
  case CELL_TYPE_Centimeters:
    return number*2.54;
  case CELL_TYPE_Miles:
    return number/63360;
  case CELL_TYPE_Millimeters:
    return number*25.4;
  case CELL_TYPE_Meters:
    return number*0.0254;
  case CELL_TYPE_Kilometers:
    return number*0.0000254;
  case CELL_TYPE_Yards:
    return number*0.0277777778;
  case CELL_TYPE_NauticalMiles:
    return number/72913.386;
  // All angles values are stored in radians
  case CELL_TYPE_Radians:
    return number;
  case CELL_TYPE_Degrees:
    return number*57.2957795;
  default:
  {
    VSD_DEBUG_MSG(("TODO Add support for cell type %d\n", cellType));
    return number;
  }
  }
}

const char *getUnitString(const unsigned short cellType)
{
  switch (cellType)
  {
  case CELL_TYPE_Number:
  case CELL_TYPE_String:
  case CELL_TYPE_StringWithoutUnit:
  case CELL_TYPE_NoCast:
  case CELL_TYPE_Invalid:
    return "";
  case CELL_TYPE_Percent:
    return "%";
  case CELL_TYPE_Acre:
    return " acres";
  case CELL_TYPE_Hectare:
    return " ha";

  case CELL_TYPE_ElapsedWeek:
    return " ew.";
  case CELL_TYPE_ElapsedDay:
    return " ed.";
  case CELL_TYPE_ElapsedHour:
    return " eh.";
  case CELL_TYPE_ElapsedMin:
    return " em.";
  case CELL_TYPE_ElapsedSec:
    return " es.";;

  case CELL_TYPE_Points:
    return " pt";
  case CELL_TYPE_Picas:
    return " p";
  case CELL_TYPE_Didots:
    return " d";
  case CELL_TYPE_Ciceros:
    return " c";
  case CELL_TYPE_Inches:
    return " in";
  case CELL_TYPE_Feet:
    return " ft";
  case CELL_TYPE_Centimeters:
    return " cm";
  case CELL_TYPE_Miles:
    return " mi";
  case CELL_TYPE_Millimeters:
    return " mm";
  case CELL_TYPE_Meters:
    return " m";
  case CELL_TYPE_Kilometers:
    return " km";
  case CELL_TYPE_Yards:
    return " yd";
  case CELL_TYPE_NauticalMiles:
    return " nm.";

  case CELL_TYPE_Radians:
    return " rad";
  case CELL_TYPE_Degrees:
    return " deg";
  default:
    return "";
  }
}

librevenge::RVNGString libvisio::VSDNumericField::getString(const std::map<unsigned, librevenge::RVNGString> &)
{
  // Augmented BNF for Syntax Specifications: ABNF
  // http://www.rfc-editor.org/rfc/rfc5234.txt
  if (m_format == VSD_FIELD_FORMAT_Unknown)
    return librevenge::RVNGString();
  switch (m_format)
  {
  case VSD_FIELD_FORMAT_NumGenNoUnits:
  {
    // 0 Format string: 0.#### Example: 30060.9167
    return doubleToString(convertNumber(m_cell_type, m_number), "%.4g%s", "");
  }
  case VSD_FIELD_FORMAT_NumGenDefUnits:
  {
    // 1 Format string: 0.#### u Example: 30060.9167 cm
    return doubleToString(convertNumber(m_cell_type, m_number), "%.4g%s", getUnitString(m_cell_type));
  }
  case VSD_FIELD_FORMAT_0PlNoUnits:
  {
    // 2 Format string: 0 Example: 30061
    return doubleToString(convertNumber(m_cell_type, m_number), "%.0f%s", "");
  }
  case VSD_FIELD_FORMAT_0PlDefUnits:
  {
    // 3 Format string: 0 u Example: 30061 cm
    return doubleToString(convertNumber(m_cell_type, m_number), "%.0f%s", getUnitString(m_cell_type));
  }
  case VSD_FIELD_FORMAT_1PlNoUnits:
  {
    // 4 Format string: 0.0 Example: 30060.9
    return doubleToString(convertNumber(m_cell_type, m_number), "%.1f%s", "");
  }
  case VSD_FIELD_FORMAT_1PlDefUnits:
  {
    // 5 Format string: 0.0 u Example: 30060.9 cm
    return doubleToString(convertNumber(m_cell_type, m_number), "%.1f%s", getUnitString(m_cell_type));
  }
  case VSD_FIELD_FORMAT_2PlNoUnits:
  {
    // 6 Format string: 0.00 Example: 30061.92
    return doubleToString(convertNumber(m_cell_type, m_number), "%.2f%s", "");
  }
  case VSD_FIELD_FORMAT_2PlDefUnits:
  {
    // 7 Format string: 0.00 u Example: 30061.92 cm
    return doubleToString(convertNumber(m_cell_type, m_number), "%.2f%s", getUnitString(m_cell_type));
  }

  case VSD_FIELD_FORMAT_3PlNoUnits:
  {
    // 8 Format string: 0.000 Example: 30061.916
    return doubleToString(convertNumber(m_cell_type, m_number), "%.3f%s", "");
  }
  case VSD_FIELD_FORMAT_3PlDefUnits:
  {
    // 9 Format string: 0.000 u Example: 30061.916 cm
    return doubleToString(convertNumber(m_cell_type, m_number), "%.3f%s", getUnitString(m_cell_type));
  }
  //TODO VSD_FIELD_FORMAT_FeetAndInches  10 Format string: <,FEET/INCH>0.000 u
  //TODO VSD_FIELD_FORMAT_Radians  11 Format string: <,rad>0.#### u
  //TODO VSD_FIELD_FORMAT_Degrees  12 Format string: <,deg>0.# u
  //TODO VSD_FIELD_FORMAT_FeetAndInches1Pl  13 Format string: <,FEET/INCH># #/# u
  //TODO VSD_FIELD_FORMAT_FeetAndInches2Pl  14 Format string: <,FEET/INCH># #/## u
  //TODO VSD_FIELD_FORMAT_Fraction1PlNoUnits  15 Format string: 0 #/#
  //TODO VSD_FIELD_FORMAT_Fraction1PlDefUnits  16 Format string: 0 #/# u
  //TODO VSD_FIELD_FORMAT_Fraction2PlNoUnits  17 Format string: 0 #/##
  //TODO VSD_FIELD_FORMAT_Fraction2PlDefUnits  18 Format string: 0 #/## u
  case VSD_FIELD_FORMAT_DateShort:
    // 20 Format string: ddddd Example: Thu
    return datetimeToString("%a", m_number);
  case VSD_FIELD_FORMAT_DateLong:
    // 21 Format string: dddddd Example: Thursday
    return datetimeToString("%A", m_number);
  case VSD_FIELD_FORMAT_DateMDYY:
  case VSD_FIELD_FORMAT_DateMMDDYY:
    // 22 Format string: M/d/y Example: 4/19/82
    // 23 Format string: MM/d/y Example: 4/19/82
    return datetimeToString("%m/%d/%y", m_number);
  case VSD_FIELD_FORMAT_DateMMMDYYYY:
    // 24 Format string: MMM d, yyyy Example: Apr 19, 1982
    return datetimeToString("%b %e, %Y", m_number);
  case VSD_FIELD_FORMAT_DateMMMMDYYYY:
    // 25 Format string: MMMM d, yyyy Example: April 19, 1982
    return datetimeToString("%B %e, %Y", m_number);
  case VSD_FIELD_FORMAT_DateDMYY:
    // 26 Format string: d/M/YY Example: 19/04/82
    return datetimeToString("%e/%m/%y", m_number);
  case VSD_FIELD_FORMAT_DateDDMMYY:
    // 27 Format string: dd/MM/YY Example: 19/04/82
    return datetimeToString("%d/%m/%y", m_number);
  case VSD_FIELD_FORMAT_DateDMMMYYYY:
    // 28 Format string: d MMM, yyyy Example: 19 Apr, 1982
    return datetimeToString("%e %b, %Y", m_number);
  case VSD_FIELD_FORMAT_DateDMMMMYYYY:
    // 29 Format string: d MMMM, yyyy Example: 19 April, 1982
    return datetimeToString("%e %B, %Y", m_number);
  case VSD_FIELD_FORMAT_TimeGen:
    // The value is formatted using a format string "h:mm:ss tt" and inserted to the result string.
    // For example, FORMAT(DATETIME("6/25/07 12:05"), "T") displays 12:05:00 PM.
    // 30 Format string: T Example: 10:02:02 PM
    return datetimeToString("%r", m_number);
  case VSD_FIELD_FORMAT_TimeHMM:
  case VSD_FIELD_FORMAT_TimeHHMM:
  case VSD_FIELD_FORMAT_TimeHMM24:
  case VSD_FIELD_FORMAT_TimeHHMM24:
    // 31 Format string: h:mm Example: 10:02
    // 32 Format string: hh:mm Example: 10:02
    // 33 Format string: H:mm Example: 10:02
    // 34 Format string: HH:mm Example: 10:02
    return datetimeToString("%H:%m:%S", m_number);
  case VSD_FIELD_FORMAT_TimeHMMAMPM:
  case VSD_FIELD_FORMAT_TimeHHMMAMPM:
    // 35 Format string: h:mm tt Example: 10:02 PM
    // 36 Format string: HH:mm tt Example: 10:02 PM
    return datetimeToString("%I:%m %p", m_number);
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
    return datetimeToString("%X", m_number);
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
  case VSD_FIELD_FORMAT_MsoFEExtra1:
  case VSD_FIELD_FORMAT_MsoFEExtra2:
  case VSD_FIELD_FORMAT_MsoFEExtra3:
  case VSD_FIELD_FORMAT_MsoFEExtra4:
  case VSD_FIELD_FORMAT_MsoFEExtra5:
    // 40-81, 200, 217-221 Format string: M/d/yyyy Example: 4/19/1982
    return datetimeToString("%m/%d/%Y", m_number);
  case VSD_FIELD_FORMAT_MsoDateLongDay:
    // 201 Format string: dddd, MMMM dd, yyyy Example: Monday, April 19, 1982
    return datetimeToString("%A, %B %d, %Y", m_number);
  case VSD_FIELD_FORMAT_MsoDateLong:
    // 202 Format string: MMMM d, yyyy Example: April 19, 1982
    return datetimeToString("%B %e, %Y", m_number);
  case VSD_FIELD_FORMAT_MsoDateShortAlt:
    // 203 Format string: M/d/yy Example: 4/19/82
    return datetimeToString("%m/%d/%y", m_number);
  case VSD_FIELD_FORMAT_MsoDateISO:
    // 204 Format string: yyyy-MM-dd Example: 1982-04-19
    return datetimeToString("%Y-%m-%d", m_number);
  case VSD_FIELD_FORMAT_MsoDateShortMon:
    // 205 Format string: d-MMM-yy Example: 19-Apr-1982
    return datetimeToString("%e-%b-%y", m_number);
  case VSD_FIELD_FORMAT_MsoDateShortSlash:
    // 206 Format string: M.d.yyyy Example: 4.19.1982
    return datetimeToString("%m.%d.%Y", m_number);
  case VSD_FIELD_FORMAT_MsoDateShortAbb:
    // 207 Format string: MMM.d, yy Example: Apr.19, 82
    return datetimeToString("%b.%d, %y", m_number);
  case VSD_FIELD_FORMAT_MsoDateEnglish:
    // 208 Format string: D MMMM yyyy Example: 19 April 1982
    return datetimeToString("%e %B %Y", m_number);
  case VSD_FIELD_FORMAT_MsoDateMonthYr:
    // 209 Format string: MMMM yy Example: April 82
    return datetimeToString("%B %y", m_number);
  case VSD_FIELD_FORMAT_MsoDateMon_Yr:
    // 210 Format string: MMM-yy Example: Apr-82
    return datetimeToString("%b-%y", m_number);
  case VSD_FIELD_FORMAT_MsoTimeDatePM:
    // 211 Format string: M/d/yyyy h:mm am/pm Example: 4/19/1982 10:02 PM
    return datetimeToString("%m/%d/%Y %I:%m %p", m_number);
  case VSD_FIELD_FORMAT_MsoTimeDateSecPM:
    // 212 Format string: M/d/yyyy h:mm:ss am/pm Example: 4/19/1982 10:02:02 PM
    return datetimeToString("%m/%d/%Y %I:%m:%S %p", m_number);
  case VSD_FIELD_FORMAT_MsoTimePM:
    // 213 Format string: H:mm am/pm Example: 10:02 PM
    return datetimeToString("%I:%m %p", m_number);
  case VSD_FIELD_FORMAT_MsoTimeSecPM:
    // 214 Format string: h:mm:ss am/pm Example: 10:02:02 PM
    return datetimeToString("%I:%m:%S %p", m_number);
  case VSD_FIELD_FORMAT_MsoTime24:
    // 215 Format string: HH:mm Example: 10:02
    return datetimeToString("%H:%m", m_number);
  case VSD_FIELD_FORMAT_MsoTimeSec24:
    // 216 Format string: HH:mm:ss Example: 10:02:01
    return datetimeToString("%H:%m:%S", m_number);
  default:
  {
    std::unique_ptr<librevenge::RVNGProperty> pProp{librevenge::RVNGPropertyFactory::newDoubleProp(m_number)};
    return pProp ? pProp->getStr() : librevenge::RVNGString();
  }
  }
}

void libvisio::VSDNumericField::setFormat(unsigned short format)
{
  m_format = format;
}

void libvisio::VSDNumericField::setCellType(unsigned short cellType)
{
  m_cell_type = cellType;
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
  for (auto iter = fieldList.m_elements.begin(); iter != fieldList.m_elements.end(); ++iter)
    m_elements[iter->first] = clone(iter->second);
}

libvisio::VSDFieldList &libvisio::VSDFieldList::operator=(const libvisio::VSDFieldList &fieldList)
{
  if (this != &fieldList)
  {
    clear();
    for (auto iter = fieldList.m_elements.begin(); iter != fieldList.m_elements.end(); ++iter)
      m_elements[iter->first] = clone(iter->second);
    m_elementsOrder = fieldList.m_elementsOrder;
    m_id = fieldList.m_id;
    m_level = fieldList.m_level;
  }
  return *this;
}

libvisio::VSDFieldList::~VSDFieldList()
{
}

void libvisio::VSDFieldList::setElementsOrder(const std::vector<unsigned> &elementsOrder)
{
  m_elementsOrder.clear();
  for (unsigned int i : elementsOrder)
    m_elementsOrder.push_back(i);
}

void libvisio::VSDFieldList::addFieldList(unsigned id, unsigned level)
{
  m_id = id;
  m_level = level;
}

void libvisio::VSDFieldList::addTextField(unsigned id, unsigned level, int nameId, int formatStringId)
{
  if (m_elements.find(id) == m_elements.end())
    m_elements[id] = make_unique<VSDTextField>(id, level, nameId, formatStringId);
}

void libvisio::VSDFieldList::addNumericField(unsigned id, unsigned level, unsigned short format, unsigned short cellType, double number, int formatStringId)
{
  if (m_elements.find(id) == m_elements.end())
    m_elements[id] = make_unique<VSDNumericField>(id, level, format, cellType, number, formatStringId);
}

void libvisio::VSDFieldList::handle(VSDCollector *collector) const
{
  if (empty())
    return;

  collector->collectFieldList(m_id, m_level);
  if (!m_elementsOrder.empty())
  {
    for (unsigned int i : m_elementsOrder)
    {
      auto iter = m_elements.find(i);
      if (iter != m_elements.end())
        iter->second->handle(collector);
    }
  }
  else
  {
    for (auto iter = m_elements.begin(); iter != m_elements.end(); ++iter)
      iter->second->handle(collector);
  }
}

void libvisio::VSDFieldList::clear()
{
  m_elements.clear();
  m_elementsOrder.clear();
}

libvisio::VSDFieldListElement *libvisio::VSDFieldList::getElement(unsigned index)
{
  if (m_elementsOrder.size() > index)
    index = m_elementsOrder[index];

  auto iter = m_elements.find(index);
  if (iter != m_elements.end())
    return iter->second.get();
  else
    return nullptr;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
