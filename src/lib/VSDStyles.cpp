/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

void libvisio::VSDStyles::addStyleThemeReference(unsigned styleIndex, const VSDOptionalThemeReference &themeRef)
{
  m_themeRefs[styleIndex] = themeRef;
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

libvisio::VSDOptionalThemeReference libvisio::VSDStyles::getOptionalThemeReference(unsigned styleIndex) const
{
  VSDOptionalThemeReference themeReference;
  if (MINUS_ONE == styleIndex)
    return themeReference;
  return themeReference;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
