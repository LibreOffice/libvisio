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

#include <string.h> // for memcpy
#include <stack>
#include <boost/spirit/include/classic.hpp>

#include "VSDContentCollector.h"
#include "VSDParser.h"
#include "VSDInternalStream.h"

#define DUMP_BITMAP 0

#if DUMP_BITMAP
static unsigned bitmapId = 0;
#include <sstream>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SURROGATE_VALUE(h,l) (((h) - 0xd800) * 0x400 + (l) - 0xdc00 + 0x10000)

namespace
{

static void _appendUCS4(WPXString &text, unsigned ucs4Character)
{
  unsigned char first;
  int len;
  if (ucs4Character < 0x80)
  {
    first = 0;
    len = 1;
  }
  else if (ucs4Character < 0x800)
  {
    first = 0xc0;
    len = 2;
  }
  else if (ucs4Character < 0x10000)
  {
    first = 0xe0;
    len = 3;
  }
  else if (ucs4Character < 0x200000)
  {
    first = 0xf0;
    len = 4;
  }
  else if (ucs4Character < 0x4000000)
  {
    first = 0xf8;
    len = 5;
  }
  else
  {
    first = 0xfc;
    len = 6;
  }

  unsigned char outbuf[6] = { 0, 0, 0, 0, 0, 0 };
  int i;
  for (i = len - 1; i > 0; --i)
  {
    outbuf[i] = (ucs4Character & 0x3f) | 0x80;
    ucs4Character >>= 6;
  }
  outbuf[0] = (ucs4Character & 0xff) | first;

  for (i = 0; i < len; i++)
    text.append(outbuf[i]);
}

} // anonymous namespace


libvisio::VSDContentCollector::VSDContentCollector(
  libwpg::WPGPaintInterface *painter,
  std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
  std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
  std::vector<std::list<unsigned> > &documentPageShapeOrders,
  VSDStyles &styles, VSDStencils &stencils
) :
  m_painter(painter), m_isPageStarted(false), m_pageWidth(0.0), m_pageHeight(0.0),
  m_shadowOffsetX(0.0), m_shadowOffsetY(0.0),
  m_scale(1.0), m_x(0.0), m_y(0.0), m_originalX(0.0), m_originalY(0.0), m_xform(),
  m_txtxform(0), m_currentFillGeometry(), m_currentLineGeometry(), m_groupXForms(groupXFormsSequence.empty() ? 0 : &groupXFormsSequence[0]),
  m_currentForeignData(), m_currentOLEData(), m_currentForeignProps(),
  m_currentShapeId(0), m_foreignType(0), m_foreignFormat(0), m_foreignOffsetX(0.0),
  m_foreignOffsetY(0.0), m_foreignWidth(0.0), m_foreignHeight(0.0), m_styleProps(),
  m_lineColour("black"), m_fillType("none"), m_linePattern(1),
  m_fillPattern(1), m_fillFGTransparency(0), m_fillBGTransparency(0),
  m_noLine(false), m_noFill(false), m_noShow(false), m_fonts(),
  m_currentLevel(0), m_isShapeStarted(false), m_groupMemberships(groupMembershipsSequence[0]),
  m_groupXFormsSequence(groupXFormsSequence), m_groupMembershipsSequence(groupMembershipsSequence),
  m_currentPageNumber(0), m_shapeOutputDrawing(0), m_shapeOutputText(0),
  m_pageOutputDrawing(), m_pageOutputText(), m_documentPageShapeOrders(documentPageShapeOrders),
  m_pageShapeOrder(documentPageShapeOrders[0]), m_isFirstGeometry(true), m_NURBSData(), m_polylineData(),
  m_textStream(), m_names(), m_stencilNames(), m_fields(), m_stencilFields(), m_fieldIndex(0),
  m_textFormat(VSD_TEXT_ANSI), m_charFormats(), m_paraFormats(), m_lineStyle(), m_fillStyle(),
  m_textBlockStyle(), m_defaultCharStyle(), m_defaultParaStyle(), m_styles(styles),
  m_stencils(stencils), m_stencilShape(0), m_isStencilStarted(false), m_currentGeometryCount(0),
  m_backgroundPageID(MINUS_ONE), m_currentPageID(0), m_currentPage(), m_pages(),
  m_splineControlPoints(), m_splineKnotVector(), m_splineX(0.0), m_splineY(0.0),
  m_splineLastKnot(0.0), m_splineDegree(0), m_splineLevel(0), m_currentShapeLevel(0),
  m_isBackgroundPage(false)
{
}

