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
#include "VSDXNameList.h"


void libvisio::VSDXNameListElement::handle(VSDXCollector *collector)
{
  collector->collectName(m_id, m_level, m_data, m_format);
}


libvisio::VSDXNameList::VSDXNameList() :
  m_elements(),
  m_id(0),
  m_level(0)
{
}

libvisio::VSDXNameList::VSDXNameList(const libvisio::VSDXNameList &nameList) :
  m_elements(nameList.m_elements),
  m_id(nameList.m_id),
  m_level(nameList.m_level)
{
}

libvisio::VSDXNameList &libvisio::VSDXNameList::operator=(const libvisio::VSDXNameList &nameList)
{
  m_elements = nameList.m_elements;
  m_id = nameList.m_id;
  m_level = nameList.m_level;
  return *this;
}

libvisio::VSDXNameList::~VSDXNameList()
{
}

void libvisio::VSDXNameList::setId(unsigned id)
{
  m_id = id;
}

void libvisio::VSDXNameList::setLevel(unsigned level)
{
  m_level = level;
}

void libvisio::VSDXNameList::addName(unsigned id, unsigned level, const WPXBinaryData &data, TextFormat format)
{
  m_elements.push_back(VSDXNameListElement(id, level, data, format));
}

void libvisio::VSDXNameList::handle(libvisio::VSDXCollector *collector)
{
  if (empty())
    return;

  collector->collectNameList(m_id, m_level);
  for (std::vector<VSDXNameListElement>::iterator iter = m_elements.begin(); iter != m_elements.end(); iter++)
    iter->handle(collector);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
