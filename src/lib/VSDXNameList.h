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

#ifndef __VSDXNAMELIST_H__
#define __VSDXNAMELIST_H__

#include <vector>
#include <map>
#include <libwpd/libwpd.h>
#include <VSDXTypes.h>

namespace libvisio
{

class VSDXCollector;

class VSDXNameListElement
{
public:
  VSDXNameListElement(unsigned id, unsigned level, const WPXBinaryData &data, TextFormat format)
    : m_data(data),
      m_id(id),
      m_level(level),
      m_format(format) {}
  VSDXNameListElement() : m_data(), m_id(0), m_level(0), m_format(VSD_TEXT_ANSI) {}
  VSDXNameListElement(const VSDXNameListElement &element)
    : m_data(element.m_data),
      m_id(element.m_id),
      m_level(element.m_level),
      m_format(element.m_format) {}
  VSDXNameListElement &operator=(const VSDXNameListElement &element)
  {
    m_id = element.m_id;
    m_level = element.m_level;
    m_data = element.m_data;
    m_format = element.m_format;
    return *this;
  }
  void handle(VSDXCollector *collector);
  WPXBinaryData m_data;
  unsigned m_id, m_level;
  TextFormat m_format;
};

class VSDXNameList
{
public:
  VSDXNameList();
  VSDXNameList(const VSDXNameList &nameList);
  ~VSDXNameList();
  VSDXNameList &operator=(const VSDXNameList &nameList);
  void setId(unsigned id);
  void setLevel(unsigned level);
  void addName(unsigned id, unsigned level, const WPXBinaryData &name, TextFormat format);
  void handle(VSDXCollector *collector);
  void clear()
  {
    m_elements.clear();
    m_id = 0;
    m_level = 0;
  }
  unsigned long size() const
  {
    return (unsigned long)m_elements.size();
  }
  bool empty() const
  {
    return (m_elements.empty());
  }
  VSDXNameListElement *getElement(unsigned index);
private:
  std::vector<VSDXNameListElement> m_elements;
  unsigned m_id, m_level;
};

} // namespace libvisio

#endif // __VSDXNAMELIST_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