void libvisio::VSDContentCollector::_fillAndShadowProperties(const Colour &colourFG, const Colour &colourBG, unsigned fillPattern,
    double fillFGTransparency, double fillBGTransparency, unsigned shadowPattern, const Colour &shfgc, double shadowOffsetX, double shadowOffsetY)
{
  m_fillPattern = fillPattern;
  m_fillFGTransparency = fillFGTransparency;
  m_fillBGTransparency = fillBGTransparency;

  if (m_fillPattern)
    m_styleProps.insert("svg:fill-rule", "evenodd");

  if (!m_fillPattern)
    m_fillType = "none";
  else if (m_fillPattern == 1)
  {
    m_fillType = "solid";
    m_styleProps.insert("draw:fill-color", getColourString(colourFG));
    if (m_fillFGTransparency > 0)
      m_styleProps.insert("draw:opacity", 1 - m_fillFGTransparency, WPX_PERCENT);
    else
      m_styleProps.remove("draw:opacity");
  }
  else if (m_fillPattern == 26 || m_fillPattern == 29)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:style", "axial");
    m_styleProps.insert("draw:start-color", getColourString(colourFG));
    m_styleProps.insert("draw:end-color", getColourString(colourBG));
    m_styleProps.remove("draw:opacity");
    if (m_fillBGTransparency > 0.0)
      m_styleProps.insert("libwpg:start-opacity", 1 - m_fillBGTransparency, WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:start-opacity", 1, WPX_PERCENT);
    if (m_fillFGTransparency > 0.0)
      m_styleProps.insert("libwpg:end-opacity", 1 - m_fillFGTransparency, WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:end-opacity", 1, WPX_PERCENT);
    m_styleProps.insert("draw:border", 0, WPX_PERCENT);

    if (m_fillPattern == 26)
      m_styleProps.insert("draw:angle", 90);
    else
      m_styleProps.insert("draw:angle", 0);
  }
  else if (m_fillPattern >= 25 && m_fillPattern <= 34)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:style", "linear");
    m_styleProps.insert("draw:start-color", getColourString(colourBG));
    m_styleProps.insert("draw:end-color", getColourString(colourFG));
    m_styleProps.remove("draw:opacity");
    if (m_fillBGTransparency > 0)
      m_styleProps.insert("libwpg:start-opacity", 1 - m_fillBGTransparency, WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:start-opacity", 1, WPX_PERCENT);
    if (m_fillFGTransparency > 0)
      m_styleProps.insert("libwpg:end-opacity", 1 - m_fillFGTransparency, WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:end-opacity", 1, WPX_PERCENT);
    m_styleProps.insert("draw:border", 0, WPX_PERCENT);

    switch(m_fillPattern)
    {
    case 25:
      m_styleProps.insert("draw:angle", 270);
      break;
    case 27:
      m_styleProps.insert("draw:angle", 90);
      break;
    case 28:
      m_styleProps.insert("draw:angle", 180);
      break;
    case 30:
      m_styleProps.insert("draw:angle", 0);
      break;
    case 31:
      m_styleProps.insert("draw:angle", 225);
      break;
    case 32:
      m_styleProps.insert("draw:angle", 135);
      break;
    case 33:
      m_styleProps.insert("draw:angle", 315);
      break;
    case 34:
      m_styleProps.insert("draw:angle", 45);
      break;
    }
  }
  else if (m_fillPattern == 35)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:style", "rectangular");
    m_styleProps.insert("svg:cx", 0.5, WPX_PERCENT);
    m_styleProps.insert("svg:cy", 0.5, WPX_PERCENT);
    m_styleProps.insert("draw:start-color", getColourString(colourBG));
    m_styleProps.insert("draw:end-color", getColourString(colourFG));
    m_styleProps.remove("draw:opacity");
    if (m_fillBGTransparency > 0)
      m_styleProps.insert("libwpg:start-opacity", 1 - m_fillBGTransparency, WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:start-opacity", 1, WPX_PERCENT);
    if (m_fillFGTransparency > 0)
      m_styleProps.insert("libwpg:end-opacity", 1 - m_fillFGTransparency, WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:end-opacity", 1, WPX_PERCENT);
    m_styleProps.insert("draw:angle", 0);
    m_styleProps.insert("draw:border", 0, WPX_PERCENT);
  }
  else if (m_fillPattern >= 36 && m_fillPattern <= 40)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:style", "radial");
    m_styleProps.insert("draw:start-color", getColourString(colourBG));
    m_styleProps.insert("draw:end-color", getColourString(colourFG));
    m_styleProps.remove("draw:opacity");
    if (m_fillBGTransparency > 0)
      m_styleProps.insert("libwpg:start-opacity", 1 - m_fillBGTransparency, WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:start-opacity", 1, WPX_PERCENT);
    if (m_fillFGTransparency > 0)
      m_styleProps.insert("libwpg:end-opacity", 1 - m_fillFGTransparency, WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:end-opacity", 1, WPX_PERCENT);
    m_styleProps.insert("draw:border", 0, WPX_PERCENT);

    switch(m_fillPattern)
    {
    case 36:
      m_styleProps.insert("svg:cx", 0, WPX_PERCENT);
      m_styleProps.insert("svg:cy", 0, WPX_PERCENT);
      break;
    case 37:
      m_styleProps.insert("svg:cx", 1, WPX_PERCENT);
      m_styleProps.insert("svg:cy", 0, WPX_PERCENT);
      break;
    case 38:
      m_styleProps.insert("svg:cx", 0, WPX_PERCENT);
      m_styleProps.insert("svg:cy", 1, WPX_PERCENT);
      break;
    case 39:
      m_styleProps.insert("svg:cx", 1, WPX_PERCENT);
      m_styleProps.insert("svg:cy", 1, WPX_PERCENT);
      break;
    case 40:
      m_styleProps.insert("svg:cx", 0.5, WPX_PERCENT);
      m_styleProps.insert("svg:cy", 0.5, WPX_PERCENT);
      break;
    }
  }
  else
    // fill types we don't handle right, but let us approximate with solid fill
  {
    m_fillType = "solid";
    m_styleProps.insert("draw:fill-color", getColourString(colourBG));
  }

  if (shadowPattern != 0)
  {
    m_styleProps.insert("draw:shadow","visible"); // for ODG
    m_styleProps.insert("draw:shadow-offset-x",shadowOffsetX != 0.0 ? shadowOffsetX : m_shadowOffsetX);
    m_styleProps.insert("draw:shadow-offset-y",shadowOffsetY != 0.0 ? shadowOffsetY : m_shadowOffsetY);
    m_styleProps.insert("draw:shadow-color",getColourString(shfgc));
    m_styleProps.insert("draw:shadow-opacity",(double)(1 - shfgc.a/255.), WPX_PERCENT);
  }
  m_styleProps.insert("draw:fill", m_fillType);
}

void libvisio::VSDContentCollector::_applyLinePattern()
{
  int dots1 = 0;
  int dots2 = 0;
  double dots1len = 0.0;
  double dots2len = 0.0;
  double gap = 0.0;

  m_styleProps.remove("draw:stroke");
  switch (m_linePattern)
  {
  case 2: // "6, 3"
    dots1 = dots2 = 1;
    dots1len = dots2len = 6.0;
    gap = 3.0;
    break;
  case 3: // "1, 3"
    dots1 = dots2 = 1;
    dots1len = dots2len = 1.0;
    gap = 3.0;
    break;
  case 4: // "6, 3, 1, 3"
    dots1 = 1;
    dots1len = 6.0;
    dots2 = 1;
    dots2len = 1.0;
    gap = 3.0;
    break;
  case 5: // "6, 3, 1, 3, 1, 3"
    dots1 = 1;
    dots1len = 6.0;
    dots2 = 2;
    dots2len = 1.0;
    gap = 3.0;
    break;
  case 6: // "6, 3, 6, 3, 1, 3"
    dots1 = 2;
    dots1len = 6.0;
    dots2 = 1;
    dots2len = 1.0;
    gap = 3.0;
    break;
  case 7: // "14, 2, 6, 2"
    dots1 = 1;
    dots1len = 14.0;
    dots2 = 1;
    dots2len = 6.0;
    gap = 2.0;
    break;
  case 8: // "14, 2, 6, 2, 6, 2"
    dots1 = 1;
    dots1len = 14.0;
    dots2 = 2;
    dots2len = 6.0;
    gap = 2.0;
    break;
  case 9: // "3, 2"
    dots1 = dots2 = 1;
    dots1len = dots2len = 3.0;
    gap = 2.0;
    break;
  case 10: // "1, 2"
    dots1 = dots2 = 1;
    dots1len = dots2len = 1.0;
    gap = 2.0;
    break;
  case 11: // "3, 2, 1, 2"
    dots1 = 1;
    dots1len = 3.0;
    dots2 = 1;
    dots2len = 1.0;
    gap = 2.0;
    break;
  case 12: // "3, 2, 1, 2, 1, 2"
    dots1 = 1;
    dots1len = 3.0;
    dots2 = 2;
    dots2len = 1.0;
    gap = 2.0;
    break;
  case 13: // "3, 2, 3, 2, 1, 2"
    dots1 = 2;
    dots1len = 3.0;
    dots2 = 1;
    dots2len = 1.0;
    gap = 2.0;
    break;
  case 14: // "7, 2, 3, 2"
    dots1 = 1;
    dots1len = 7.0;
    dots2 = 1;
    dots2len = 3.0;
    gap = 2.0;
    break;
  case 15: // "7, 2, 3, 2, 3, 2"
    dots1 = 1;
    dots1len = 7.0;
    dots2 = 2;
    dots2len = 3.0;
    gap = 2.0;
    break;
  case 16: // "11, 5"
    dots1 = dots2 = 1;
    dots1len = dots2len = 11.0;
    gap = 5.0;
    break;
  case 17: // "1, 5"
    dots1 = dots2 = 1;
    dots1len = dots2len = 1.0;
    gap = 5.0;
    break;
  case 18: // "11, 5, 1, 5"
    dots1 = 1;
    dots1len = 11.0;
    dots2 = 1;
    dots2len = 1.0;
    gap = 5.0;
    break;
  case 19: // "11, 5, 1, 5, 1, 5"
    dots1 = 1;
    dots1len = 11.0;
    dots2 = 2;
    dots2len = 1.0;
    gap = 5.0;
    break;
  case 20: // "11, 5, 11, 5, 1, 5"
    dots1 = 2;
    dots1len = 11.0;
    dots2 = 1;
    dots2len = 1.0;
    gap = 5.0;
    break;
  case 21: // "27, 5, 11, 5"
    dots1 = 1;
    dots1len = 27.0;
    dots2 = 1;
    dots2len = 11.0;
    gap = 5.0;
    break;
  case 22: // "27, 5, 11, 5, 11, 5"
    dots1 = 1;
    dots1len = 27.0;
    dots2 = 2;
    dots2len = 11.0;
    gap = 5.0;
    break;
  case 23: // "2, 2"
    dots1 = dots2 = 1;
    dots1len = dots2len = 2.0;
    gap = 2.0;
    break;
  default:
    break;
  }

  if (m_linePattern == 0)
    m_styleProps.insert("draw:stroke", "none");
  else if (m_linePattern == 1)
    m_styleProps.insert("draw:stroke", "solid");
  else if (m_linePattern > 1 && m_linePattern <= 23)
  {
    m_styleProps.insert("draw:stroke", "dash");
    m_styleProps.insert("draw:dots1", dots1);
    m_styleProps.insert("draw:dots1-length", dots1len, WPX_PERCENT);
    m_styleProps.insert("draw:dots2", dots2);
    m_styleProps.insert("draw:dots2-length", dots2len, WPX_PERCENT);
    m_styleProps.insert("draw:distance", gap, WPX_PERCENT);
  }
  else
    // FIXME: later it will require special treatment for custom line patterns
    // patt ID is 0xfe, link to stencil name is in 'Line' blocks
    m_styleProps.insert("draw:stroke", "solid");
}

void libvisio::VSDContentCollector::_lineProperties(double strokeWidth, Colour c, unsigned linePattern, unsigned startMarker, unsigned endMarker, unsigned lineCap)
{
  m_linePattern = linePattern;

  if (!linePattern)
  {
    m_styleProps.insert("draw:stroke", "none");
    return;
  }

  m_styleProps.insert("svg:stroke-width", m_scale*strokeWidth);
  m_lineColour = getColourString(c);
  m_styleProps.insert("svg:stroke-color", m_lineColour);
  if (c.a)
    m_styleProps.insert("svg:stroke-opacity", (1 - c.a/255.0), WPX_PERCENT);
  else
    m_styleProps.insert("svg:stroke-opacity", 1.0, WPX_PERCENT);
  switch (lineCap)
  {
  case 0:
    m_styleProps.insert("svg:stroke-linecap", "round");
    m_styleProps.insert("svg:stroke-linejoin", "round");
    break;
  case 2:
    m_styleProps.insert("svg:stroke-linecap", "square");
    m_styleProps.insert("svg:stroke-linejoin", "miter");
    break;
  default:
    m_styleProps.insert("svg:stroke-linecap", "butt");
    m_styleProps.insert("svg:stroke-linejoin", "miter");
    break;
  }

  _applyLinePattern();

  // Deal with line markers (arrows, etc.)
  if (startMarker > 0)
  {
    m_styleProps.insert("draw:marker-start-viewbox", _linePropertiesMarkerViewbox(startMarker));
    m_styleProps.insert("draw:marker-start-path", _linePropertiesMarkerPath(startMarker));
    m_styleProps.insert("draw:marker-start-width", m_scale*_linePropertiesMarkerScale(startMarker)*(0.1/(strokeWidth*strokeWidth+1)+2.54*strokeWidth));
  }
  if (endMarker > 0)
  {
    m_styleProps.insert("draw:marker-end-viewbox", _linePropertiesMarkerViewbox(endMarker));
    m_styleProps.insert("draw:marker-end-path", _linePropertiesMarkerPath(endMarker));
    m_styleProps.insert("draw:marker-end-width", m_scale*_linePropertiesMarkerScale(endMarker)*(0.1/(strokeWidth*strokeWidth+1)+2.54*strokeWidth));
  }
}

const char *libvisio::VSDContentCollector::_linePropertiesMarkerViewbox(unsigned marker)
{
  switch (marker)
  {
  case 1:
  case 2:
  case 9:
  case 15:
    return "0 0 20 10";
  case 8:
    return "0 0 20 18";
  case 3:
  case 4:
  case 5:
  case 6:
  case 11:
  case 16:
  case 17:
  case 18:
    return "0 0 20 20";
  case 12:
  case 13:
  case 14:
    return "0 0 20 30";
  case 22:
  case 39:
    return "0 0 20 40";
  case 21:
    return "0 0 30 30";
  case 10:
    return "0 0 1131 1131";
  default:
    return "0 0 20 30";
  }
}

const char *libvisio::VSDContentCollector::_linePropertiesMarkerPath(unsigned marker)
{
  switch (marker)
  {
  case 1:
    return "m10 -4l-14 14l4 4l10 -10l10 10l4 -4z";
  case 2:
    return "m10 0-10 10h20z";
  case 3:
    return "m10 -8l-14 28l6 3l8 -16l8 16l6 -3z";
  case 4:
    return "m10 0-10 20h20z";
  case 5:
    return "m10 0-10 20q10,-5 20,0z";
  case 6:
    return "m10 0-10 20q10,5 20,0z";
  case 8:
    return "m10 0q-2.6,13.4 -10,18q10,-5 20,0q-7.4,-4.6 -10,-18";
  case 9:
    return "m-2 -8l4 -4l20 20l-4 4z";
  case 10: // Copied from what LO exports when using the "circle" marker
    return "m462 1118-102-29-102-51-93-72-72-93-51-102-29-102-13-105 13-102 29-106 51-102 72-89 93-72 102-50 102-34 106-9 101 9 106 34 98 50 93 72 72 89 51 102 29 106 13 102-13 105-29 102-51 102-72 93-93 72-98 51-106 29-101 13z";
  case 11:
    return "m0 0v10h10v-10z";
  case 12:
    return "m10 -12l-14 42l9 3l5 -15l5 15l9 -3z";
  case 13:
    return "m10 0-10 30h20z";
  case 14:
    return "m10 0-10 30h20z m0 12l-5 15h10z";
  case 15:
    return "m10 0-10 10h20z m0 3l-5 5h10z";
  case 16:
    return "m10 0-10 20h20z m0 7l-5 10h10z";
  case 17:
    return "m10 0-10 20q10,-5 20,0z m0 7l-4 8q4,-2 8,0z";
  case 18:
    return "m10 0-10 20q10,5 20,0z m0 7l-5 10q5,2 10,0z";
  case 21:
    return "m0 0v30h30v-30z m10 10v10h10v-10z";
  case 22:
    return "m10 0-10 20l10 20l10 -20z m0 8l-6 12l6 12l6 -12z";
  case 39:
    return "m10 0-10 20h20z m0 20-10 20h20z";
  default:
    return "m10 0-10 30h20z";
  }
}

double libvisio::VSDContentCollector::_linePropertiesMarkerScale(unsigned marker)
{
  switch (marker)
  {
  case 11:
  case 10:
    return 0.7;
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 22:
    return 1.2;
  default:
    return 1.0;
  }
}

void libvisio::VSDContentCollector::_flushCurrentPath()
{
  m_styleProps.clear();
  _lineProperties(m_lineStyle);
  _fillAndShadowProperties(m_fillStyle);
  WPXPropertyList fillPathProps(m_styleProps);
  fillPathProps.insert("draw:stroke", "none");
  WPXPropertyList linePathProps(m_styleProps);
  linePathProps.insert("draw:fill", "none");
  bool needsGroup = true;

  if (!m_styleProps["draw:fill"] || m_styleProps["draw:fill"]->getStr() == "none")
    needsGroup = false;
  if (m_currentFillGeometry.empty())
    needsGroup = false;
  if (!m_styleProps["draw:stroke"] || m_styleProps["draw:stroke"]->getStr() == "none")
    needsGroup = false;
  if (m_currentLineGeometry.empty())
    needsGroup = false;

  if (needsGroup)
    m_shapeOutputDrawing->addStartLayer(WPXPropertyList());

  std::vector<WPXPropertyList> tmpPath;
  if (m_styleProps["draw:fill"] && m_styleProps["draw:fill"]->getStr() != "none")
  {
    bool firstPoint = true;
    bool wasMove = false;
    for (unsigned i = 0; i < m_currentFillGeometry.size(); i++)
    {
      if (firstPoint)
      {
        firstPoint = false;
        wasMove = true;
      }
      else if (m_currentFillGeometry[i]["libwpg:path-action"]->getStr() == "M")
      {
        if (!tmpPath.empty())
        {
          if (!wasMove)
          {
            if (tmpPath.back()["libwpg:path-action"]->getStr() != "Z")
            {
              WPXPropertyList closedPath;
              closedPath.insert("libwpg:path-action", "Z");
              tmpPath.push_back(closedPath);
            }
          }
          else
          {
            tmpPath.pop_back();
          }
        }
        wasMove = true;
      }
      else
        wasMove = false;
      tmpPath.push_back(m_currentFillGeometry[i]);
    }
    if (!tmpPath.empty())
    {
      if (!wasMove)
      {
        if (tmpPath.back()["libwpg:path-action"]->getStr() != "Z")
        {
          WPXPropertyList closedPath;
          closedPath.insert("libwpg:path-action", "Z");
          tmpPath.push_back(closedPath);
        }
      }
      else
        tmpPath.pop_back();
    }
    if (!tmpPath.empty())
    {
      WPXPropertyListVector path;
      for (unsigned i = 0; i < tmpPath.size(); ++i)
        path.append(tmpPath[i]);
      m_shapeOutputDrawing->addStyle(fillPathProps, WPXPropertyListVector());
      m_shapeOutputDrawing->addPath(path);
    }
  }
  m_currentFillGeometry.clear();
  tmpPath.clear();

  if (m_styleProps["draw:stroke"] && m_styleProps["draw:stroke"]->getStr() != "none")
  {
    bool firstPoint = true;
    bool wasMove = false;
    double x = 0.0;
    double y = 0.0;
    double prevX = 0.0;
    double prevY = 0.0;
    for (unsigned i = 0; i < m_currentLineGeometry.size(); i++)
    {
      if (firstPoint)
      {
        firstPoint = false;
        wasMove = true;
        x = m_currentLineGeometry[i]["svg:x"]->getDouble();
        y = m_currentLineGeometry[i]["svg:y"]->getDouble();
      }
      else if (m_currentLineGeometry[i]["libwpg:path-action"]->getStr() == "M")
      {
        if (!tmpPath.empty())
        {
          if (!wasMove)
          {
            if ((x == prevX) && (y == prevY))
            {
              if (tmpPath.back()["libwpg:path-action"]->getStr() != "Z")
              {
                WPXPropertyList closedPath;
                closedPath.insert("libwpg:path-action", "Z");
                tmpPath.push_back(closedPath);
              }
            }
          }
          else
          {
            tmpPath.pop_back();
          }
        }
        x = m_currentLineGeometry[i]["svg:x"]->getDouble();
        y = m_currentLineGeometry[i]["svg:y"]->getDouble();
        wasMove = true;
      }
      else
        wasMove = false;
      tmpPath.push_back(m_currentLineGeometry[i]);
      if (m_currentLineGeometry[i]["svg:x"])
        prevX = m_currentLineGeometry[i]["svg:x"]->getDouble();
      if (m_currentLineGeometry[i]["svg:y"])
        prevY = m_currentLineGeometry[i]["svg:y"]->getDouble();
    }
    if (!tmpPath.empty())
    {
      if (!wasMove)
      {
        if ((x == prevX) && (y == prevY))
        {
          if (tmpPath.back()["libwpg:path-action"]->getStr() != "Z")
          {
            WPXPropertyList closedPath;
            closedPath.insert("libwpg:path-action", "Z");
            tmpPath.push_back(closedPath);
          }
        }
      }
      else
      {
        tmpPath.pop_back();
      }
    }
    if (!tmpPath.empty())
    {
      WPXPropertyListVector path;
      for (unsigned i = 0; i < tmpPath.size(); ++i)
        path.append(tmpPath[i]);
      m_shapeOutputDrawing->addStyle(linePathProps, WPXPropertyListVector());
      m_shapeOutputDrawing->addPath(path);
    }
  }
  m_currentLineGeometry.clear();

  if (needsGroup)
    m_shapeOutputDrawing->addEndLayer();
}

void libvisio::VSDContentCollector::_flushText()
{
  if (!m_textStream.size()) return;

  double xmiddle = m_txtxform ? m_txtxform->width / 2.0 : m_xform.width / 2.0;
  double ymiddle = m_txtxform ? m_txtxform->height / 2.0 : m_xform.height / 2.0;

  transformPoint(xmiddle,ymiddle, m_txtxform);

  double x = xmiddle - (m_txtxform ? m_txtxform->width / 2.0 : m_xform.width / 2.0);
  double y = ymiddle - (m_txtxform ? m_txtxform->height / 2.0 : m_xform.height / 2.0);

  double angle = 0.0;
  transformAngle(angle, m_txtxform);

  WPXPropertyList textBlockProps;

  bool flipX = false;
  bool flipY = false;
  transformFlips(flipX, flipY);

  if (flipX)
    angle -= M_PI;

  while (angle > M_PI)
    angle -= 2 * M_PI;
  while (angle < -M_PI)
    angle += 2 * M_PI;

  textBlockProps.insert("svg:x", m_scale * x);
  textBlockProps.insert("svg:y", m_scale * y);
  textBlockProps.insert("svg:height", m_scale * (m_txtxform ? m_txtxform->height : m_xform.height));
  textBlockProps.insert("svg:width", m_scale * (m_txtxform ? m_txtxform->width : m_xform.width));
  textBlockProps.insert("fo:padding-top", m_textBlockStyle.topMargin);
  textBlockProps.insert("fo:padding-bottom", m_textBlockStyle.bottomMargin);
  textBlockProps.insert("fo:padding-left", m_textBlockStyle.leftMargin);
  textBlockProps.insert("fo:padding-right", m_textBlockStyle.rightMargin);
  textBlockProps.insert("libwpg:rotate", angle*180/M_PI, WPX_GENERIC);

  switch (m_textBlockStyle.verticalAlign)
  {
  case 0: // Top
    textBlockProps.insert("draw:textarea-vertical-align", "top");
    break;
  case 2: // Bottom
    textBlockProps.insert("draw:textarea-vertical-align", "bottom");
    break;
  default: // Center
    textBlockProps.insert("draw:textarea-vertical-align", "middle");
    break;
  }

  if (m_charFormats.empty())
    m_charFormats.push_back(m_defaultCharStyle);
  if (m_paraFormats.empty())
    m_paraFormats.push_back(m_defaultParaStyle);

  unsigned numCharsInText =  (unsigned)(m_textFormat == VSD_TEXT_UTF16 ? m_textStream.size() / 2 : m_textStream.size());

  for (unsigned iChar = 0; iChar < m_charFormats.size(); iChar++)
  {
    if (m_charFormats[iChar].charCount)
      numCharsInText -= m_charFormats[iChar].charCount;
    else
      m_charFormats[iChar].charCount = numCharsInText;
  }

  numCharsInText =  (unsigned)(m_textFormat == VSD_TEXT_UTF16 ? m_textStream.size() / 2 : m_textStream.size());

  for (unsigned iPara = 0; iPara < m_paraFormats.size(); iPara++)
  {
    if (m_paraFormats[iPara].charCount)
      numCharsInText -= m_paraFormats[iPara].charCount;
    else
      m_paraFormats[iPara].charCount = numCharsInText;
  }

  m_shapeOutputText->addStartTextObject(textBlockProps, WPXPropertyListVector());

  unsigned int charIndex = 0;
  unsigned int paraCharCount = 0;
  unsigned long textBufferPosition = 0;
  const unsigned char *pTextBuffer = m_textStream.getDataBuffer();

  for (std::vector<VSDParaStyle>::iterator paraIt = m_paraFormats.begin();
       paraIt < m_paraFormats.end() || charIndex < m_charFormats.size(); ++paraIt)
  {
    WPXPropertyList paraProps;

    paraProps.insert("fo:text-indent", (*paraIt).indFirst);
    paraProps.insert("fo:margin-left", (*paraIt).indLeft);
    paraProps.insert("fo:margin-right", (*paraIt).indRight);
    paraProps.insert("fo:margin-top", (*paraIt).spBefore);
    paraProps.insert("fo:margin-bottom", (*paraIt).spAfter);
    switch ((*paraIt).align)
    {
    case 0: // left
      if (!(*paraIt).flags)
        paraProps.insert("fo:text-align", "left");
      else
        paraProps.insert("fo:text-align", "end");
      break;
    case 2: // right
      if (!(*paraIt).flags)
        paraProps.insert("fo:text-align", "end");
      else
        paraProps.insert("fo:text-align", "left");
      break;
    case 3: // justify
      paraProps.insert("fo:text-align", "justify");
      break;
    case 4: // full
      paraProps.insert("fo:text-align", "full");
      break;
    default: // center
      paraProps.insert("fo:text-align", "center");
      break;
    }
    if ((*paraIt).spLine > 0)
      paraProps.insert("fo:line-height", (*paraIt).spLine, WPX_POINT);
    else
      paraProps.insert("fo:line-height", -(*paraIt).spLine, WPX_PERCENT);

    m_shapeOutputText->addStartTextLine(paraProps);

    paraCharCount = (*paraIt).charCount;

    TextFormat encoding = libvisio::VSD_TEXT_ANSI;
    // Find char format that overlaps
    while (charIndex < m_charFormats.size() && paraCharCount)
    {
      paraCharCount -= m_charFormats[charIndex].charCount;

      WPXPropertyList textProps;
      if (m_fonts[m_charFormats[charIndex].faceID].name == "")
      {
        textProps.insert("style:font-name", m_charFormats[charIndex].face.name);
        encoding = m_charFormats[charIndex].face.encoding;
      }
      else
      {
        textProps.insert("style:font-name", m_fonts[m_charFormats[charIndex].faceID].name);
        encoding = m_fonts[m_charFormats[charIndex].faceID].encoding;
      }

      if (m_charFormats[charIndex].bold) textProps.insert("fo:font-weight", "bold");
      if (m_charFormats[charIndex].italic) textProps.insert("fo:font-style", "italic");
      if (m_charFormats[charIndex].underline) textProps.insert("style:text-underline-type", "single");
      if (m_charFormats[charIndex].doubleunderline) textProps.insert("style:text-underline-type", "double");
      if (m_charFormats[charIndex].strikeout) textProps.insert("style:text-line-through-type", "single");
      if (m_charFormats[charIndex].doublestrikeout) textProps.insert("style:text-line-through-type", "double");
      if (m_charFormats[charIndex].allcaps) textProps.insert("fo:text-transform", "uppercase");
      if (m_charFormats[charIndex].initcaps) textProps.insert("fo:text-transform", "capitalize");
      if (m_charFormats[charIndex].smallcaps) textProps.insert("fo:font-variant", "small-caps");
      if (m_charFormats[charIndex].superscript) textProps.insert("style:text-position", "super");
      if (m_charFormats[charIndex].subscript) textProps.insert("style:text-position", "sub");
      textProps.insert("fo:font-size", m_charFormats[charIndex].size*72.0, WPX_POINT);
      textProps.insert("fo:color", getColourString(m_charFormats[charIndex].colour));
      double opacity = 1.0;
      if (m_charFormats[charIndex].colour.a)
        opacity -= (double)(m_charFormats[charIndex].colour.a)/255.0;
      textProps.insert("svg:stroke-opacity", opacity, WPX_PERCENT);
      textProps.insert("svg:fill-opacity", opacity, WPX_PERCENT);
      // TODO: In draw, text span background cannot be specified the same way as in writer span
      if (m_textBlockStyle.isTextBkgndFilled)
      {
        textProps.insert("fo:background-color", getColourString(m_textBlockStyle.textBkgndColour));
#if 0
        if (m_textBlockStyle.textBkgndColour.a)
          textProps.insert("fo:background-opacity", 1.0 - m_textBlockStyle.textBkgndColour.a/255.0, WPX_PERCENT);
#endif
      }

      WPXString text;

      if (m_textFormat == VSD_TEXT_UTF16)
      {
        unsigned long max = m_charFormats[charIndex].charCount <= (m_textStream.size()/2) ? m_charFormats[charIndex].charCount : (m_textStream.size()/2);
        VSD_DEBUG_MSG(("Charcount: %d, max: %lu, stream size: %lu\n", m_charFormats[charIndex].charCount, max, (unsigned long)m_textStream.size()));
        max = (m_charFormats[charIndex].charCount == 0 && m_textStream.size()) ? m_textStream.size()/2 : max;
        VSD_DEBUG_MSG(("Charcount: %d, max: %lu, stream size: %lu\n", m_charFormats[charIndex].charCount, max, (unsigned long)m_textStream.size()));
        std::vector<unsigned char> tmpBuffer;
        for (unsigned i = 0; i < max*2; ++i)
          tmpBuffer.push_back(pTextBuffer[textBufferPosition+i]);
        if (!paraCharCount && tmpBuffer.size() >= 2)
        {
          while (tmpBuffer.size() >= 2 && tmpBuffer[tmpBuffer.size() - 2] == 0 && tmpBuffer[tmpBuffer.size() - 1] == 0)
          {
            tmpBuffer.pop_back();
            tmpBuffer.pop_back();
          }
          if (tmpBuffer.size() >= 2)
          {
            tmpBuffer[tmpBuffer.size() - 2] = 0;
            tmpBuffer[tmpBuffer.size() - 1] = 0;
          }
          else
            tmpBuffer.clear();
        }

        if (!tmpBuffer.empty())
          appendCharacters(text, tmpBuffer);
        textBufferPosition += max*2;
      }
      else
      {
        unsigned long max = m_charFormats[charIndex].charCount <= m_textStream.size() ? m_charFormats[charIndex].charCount : m_textStream.size();
        max = (m_charFormats[charIndex].charCount == 0 && m_textStream.size()) ? m_textStream.size() : max;
        std::vector<unsigned char> tmpBuffer;
        for (unsigned i = 0; i < max; ++i)
          tmpBuffer.push_back(pTextBuffer[textBufferPosition+i]);
        if (!paraCharCount && !tmpBuffer.empty())
        {
          while (!tmpBuffer.empty() && tmpBuffer.back() == 0)
            tmpBuffer.pop_back();
          if (!tmpBuffer.empty() && (tmpBuffer.back() == 0x0a || tmpBuffer.back() == '\n' || tmpBuffer.back() == 0x0e))
            tmpBuffer.back() = 0;
        }
        if (!tmpBuffer.empty())
          appendCharacters(text, tmpBuffer, encoding);
        textBufferPosition += max;
      }

      VSD_DEBUG_MSG(("Text: %s\n", text.cstr()));
      m_shapeOutputText->addStartTextSpan(textProps);
      m_shapeOutputText->addInsertText(text);
      m_shapeOutputText->addEndTextSpan();

      charIndex++;
      if (charIndex < m_charFormats.size() && paraCharCount && m_charFormats[charIndex].charCount > paraCharCount)
      {
        // Insert duplicate
        std::vector<VSDCharStyle>::iterator charIt = m_charFormats.begin() + charIndex;
        m_charFormats.insert(charIt, m_charFormats[charIndex]);
        m_charFormats[charIndex].charCount = paraCharCount;
        m_charFormats[charIndex+1].charCount -= paraCharCount;
      }
    }
    m_shapeOutputText->addEndTextLine();
  }

  m_shapeOutputText->addEndTextObject();
}

void libvisio::VSDContentCollector::_flushCurrentForeignData()
{
  double xmiddle = m_foreignOffsetX + m_foreignWidth / 2.0;
  double ymiddle = m_foreignOffsetY + m_foreignHeight / 2.0;

  transformPoint(xmiddle, ymiddle);

  bool flipX = false;
  bool flipY = false;

  transformFlips(flipX, flipY);

  WPXPropertyList styleProps;

  m_currentForeignProps.insert("svg:x", m_scale*(xmiddle - (m_foreignWidth / 2.0)));
  m_currentForeignProps.insert("svg:width", m_scale*m_foreignWidth);
  m_currentForeignProps.insert("svg:y", m_scale*(ymiddle - (m_foreignHeight / 2.0)));
  m_currentForeignProps.insert("svg:height", m_scale*m_foreignHeight);

  double angle = 0.0;
  transformAngle(angle);

  if (flipX)
  {
    m_currentForeignProps.insert("draw:mirror-horizontal", true);
    angle = M_PI - angle;
  }
  if (flipY)
  {
    m_currentForeignProps.insert("draw:mirror-vertical", true);
    angle *= -1.0;
  }

  if (angle != 0.0)
    m_currentForeignProps.insert("libwpg:rotate", angle * 180 / M_PI, WPX_GENERIC);

  if (m_currentForeignData.size() && m_currentForeignProps["libwpg:mime-type"] && m_foreignWidth != 0.0 && m_foreignHeight != 0.0)
  {
    m_shapeOutputDrawing->addStyle(styleProps, WPXPropertyListVector());
    m_shapeOutputDrawing->addGraphicObject(m_currentForeignProps, m_currentForeignData);
  }
  m_currentForeignData.clear();
  m_currentForeignProps.clear();
}

void libvisio::VSDContentCollector::_flushCurrentPage()
{
  if (!m_pageShapeOrder.empty())
  {
    std::stack<std::pair<unsigned, VSDOutputElementList> > groupTextStack;
    for (std::list<unsigned>::iterator iterList = m_pageShapeOrder.begin(); iterList != m_pageShapeOrder.end(); ++iterList)
    {
      std::map<unsigned, unsigned>::iterator iterGroup = m_groupMemberships.find(*iterList);
      if (iterGroup == m_groupMemberships.end())
      {
        while (!groupTextStack.empty())
        {
          m_currentPage.append(groupTextStack.top().second);
          groupTextStack.pop();
        }
      }
      else if (!groupTextStack.empty() && iterGroup->second != groupTextStack.top().first)
      {
        while (!groupTextStack.empty() && groupTextStack.top().first != iterGroup->second)
        {
          m_currentPage.append(groupTextStack.top().second);
          groupTextStack.pop();
        }
      }

      std::map<unsigned, VSDOutputElementList>::iterator iter;
      iter = m_pageOutputDrawing.find(*iterList);
      if (iter != m_pageOutputDrawing.end())
        m_currentPage.append(iter->second);
      iter = m_pageOutputText.find(*iterList);
      if (iter != m_pageOutputText.end())
        groupTextStack.push(std::make_pair(*iterList, iter->second));
      else
        groupTextStack.push(std::make_pair(*iterList, VSDOutputElementList()));
    }
    while (!groupTextStack.empty())
    {
      m_currentPage.append(groupTextStack.top().second);
      groupTextStack.pop();
    }
  }
  m_pageOutputDrawing.clear();
  m_pageOutputText.clear();
}

#define LIBVISIO_EPSILON 1E-10
void libvisio::VSDContentCollector::collectEllipticalArcTo(unsigned /* id */, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc)
{
  _handleLevelChange(level);

  m_originalX = x3;
  m_originalY = y3;
  transformPoint(x2, y2);
  transformPoint(x3, y3);
  transformAngle(angle);

  double x1 = m_x*cos(angle) + m_y*sin(angle);
  double y1 = ecc*(m_y*cos(angle) - m_x*sin(angle));
  double x2n = x2*cos(angle) + y2*sin(angle);
  double y2n = ecc*(y2*cos(angle) -x2*sin(angle));
  double x3n = x3*cos(angle) + y3*sin(angle);
  double y3n = ecc*(y3*cos(angle) - x3*sin(angle));

  m_x = x3;
  m_y = y3;

  if (fabs(((x1-x2n)*(y2n-y3n) - (x2n-x3n)*(y1-y2n))) <= LIBVISIO_EPSILON || fabs(((x2n-x3n)*(y1-y2n) - (x1-x2n)*(y2n-y3n))) <= LIBVISIO_EPSILON)
    // most probably all of the points lie on the same line, so use lineTo instead
  {
    WPXPropertyList end;
    end.insert("svg:x", m_scale*m_x);
    end.insert("svg:y", m_scale*m_y);
    end.insert("libwpg:path-action", "L");
    if (!m_noFill && !m_noShow)
      m_currentFillGeometry.push_back(end);
    if (!m_noLine && !m_noShow)
      m_currentLineGeometry.push_back(end);
    return;
  }

  double x0 = ((x1-x2n)*(x1+x2n)*(y2n-y3n) - (x2n-x3n)*(x2n+x3n)*(y1-y2n) +
               (y1-y2n)*(y2n-y3n)*(y1-y3n)) /
              (2*((x1-x2n)*(y2n-y3n) - (x2n-x3n)*(y1-y2n)));
  double y0 = ((x1-x2n)*(x2n-x3n)*(x1-x3n) + (x2n-x3n)*(y1-y2n)*(y1+y2n) -
               (x1-x2n)*(y2n-y3n)*(y2n+y3n)) /
              (2*((x2n-x3n)*(y1-y2n) - (x1-x2n)*(y2n-y3n)));

  VSD_DEBUG_MSG(("Centre: (%f,%f), angle %f\n", x0, y0, angle));

  double rx = sqrt(pow(x1-x0, 2) + pow(y1-y0, 2));
  double ry = rx / ecc;
  WPXPropertyList arc;
  int largeArc = 0;
  int sweep = 1;

  // Calculate side of chord that ellipse centre and control point fall on
  double centreSide = (x3n-x1)*(y0-y1) - (y3n-y1)*(x0-x1);
  double midSide = (x3n-x1)*(y2n-y1) - (y3n-y1)*(x2n-x1);
  // Large arc if centre and control point are on the same side
  if ((centreSide > 0 && midSide > 0) || (centreSide < 0 && midSide < 0))
    largeArc = 1;
  // Change direction depending of side of control point
  if (midSide > 0)
    sweep = 0;

  arc.insert("svg:rx", m_scale*rx);
  arc.insert("svg:ry", m_scale*ry);
  arc.insert("libwpg:rotate", angle * 180 / M_PI, WPX_GENERIC);
  arc.insert("libwpg:large-arc", largeArc);
  arc.insert("libwpg:sweep", sweep);
  arc.insert("svg:x", m_scale*m_x);
  arc.insert("svg:y", m_scale*m_y);
  arc.insert("libwpg:path-action", "A");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(arc);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(arc);
}

void libvisio::VSDContentCollector::collectEllipse(unsigned /* id */, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop)
{
  _handleLevelChange(level);
  WPXPropertyList ellipse;
  double angle = fmod(2.0*M_PI + (cy > yleft ? 1.0 : -1.0)*acos((cx-xleft) / sqrt((xleft - cx)*(xleft - cx) + (yleft - cy)*(yleft - cy))), 2.0*M_PI);
  transformPoint(cx, cy);
  transformPoint(xleft, yleft);
  transformPoint(xtop, ytop);
  transformAngle(angle);

  double rx = sqrt((xleft - cx)*(xleft - cx) + (yleft - cy)*(yleft - cy));
  double ry = sqrt((xtop - cx)*(xtop - cx) + (ytop - cy)*(ytop - cy));

  int largeArc = 0;
  double centreSide = (xleft-xtop)*(cy-ytop) - (yleft-ytop)*(cx-xtop);
  if (centreSide > 0)
  {
    largeArc = 1;
  }
  ellipse.insert("svg:x",m_scale*xleft);
  ellipse.insert("svg:y",m_scale*yleft);
  ellipse.insert("libwpg:path-action", "M");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(ellipse);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(ellipse);
  ellipse.insert("svg:rx",m_scale*rx);
  ellipse.insert("svg:ry",m_scale*ry);
  ellipse.insert("svg:x",m_scale*xtop);
  ellipse.insert("svg:y",m_scale*ytop);
  ellipse.insert("libwpg:large-arc", largeArc?1:0);
  ellipse.insert("libwpg:path-action", "A");
  ellipse.insert("libwpg:rotate", angle * 180/M_PI, WPX_GENERIC);
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(ellipse);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(ellipse);
  ellipse.insert("svg:x",m_scale*xleft);
  ellipse.insert("svg:y",m_scale*yleft);
  ellipse.insert("libwpg:large-arc", largeArc?0:1);
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(ellipse);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(ellipse);
  ellipse.clear();
  ellipse.insert("libwpg:path-action", "Z");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(ellipse);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(ellipse);
}

void libvisio::VSDContentCollector::collectInfiniteLine(unsigned /* id */, unsigned level, double x1, double y1, double x2, double y2)
{
  _handleLevelChange(level);
  transformPoint(x1, y1);
  transformPoint(x2, y2);

  double xmove = 0.0;
  double ymove = 0.0;
  double xline = 0.0;
  double yline = 0.0;

  if (x1 == x2)
  {
    xmove = x1;
    ymove = 0;
    xline = x1;
    yline = m_pageHeight;
  }
  else if (y1 == y2)
  {
    xmove = 0;
    ymove = y1;
    xline = m_pageWidth;
    yline = y1;
  }
  else
  {
    // coming from equation: y = p*x + q => x = y/p - q/p

    double p = (y1-y2)/(x1-x2);
    double q = (x1*y2 - x2*y1)/(x1-x2);
    std::map<double, double> points;

    // compute intersection with left border of the page
    double x = 0.0;
    double y = p*x + q;
    if (y <= m_pageHeight && y >= 0) // line intersects the left border inside the viewport
      points[x] = y;

    // compute intersection with right border of the page
    x = m_pageWidth;
    y = p*x + q;
    if (y <= m_pageHeight && y >= 0) // line intersects the right border inside the viewport
      points[x] = y;

    // compute intersection with top border of the page
    y = 0.0;
    x = y/p - q/p;
    if (x <= m_pageWidth && x >= 0)
      points[x] = y;

    // compute intersection with bottom border of the page
    y = m_pageHeight;
    x = y/p - q/p;
    if (x <= m_pageWidth && x >= 0)
      points[x] = y;

    if (!points.empty())
    {
      xmove = points.begin()->first;
      ymove = points.begin()->second;
      for (std::map<double, double>::iterator iter = points.begin(); iter != points.end(); ++iter)
      {
        if (iter->first != xmove || iter->second != ymove)
        {
          xline = iter->first;
          yline = iter->second;
        }
      }
    }
  }

  WPXPropertyList infLine;
  infLine.insert("svg:x",m_scale*xmove);
  infLine.insert("svg:y",m_scale*ymove);
  infLine.insert("libwpg:path-action", "M");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(infLine);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(infLine);
  infLine.insert("svg:x",m_scale*xline);
  infLine.insert("svg:y",m_scale*yline);
  infLine.insert("libwpg:path-action", "L");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(infLine);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(infLine);
}

void libvisio::VSDContentCollector::collectRelCubBezTo(unsigned /* id */, unsigned level, double x, double y, double a, double b, double c, double d)
{
  _handleLevelChange(level);
  x *= m_xform.width;
  y *= m_xform.height;
  a *= m_xform.width;
  b *= m_xform.height;
  c *= m_xform.width;
  d *= m_xform.height;
  transformPoint(x, y);
  transformPoint(a, b);
  transformPoint(c, d);
  WPXPropertyList node;
  node.insert("svg:x",m_scale*x);
  node.insert("svg:y",m_scale*x);
  node.insert("svg:x1",m_scale*a);
  node.insert("svg:y1",m_scale*b);
  node.insert("svg:x2",m_scale*c);
  node.insert("svg:y2",m_scale*d);
  node.insert("libwpg:path-action", "C");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(node);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(node);
}

void libvisio::VSDContentCollector::collectRelEllipticalArcTo(unsigned id, unsigned level, double x, double y, double a, double b, double c, double d)
{
  x *= m_xform.width;
  y *= m_xform.height;
  a *= m_xform.width;
  b *= m_xform.height;
  collectEllipticalArcTo(id, level, x, y, a, b, c, d);
}

void libvisio::VSDContentCollector::collectRelLineTo(unsigned id, unsigned level, double x, double y)
{
  x *= m_xform.width;
  y *= m_xform.height;
  collectLineTo(id, level, x, y);
}

void libvisio::VSDContentCollector::collectRelMoveTo(unsigned id, unsigned level, double x, double y)
{
  x *= m_xform.width;
  y *= m_xform.height;
  collectMoveTo(id, level, x, y);
}

void libvisio::VSDContentCollector::collectRelQuadBezTo(unsigned /* id */, unsigned level, double x, double y, double a, double b)
{
  _handleLevelChange(level);
  _handleLevelChange(level);
  x *= m_xform.width;
  y *= m_xform.height;
  a *= m_xform.width;
  b *= m_xform.height;
  transformPoint(x, y);
  transformPoint(a, b);
  WPXPropertyList node;
  node.insert("svg:x",m_scale*x);
  node.insert("svg:y",m_scale*x);
  node.insert("svg:x1",m_scale*a);
  node.insert("svg:y1",m_scale*b);
  node.insert("libwpg:path-action", "Q");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(node);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(node);
}

void libvisio::VSDContentCollector::collectLine(unsigned level, const boost::optional<double> &strokeWidth, const boost::optional<Colour> &c, const boost::optional<unsigned char> &linePattern,
    const boost::optional<unsigned char> &startMarker, const boost::optional<unsigned char> &endMarker, const boost::optional<unsigned char> &lineCap)
{
  _handleLevelChange(level);
  m_lineStyle.override(VSDOptionalLineStyle(strokeWidth, c, linePattern, startMarker, endMarker, lineCap));
}

void libvisio::VSDContentCollector::collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
    const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency, const boost::optional<double> &fillBGTransparency,
    const boost::optional<unsigned char> &shadowPattern, const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX,
    const boost::optional<double> &shadowOffsetY)
{
  _handleLevelChange(level);
  m_fillStyle.override(VSDOptionalFillStyle(colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shfgc, shadowPattern, shadowOffsetX, shadowOffsetY));
}

void libvisio::VSDContentCollector::collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
    const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency, const boost::optional<double> &fillBGTransparency,
    const boost::optional<unsigned char> &shadowPattern, const boost::optional<Colour> &shfgc)
{
  collectFillAndShadow(level, colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc, m_shadowOffsetX, m_shadowOffsetY);
}

void libvisio::VSDContentCollector::collectForeignData(unsigned level, const WPXBinaryData &binaryData)
{
  _handleLevelChange(level);
  _handleForeignData(binaryData);
}

void libvisio::VSDContentCollector::collectOLEList(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
  m_currentForeignData.clear();
  WPXBinaryData binaryData;
  _handleForeignData(binaryData);
}

void libvisio::VSDContentCollector::collectOLEData(unsigned /* id */, unsigned level, const WPXBinaryData &oleData)
{
  _handleLevelChange(level);
  m_currentForeignData.append(oleData);
}

void libvisio::VSDContentCollector::_handleForeignData(const WPXBinaryData &binaryData)
{
  if (m_foreignType == 1 || m_foreignType == 4) // Image
  {
    m_currentForeignData.clear();
    // If bmp data found, reconstruct header
    if (m_foreignType == 1 && m_foreignFormat == 0)
    {
      m_currentForeignData.append(0x42);
      m_currentForeignData.append(0x4d);

      m_currentForeignData.append((unsigned char)((binaryData.size() + 14) & 0x000000ff));
      m_currentForeignData.append((unsigned char)(((binaryData.size() + 14) & 0x0000ff00) >> 8));
      m_currentForeignData.append((unsigned char)(((binaryData.size() + 14) & 0x00ff0000) >> 16));
      m_currentForeignData.append((unsigned char)(((binaryData.size() + 14) & 0xff000000) >> 24));

      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);

      m_currentForeignData.append(0x36);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
    }
    m_currentForeignData.append(binaryData);

#if DUMP_BITMAP
    if (m_foreignType == 1 || m_foreignType == 4)
    {
      ::WPXString filename;
      switch(m_foreignFormat)
      {
      case 0:
      case 255:
        filename.sprintf("binarydump%i.bmp", bitmapId++);
        break;
      case 1:
        filename.sprintf("binarydump%i.jpeg", bitmapId++);
        break;
      case 2:
        filename.sprintf("binarydump%i.gif", bitmapId++);
        break;
      case 3:
        filename.sprintf("binarydump%i.tiff", bitmapId++);
        break;
      case 4:
        filename.sprintf("binarydump%i.png", bitmapId++);
        break;
      default:
        filename.sprintf("binarydump%i.bin", bitmapId++);
        break;
      }
      FILE *f = fopen(filename.cstr(), "wb");
      if (f)
      {
        const unsigned char *tmpBuffer = m_currentForeignData.getDataBuffer();
        for (unsigned long k = 0; k < m_currentForeignData.size(); k++)
          fprintf(f, "%c",tmpBuffer[k]);
        fclose(f);
      }
    }
#endif

    if (m_foreignType == 1)
    {
      switch(m_foreignFormat)
      {
      case 0:
      case 255:
        m_currentForeignProps.insert("libwpg:mime-type", "image/bmp");
        break;
      case 1:
        m_currentForeignProps.insert("libwpg:mime-type", "image/jpeg");
        break;
      case 2:
        m_currentForeignProps.insert("libwpg:mime-type", "image/gif");
        break;
      case 3:
        m_currentForeignProps.insert("libwpg:mime-type", "image/tiff");
        break;
      case 4:
        m_currentForeignProps.insert("libwpg:mime-type", "image/png");
        break;
      }
    }
    else if (m_foreignType == 4)
    {
      const unsigned char *tmpBinData = m_currentForeignData.getDataBuffer();
      // Check for EMF signature
      if (tmpBinData[0x28] == 0x20 && tmpBinData[0x29] == 0x45 && tmpBinData[0x2A] == 0x4D && tmpBinData[0x2B] == 0x46)
      {
        m_currentForeignProps.insert("libwpg:mime-type", "image/emf");
      }
      else
      {
        m_currentForeignProps.insert("libwpg:mime-type", "image/wmf");
      }
    }
  }
  else if (m_foreignType == 2)
  {
    m_currentForeignProps.insert("libwpg:mime-type", "object/ole");
    m_currentForeignData.append(binaryData);
  }
}

void libvisio::VSDContentCollector::collectGeometry(unsigned /* id */, unsigned level, bool noFill, bool noLine, bool noShow)
{
  _handleLevelChange(level);
  m_x = 0.0;
  m_y = 0.0;
  m_originalX = 0.0;
  m_originalY = 0.0;
  m_noFill = noFill;
  m_noLine = noLine;
  m_noShow = noShow;

  VSD_DEBUG_MSG(("NoFill: %d NoLine: %d NoShow: %d\n", m_noFill, m_noLine, m_noShow));
  m_currentGeometryCount++;
}

void libvisio::VSDContentCollector::collectMoveTo(unsigned /* id */, unsigned level, double x, double y)
{
  _handleLevelChange(level);
  m_originalX = x;
  m_originalY = y;
  transformPoint(x, y);
  m_x = x;
  m_y = y;
  WPXPropertyList end;
  end.insert("svg:x", m_scale*m_x);
  end.insert("svg:y", m_scale*m_y);
  end.insert("libwpg:path-action", "M");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(end);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(end);
}

void libvisio::VSDContentCollector::collectLineTo(unsigned /* id */, unsigned level, double x, double y)
{
  _handleLevelChange(level);
  m_originalX = x;
  m_originalY = y;
  transformPoint(x, y);
  m_x = x;
  m_y = y;
  WPXPropertyList end;
  end.insert("svg:x", m_scale*m_x);
  end.insert("svg:y", m_scale*m_y);
  end.insert("libwpg:path-action", "L");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(end);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(end);
}

void libvisio::VSDContentCollector::collectArcTo(unsigned /* id */, unsigned level, double x2, double y2, double bow)
{
  _handleLevelChange(level);
  m_originalX = x2;
  m_originalY = y2;
  transformPoint(x2, y2);
  double angle = 0.0;
  transformAngle(angle);

  if (bow == 0)
  {
    m_x = x2;
    m_y = y2;
    WPXPropertyList end;
    end.insert("svg:x", m_scale*m_x);
    end.insert("svg:y", m_scale*m_y);
    end.insert("libwpg:path-action", "L");
    if (!m_noFill && !m_noShow)
      m_currentFillGeometry.push_back(end);
    if (!m_noLine && !m_noShow)
      m_currentLineGeometry.push_back(end);
  }
  else
  {
    WPXPropertyList arc;
    double chord = sqrt(pow((y2 - m_y),2) + pow((x2 - m_x),2));
    double radius = (4 * bow * bow + chord * chord) / (8 * fabs(bow));
    int largeArc = fabs(bow) > radius ? 1 : 0;
    bool sweep = (bow < 0);
    transformFlips(sweep, sweep);

    m_x = x2;
    m_y = y2;
    arc.insert("svg:rx", m_scale*radius);
    arc.insert("svg:ry", m_scale*radius);
    arc.insert("libwpg:rotate", angle*180/M_PI, WPX_GENERIC);
    arc.insert("libwpg:large-arc", largeArc);
    arc.insert("libwpg:sweep", sweep);
    arc.insert("svg:x", m_scale*m_x);
    arc.insert("svg:y", m_scale*m_y);
    arc.insert("libwpg:path-action", "A");
    if (!m_noFill && !m_noShow)
      m_currentFillGeometry.push_back(arc);
    if (!m_noLine && !m_noShow)
      m_currentLineGeometry.push_back(arc);
  }
}

#define VSD_NUM_POLYLINES_PER_NURBS 200

void libvisio::VSDContentCollector::collectNURBSTo(unsigned /* id */, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights)
{
  _handleLevelChange(level);

  if (!knotVector.size() || !controlPoints.size() || !weights.size())
    // Here, maybe we should just draw line to (x2,y2)
    return;

  // Fill in end knots
  while (knotVector.size() < (controlPoints.size() + degree + 2))
  {
    double tmpBack = knotVector.back();
    knotVector.push_back(tmpBack);
  }

  // Convert control points to static co-ordinates
  for (std::vector<std::pair<double, double> >::iterator it = controlPoints.begin();
       it != controlPoints.end(); ++it)
  {
    if (xType == 0) // Percentage
      (*it).first *= m_xform.width;

    if (yType == 0) // Percentage
      (*it).second *= m_xform.height;
  }

  controlPoints.push_back(std::pair<double,double>(x2, y2));
  controlPoints.insert(controlPoints.begin(), std::pair<double, double>(m_originalX, m_originalY));

  // Generate NURBS using VSD_NUM_POLYLINES_PER_NURBS polylines
  WPXPropertyList NURBS;
  double step = (knotVector.back() - knotVector[0]) / VSD_NUM_POLYLINES_PER_NURBS;

  for (unsigned i = 0; i < VSD_NUM_POLYLINES_PER_NURBS; i++)
  {
    NURBS.clear();
    NURBS.insert("libwpg:path-action", "L");
    double nextX = 0;
    double nextY = 0;
    double denominator = 1E-10;

    for (unsigned p = 0; p < controlPoints.size() && p < weights.size(); p++)
    {
      double basis = _NURBSBasis(p, degree, knotVector[0] + i * step, knotVector);
      nextX += basis * controlPoints[p].first * weights[p];
      nextY += basis * controlPoints[p].second * weights[p];
      denominator += weights[p] * basis;
    }
    nextX = (nextX/denominator);
    nextY = (nextY/denominator);
    transformPoint(nextX, nextY);
    NURBS.insert("svg:x", m_scale*nextX);
    NURBS.insert("svg:y", m_scale*nextY);
    if (!m_noFill && !m_noShow)
      m_currentFillGeometry.push_back(NURBS);
    if (!m_noLine && !m_noShow)
      m_currentLineGeometry.push_back(NURBS);
  }

  m_originalX = x2;
  m_originalY = y2;
  m_x = x2;
  m_y = y2;
  transformPoint(m_x, m_y);
  NURBS.clear();
  NURBS.insert("libwpg:path-action", "L");
  NURBS.insert("svg:x", m_scale*m_x);
  NURBS.insert("svg:y", m_scale*m_y);
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(NURBS);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(NURBS);
}

double libvisio::VSDContentCollector::_NURBSBasis(unsigned knot, unsigned degree, double point, const std::vector<double> &knotVector)
{
  double basis = 0;
  if (!knotVector.size())
    return basis;
  if (degree == 0)
  {
    if (knotVector[knot] <= point && point < knotVector[knot+1])
      return 1;
    else
      return 0;
  }
  if (knotVector.size() > knot+degree && knotVector[knot+degree]-knotVector[knot] > 0)
    basis = (point-knotVector[knot])/(knotVector[knot+degree]-knotVector[knot]) * _NURBSBasis(knot, degree-1, point, knotVector);

  if (knotVector.size() > knot+degree+1 && knotVector[knot+degree+1] - knotVector[knot+1] > 0)
    basis += (knotVector[knot+degree+1]-point)/(knotVector[knot+degree+1]-knotVector[knot+1]) * _NURBSBasis(knot+1, degree-1, point, knotVector);

  return basis;
}

/* NURBS with incomplete data */
void libvisio::VSDContentCollector::collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID)
{
  std::map<unsigned, NURBSData>::const_iterator iter;
  std::map<unsigned, NURBSData>::const_iterator iterEnd;
  NURBSData data;
  if (dataID == 0xFFFFFFFE) // Use stencil NURBS data
  {
    if (!m_stencilShape)
    {
      _handleLevelChange(level);
      return;
    }

    // Get stencil geometry so as to find stencil NURBS data ID
    std::map<unsigned, VSDGeometryList>::const_iterator cstiter = m_stencilShape->m_geometries.find(m_currentGeometryCount-1);
    VSDGeometryListElement *element = 0;
    if (cstiter == m_stencilShape->m_geometries.end())
    {
      _handleLevelChange(level);
      return;
    }
    element = cstiter->second.getElement(id);
    iter = m_stencilShape->m_nurbsData.find(element ? element->getDataID() : MINUS_ONE);
    iterEnd =  m_stencilShape->m_nurbsData.end();
  }
  else // No stencils involved, directly get dataID and fill in missing parts
  {
    iter = m_NURBSData.find(dataID);
    iterEnd = m_NURBSData.end();
  }

  if (iter != iterEnd)
  {
    data = iter->second;
    data.knots.push_back(knot);
    data.knots.push_back(data.lastKnot);
    data.knots.insert(data.knots.begin(), knotPrev);
    data.weights.push_back(weight);
    data.weights.insert(data.weights.begin(), weightPrev);
    collectNURBSTo(id, level, x2, y2, data.xType, data.yType, data.degree, data.points, data.knots, data.weights);
  }
  else
    _handleLevelChange(level);
}

