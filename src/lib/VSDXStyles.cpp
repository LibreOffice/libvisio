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

#include "VSDXStyles.h"

#define NOMASTER 0xffffffff

libvisio::VSDXStyles::VSDXStyles() :
  m_lineStyles(), m_fillStyles(), m_textBlockStyles(), m_charStyles(), m_paraStyles(),
  m_lineStyleMasters(), m_fillStyleMasters(), m_textStyleMasters()
{
}

libvisio::VSDXStyles::VSDXStyles(const libvisio::VSDXStyles &styles) :
  m_lineStyles(), m_fillStyles(), m_textBlockStyles(), m_charStyles(), m_paraStyles(),
  m_lineStyleMasters(styles.m_lineStyleMasters), m_fillStyleMasters(styles.m_fillStyleMasters),
  m_textStyleMasters(styles.m_textStyleMasters)
{
  for (std::map<unsigned, VSDXFillStyle *>::const_iterator iterFillStyle = styles.m_fillStyles.begin();
       iterFillStyle != styles.m_fillStyles.end(); iterFillStyle++)
  {
    if (iterFillStyle->second)
      m_fillStyles[iterFillStyle->first] = new VSDXFillStyle(*(iterFillStyle->second));
  }
  for (std::map<unsigned, VSDXLineStyle *>::const_iterator iterLineStyle = styles.m_lineStyles.begin();
       iterLineStyle != styles.m_lineStyles.end(); iterLineStyle++)
  {
    if (iterLineStyle->second)
      m_lineStyles[iterLineStyle->first] = new VSDXLineStyle(*(iterLineStyle->second));
  }
  for (std::map<unsigned, VSDXTextBlockStyle *>::const_iterator iterTextBlockStyle = styles.m_textBlockStyles.begin();
       iterTextBlockStyle != styles.m_textBlockStyles.end(); iterTextBlockStyle++)
  {
    if (iterTextBlockStyle->second)
      m_textBlockStyles[iterTextBlockStyle->first] = new VSDXTextBlockStyle(*(iterTextBlockStyle->second));
  }
  for (std::map<unsigned, VSDXCharStyle *>::const_iterator iterCharStyle = styles.m_charStyles.begin();
       iterCharStyle != styles.m_charStyles.end(); iterCharStyle++)
  {
    if (iterCharStyle->second)
      m_charStyles[iterCharStyle->first] = new VSDXCharStyle(*(iterCharStyle->second));
  }
  for (std::map<unsigned, VSDXParaStyle *>::const_iterator iterParaStyle = styles.m_paraStyles.begin();
       iterParaStyle != styles.m_paraStyles.end(); iterParaStyle++)
  {
    if (iterParaStyle->second)
      m_paraStyles[iterParaStyle->first] = new VSDXParaStyle(*(iterParaStyle->second));
  }
}

libvisio::VSDXStyles::~VSDXStyles()
{
  for (std::map<unsigned, VSDXFillStyle *>::iterator iterFillStyle = m_fillStyles.begin();
       iterFillStyle != m_fillStyles.end(); iterFillStyle++)
  {
    if (iterFillStyle->second)
      delete iterFillStyle->second;
  }
  for (std::map<unsigned, VSDXLineStyle *>::iterator iterLineStyle = m_lineStyles.begin();
       iterLineStyle != m_lineStyles.end(); iterLineStyle++)
  {
    if (iterLineStyle->second)
      delete iterLineStyle->second;
  }
  for (std::map<unsigned, VSDXTextBlockStyle *>::iterator iterTextBlockStyle = m_textBlockStyles.begin();
       iterTextBlockStyle != m_textBlockStyles.end(); iterTextBlockStyle++)
  {
    if (iterTextBlockStyle->second)
      delete (iterTextBlockStyle->second);
  }
  for (std::map<unsigned, VSDXCharStyle *>::iterator iterCharStyle = m_charStyles.begin();
       iterCharStyle != m_charStyles.end(); iterCharStyle++)
  {
    if (iterCharStyle->second)
      delete (iterCharStyle->second);
  }
  for (std::map<unsigned, VSDXParaStyle *>::iterator iterParaStyle = m_paraStyles.begin();
       iterParaStyle != m_paraStyles.end(); iterParaStyle++)
  {
    if (iterParaStyle->second)
      delete iterParaStyle->second;
  }
}

