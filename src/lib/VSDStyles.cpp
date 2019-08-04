/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDStyles.h"

#include <set>
#include <stack>
#include "VSDTypes.h"

namespace libvisio
{

namespace
{

template<typename T>
T getOptionalStyle(const std::map<unsigned, unsigned> &styleMasters, const std::map<unsigned, T> &styles, const unsigned styleIndex)
{
  T style;
  if (MINUS_ONE == styleIndex)
    return style;
  std::stack<unsigned> styleIdStack;
  std::set<unsigned> foundStyles;
  styleIdStack.push(styleIndex);
  while (true)
  {
    auto iter = styleMasters.find(styleIdStack.top());
    if (iter != styleMasters.end() && iter->second != MINUS_ONE)
    {
      if (foundStyles.insert(iter->second).second)
        styleIdStack.push(iter->second);
      else // we already have this style -> stop incoming endless loop
        break;
    }
    else
      break;
  }
  while (!styleIdStack.empty())
  {
    auto iter = styles.find(styleIdStack.top());
    if (iter != styles.end())
      style.override(iter->second);
    styleIdStack.pop();
  }
  return style;
}

}

}

libvisio::VSDStyles::VSDStyles() :
  m_lineStyles(), m_fillStyles(), m_textBlockStyles(), m_charStyles(), m_paraStyles(),
  m_lineStyleMasters(), m_fillStyleMasters(), m_textStyleMasters()
{
}

libvisio::VSDStyles::~VSDStyles()
{
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
  return getOptionalStyle(m_lineStyleMasters, m_lineStyles, lineStyleIndex);
}

libvisio::VSDOptionalFillStyle libvisio::VSDStyles::getOptionalFillStyle(unsigned fillStyleIndex) const
{
  return getOptionalStyle(m_fillStyleMasters, m_fillStyles, fillStyleIndex);
}

libvisio::VSDFillStyle libvisio::VSDStyles::getFillStyle(unsigned fillStyleIndex, const libvisio::VSDXTheme *theme) const
{
  VSDFillStyle fillStyle;
  fillStyle.override(getOptionalFillStyle(fillStyleIndex), theme);
  return fillStyle;
}

libvisio::VSDOptionalTextBlockStyle libvisio::VSDStyles::getOptionalTextBlockStyle(unsigned textStyleIndex) const
{
  return getOptionalStyle(m_textStyleMasters, m_textBlockStyles, textStyleIndex);
}

libvisio::VSDOptionalCharStyle libvisio::VSDStyles::getOptionalCharStyle(unsigned textStyleIndex) const
{
  return getOptionalStyle(m_textStyleMasters, m_charStyles, textStyleIndex);
}

libvisio::VSDOptionalParaStyle libvisio::VSDStyles::getOptionalParaStyle(unsigned textStyleIndex) const
{
  return getOptionalStyle(m_textStyleMasters, m_paraStyles, textStyleIndex);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
