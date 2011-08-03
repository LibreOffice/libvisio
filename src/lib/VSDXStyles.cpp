/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02111-1301 USA
 */

#include "VSDXStyles.h"

#define NOMASTER 0xffffffff

libvisio::VSDXStyles::VSDXStyles()
{
}

libvisio::VSDXStyles::~VSDXStyles()
{
}

void libvisio::VSDXStyles::addLineStyle(unsigned lineStyleIndex, VSDXLineStyle *lineStyle, unsigned lineStyleMaster)
{
  if (lineStyle)
    m_lineStyles[lineStyleIndex] = *lineStyle;
  m_lineStyleMasters[lineStyleIndex] = lineStyleMaster;
}

void libvisio::VSDXStyles::addFillStyle(unsigned fillStyleIndex, VSDXFillStyle *fillStyle, unsigned fillStyleMaster)
{
  if (fillStyle)
    m_fillStyles[fillStyleIndex] = *fillStyle;
  m_fillStyleMasters[fillStyleIndex] = fillStyleMaster;
}

void libvisio::VSDXStyles::addTextStyle(unsigned textStyleIndex, VSDXTextStyle *textStyle, unsigned textStyleMaster)
{
  if (textStyle)
    m_textStyles[textStyleIndex] = *textStyle;
  m_textStyleMasters[textStyleIndex] = textStyleMaster;
}
	
const libvisio::VSDXLineStyle libvisio::VSDXStyles::getLineStyle(unsigned lineStyleIndex) const
{
  unsigned tmpIndex = lineStyleIndex;
  while (true)
  {
    std::map<unsigned, unsigned>::const_iterator iter = m_lineStyleMasters.find(tmpIndex);
    if (iter != m_lineStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }
  std::map<unsigned, VSDXLineStyle>::const_iterator iterStyle = m_lineStyles.find(tmpIndex);
  if (iterStyle != m_lineStyles.end())
    return iterStyle->second;

  return libvisio::VSDXLineStyle();
}

const libvisio::VSDXFillStyle libvisio::VSDXStyles::getFillStyle(unsigned fillStyleIndex) const
{
  unsigned tmpIndex = fillStyleIndex;
  while (true)
  {
    std::map<unsigned, unsigned>::const_iterator iter = m_fillStyleMasters.find(tmpIndex);
    if (iter != m_fillStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }
  std::map<unsigned, VSDXFillStyle>::const_iterator iterStyle = m_fillStyles.find(tmpIndex);
  if (iterStyle != m_fillStyles.end())
    return iterStyle->second;

  return libvisio::VSDXFillStyle();
}

const libvisio::VSDXTextStyle libvisio::VSDXStyles::getTextStyle(unsigned textStyleIndex) const
{
  unsigned tmpIndex = textStyleIndex;
  while (true)
  {
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(tmpIndex);
    if (iter != m_textStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }
  std::map<unsigned, VSDXTextStyle>::const_iterator iterStyle = m_textStyles.find(tmpIndex);
  if (iterStyle != m_textStyles.end())
    return iterStyle->second;

  return libvisio::VSDXTextStyle();
}
