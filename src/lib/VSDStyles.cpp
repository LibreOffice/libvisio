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

#include "VSDStyles.h"

#define NOMASTER (unsigned)-1

libvisio::VSDStyles::VSDStyles() :
  m_lineStyles(), m_fillStyles(), m_textBlockStyles(), m_charStyles(), m_paraStyles(),
  m_lineStyleMasters(), m_fillStyleMasters(), m_textStyleMasters()
{
}

libvisio::VSDStyles::VSDStyles(const libvisio::VSDStyles &styles) :
  m_lineStyles(styles.m_lineStyles), m_fillStyles(styles.m_fillStyles), m_textBlockStyles(styles.m_textBlockStyles),
  m_charStyles(styles.m_charStyles), m_paraStyles(styles.m_paraStyles), m_lineStyleMasters(styles.m_lineStyleMasters),
  m_fillStyleMasters(styles.m_fillStyleMasters), m_textStyleMasters(styles.m_textStyleMasters)
{
}

libvisio::VSDStyles::~VSDStyles()
{
}

libvisio::VSDStyles &libvisio::VSDStyles::operator=(const libvisio::VSDStyles &styles)
{
  if (this != &styles)
  {
    m_lineStyles = styles.m_lineStyles;
    m_fillStyles = styles.m_fillStyles;
    m_textBlockStyles = styles.m_textBlockStyles;
    m_charStyles = styles.m_charStyles;
    m_paraStyles = styles.m_paraStyles;

    m_lineStyleMasters = styles.m_lineStyleMasters;
    m_fillStyleMasters = styles.m_fillStyleMasters;
    m_textStyleMasters = styles.m_textStyleMasters;
  }
  return *this;
}

void libvisio::VSDStyles::addLineStyle(unsigned lineStyleIndex, VSDLineStyle *lineStyle)
{
  if (lineStyle)
    m_lineStyles[lineStyleIndex] = *lineStyle;
}

void libvisio::VSDStyles::addFillStyle(unsigned fillStyleIndex, VSDFillStyle *fillStyle)
{
  if (fillStyle)
    m_fillStyles[fillStyleIndex] = *fillStyle;
}

void libvisio::VSDStyles::addTextBlockStyle(unsigned textStyleIndex, VSDTextBlockStyle *textBlockStyle)
{
  if (textBlockStyle)
    m_textBlockStyles[textStyleIndex] = *textBlockStyle;
}

void libvisio::VSDStyles::addCharStyle(unsigned textStyleIndex, VSDCharStyle *charStyle)
{
  if (charStyle)
    m_charStyles[textStyleIndex] = *charStyle;
}

void libvisio::VSDStyles::addParaStyle(unsigned textStyleIndex, VSDParaStyle *paraStyle)
{
  if (paraStyle)
    m_paraStyles[textStyleIndex] = *paraStyle;
}

void libvisio::VSDStyles::addLineStyleMaster(unsigned lineStyleIndex, unsigned lineStyleMaster)
{
  m_lineStyleMasters[lineStyleIndex] = lineStyleMaster;
}

void libvisio::VSDStyles::addFillStyleMaster(unsigned fillStyleIndex, unsigned fillStyleMaster)
{
  m_fillStyleMasters[fillStyleIndex] = fillStyleMaster;
}

void libvisio::VSDStyles::addTextStyleMaster(unsigned textStyleIndex, unsigned textStyleMaster)
{
  m_textStyleMasters[textStyleIndex] = textStyleMaster;
}

const libvisio::VSDLineStyle *libvisio::VSDStyles::getLineStyle(unsigned lineStyleIndex) const
{
  if ((unsigned)-1 == lineStyleIndex)
    return 0;
  std::map<unsigned, VSDLineStyle>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_lineStyles.find(lineStyleIndex);
    if (iterStyle != m_lineStyles.end())
      return &iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_lineStyleMasters.find(lineStyleIndex);
    if (iter != m_lineStyleMasters.end() && iter->second != NOMASTER)
      lineStyleIndex = iter->second;
    else
      break;
  }

  return 0;
}

const libvisio::VSDFillStyle *libvisio::VSDStyles::getFillStyle(unsigned fillStyleIndex) const
{
  if ((unsigned)-1 == fillStyleIndex)
    return 0;
  std::map<unsigned, VSDFillStyle>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_fillStyles.find(fillStyleIndex);
    if (iterStyle != m_fillStyles.end())
      return &iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_fillStyleMasters.find(fillStyleIndex);
    if (iter != m_fillStyleMasters.end() && iter->second != NOMASTER)
      fillStyleIndex = iter->second;
    else
      break;
  }

  return 0;
}

const libvisio::VSDTextBlockStyle *libvisio::VSDStyles::getTextBlockStyle(unsigned textStyleIndex) const
{
  if ((unsigned)-1 == textStyleIndex)
    return 0;
  std::map<unsigned, VSDTextBlockStyle>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_textBlockStyles.find(textStyleIndex);
    if (iterStyle != m_textBlockStyles.end())
      return &iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(textStyleIndex);
    if (iter != m_textStyleMasters.end() && iter->second != NOMASTER)
      textStyleIndex = iter->second;
    else
      break;
  }

  return 0;
}

const libvisio::VSDCharStyle *libvisio::VSDStyles::getCharStyle(unsigned textStyleIndex) const
{
  if ((unsigned)-1 == textStyleIndex)
    return 0;
  std::map<unsigned, VSDCharStyle>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_charStyles.find(textStyleIndex);
    if (iterStyle != m_charStyles.end())
      return &iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(textStyleIndex);
    if (iter != m_textStyleMasters.end() && iter->second != NOMASTER)
      textStyleIndex = iter->second;
    else
      break;
  }

  return 0;
}

const libvisio::VSDParaStyle *libvisio::VSDStyles::getParaStyle(unsigned textStyleIndex) const
{
  if ((unsigned)-1 == textStyleIndex)
    return 0;
  std::map<unsigned, VSDParaStyle>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_paraStyles.find(textStyleIndex);
    if (iterStyle != m_paraStyles.end())
      return &iterStyle->second;
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(textStyleIndex);
    if (iter != m_textStyleMasters.end() && iter->second != NOMASTER)
      textStyleIndex = iter->second;
    else
      break;
  }

  return 0;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
