/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VSDTYPES_H
#define VSDTYPES_H

#include <vector>
#include <map>
#include <librevenge/librevenge.h>

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
  XForm(const XForm &xform) = default;
  XForm &operator=(const XForm &xform) = default;
};

struct XForm1D
{
  double beginX;
  double beginY;
  unsigned beginId;
  double endX;
  double endY;
  unsigned endId;
  XForm1D() : beginX(0.0), beginY(0.0), beginId(MINUS_ONE),
    endX(0.0), endY(0.0), endId(MINUS_ONE) {}
  XForm1D(const XForm1D &xform1d) : beginX(xform1d.beginX),
    beginY(xform1d.beginY), beginId(xform1d.beginId),
    endX(xform1d.endX), endY(xform1d.endY), endId(xform1d.endId) {}
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
  inline bool operator==(const Colour &col) const
  {
    return ((r == col.r) && (g == col.g) && (b == col.b) && (a == col.a));
  }
  inline bool operator!=(const Colour &col) const
  {
    return !operator==(col);
  }
  inline bool operator!() const
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
  NURBSData(const NURBSData &data) = default;
  NURBSData &operator=(const NURBSData &data) = default;
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
  VSDName(const VSDName &name) = default;
  VSDName &operator=(const VSDName &name) = default;
  bool empty() const
  {
    return !m_data.size();
  }
  void clear()
  {
    m_data.clear();
    m_format = VSD_TEXT_ANSI;
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
  VSDMisc(const VSDMisc &misc) = default;
  VSDMisc &operator=(const VSDMisc &misc) = default;
};

struct VSDTabStop
{
  double m_position;
  unsigned char m_alignment;
  unsigned char m_leader;
  VSDTabStop() : m_position(0.0), m_alignment(0), m_leader(0) {}
  VSDTabStop(const VSDTabStop &tabStop) :
    m_position(tabStop.m_position), m_alignment(tabStop.m_alignment),
    m_leader(tabStop.m_leader) {}
};

struct VSDTabSet
{
  unsigned m_numChars;
  std::map<unsigned, VSDTabStop> m_tabStops;
  VSDTabSet() : m_numChars(0), m_tabStops() {}
  VSDTabSet(const VSDTabSet &tabSet) :
    m_numChars(tabSet.m_numChars), m_tabStops(tabSet.m_tabStops) {}
};

struct VSDBullet
{
  librevenge::RVNGString m_bulletStr;
  librevenge::RVNGString m_bulletFont;
  double m_bulletFontSize;
  double m_textPosAfterBullet;
  VSDBullet()
    : m_bulletStr(),
      m_bulletFont(),
      m_bulletFontSize(0.0),
      m_textPosAfterBullet(0.0) {}
  VSDBullet(const VSDBullet &bullet) = default;
  VSDBullet &operator=(const VSDBullet &bullet) = default;
  inline bool operator==(const VSDBullet &bullet) const
  {
    return ((m_bulletStr == bullet.m_bulletStr) &&
            (m_bulletFont == bullet.m_bulletFont) &&
            (m_bulletFontSize == bullet.m_bulletFontSize) &&
            (m_textPosAfterBullet == bullet.m_textPosAfterBullet));
  }
  inline bool operator!=(const VSDBullet &bullet) const
  {
    return !operator==(bullet);
  }
  inline bool operator!() const
  {
    return m_bulletStr.empty();
  }
};

} // namespace libvisio

#endif /* VSDTYPES_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
