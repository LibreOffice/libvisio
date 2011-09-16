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
  unsigned r;
  unsigned g;
  unsigned b;
  unsigned a;
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
  CharFormat() :
    charCount(0),
    faceID(0),
    colour(),
    langID(0),
    size(12.0/72.0),
    bold(false),
    italic(false),
    underline(false),
    doubleunderline(false),
    strikeout(false),
    doublestrikeout(false),
    allcaps(false),
    initcaps(false),
    smallcaps(false),
    superscript(false),
    subscript(false),
    face("Times New Roman") {}
  CharFormat(unsigned cc, unsigned short id, Colour c, unsigned lang, double s, bool b, bool i, bool u, bool du, bool so, bool dso, bool ac, bool ic, bool sc, bool super, bool sub, WPXString f) :
    charCount(cc),
    faceID(id),
    colour(c),
    langID(lang),
    size(s),
    bold(b),
    italic(i),
    underline(u),
    doubleunderline(du),
    strikeout(so),
    doublestrikeout(dso),
    allcaps(ac),
    initcaps(ic),
    smallcaps(sc),
    superscript(super),
    subscript(sub),
    face(f) {}
  unsigned charCount;
  unsigned short faceID;
  Colour colour;
  unsigned langID;
  double size;
  bool bold;
  bool italic;
  bool underline;
  bool doubleunderline;
  bool strikeout;
  bool doublestrikeout;
  bool allcaps;
  bool initcaps;
  bool smallcaps;
  bool superscript;
  bool subscript;
  WPXString face;
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
