/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
 * Copyright (C) 2011 Eilidh McAdam <tibbylickle@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02111-1301 USA
 */

#ifndef VSDXTYPES_H
#define VSDXTYPES_H

namespace libvisio {
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
  unsigned level;      // 2 bytes
  unsigned unknown;    // 1 byte
  unsigned trailer; // Derived
};

struct Colour
{
  Colour(unsigned red, unsigned green, unsigned blue, unsigned alpha)
  : r(red), g(green), b(blue), a(alpha) {}
  Colour() : r(0), g(0), b(0), a(0xff) {}
  unsigned int r;
  unsigned int g;
  unsigned int b;
  unsigned int a;
};

struct NURBSData
{
  double lastKnot;
  unsigned degree;
  unsigned xType;
  unsigned yType;
  std::vector<double> knots;
  std::vector<double> weights;
  std::vector<std::pair<double, double> > points;
};

struct PolylineData
{
  unsigned xType;
  unsigned yType;
  std::vector<std::pair<double, double> > points;
};

struct CharFormat
{
  unsigned charCount;
  double fontSize;
  bool bold;
  bool italic;
  bool underline;
  WPXString fontFace;
};

struct ForeignData
{
  unsigned typeId;
  unsigned dataId;
  unsigned typeLevel;
  unsigned dataLevel;
  unsigned type;
  unsigned format;
  WPXBinaryData data;
};

enum TextFormat { VSD_TEXT_ANSI, VSD_TEXT_UTF16 };

} // namespace libvisio

#endif /* VSDXTYPES_H */