libvisio::VSDXStyles &libvisio::VSDXStyles::operator=(const libvisio::VSDXStyles &styles)
{
  for (std::map<unsigned, VSDXFillStyle *>::iterator iterFillStyle = m_fillStyles.begin();
       iterFillStyle != m_fillStyles.end(); iterFillStyle++)
  {
    if (iterFillStyle->second)
      delete iterFillStyle->second;
  }
  m_fillStyles.clear();
  for (std::map<unsigned, VSDXLineStyle *>::iterator iterLineStyle = m_lineStyles.begin();
       iterLineStyle != m_lineStyles.end(); iterLineStyle++)
  {
    if (iterLineStyle->second)
      delete iterLineStyle->second;
  }
  m_lineStyles.clear();
  for (std::map<unsigned, VSDXTextBlockStyle *>::iterator iterTextBlockStyle = m_textBlockStyles.begin();
       iterTextBlockStyle != m_textBlockStyles.end(); iterTextBlockStyle++)
  {
    if (iterTextBlockStyle->second)
      delete (iterTextBlockStyle->second);
  }
  m_textBlockStyles.clear();
  for (std::map<unsigned, VSDXCharStyle *>::iterator iterCharStyle = m_charStyles.begin();
       iterCharStyle != m_charStyles.end(); iterCharStyle++)
  {
    if (iterCharStyle->second)
      delete (iterCharStyle->second);
  }
  m_charStyles.clear();
  for (std::map<unsigned, VSDXParaStyle *>::iterator iterParaStyle = m_paraStyles.begin();
       iterParaStyle != m_paraStyles.end(); iterParaStyle++)
  {
    if (iterParaStyle->second)
      delete iterParaStyle->second;
  }
  m_paraStyles.clear();

  for (std::map<unsigned, VSDXFillStyle *>::const_iterator cstIterFillStyle = styles.m_fillStyles.begin();
       cstIterFillStyle != styles.m_fillStyles.end(); cstIterFillStyle++)
  {
    if (cstIterFillStyle->second)
      m_fillStyles[cstIterFillStyle->first] = new VSDXFillStyle(*(cstIterFillStyle->second));
  }
  for (std::map<unsigned, VSDXLineStyle *>::const_iterator cstIterLineStyle = styles.m_lineStyles.begin();
       cstIterLineStyle != styles.m_lineStyles.end(); cstIterLineStyle++)
  {
    if (cstIterLineStyle->second)
      m_lineStyles[cstIterLineStyle->first] = new VSDXLineStyle(*(cstIterLineStyle->second));
  }
  for (std::map<unsigned, VSDXTextBlockStyle *>::const_iterator cstIterTextBlockStyle = styles.m_textBlockStyles.begin();
       cstIterTextBlockStyle != styles.m_textBlockStyles.end(); cstIterTextBlockStyle++)
  {
    if (cstIterTextBlockStyle->second)
      m_textBlockStyles[cstIterTextBlockStyle->first] = new VSDXTextBlockStyle(*(cstIterTextBlockStyle->second));
  }
  for (std::map<unsigned, VSDXCharStyle *>::const_iterator cstIterCharStyle = styles.m_charStyles.begin();
       cstIterCharStyle != styles.m_charStyles.end(); cstIterCharStyle++)
  {
    if (cstIterCharStyle->second)
      m_charStyles[cstIterCharStyle->first] = new VSDXCharStyle(*(cstIterCharStyle->second));
  }
  for (std::map<unsigned, VSDXParaStyle *>::const_iterator cstIterParaStyle = styles.m_paraStyles.begin();
       cstIterParaStyle != styles.m_paraStyles.end(); cstIterParaStyle++)
  {
    if (cstIterParaStyle->second)
      m_paraStyles[cstIterParaStyle->first] = new VSDXParaStyle(*(cstIterParaStyle->second));
  }

  m_lineStyleMasters = styles.m_lineStyleMasters;
  m_fillStyleMasters = styles.m_fillStyleMasters;
  m_textStyleMasters = styles.m_textStyleMasters;

  return *this;
}

void libvisio::VSDXStyles::addLineStyle(unsigned lineStyleIndex, VSDXLineStyle *lineStyle)
{
  if (lineStyle)
  {
    std::map<unsigned, VSDXLineStyle *>::iterator iter = m_lineStyles.lower_bound(lineStyleIndex);
    if (iter != m_lineStyles.end() && !(m_lineStyles.key_comp()(lineStyleIndex, iter->first)) && iter->second)
      delete iter->second;
    m_lineStyles.insert(iter, std::map<unsigned, VSDXLineStyle *>::value_type(lineStyleIndex, new VSDXLineStyle(*lineStyle)));
  }
}