void libvisio::VSDContentCollector::collectPolylineTo(unsigned /* id */ , unsigned level, double x, double y, unsigned char xType, unsigned char yType, const std::vector<std::pair<double, double> > &points)
{
  _handleLevelChange(level);

  WPXPropertyList polyline;
  std::vector<std::pair<double, double> > tmpPoints(points);
  for (unsigned i = 0; i< points.size(); i++)
  {
    polyline.clear();
    if (xType == 0)
      tmpPoints[i].first *= m_xform.width;
    if (yType == 0)
      tmpPoints[i].second *= m_xform.height;

    transformPoint(tmpPoints[i].first, tmpPoints[i].second);
    polyline.insert("libwpg:path-action", "L");
    polyline.insert("svg:x", m_scale*tmpPoints[i].first);
    polyline.insert("svg:y", m_scale*tmpPoints[i].second);
    if (!m_noFill && !m_noShow)
      m_currentFillGeometry.push_back(polyline);
    if (!m_noLine && !m_noShow)
      m_currentLineGeometry.push_back(polyline);
  }

  m_originalX = x;
  m_originalY = y;
  m_x = x;
  m_y = y;
  transformPoint(m_x, m_y);
  polyline.insert("libwpg:path-action", "L");
  polyline.insert("svg:x", m_scale*m_x);
  polyline.insert("svg:y", m_scale*m_y);
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(polyline);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(polyline);
}

