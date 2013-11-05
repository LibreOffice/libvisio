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

#ifndef VSDTYPES_H
#define VSDTYPES_H

#include <vector>
#include <librevenge/librevenge.h>

#define FROM_OPTIONAL(t, u) !!t ? t.get() : u
#define ASSIGN_OPTIONAL(t, u) if(!!t) u = t.get()
#define MINUS_ONE (unsigned)-1

namespace libvisio
{
struct XForm
{
  double pinX;
  double pinY;
  double height;
  double width;
  double pinLocX;
  double pinLocY;
  double angle;
  bool flipX;
  bool flipY;
  double x;
  double y;
  XForm() : pinX(0.0), pinY(0.0), height(0.0), width(0.0),
    pinLocX(0.0), pinLocY(0.0), angle(0.0),
    flipX(false), flipY(false), x(0.0), y(0.0) {}
  XForm(const XForm &xform) : pinX(xform.pinX), pinY(xform.pinY), height(xform.height),
    width(xform.width), pinLocX(xform.pinLocX), pinLocY(xform.pinLocY), angle(xform.angle),
    flipX(xform.flipX), flipY(xform.flipY), x(xform.x), y(xform.y) {}

};

// Utilities
struct ChunkHeader
{
  ChunkHeader() : chunkType(0), id(0), list(0), dataLength(0), level(0), unknown(0), trailer(0) {}
  unsigned chunkType;  // 4 bytes
  unsigned id;         // 4 bytes
  unsigned list;       // 4 bytes
  unsigned dataLength; // 4 bytes
  unsigned short level;      // 2 bytes
  unsigned char unknown;    // 1 byte
  unsigned trailer; // Derived
};

struct Colour
{
  Colour(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
    : r(red), g(green), b(blue), a(alpha) {}
  Colour() : r(0), g(0), b(0), a(0) {}
  inline bool operator==(const Colour &col)
  {
    return ((r == col.r) && (g == col.g) && (b == col.b) && (a == col.a));
  }
  inline bool operator!=(const Colour &col)
  {
    return !operator==(col);
  }
  inline bool operator!()
  {
    return (!r && !g && !b && !a);
  }
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
};

struct NURBSData
{
  double lastKnot;
  unsigned degree;
  unsigned char xType;
  unsigned char yType;
  std::vector<double> knots;
  std::vector<double> weights;
  std::vector<std::pair<double, double> > points;
  NURBSData()
    : lastKnot(0.0),
      degree(0),
      xType(0x00),
      yType(0x00),
      knots(),
      weights(),
      points() {}
  NURBSData(const NURBSData &data)
    : lastKnot(data.lastKnot),
      degree(data.degree),
      xType(data.xType),
      yType(data.yType),
      knots(data.knots),
      weights(data.weights),
      points(data.points) {}
};

struct PolylineData
{
  unsigned char xType;
  unsigned char yType;
  std::vector<std::pair<double, double> > points;
  PolylineData()
    : xType(0x00),
      yType(0x00),
      points() {}
};


struct ForeignData
{
  unsigned typeId;
  unsigned dataId;
  unsigned type;
  unsigned format;
  double offsetX;
  double offsetY;
  double width;
  double height;
  librevenge::RVNGBinaryData data;
  ForeignData()
    : typeId(0),
      dataId(0),
      type(0),
      format(0),
      offsetX(0.0),
      offsetY(0.0),
      width(0.0),
      height(0.0),
      data() {}
};

enum TextFormat
{
  VSD_TEXT_ANSI = 0,
  VSD_TEXT_SYMBOL,
  VSD_TEXT_GREEK,
  VSD_TEXT_TURKISH,
  VSD_TEXT_VIETNAMESE,
  VSD_TEXT_HEBREW,
  VSD_TEXT_ARABIC,
  VSD_TEXT_BALTIC,
  VSD_TEXT_RUSSIAN,
  VSD_TEXT_THAI,
  VSD_TEXT_CENTRAL_EUROPE,
  VSD_TEXT_JAPANESE,
  VSD_TEXT_KOREAN,
  VSD_TEXT_CHINESE_SIMPLIFIED,
  VSD_TEXT_CHINESE_TRADITIONAL,
  VSD_TEXT_UTF8,
  VSD_TEXT_UTF16
};

class VSDName
{
public:
  VSDName(const librevenge::RVNGBinaryData &data, TextFormat format)
    : m_data(data),
      m_format(format) {}
  VSDName() : m_data(), m_format(VSD_TEXT_ANSI) {}
  VSDName(const VSDName &name) : m_data(name.m_data), m_format(name.m_format) {}
  bool empty() const
  {
    return !m_data.size();
  }
  librevenge::RVNGBinaryData m_data;
  TextFormat m_format;
};

struct VSDFont
{
  librevenge::RVNGString m_name;
  TextFormat m_encoding;
  VSDFont() : m_name("Arial"), m_encoding(libvisio::VSD_TEXT_ANSI) {}
  VSDFont(const librevenge::RVNGString &name, const TextFormat &encoding) :
    m_name(name), m_encoding(encoding) {}
  VSDFont(const VSDFont &font) :
    m_name(font.m_name), m_encoding(font.m_encoding) {}
};

struct VSDMisc
{
  bool m_hideText;
  VSDMisc() : m_hideText(false) {}
  VSDMisc(const VSDMisc &misc) : m_hideText(misc.m_hideText) {}
};

} // namespace libvisio

#endif /* VSDTYPES_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
