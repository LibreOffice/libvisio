/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDXTHEME_H__
#define __VSDXTHEME_H__

#include <vector>
#include <map>
#include <boost/optional.hpp>
#include <librevenge-stream/librevenge-stream.h>
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

  VSDXVariationClrScheme();
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
  Colour m_bkgnd;
  std::vector<VSDXVariationClrScheme> m_variationClrSchemeLst;

  VSDXClrScheme();
};

struct VSDXFont
{
  librevenge::RVNGString m_latinTypeFace;
  librevenge::RVNGString m_eaTypeFace;
  librevenge::RVNGString m_csTypeFace;
  std::map<unsigned, librevenge::RVNGString> m_typeFaces;

  VSDXFont();
};

struct VSDXFontScheme
{
  VSDXFont m_majorFont;
  VSDXFont m_minorFont;
  unsigned m_schemeId;

  VSDXFontScheme();
};

class VSDXTheme
{
public:
  VSDXTheme();
  ~VSDXTheme();
  bool parse(librevenge::RVNGInputStream *input);
  boost::optional<Colour> getThemeColour(unsigned value, unsigned variationIndex = 0) const;

private:
  VSDXTheme(const VSDXTheme &);
  VSDXTheme &operator=(const VSDXTheme &);

  boost::optional<Colour> readSrgbClr(xmlTextReaderPtr reader);
  boost::optional<Colour> readSysClr(xmlTextReaderPtr reader);

  void readClrScheme(xmlTextReaderPtr reader);
  void readThemeColour(xmlTextReaderPtr reader, int idToken, Colour &clr);
  void readVariationClrSchemeLst(xmlTextReaderPtr reader);
  void readVariationClrScheme(xmlTextReaderPtr reader, VSDXVariationClrScheme &varClrSch);
  void readFontScheme(xmlTextReaderPtr reader);
  void readFont(xmlTextReaderPtr reader, int idToken, VSDXFont &font);
  bool readTypeFace(xmlTextReaderPtr reader, librevenge::RVNGString &typeFace);
  bool readTypeFace(xmlTextReaderPtr reader, int &script, librevenge::RVNGString &typeFace);

  int getElementToken(xmlTextReaderPtr reader);

  VSDXClrScheme m_clrScheme;
  VSDXFontScheme m_fontScheme;
};

} // namespace libvisio

#endif // __VSDXTHEME_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