/* Polyline with incomplete data */
void libvisio::VSDContentCollector::collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned dataID)
{
  std::map<unsigned, PolylineData>::const_iterator iter;
  std::map<unsigned, PolylineData>::const_iterator iterEnd;
  if (dataID == 0xFFFFFFFE) // Use stencil polyline data
  {
    if (!m_stencilShape || m_stencilShape->m_geometries.size() < m_currentGeometryCount)
    {
      _handleLevelChange(level);
      return;
    }

    // Get stencil geometry so as to find stencil polyline data ID
    std::map<unsigned, VSDGeometryList>::const_iterator cstiter = m_stencilShape->m_geometries.find(m_currentGeometryCount-1);
    VSDGeometryListElement *element = 0;
    if (cstiter == m_stencilShape->m_geometries.end())
    {
      _handleLevelChange(level);
      return;
    }
    element = cstiter->second.getElement(id);
    iter = m_stencilShape->m_polylineData.find(element ? element->getDataID() : MINUS_ONE);
    iterEnd = m_stencilShape->m_polylineData.end();
  }
  else // No stencils involved, directly get dataID
  {
    iter = m_polylineData.find(dataID);
    iterEnd = m_polylineData.end();
  }

  if (iter != iterEnd)
  {
    PolylineData data = iter->second;
    collectPolylineTo(id, level, x, y, data.xType, data.yType, data.points);
  }
  else
    _handleLevelChange(level);
}

