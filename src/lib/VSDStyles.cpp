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

#include <stack>
#include "VSDStyles.h"
#include "VSDTypes.h"

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

void libvisio::VSDStyles::addLineStyle(unsigned lineStyleIndex, const VSDOptionalLineStyle &lineStyle)
{
  m_lineStyles[lineStyleIndex] = lineStyle;
}

void libvisio::VSDStyles::addFillStyle(unsigned fillStyleIndex, const VSDOptionalFillStyle &fillStyle)
{
  m_fillStyles[fillStyleIndex] = fillStyle;
}

void libvisio::VSDStyles::addTextBlockStyle(unsigned textStyleIndex, const VSDOptionalTextBlockStyle &textBlockStyle)
{
  m_textBlockStyles[textStyleIndex] = textBlockStyle;
}

void libvisio::VSDStyles::addCharStyle(unsigned textStyleIndex, const VSDOptionalCharStyle &charStyle)
{
  m_charStyles[textStyleIndex] = charStyle;
}

void libvisio::VSDStyles::addParaStyle(unsigned textStyleIndex, const VSDOptionalParaStyle &paraStyle)
{
  m_paraStyles[textStyleIndex] = paraStyle;
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

libvisio::VSDOptionalLineStyle libvisio::VSDStyles::getOptionalLineStyle(unsigned lineStyleIndex) const
{
  VSDOptionalLineStyle lineStyle;
  if (MINUS_ONE == lineStyleIndex)
    return lineStyle;
  std::stack<unsigned> styleIdStack;
  styleIdStack.push(lineStyleIndex);
  while (true)
  {
    std::map<unsigned, unsigned>::const_iterator iter = m_lineStyleMasters.find(styleIdStack.top());
    if (iter != m_lineStyleMasters.end() && iter->second != MINUS_ONE)
      styleIdStack.push(iter->second);
    else
      break;
  }
  while (!styleIdStack.empty())
  {
    std::map<unsigned, VSDOptionalLineStyle>::const_iterator iter = m_lineStyles.find(styleIdStack.top());
    if (iter != m_lineStyles.end())
      lineStyle.override(iter->second);
    styleIdStack.pop();
  }
  return lineStyle;
}

libvisio::VSDOptionalFillStyle libvisio::VSDStyles::getOptionalFillStyle(unsigned fillStyleIndex) const
{
  VSDOptionalFillStyle fillStyle;
  if (MINUS_ONE == fillStyleIndex)
    return fillStyle;
  std::stack<unsigned> styleIdStack;
  styleIdStack.push(fillStyleIndex);
  while (true)
  {
    std::map<unsigned, unsigned>::const_iterator iter = m_fillStyleMasters.find(styleIdStack.top());
    if (iter != m_fillStyleMasters.end() && iter->second != MINUS_ONE)
      styleIdStack.push(iter->second);
    else
      break;
  }
  while (!styleIdStack.empty())
  {
    std::map<unsigned, VSDOptionalFillStyle>::const_iterator iter = m_fillStyles.find(styleIdStack.top());
    if (iter != m_fillStyles.end())
      fillStyle.override(iter->second);
    styleIdStack.pop();
  }
  return fillStyle;
}

libvisio::VSDFillStyle libvisio::VSDStyles::getFillStyle(unsigned fillStyleIndex) const
{
  VSDFillStyle fillStyle;
  fillStyle.override(getOptionalFillStyle(fillStyleIndex));
  return fillStyle;
}

libvisio::VSDOptionalTextBlockStyle libvisio::VSDStyles::getOptionalTextBlockStyle(unsigned textStyleIndex) const
{
  VSDOptionalTextBlockStyle textBlockStyle;
  if (MINUS_ONE == textStyleIndex)
    return textBlockStyle;
  std::stack<unsigned> styleIdStack;
  styleIdStack.push(textStyleIndex);
  while (true)
  {
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(styleIdStack.top());
    if (iter != m_textStyleMasters.end() && iter->second != MINUS_ONE)
      styleIdStack.push(iter->second);
    else
      break;
  }
  while (!styleIdStack.empty())
  {
    std::map<unsigned, VSDOptionalTextBlockStyle>::const_iterator iter = m_textBlockStyles.find(styleIdStack.top());
    if (iter != m_textBlockStyles.end())
      textBlockStyle.override(iter->second);
    styleIdStack.pop();
  }
  return textBlockStyle;
}

libvisio::VSDOptionalCharStyle libvisio::VSDStyles::getOptionalCharStyle(unsigned textStyleIndex) const
{
  VSDOptionalCharStyle charStyle;
  if (MINUS_ONE == textStyleIndex)
    return charStyle;
  std::stack<unsigned> styleIdStack;
  styleIdStack.push(textStyleIndex);
  while (true)
  {
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(styleIdStack.top());
    if (iter != m_textStyleMasters.end() && iter->second != MINUS_ONE)
      styleIdStack.push(iter->second);
    else
      break;
  }
  while (!styleIdStack.empty())
  {
    std::map<unsigned, VSDOptionalCharStyle>::const_iterator iter = m_charStyles.find(styleIdStack.top());
    if (iter != m_charStyles.end())
      charStyle.override(iter->second);
    styleIdStack.pop();
  }
  return charStyle;
}

libvisio::VSDOptionalParaStyle libvisio::VSDStyles::getOptionalParaStyle(unsigned textStyleIndex) const
{
  VSDOptionalParaStyle paraStyle;
  if (MINUS_ONE == textStyleIndex)
    return paraStyle;
  std::stack<unsigned> styleIdStack;
  styleIdStack.push(textStyleIndex);
  while (true)
  {
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(styleIdStack.top());
    if (iter != m_textStyleMasters.end() && iter->second != MINUS_ONE)
      styleIdStack.push(iter->second);
    else
      break;
  }
  while (!styleIdStack.empty())
  {
    std::map<unsigned, VSDOptionalParaStyle>::const_iterator iter = m_paraStyles.find(styleIdStack.top());
    if (iter != m_paraStyles.end())
      paraStyle.override(iter->second);
    styleIdStack.pop();
  }
  return paraStyle;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
