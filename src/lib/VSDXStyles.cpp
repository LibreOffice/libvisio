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

libvisio::VSDXLineStyle::VSDXLineStyle()
  : width(0), colour(), pattern(0), cap(0)
{}

libvisio::VSDXFillStyle::VSDXFillStyle()
  : fgColourId(1), fgColour(0xff, 0xff, 0xff, 0xff), bgColourId(0),
    bgColour(), pattern(0), shadowFgColourId(0), shadowFgColour(),
    shadowBgColourId(1), shadowBGColour(0xff, 0xff, 0xff, 0xff),
    shadowPattern(0), shadowType(0), shadowOffsetX(0), shadowOffsetY(0),
    shadowAngle(0), shadowScale(1)
{}

libvisio::VSDXTextStyle::VSDXTextStyle()
  : charFormats()
{
}
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
  while (true)
  {
    std::map<unsigned, VSDXLineStyle>::const_iterator iterStyle = m_lineStyles.find(tmpIndex);
	if (iterStyle != m_lineStyles.end())
	  return iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_lineStyleMasters.find(tmpIndex);
    if (iter != m_lineStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  return libvisio::VSDXLineStyle();
}

const libvisio::VSDXFillStyle libvisio::VSDXStyles::getFillStyle(unsigned fillStyleIndex) const
{
  unsigned tmpIndex = fillStyleIndex;
  while (true)
  {
    std::map<unsigned, VSDXFillStyle>::const_iterator iterStyle = m_fillStyles.find(tmpIndex);
	if (iterStyle != m_fillStyles.end())
	  return iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_fillStyleMasters.find(tmpIndex);
    if (iter != m_fillStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  return libvisio::VSDXFillStyle();
}

const libvisio::VSDXTextStyle libvisio::VSDXStyles::getTextStyle(unsigned textStyleIndex) const
{
  unsigned tmpIndex = textStyleIndex;
  while (true)
  {
    std::map<unsigned, VSDXTextStyle>::const_iterator iterStyle = m_textStyles.find(tmpIndex);
	if (iterStyle != m_textStyles.end())
	  return iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(tmpIndex);
    if (iter != m_textStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  return libvisio::VSDXTextStyle();
}