/* NURBS shape data */
void libvisio::VSDContentCollector::collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, unsigned degree, double lastKnot, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights)
{
  _handleLevelChange(level);
  NURBSData data;
  data.xType = xType;
  data.yType = yType;
  data.degree = degree;
  data.lastKnot = lastKnot;
  data.points = controlPoints;
  data.knots = knotVector;
  data.weights = weights;
  m_NURBSData[id] = data;
}

/* Polyline shape data */
void libvisio::VSDContentCollector::collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points)
{
  _handleLevelChange(level);
  PolylineData data;
  data.xType = xType;
  data.yType = yType;
  data.points = points;
  m_polylineData[id] = data;
}

void libvisio::VSDContentCollector::collectXFormData(unsigned level, const XForm &xform)
{
  _handleLevelChange(level);
  m_xform = xform;
}

void libvisio::VSDContentCollector::collectTxtXForm(unsigned level, const XForm &txtxform)
{
  _handleLevelChange(level);
  if (m_txtxform)
    delete(m_txtxform);
  m_txtxform = new XForm(txtxform);
  m_txtxform->x = m_txtxform->pinX - m_txtxform->pinLocX;
  m_txtxform->y = m_txtxform->pinY - m_txtxform->pinLocY;
}

void libvisio::VSDContentCollector::applyXForm(double &x, double &y, const XForm &xform)
{
  x -= xform.pinLocX;
  y -= xform.pinLocY;
  if (xform.flipX)
    x = -x;
  if (xform.flipY)
    y = -y;
  if (xform.angle != 0.0)
  {
    double tmpX = x*cos(xform.angle) - y*sin(xform.angle);
    double tmpY = y*cos(xform.angle) + x*sin(xform.angle);
    x = tmpX;
    y = tmpY;
  }
  x += xform.pinX;
  y += xform.pinY;
}