void libvisio::VSDXStyles::addFillStyle(unsigned fillStyleIndex, VSDXFillStyle *fillStyle)
{
  if (fillStyle)
  {
    std::map<unsigned, VSDXFillStyle *>::iterator iter = m_fillStyles.lower_bound(fillStyleIndex);
    if (iter != m_fillStyles.end() && !(m_fillStyles.key_comp()(fillStyleIndex, iter->first))&& iter->second)
      delete iter->second;
    m_fillStyles.insert(iter, std::map<unsigned, VSDXFillStyle *>::value_type(fillStyleIndex, new VSDXFillStyle(*fillStyle)));
  }
}

void libvisio::VSDXStyles::addTextBlockStyle(unsigned textStyleIndex, VSDXTextBlockStyle *textBlockStyle)
{
  if (textBlockStyle)
  {
    std::map<unsigned, VSDXTextBlockStyle *>::iterator iter = m_textBlockStyles.lower_bound(textStyleIndex);
    if (iter != m_textBlockStyles.end() && !(m_textBlockStyles.key_comp()(textStyleIndex, iter->first)) && iter->second)
      delete iter->second;
    m_textBlockStyles.insert(iter, std::map<unsigned, VSDXTextBlockStyle *>::value_type(textStyleIndex, new VSDXTextBlockStyle(*textBlockStyle)));
  }
}

void libvisio::VSDXStyles::addCharStyle(unsigned textStyleIndex, VSDXCharStyle *charStyle)
{
  if (charStyle)
  {
    std::map<unsigned, VSDXCharStyle *>::iterator iter = m_charStyles.lower_bound(textStyleIndex);
    if (iter != m_charStyles.end() && !(m_charStyles.key_comp()(textStyleIndex, iter->first)) && iter->second)
      delete iter->second;
    m_charStyles.insert(iter, std::map<unsigned, VSDXCharStyle *>::value_type(textStyleIndex, new VSDXCharStyle(*charStyle)));
  }
}

void libvisio::VSDXStyles::addParaStyle(unsigned textStyleIndex, VSDXParaStyle *paraStyle)
{
  if (paraStyle)
  {
    std::map<unsigned, VSDXParaStyle *>::iterator iter = m_paraStyles.lower_bound(textStyleIndex);
    if (iter != m_paraStyles.end() && !(m_paraStyles.key_comp()(textStyleIndex, iter->first)) && iter->second)
      delete iter->second;
    m_paraStyles.insert(iter, std::map<unsigned, VSDXParaStyle *>::value_type(textStyleIndex, new VSDXParaStyle(*paraStyle)));
  }
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
  std::map<unsigned, VSDXLineStyle *>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_lineStyles.find(tmpIndex);
    if (iterStyle != m_lineStyles.end() && iterStyle->second)
      return *(iterStyle->second);
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
  std::map<unsigned, VSDXFillStyle *>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_fillStyles.find(tmpIndex);
    if (iterStyle != m_fillStyles.end() && iterStyle->second)
      return *(iterStyle->second);
    std::map<unsigned, unsigned>::const_iterator iter = m_fillStyleMasters.find(tmpIndex);
    if (iter != m_fillStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  return libvisio::VSDXFillStyle();
}

const libvisio::VSDXTextBlockStyle libvisio::VSDXStyles::getTextBlockStyle(unsigned textStyleIndex) const
{
  unsigned tmpIndex = textStyleIndex;
  std::map<unsigned, VSDXTextBlockStyle *>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_textBlockStyles.find(tmpIndex);
    if (iterStyle != m_textBlockStyles.end() && iterStyle->second)
      return *(iterStyle->second);
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(tmpIndex);
    if (iter != m_textStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  return libvisio::VSDXTextBlockStyle();
}

const libvisio::VSDXCharStyle libvisio::VSDXStyles::getCharStyle(unsigned textStyleIndex) const
{
  unsigned tmpIndex = textStyleIndex;
  std::map<unsigned, VSDXCharStyle *>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_charStyles.find(tmpIndex);
    if (iterStyle != m_charStyles.end() && iterStyle->second)
      return *(iterStyle->second);
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(tmpIndex);
    if (iter != m_textStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  return libvisio::VSDXCharStyle();
}

const libvisio::VSDXParaStyle libvisio::VSDXStyles::getParaStyle(unsigned textStyleIndex) const
{
  unsigned tmpIndex = textStyleIndex;
  std::map<unsigned, VSDXParaStyle *>::const_iterator iterStyle;
  while (true)
  {
    iterStyle = m_paraStyles.find(tmpIndex);
    if (iterStyle != m_paraStyles.end() && iterStyle->second)
      return *(iterStyle->second);
    std::map<unsigned, unsigned>::const_iterator iter = m_textStyleMasters.find(tmpIndex);
    if (iter != m_textStyleMasters.end() && iter->second != NOMASTER)
      tmpIndex = iter->second;
    else
      break;
  }

  return libvisio::VSDXParaStyle();
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
