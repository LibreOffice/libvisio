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

#ifndef __VSDXPARSER_H__
#define __VSDXPARSER_H__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>

namespace libvisio
{

typedef struct _VSDXForm VSDXForm;
class VSDXParser
{
public:
  explicit VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter);
  virtual ~VSDXParser();
  virtual bool parse() = 0;
protected:
  WPXInputStream *m_input;
  libwpg::WPGPaintInterface *m_painter;
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

  bool m_isPageStarted;
  double m_pageWidth;
  double m_pageHeight;
  double m_scale;
  double m_x;
  double m_y;
  XForm m_xform;
  ChunkHeader m_header;
  std::vector<unsigned> m_currentGeometryOrder;
  std::map<unsigned, WPXPropertyList> m_currentGeometry;
  std::map<unsigned, WPXPropertyListVector> m_currentComplexGeometry;
  std::map<unsigned, XForm> m_groupXForms;
  WPXBinaryData m_currentForeignData;
  WPXPropertyList m_currentForeignProps;
  unsigned m_currentShapeId;
  unsigned m_foreignType;
  unsigned m_foreignFormat;
  WPXPropertyList m_styleProps;
  ::WPXString m_lineColour;
  ::WPXString m_fillType;
  unsigned m_linePattern;
  unsigned m_fillPattern;
  WPXPropertyListVector m_gradientProps;
  bool m_noLine;
  bool m_noFill;
  bool m_noShow;
};

} // namespace libvisio

#endif // __VSDXPARSER_H__