void libvisio::VSDContentCollector::transformPoint(double &x, double &y, XForm *txtxform)
{
  // We are interested for the while in shapes xforms only
  if (!m_isShapeStarted)
    return;

  if (!m_currentShapeId)
    return;

  unsigned shapeId = m_currentShapeId;

  if (txtxform)
    applyXForm(x, y, *txtxform);

  while (true && m_groupXForms)
  {
    std::map<unsigned, XForm>::iterator iterX = m_groupXForms->find(shapeId);
    if (iterX != m_groupXForms->end())
    {
      XForm xform = iterX->second;
      applyXForm(x, y, xform);
    }
    else
      break;
    std::map<unsigned, unsigned>::iterator iter = m_groupMemberships.find(shapeId);
    if (iter != m_groupMemberships.end())
      shapeId = iter->second;
    else
      break;
  }
  y = m_pageHeight - y;
}

void libvisio::VSDContentCollector::transformAngle(double &angle, XForm *txtxform)
{
  // We are interested for the while in shape xforms only
  if (!m_isShapeStarted)
    return;

  if (!m_currentShapeId)
    return;

  double x0 = m_xform.pinLocX;
  double y0 = m_xform.pinLocY;
  double x1 = m_xform.pinLocX + cos(angle);
  double y1 =m_xform.pinLocY + sin(angle);
  transformPoint(x0, y0, txtxform);
  transformPoint(x1, y1, txtxform);
  angle = fmod(2.0*M_PI + (y1 > y0 ? 1.0 : -1.0)*acos((x1-x0) / sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0))), 2.0*M_PI);
}

void libvisio::VSDContentCollector::transformFlips(bool &flipX, bool &flipY)
{
  if (!m_isShapeStarted)
    return;

  if (!m_currentShapeId)
    return;

  unsigned shapeId = m_currentShapeId;

  while (true && m_groupXForms)
  {
    std::map<unsigned, XForm>::iterator iterX = m_groupXForms->find(shapeId);
    if (iterX != m_groupXForms->end())
    {
      XForm xform = iterX->second;
      if (xform.flipX)
        flipX = !flipX;
      if (xform.flipY)
        flipY = !flipY;
    }
    else
      break;
    std::map<unsigned, unsigned>::iterator iter = m_groupMemberships.find(shapeId);
    if (iter != m_groupMemberships.end())
      shapeId = iter->second;
    else
      break;
  }
}

void libvisio::VSDContentCollector::collectShapesOrder(unsigned /* id */, unsigned level, const std::vector<unsigned> & /* shapeIds */)
{
  _handleLevelChange(level);
}

void libvisio::VSDContentCollector::collectForeignDataType(unsigned level, unsigned foreignType, unsigned foreignFormat, double offsetX, double offsetY, double width, double height)
{
  _handleLevelChange(level);
  m_foreignType = foreignType;
  m_foreignFormat = foreignFormat;
  m_foreignOffsetX = offsetX;
  m_foreignOffsetY = offsetY;
  m_foreignWidth = width;
  m_foreignHeight = height;
}

void libvisio::VSDContentCollector::collectPageProps(unsigned /* id */, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale)
{
  _handleLevelChange(level);
  m_pageWidth = pageWidth;
  m_pageHeight = pageHeight;
  m_scale = scale;
  m_shadowOffsetX = shadowOffsetX;
  m_shadowOffsetY = shadowOffsetY;

  m_currentPage.m_pageWidth = m_scale*m_pageWidth;
  m_currentPage.m_pageHeight = m_scale*m_pageHeight;
}

void libvisio::VSDContentCollector::collectPage(unsigned /* id */, unsigned level, unsigned backgroundPageID, bool isBackgroundPage)
{
  _handleLevelChange(level);
  m_currentPage.m_backgroundPageID = backgroundPageID;
  m_isBackgroundPage = isBackgroundPage;
}

void libvisio::VSDContentCollector::collectShape(unsigned id, unsigned level, unsigned /*parent*/, unsigned masterPage, unsigned masterShape, unsigned lineStyleId, unsigned fillStyleId, unsigned textStyleId)
{
  _handleLevelChange(level);
  m_currentShapeLevel = level;

  m_foreignType = 0; // Tracks current foreign data type
  m_foreignFormat = 0; // Tracks foreign data format
  m_foreignOffsetX = 0.0;
  m_foreignOffsetY = 0.0;
  m_foreignWidth = 0.0;
  m_foreignHeight = 0.0;

  m_originalX = 0.0;
  m_originalY = 0.0;
  m_x = 0;
  m_y = 0;

  // Geometry flags
  m_noLine = false;
  m_noFill = false;
  m_noShow = false;
  m_isFirstGeometry = true;

  // Save line colour and pattern, fill type and pattern
  m_fillType = "none";
  m_fillPattern = 1; // same as "solid"
  m_fillFGTransparency = 0.0;
  m_fillBGTransparency = 0.0;

  m_textStream.clear();
  m_charFormats.clear();
  m_paraFormats.clear();

  m_defaultCharStyle = m_styles.getCharStyle(0);
  m_defaultParaStyle = m_styles.getParaStyle(0);
  m_textBlockStyle = m_styles.getTextBlockStyle(0);

  m_currentShapeId = id;
  m_pageOutputDrawing[m_currentShapeId] = VSDOutputElementList();
  m_pageOutputText[m_currentShapeId] = VSDOutputElementList();
  m_shapeOutputDrawing = &m_pageOutputDrawing[m_currentShapeId];
  m_shapeOutputText = &m_pageOutputText[m_currentShapeId];
  m_isShapeStarted = true;
  m_isFirstGeometry = true;

  m_names.clear();
  m_stencilNames.clear();
  m_fields.clear();
  m_stencilFields.clear();

  // Get stencil shape
  m_stencilShape = m_stencils.getStencilShape(masterPage, masterShape);
  // Initialize the shape from stencil content
  m_lineStyle = VSDLineStyle();
  m_fillStyle = VSDFillStyle();
  if (m_stencilShape)
  {
    if (m_stencilShape->m_foreign)
    {
      m_foreignType = m_stencilShape->m_foreign->type;
      m_foreignFormat = m_stencilShape->m_foreign->format;
      m_foreignOffsetX = m_stencilShape->m_foreign->offsetX;
      m_foreignOffsetY = m_stencilShape->m_foreign->offsetY;
      m_foreignWidth = m_stencilShape->m_foreign->width;
      m_foreignHeight = m_stencilShape->m_foreign->height;
      m_currentForeignData.clear();
      _handleForeignData(m_stencilShape->m_foreign->data);
    }

    m_textStream = m_stencilShape->m_text;
    m_textFormat = m_stencilShape->m_textFormat;

    for (std::map< unsigned, VSDName>::const_iterator iterData = m_stencilShape->m_names.begin(); iterData != m_stencilShape->m_names.end(); ++iterData)
    {
      WPXString nameString;
      _convertDataToString(nameString, iterData->second.m_data, iterData->second.m_format);
      m_stencilNames[iterData->first] = nameString;
    }

    m_stencilFields = m_stencilShape->m_fields;
    for (unsigned i = 0; i < m_stencilFields.size(); i++)
    {
      VSDFieldListElement *elem = m_stencilFields.getElement(i);
      if (elem)
        m_fields.push_back(elem->getString(m_stencilNames));
      else
        m_fields.push_back(WPXString());
    }

    if (m_stencilShape->m_lineStyleId != MINUS_ONE)
      m_lineStyle.override(m_styles.getOptionalLineStyle(m_stencilShape->m_lineStyleId));

    m_lineStyle.override(m_stencilShape->m_lineStyle);

    if (m_stencilShape->m_fillStyleId != MINUS_ONE)
      m_fillStyle.override(m_styles.getOptionalFillStyle(m_stencilShape->m_fillStyleId));

    m_fillStyle.override(m_stencilShape->m_fillStyle);

    if (m_stencilShape->m_textStyleId)
    {
      m_defaultCharStyle = m_styles.getCharStyle(m_stencilShape->m_textStyleId);
      m_defaultParaStyle = m_styles.getParaStyle(m_stencilShape->m_textStyleId);
      m_textBlockStyle = m_styles.getTextBlockStyle(m_stencilShape->m_textStyleId);
    }

    m_textBlockStyle.override(m_stencilShape->m_textBlockStyle);
    m_defaultCharStyle.override(m_stencilShape->m_charStyle);
    m_defaultParaStyle.override(m_stencilShape->m_paraStyle);
  }

  if (lineStyleId != MINUS_ONE)
    m_lineStyle.override(m_styles.getOptionalLineStyle(lineStyleId));

  if (fillStyleId != MINUS_ONE)
    m_fillStyle = m_styles.getFillStyle(fillStyleId);
  if (textStyleId != MINUS_ONE)
  {
    m_defaultCharStyle = m_styles.getCharStyle(textStyleId);
    m_defaultParaStyle = m_styles.getParaStyle(textStyleId);
    m_textBlockStyle = m_styles.getTextBlockStyle(textStyleId);
  }

  m_currentGeometryCount = 0;
  m_fieldIndex = 0;
}

