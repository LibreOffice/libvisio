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

#ifndef VSDXTYPES_H
#define VSDXTYPES_H

#include <libwpd/libwpd.h>

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
};

// Utilities
struct ChunkHeader
{
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
  Colour(unsigned red, unsigned char green, unsigned char blue, unsigned char alpha)
    : r(red), g(green), b(blue), a(alpha) {}
  Colour() : r(0), g(0), b(0), a(0) {}
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
  NURBSData &operator=(const NURBSData &data)
  {
    lastKnot = data.lastKnot;
    degree = data.degree;
    xType = data.xType;
    yType = data.yType;
    knots = data.knots;
    weights = data.weights;
    points = data.points;
    return *this;
  }
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
  PolylineData(const PolylineData &data)
    : xType(data.xType),
      yType(data.yType),
      points(data.points) {}
  PolylineData &operator=(const PolylineData &data)
  {
    xType = data.xType;
    yType = data.yType;
    points = data.points;
    return *this;
  }
};


struct ForeignData
{
  unsigned typeId;
  unsigned dataId;
  unsigned typeLevel;
  unsigned dataLevel;
  unsigned type;
  unsigned format;
  double offsetX;
  double offsetY;
  double width;
  double height;
  WPXBinaryData data;
  ForeignData()
    : typeId(0),
      dataId(0),
      typeLevel(0),
      dataLevel(0),
      type(0),
      format(0),
      offsetX(0.0),
      offsetY(0.0),
      width(0.0),
      height(0.0),
      data() {}
  ForeignData(const ForeignData &fd)
    : typeId(fd.typeId),
      dataId(fd.dataId),
      typeLevel(fd.typeLevel),
      dataLevel(fd.dataLevel),
      type(fd.type),
      format(fd.format),
      offsetX(fd.offsetX),
      offsetY(fd.offsetY),
      width(fd.width),
      height(fd.height),
      data(fd.data) {}
};

enum TextFormat
{
  VSD_TEXT_ANSI = 0,
  VSD_TEXT_GREEK,
  VSD_TEXT_TURKISH,
  VSD_TEXT_VIETNAMESE,
  VSD_TEXT_HEBREW,
  VSD_TEXT_ARABIC,
  VSD_TEXT_BALTIC,
  VSD_TEXT_RUSSIAN,
  VSD_TEXT_THAI,
  VSD_TEXT_CENTRAL_EUROPE,
  VSD_TEXT_UTF16
};

class VSDXName
{
public:
  VSDXName(const WPXBinaryData &data, TextFormat format)
    : m_data(data),
      m_format(format) {}
  VSDXName() : m_data(), m_format(VSD_TEXT_ANSI) {}
  VSDXName(const VSDXName &element)
    : m_data(element.m_data),
      m_format(element.m_format) {}
  VSDXName &operator=(const VSDXName &element)
  {
    m_data = element.m_data;
    m_format = element.m_format;
    return *this;
  }
  WPXBinaryData m_data;
  TextFormat m_format;
};

} // namespace libvisio

#endif /* VSDXTYPES_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
