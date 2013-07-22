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
 * Copyright (C) 2013 Fridrich Strba <fridrich.strba@bluewin.ch>
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

#ifndef __VSDXTHEME_H__
#define __VSDXTHEME_H__

#include <vector>
#include <boost/optional.hpp>
#include <libwpd-stream/libwpd-stream.h>
#include "VSDXMLHelper.h"

namespace libvisio
{

class VSDCollector;

struct VSDXVariationClrScheme
{
  Colour m_varColor1;
  Colour m_varColor2;
  Colour m_varColor3;
  Colour m_varColor4;
  Colour m_varColor5;
  Colour m_varColor6;
  Colour m_varColor7;
};

struct VSDXClrScheme
{
  Colour m_dk1;
  Colour m_lt1;
  Colour m_dk2;
  Colour m_lt2;
  Colour m_accent1;
  Colour m_accent2;
  Colour m_accent3;
  Colour m_accent4;
  Colour m_accent5;
  Colour m_accent6;
  Colour m_hlink;
  Colour m_folHlink;
  std::vector<VSDXVariationClrScheme> m_variationClrSchemeLst;
};

class VSDXTheme
{
public:
  VSDXTheme();
  ~VSDXTheme();
  bool parse(WPXInputStream *input);

private:
  VSDXTheme(const VSDXTheme &);
  VSDXTheme &operator=(const VSDXTheme &);

  boost::optional<Colour> readSrgbClr(xmlTextReaderPtr reader);
  boost::optional<Colour> readSysClr(xmlTextReaderPtr reader);

  void readClrScheme(xmlTextReaderPtr reader);
  void readThemeColour(xmlTextReaderPtr reader, int idToken, Colour &clr);
  void readVariationClrSchemeLst(xmlTextReaderPtr reader);
  void readVariationClrScheme(xmlTextReaderPtr reader, VSDXVariationClrScheme &varClrSch);

  int getElementToken(xmlTextReaderPtr reader);

  VSDXClrScheme m_clrScheme;
};

} // namespace libvisio

#endif // __VSDXTHEME_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