void libvisio::VSDContentCollector::collectUnhandledChunk(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDContentCollector::collectFont(unsigned short fontID, const WPXBinaryData &textStream, TextFormat format)
{
  VSDFont font;
  font.name.clear();
  _convertDataToString(font.name, textStream, format);
  font.encoding = format;
  m_fonts[fontID] = font;
}


void libvisio::VSDContentCollector::collectSplineStart(unsigned /* id */, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree)
{
  m_splineLevel = level;
  m_splineKnotVector.push_back(firstKnot);
  m_splineKnotVector.push_back(secondKnot);
  m_splineLastKnot = lastKnot;
  m_splineX = x;
  m_splineY = y;
  m_splineDegree = degree;
}


void libvisio::VSDContentCollector::collectSplineKnot(unsigned /* id */, unsigned /* level */, double x, double y, double knot)
{
  m_splineKnotVector.push_back(knot);
  m_splineControlPoints.push_back(std::pair<double,double>(m_splineX,m_splineY));
  m_splineX = x;
  m_splineY = y;
}


void libvisio::VSDContentCollector::collectSplineEnd()
{
  if (m_splineKnotVector.empty() || m_splineControlPoints.empty())
  {
    m_splineKnotVector.clear();
    m_splineControlPoints.clear();
    return;
  }
  m_splineKnotVector.push_back(m_splineLastKnot);
  std::vector<double> weights;
  for (unsigned i=0; i < m_splineControlPoints.size()+2; i++)
    weights.push_back(1.0);
  collectNURBSTo(0, m_splineLevel, m_splineX, m_splineY, 1, 1, m_splineDegree, m_splineControlPoints, m_splineKnotVector, weights);
  m_splineKnotVector.clear();
  m_splineControlPoints.clear();
}


void libvisio::VSDContentCollector::collectText(unsigned level, const WPXBinaryData &textStream, TextFormat format)
{
  _handleLevelChange(level);

  m_textStream = textStream;
  m_textFormat = format;
}

void libvisio::VSDContentCollector::collectParaIX(unsigned /* id */ , unsigned level, const boost::optional<unsigned> &charCount, const boost::optional<double> &indFirst,
    const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine, const boost::optional<double> &spBefore,
    const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align, const boost::optional<unsigned> &flags)
{
  _handleLevelChange(level);
  VSDParaStyle format;
  format.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align, flags));
  m_paraFormats.push_back(format);
}

void libvisio::VSDContentCollector::collectCharIX(unsigned /* id */ , unsigned level, const boost::optional<unsigned> &charCount,
    const boost::optional<unsigned short> &fontID, const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize,
    const boost::optional<bool> &bold, const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline,
    const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps,
    const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript, const boost::optional<bool> &subscript, const boost::optional<VSDFont> &fontFace)
{
  _handleLevelChange(level);
  VSDCharStyle format;
  format.override(VSDOptionalCharStyle(charCount, fontID, fontColour, fontSize, bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                       allcaps, initcaps, smallcaps, superscript, subscript, fontFace));
  m_charFormats.push_back(format);
}

void libvisio::VSDContentCollector::collectTextBlock(unsigned level, const boost::optional<double> &leftMargin, const boost::optional<double> &rightMargin,
    const boost::optional<double> &topMargin, const boost::optional<double> &bottomMargin, const boost::optional<unsigned char> &verticalAlign,
    const boost::optional<bool> &isBgFilled, const boost::optional<Colour> &bgColour, const boost::optional<double> &defaultTabStop,
    const boost::optional<unsigned char> &textDirection)
{
  _handleLevelChange(level);
  m_textBlockStyle.override(VSDOptionalTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin, verticalAlign, isBgFilled, bgColour, defaultTabStop, textDirection));
}

void libvisio::VSDContentCollector::collectNameList(unsigned /*id*/, unsigned level)
{
  _handleLevelChange(level);

  m_names.clear();
}

void libvisio::VSDContentCollector::_convertDataToString(WPXString &result, const WPXBinaryData &data, TextFormat format)
{
  if (!data.size())
    return;
  std::vector<unsigned char> tmpData(data.size());
  memcpy(&tmpData[0], data.getDataBuffer(), data.size());
  appendCharacters(result, tmpData, format);
}

void libvisio::VSDContentCollector::collectName(unsigned id, unsigned level, const WPXBinaryData &name, TextFormat format)
{
  _handleLevelChange(level);

  WPXString nameString;
  _convertDataToString(nameString, name, format);
  m_names[id] = nameString;
}

void libvisio::VSDContentCollector::collectPageSheet(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
  m_currentShapeLevel = level;
}

void libvisio::VSDContentCollector::collectStyleSheet(unsigned /* id */, unsigned level, unsigned /* parentLineStyle */, unsigned /* parentFillStyle */, unsigned /* parentTextStyle */)
{
  _handleLevelChange(level);
}

void libvisio::VSDContentCollector::collectLineStyle(unsigned level, const boost::optional<double> & /* strokeWidth */, const boost::optional<Colour> & /* c */,
    const boost::optional<unsigned char> & /* linePattern */, const boost::optional<unsigned char> & /* startMarker */, const boost::optional<unsigned char> & /* endMarker */,
    const boost::optional<unsigned char> & /* lineCap */)
{
  _handleLevelChange(level);
}

void libvisio::VSDContentCollector::collectFillStyle(unsigned level, const boost::optional<Colour> & /* colourFG */, const boost::optional<Colour> & /* colourBG */,
    const boost::optional<unsigned char> & /* fillPattern */, const boost::optional<double> & /* fillFGTransparency */, const boost::optional<double> & /* fillBGTransparency */,
    const boost::optional<unsigned char> & /* shadowPattern */, const boost::optional<Colour> & /* shfgc */, const boost::optional<double> & /* shadowOffsetX */,
    const boost::optional<double> & /* shadowOffsetY */)
{
  _handleLevelChange(level);
}

void libvisio::VSDContentCollector::collectFillStyle(unsigned level, const boost::optional<Colour> & /* colourFG */, const boost::optional<Colour> & /* colourBG */,
    const boost::optional<unsigned char> & /* fillPattern */, const boost::optional<double> & /* fillFGTransparency */, const boost::optional<double> & /* fillBGTransparency */,
    const boost::optional<unsigned char> & /* shadowPattern */, const boost::optional<Colour> & /* shfgc */)
{
  _handleLevelChange(level);
}

void libvisio::VSDContentCollector::collectCharIXStyle(unsigned /* id */, unsigned level, const boost::optional<unsigned> & /* charCount */, const boost::optional<unsigned short> & /* fontID */,
    const boost::optional<Colour> & /* fontColour */, const boost::optional<double> & /* fontSize */, const boost::optional<bool> & /* bold */, const boost::optional<bool> & /* italic */,
    const boost::optional<bool> & /* underline */, const boost::optional<bool> & /* doubleunderline */, const boost::optional<bool> & /* strikeout */,
    const boost::optional<bool> & /* doublestrikeout */, const boost::optional<bool> & /* allcaps */, const boost::optional<bool> & /* initcaps */, const boost::optional<bool> & /* smallcaps */,
    const boost::optional<bool> & /* superscript */, const boost::optional<bool> & /* subscript */, const boost::optional<VSDFont> & /* fontFace */)
{
  _handleLevelChange(level);
}

void libvisio::VSDContentCollector::collectParaIXStyle(unsigned /* id */, unsigned level, const boost::optional<unsigned> & /* charCount */, const boost::optional<double> & /* indFirst */,
    const boost::optional<double> & /* indLeft */, const boost::optional<double> & /* indRight */, const boost::optional<double> & /* spLine */, const boost::optional<double> & /* spBefore */,
    const boost::optional<double> & /* spAfter */, const boost::optional<unsigned char> & /* align */, const boost::optional<unsigned> & /* flags */)
{
  _handleLevelChange(level);
}


void libvisio::VSDContentCollector::collectTextBlockStyle(unsigned level, const boost::optional<double> & /* leftMargin */, const boost::optional<double> & /* rightMargin */,
    const boost::optional<double> & /* topMargin */, const boost::optional<double> & /* bottomMargin */, const boost::optional<unsigned char> & /* verticalAlign */,
    const boost::optional<bool> & /* isBgFilled */, const boost::optional<Colour> & /* bgColour */, const boost::optional<double> & /* defaultTabStop */,
    const boost::optional<unsigned char> & /* textDirection */)
{
  _handleLevelChange(level);
}

void libvisio::VSDContentCollector::_lineProperties(const VSDLineStyle &style)
{
  _lineProperties(style.width, style.colour, style.pattern, style.startMarker, style.endMarker, style.cap);
}

void libvisio::VSDContentCollector::_fillAndShadowProperties(const VSDFillStyle &style)
{
  _fillAndShadowProperties(style.fgColour, style.bgColour, style.pattern, style.fgTransparency, style.bgTransparency,
                           style.shadowPattern, style.shadowFgColour, style.shadowOffsetX, style.shadowOffsetY);
}

void libvisio::VSDContentCollector::collectFieldList(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
  m_fields.clear();
}

void libvisio::VSDContentCollector::collectTextField(unsigned id, unsigned level, int nameId, int formatStringId)
{
  _handleLevelChange(level);
  VSDFieldListElement *element = m_stencilFields.getElement(m_fields.size());
  if (element)
  {
    if (nameId == -2)
      m_fields.push_back(element->getString(m_stencilNames));
    else
    {
      if (nameId >= 0)
        m_fields.push_back(m_names[nameId]);
      else
        m_fields.push_back(WPXString());
    }
  }
  else
  {
    VSDTextField tmpField(id, level, nameId, formatStringId);
    m_fields.push_back(tmpField.getString(m_names));
  }
}

void libvisio::VSDContentCollector::collectNumericField(unsigned id, unsigned level, unsigned short format, double number, int formatStringId)
{
  _handleLevelChange(level);
  VSDFieldListElement *pElement = m_stencilFields.getElement(m_fields.size());
  if (pElement)
  {
    VSDFieldListElement *element = pElement->clone();
    if (element)
    {
      element->setValue(number);
      if (format == 0xffff)
      {
        std::map<unsigned, WPXString>::const_iterator iter = m_names.find(formatStringId);
        if (iter != m_names.end())
          parseFormatId(iter->second.cstr(), format);
      }
      if (format != 0xffff)
        element->setFormat(format);

      m_fields.push_back(element->getString(m_names));
      delete element;
    }
  }
  else
  {
    VSDNumericField tmpField(id, level, format, number, formatStringId);
    m_fields.push_back(tmpField.getString(m_names));
  }
}

