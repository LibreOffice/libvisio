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

#include "VSDXStyles.h"

#define NOMASTER 0xffffffff

libvisio::VSDXStyles::VSDXStyles()
{
}

libvisio::VSDXStyles::~VSDXStyles()
{
}

void libvisio::VSDXStyles::addLineStyle(unsigned lineStyleIndex, VSDXLineStyle *lineStyle)
{
  if (lineStyle)
    m_lineStyles[lineStyleIndex] = *lineStyle;
}

void libvisio::VSDXStyles::addFillStyle(unsigned fillStyleIndex, VSDXFillStyle *fillStyle)
{
  if (fillStyle)
    m_fillStyles[fillStyleIndex] = *fillStyle;
}

void libvisio::VSDXStyles::addTextStyle(unsigned textStyleIndex, VSDXTextStyle *textStyle)
{
  if (textStyle)
    m_textStyles[textStyleIndex] = *textStyle;
}
	
void libvisio::VSDXStyles::addLineStyleMaster(unsigned lineStyleIndex, unsigned lineStyleMaster)
{
  m_lineStyleMasters[lineStyleIndex] = lineStyleMaster;
}

void libvisio::VSDXStyles::addFillStyleMaster(unsigned fillStyleIndex, unsigned fillStyleMaster)
{
  m_fillStyleMasters[fillStyleIndex] = fillStyleMaster;
}

void libvisio::VSDXStyles::addTextStyleMaster(unsigned textStyleIndex, unsigned textStyleMaster)
{
  m_textStyleMasters[textStyleIndex] = textStyleMaster;
}
	
const libvisio::VSDXLineStyle libvisio::VSDXStyles::getLineStyle(unsigned lineStyleIndex) const
{
  unsigned tmpIndex = lineStyleIndex;
  std::map<unsigned, VSDXLineStyle>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_lineStyles.find(tmpIndex);
    if (iterStyle != m_lineStyles.end())
      return iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_lineStyleMasters.find(tmpIndex);
    if (iter != m_lineStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  iterStyle = m_lineStyles.find(0);
  if (iterStyle != m_lineStyles.end())
    return iterStyle->second;

  return libvisio::VSDXLineStyle();
}

const libvisio::VSDXFillStyle libvisio::VSDXStyles::getFillStyle(unsigned fillStyleIndex) const
{
  unsigned tmpIndex = fillStyleIndex;
  std::map<unsigned, VSDXFillStyle>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_fillStyles.find(tmpIndex);
    if (iterStyle != m_fillStyles.end())
      return iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_fillStyleMasters.find(tmpIndex);
    if (iter != m_fillStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  iterStyle = m_fillStyles.find(0);
  if (iterStyle != m_fillStyles.end())
    return iterStyle->second;

  return libvisio::VSDXFillStyle();
}

const libvisio::VSDXTextStyle libvisio::VSDXStyles::getTextStyle(unsigned textStyleIndex) const
{
  unsigned tmpIndex = textStyleIndex;
  std::map<unsigned, VSDXTextStyle>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_textStyles.find(tmpIndex);
    if (iterStyle != m_textStyles.end())
      return iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(tmpIndex);
    if (iter != m_textStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  iterStyle = m_textStyles.find(0);
  if (iterStyle != m_textStyles.end())
    return iterStyle->second;

  return libvisio::VSDXTextStyle();
}