void libvisio::VSDContentCollector::_handleLevelChange(unsigned level)
{
  if (m_currentLevel == level)
    return;
  if (level <= m_currentShapeLevel)
  {
    if (m_isShapeStarted)
    {
      if (m_stencilShape != 0 && !m_isStencilStarted)
      {
        m_isStencilStarted = true;
        m_NURBSData = m_stencilShape->m_nurbsData;
        m_polylineData = m_stencilShape->m_polylineData;

        if (m_currentFillGeometry.empty() && m_currentLineGeometry.empty() && !m_noShow)
        {
          for (std::map<unsigned, VSDGeometryList>::const_iterator cstiter = m_stencilShape->m_geometries.begin();
               cstiter != m_stencilShape->m_geometries.end(); ++cstiter)
          {
            m_x = 0.0;
            m_y = 0.0;
            cstiter->second.handle(this);
          }
        }
        m_isStencilStarted = false;
      }

      _flushCurrentPath();
      _flushCurrentForeignData();
      if (m_textStream.size())
        _flushText();
      m_isShapeStarted = false;
    }
    m_originalX = 0.0;
    m_originalY = 0.0;
    m_x = 0;
    m_y = 0;
    if (m_txtxform)
      delete(m_txtxform);
    m_txtxform = 0;
    m_xform = XForm();
    m_NURBSData.clear();
    m_polylineData.clear();
  }

  m_currentLevel = level;
}

void libvisio::VSDContentCollector::startPage(unsigned pageId)
{
  if (m_isShapeStarted)
  {
    _flushCurrentPath();
    _flushCurrentForeignData();
    m_isShapeStarted = false;
    if (m_textStream.size())
      _flushText();
  }
  m_originalX = 0.0;
  m_originalY = 0.0;
  if (m_txtxform)
    delete(m_txtxform);
  m_txtxform = 0;
  m_xform = XForm();
  m_x = 0;
  m_y = 0;
  m_currentPageNumber++;
  if (m_groupXFormsSequence.size() >= m_currentPageNumber)
    m_groupXForms = m_groupXFormsSequence.size() > m_currentPageNumber-1 ? &m_groupXFormsSequence[m_currentPageNumber-1] : 0;
  if (m_groupMembershipsSequence.size() >= m_currentPageNumber)
    m_groupMemberships = m_groupMembershipsSequence[m_currentPageNumber-1];
  if (m_documentPageShapeOrders.size() >= m_currentPageNumber)
    m_pageShapeOrder = m_documentPageShapeOrders[m_currentPageNumber-1];
  m_currentPage = libvisio::VSDPage();
  m_currentPage.m_currentPageID = pageId;
  m_isPageStarted = true;
}

void libvisio::VSDContentCollector::endPage()
{
  if (m_isPageStarted)
  {
    _handleLevelChange(0);
    _flushCurrentPage();
    if (m_isBackgroundPage)
      m_pages.addBackgroundPage(m_currentPage);
    else
      m_pages.addPage(m_currentPage);
    m_isPageStarted = false;
    m_isBackgroundPage = false;
  }
}

void libvisio::VSDContentCollector::endPages()
{
  m_pages.draw(m_painter);
}

bool libvisio::VSDContentCollector::parseFormatId( const char *formatString, unsigned short &result )
{
  using namespace ::boost::spirit::classic;

  result = 0xffff;

  uint_parser<unsigned short,10,1,5> ushort_p;
  if (parse(formatString,
            // Begin grammar
            (
              (
                str_p("{<") >>
                ushort_p[assign_a(result)]
                >> str_p(">}")
              )
              |
              (
                str_p("esc(") >>
                ushort_p[assign_a(result)]
                >> ')'
              )
            )>> end_p,
            // End grammar
            space_p).full )
  {
    return true;
  }
  return false;
}

void libvisio::VSDContentCollector::appendCharacters(WPXString &text, const std::vector<unsigned char> &characters, TextFormat format)
{
  if (format == VSD_TEXT_UTF16)
    return appendCharacters(text, characters);
  if (format == VSD_TEXT_UTF8)
  {
    for (std::vector<unsigned char>::const_iterator iter = characters.begin();
         iter != characters.end(); ++iter)
    {
      text.append((const char)*iter);
    }
    return;
  }

  static const unsigned short cp874map[] =
  {
    0x20AC, 0x0020, 0x0020, 0x0020, 0x0020, 0x2026, 0x0020, 0x0020,
    0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
    0x0020, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
    0x00A0, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07,
    0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
    0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17,
    0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
    0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27,
    0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F,
    0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37,
    0x0E38, 0x0E39, 0x0E3A, 0x0020, 0x0020, 0x0020, 0x0020, 0x0E3F,
    0x0E40, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47,
    0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F,
    0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57,
    0x0E58, 0x0E59, 0x0E5A, 0x0E5B, 0x0020, 0x0020, 0x0020, 0x0020
  };

  static const unsigned short cp1250map[] =
  {
    0x20AC, 0x0020, 0x201A, 0x0020, 0x201E, 0x2026, 0x2020, 0x2021,
    0x0020, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
    0x0020, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0020, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
    0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
    0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
    0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
    0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
    0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
    0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
    0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
    0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9
  };

  static const unsigned short cp1251map[] =
  {
    0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
    0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
    0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0020, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
    0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
    0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
    0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7,
    0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
  };

  static const unsigned short cp1252map[] =
  {
    0x20AC, 0x0020, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0020, 0x017D, 0x0020,
    0x0020, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0020, 0x017E, 0x0178,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
  };

  static const unsigned short cp1253map[] =
  {
    0x20AC, 0x0020, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x0020, 0x2030, 0x0020, 0x2039, 0x0020, 0x0020, 0x0020, 0x0020,
    0x0020, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0020, 0x2122, 0x0020, 0x203A, 0x0020, 0x0020, 0x0020, 0x0020,
    0x00A0, 0x0385, 0x0386, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x0020, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x00B5, 0x00B6, 0x00B7,
    0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
    0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
    0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
    0x03A0, 0x03A1, 0x0020, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
    0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
    0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
    0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
    0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
    0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x0020
  };

  static const unsigned short cp1254map[] =
  {
    0x20AC, 0x0020, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0020, 0x0020, 0x0020,
    0x0020, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0020, 0x0020, 0x0178,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF
  };

  static const unsigned short cp1255map[] =
  {
    0x20AC, 0x0020, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0020, 0x2039, 0x0020, 0x0020, 0x0020, 0x0020,
    0x0020, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0020, 0x203A, 0x0020, 0x0020, 0x0020, 0x0020,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AA, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x05B0, 0x05B1, 0x05B2, 0x05B3, 0x05B4, 0x05B5, 0x05B6, 0x05B7,
    0x05B8, 0x05B9, 0x0020, 0x05BB, 0x05BC, 0x05BD, 0x05BE, 0x05BF,
    0x05C0, 0x05C1, 0x05C2, 0x05C3, 0x05F0, 0x05F1, 0x05F2, 0x05F3,
    0x05F4, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020, 0x0020,
    0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
    0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
    0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
    0x05E8, 0x05E9, 0x05EA, 0x0020, 0x0020, 0x200E, 0x200F, 0x0020
  };

  static const unsigned short cp1256map[] =
  {
    0x20AC, 0x067E, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0679, 0x2039, 0x0152, 0x0686, 0x0698, 0x0688,
    0x06AF, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x06A9, 0x2122, 0x0691, 0x203A, 0x0153, 0x200C, 0x200D, 0x06BA,
    0x00A0, 0x060C, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x06BE, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x061B, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x061F,
    0x06C1, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627,
    0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
    0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x00D7,
    0x0637, 0x0638, 0x0639, 0x063A, 0x0640, 0x0641, 0x0642, 0x0643,
    0x00E0, 0x0644, 0x00E2, 0x0645, 0x0646, 0x0647, 0x0648, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0649, 0x064A, 0x00EE, 0x00EF,
    0x064B, 0x064C, 0x064D, 0x064E, 0x00F4, 0x064F, 0x0650, 0x00F7,
    0x0651, 0x00F9, 0x0652, 0x00FB, 0x00FC, 0x200E, 0x200F, 0x06D2
  };

  static const unsigned short cp1257map[] =
  {
    0x20AC, 0x0020, 0x201A, 0x0020, 0x201E, 0x2026, 0x2020, 0x2021,
    0x0020, 0x2030, 0x0020, 0x2039, 0x0020, 0x00A8, 0x02C7, 0x00B8,
    0x0020, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0020, 0x2122, 0x0020, 0x203A, 0x0020, 0x00AF, 0x02DB, 0x0020,
    0x00A0, 0x0020, 0x00A2, 0x00A3, 0x00A4, 0x0020, 0x00A6, 0x00A7,
    0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
    0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112,
    0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
    0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7,
    0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
    0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
    0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
    0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7,
    0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x02D9
  };

  static const unsigned short cp1258map[] =
  {
    0x20AC, 0x0020, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0020, 0x2039, 0x0152, 0x0020, 0x0020, 0x0020,
    0x0020, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x02DC, 0x2122, 0x0020, 0x203A, 0x0153, 0x0020, 0x0020, 0x0178,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x0300, 0x00CD, 0x00CE, 0x00CF,
    0x0110, 0x00D1, 0x0309, 0x00D3, 0x00D4, 0x01A0, 0x00D6, 0x00D7,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x01AF, 0x0303, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0301, 0x00ED, 0x00EE, 0x00EF,
    0x0111, 0x00F1, 0x0323, 0x00F3, 0x00F4, 0x01A1, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x01B0, 0x20AB, 0x00FF
  };

  std::vector<unsigned char>::const_iterator iter = characters.begin();
  while (iter != characters.end())
  {
    uint32_t ucs4Character = 0;
    switch (*iter)
    {
    case 0:
      ++iter;
      break;
    case 0x09:
      ucs4Character = '\t';
      ++iter;
      break;
    case 0x0a:
    case 0x0e:
      ucs4Character = '\n';
      ++iter;
      break;
    case 0x1e:
      if (m_fieldIndex < m_fields.size())
        text.append(m_fields[m_fieldIndex++].cstr());
      else
        m_fieldIndex++;
      ++iter;
      continue;
    default:
      if (*iter < 0x20)
      {
        ucs4Character = 0x20;
        ++iter;
      }
      else if (*iter >= 0x20 && *iter < 0x7f)
        ucs4Character = *iter++;
      else if (*iter == 0x7f)
      {
        ucs4Character = 0x20;
        ++iter;
      }
      else
      {
        switch (format)
        {
        case VSD_TEXT_ANSI:
          ucs4Character = cp1252map[*iter++ - 0x80];
          break;
        case VSD_TEXT_GREEK:
          ucs4Character = cp1253map[*iter++ - 0x80];
          break;
        case VSD_TEXT_TURKISH:
          ucs4Character = cp1254map[*iter++ - 0x80];
          break;
        case VSD_TEXT_VIETNAMESE:
          ucs4Character = cp1258map[*iter++ - 0x80];
          break;
        case VSD_TEXT_HEBREW:
          ucs4Character = cp1255map[*iter++ - 0x80];
          break;
        case VSD_TEXT_ARABIC:
          ucs4Character = cp1256map[*iter++ - 0x80];
          break;
        case VSD_TEXT_BALTIC:
          ucs4Character = cp1257map[*iter++ - 0x80];
          break;
        case VSD_TEXT_RUSSIAN:
          ucs4Character = cp1251map[*iter++ - 0x80];
          break;
        case VSD_TEXT_THAI:
          ucs4Character = cp874map[*iter++ - 0x80];
          break;
        case VSD_TEXT_CENTRAL_EUROPE:
          ucs4Character = cp1250map[*iter++ - 0x80];
          break;
        default:
          ucs4Character = *iter++;
          break;
        }
      }
      break;
    }
    _appendUCS4(text, ucs4Character);
  }
}

void libvisio::VSDContentCollector::appendCharacters(WPXString &text, const std::vector<unsigned char> &characters)
{
  std::vector<unsigned char>::const_iterator iter = characters.begin();
  while (iter != characters.end())
  {
    uint16_t high_surrogate = 0;
    bool fail = false;
    uint32_t ucs4Character = 0;
    while (true)
    {
      if (iter == characters.end())
      {
        fail = true;
        break;
      }
      uint16_t character = *iter++;
      character |= (uint16_t)(*iter++) << 8;
      if (character == 0xfffc)
      {
        if (m_fieldIndex < m_fields.size())
          text.append(m_fields[m_fieldIndex++].cstr());
        else
          m_fieldIndex++;
        continue;
      }
      else if (character >= 0xdc00 && character < 0xe000) /* low surrogate */
      {
        if (high_surrogate)
        {
          ucs4Character = SURROGATE_VALUE(high_surrogate, character);
          high_surrogate = 0;
          break;
        }
        else
        {
          fail = true;
          break;
        }
      }
      else
      {
        if (high_surrogate)
        {
          fail = true;
          break;
        }
        if (character >= 0xd800 && character < 0xdc00) /* high surrogate */
          high_surrogate = character;
        else
        {
          ucs4Character = character;
          break;
        }
      }
    }
    if (fail)
      throw libvisio::GenericException();

    if (ucs4Character == 0xa)
      _appendUCS4(text, (uint32_t)'\n');
    else
      _appendUCS4(text, ucs4Character);
  }
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
