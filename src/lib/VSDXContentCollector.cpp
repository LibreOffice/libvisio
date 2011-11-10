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

#include <stack>
#include "VSDXContentCollector.h"
#include "VSDXParser.h"
#include "VSDInternalStream.h"

#define DUMP_BITMAP 0

#if DUMP_BITMAP
static unsigned bitmapId = 0;
#include <sstream>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

libvisio::VSDXContentCollector::VSDXContentCollector(
  libwpg::WPGPaintInterface *painter,
  std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
  std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
  std::vector<std::list<unsigned> > &documentPageShapeOrders,
  VSDXStyles &styles, VSDXStencils &stencils
) :
  m_painter(painter), m_isPageStarted(false), m_pageWidth(0.0), m_pageHeight(0.0),
  m_shadowOffsetX(0.0), m_shadowOffsetY(0.0),
  m_scale(1.0), m_x(0.0), m_y(0.0), m_originalX(0.0), m_originalY(0.0), m_xform(),
  m_txtxform(0), m_currentGeometry(), m_groupXForms(groupXFormsSequence[0]),
  m_currentForeignData(), m_currentForeignProps(),
  m_currentShapeId(0), m_foreignType(0), m_foreignFormat(0), m_styleProps(),
  m_lineColour("black"), m_fillType("none"), m_linePattern(1),
  m_fillPattern(1), m_fillFGTransparency(0), m_fillBGTransparency(0),
  m_noLine(false), m_noFill(false), m_noShow(false), m_colours(), m_fonts(),
  m_currentLevel(0), m_isShapeStarted(false), m_groupMemberships(groupMembershipsSequence[0]),
  m_groupXFormsSequence(groupXFormsSequence), m_groupMembershipsSequence(groupMembershipsSequence),
  m_currentPageNumber(0), m_shapeOutputDrawing(0), m_shapeOutputText(0),
  m_pageOutputDrawing(), m_pageOutputText(), m_documentPageShapeOrders(documentPageShapeOrders),
  m_pageShapeOrder(documentPageShapeOrders[0]), m_isFirstGeometry(true),
  m_NURBSData(), m_polylineData(), m_textStream(), m_names(), m_fields(), m_fieldIndex(0),
  m_textFormat(VSD_TEXT_ANSI), m_charFormats(), m_paraFormats(), m_textBlockStyle(),
  m_defaultCharStyle(), m_defaultParaStyle(), m_styles(styles),
  m_stencils(stencils), m_stencilShape(0), m_isStencilStarted(false), m_currentGeometryCount(0),
  m_backgroundPageID(0xffffffff), m_currentPageID(0), m_currentPage(), m_pages(),
  m_splineControlPoints(), m_splineKnotVector(), m_splineX(0.0), m_splineY(0.0),
  m_splineLastKnot(0.0), m_splineDegree(0), m_splineLevel(0)
{
}

void libvisio::VSDXContentCollector::_fillAndShadowProperties(unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern,
    unsigned fillFGTransparency, unsigned fillBGTransparency,
    unsigned shadowPattern, Colour shfgc, double shadowOffsetX, double shadowOffsetY)
{
  m_fillPattern = fillPattern;
  m_fillFGTransparency = fillFGTransparency;
  m_fillBGTransparency = fillBGTransparency;

  if (m_fillPattern == 0)
    m_fillType = "none";
  else if (m_fillPattern == 1)
  {
    m_fillType = "solid";
    m_styleProps.insert("draw:fill-color", getColourString(m_colours[colourIndexFG]));
    if (m_fillFGTransparency > 0)
      m_styleProps.insert("draw:opacity", (double)(1 - m_fillFGTransparency/255.0), WPX_PERCENT);
    else
      m_styleProps.remove("draw:opacity");
  }
  else if (m_fillPattern == 26 || m_fillPattern == 29)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:style", "axial");
    m_styleProps.insert("draw:start-color", getColourString(m_colours[colourIndexFG]));
    m_styleProps.insert("draw:end-color", getColourString(m_colours[colourIndexBG]));
    m_styleProps.remove("draw:opacity");
    if (m_fillBGTransparency > 0)
      m_styleProps.insert("libwpg:start-opacity", (double)(1 - m_fillBGTransparency/255.0), WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:start-opacity", 1, WPX_PERCENT);
    if (m_fillFGTransparency > 0)
      m_styleProps.insert("libwpg:end-opacity", (double)(1 - m_fillFGTransparency/255.0), WPX_PERCENT);
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
    m_styleProps.insert("draw:start-color", getColourString(m_colours[colourIndexBG]));
    m_styleProps.insert("draw:end-color", getColourString(m_colours[colourIndexFG]));
    m_styleProps.remove("draw:opacity");
    if (m_fillBGTransparency > 0)
      m_styleProps.insert("libwpg:start-opacity", (double)(1 - m_fillBGTransparency/255.0), WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:start-opacity", 1, WPX_PERCENT);
    if (m_fillFGTransparency > 0)
      m_styleProps.insert("libwpg:end-opacity", (double)(1 - m_fillFGTransparency/255.0), WPX_PERCENT);
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
    m_styleProps.insert("draw:start-color", getColourString(m_colours[colourIndexBG]));
    m_styleProps.insert("draw:end-color", getColourString(m_colours[colourIndexFG]));
    m_styleProps.remove("draw:opacity");
    if (m_fillBGTransparency > 0)
      m_styleProps.insert("libwpg:start-opacity", (double)(1 - m_fillBGTransparency/255.0), WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:start-opacity", 1, WPX_PERCENT);
    if (m_fillFGTransparency > 0)
      m_styleProps.insert("libwpg:end-opacity", (double)(1 - m_fillFGTransparency/255.0), WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:end-opacity", 1, WPX_PERCENT);
    m_styleProps.insert("draw:angle", 0);
    m_styleProps.insert("draw:border", 0, WPX_PERCENT);
  }
  else if (m_fillPattern >= 36 && m_fillPattern <= 40)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:style", "radial");
    m_styleProps.insert("draw:start-color", getColourString(m_colours[colourIndexBG]));
    m_styleProps.insert("draw:end-color", getColourString(m_colours[colourIndexFG]));
    m_styleProps.remove("draw:opacity");
    if (m_fillBGTransparency > 0)
      m_styleProps.insert("libwpg:start-opacity", (double)(1 - m_fillBGTransparency/255.0), WPX_PERCENT);
    else
      m_styleProps.insert("libwpg:start-opacity", 1, WPX_PERCENT);
    if (m_fillFGTransparency > 0)
      m_styleProps.insert("libwpg:end-opacity", (double)(1 - m_fillFGTransparency/255.0), WPX_PERCENT);
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
    m_styleProps.insert("draw:fill-color", getColourString(m_colours[colourIndexBG]));
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

void libvisio::VSDXContentCollector::_lineProperties(double strokeWidth, Colour c, unsigned linePattern, unsigned startMarker, unsigned endMarker, unsigned lineCap)
{
  m_linePattern = linePattern;

  if (linePattern == 0) return; // No need to add style

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

  int dots1 = 0;
  int dots2 = 0;
  double dots1len = 0.0;
  double dots2len = 0.0;
  double gap = 0.0;

  m_styleProps.remove("draw:stroke");
  switch (linePattern)
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

  if (linePattern == 0)
    m_styleProps.insert("draw:stroke", "none");
  else if (linePattern == 1)
    m_styleProps.insert("draw:stroke", "solid");
  else if (linePattern > 1 && linePattern <= 23)
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

  // Deal with line markers (arrows, etc.)
  if (startMarker > 0)
  {
    m_styleProps.insert("draw:marker-start-viewbox", "0 0 20 30");
    m_styleProps.insert("draw:marker-start-path", "m10 0-10 30h20z");
    m_styleProps.insert("draw:marker-start-width", m_scale*0.118);
  }
  else if (endMarker > 0)
  {
    m_styleProps.insert("draw:marker-end-viewbox", "0 0 20 30");
    m_styleProps.insert("draw:marker-end-path", "m10 0-10 30h20z");
    m_styleProps.insert("draw:marker-end-width", m_scale*0.118);
  }
}

void libvisio::VSDXContentCollector::_flushCurrentPath()
{
  WPXPropertyListVector path;
  double startX = 0;
  double startY = 0;
  double x = 0;
  double y = 0;
  bool firstPoint = true;

  for (unsigned i = 0; i < m_currentGeometry.size(); i++)
  {
    if (firstPoint)
    {
      x = m_currentGeometry[i]["svg:x"]->getDouble();
      y = m_currentGeometry[i]["svg:y"]->getDouble();
      startX = x;
      startY = y;
      firstPoint = false;
    }
    else if (m_currentGeometry[i]["libwpg:path-action"]->getStr() == "M")
    {
      if (((startX == x && startY == y) || (m_styleProps["draw:fill"] && m_styleProps["draw:fill"]->getStr() != "none")) && path.count())
      {
        WPXPropertyList closedPath;
        closedPath.insert("libwpg:path-action", "Z");
        path.append(closedPath);
      }
      x = m_currentGeometry[i]["svg:x"]->getDouble();
      y = m_currentGeometry[i]["svg:y"]->getDouble();
      startX = x;
      startY = y;
    }
    else
    {
      x = m_currentGeometry[i]["svg:x"]->getDouble();
      y = m_currentGeometry[i]["svg:y"]->getDouble();
    }
    path.append(m_currentGeometry[i]);
  }
  if (((startX == x && startY == y) || (m_styleProps["draw:fill"] && m_styleProps["draw:fill"]->getStr() != "none")) && path.count())
  {
    WPXPropertyList closedPath;
    closedPath.insert("libwpg:path-action", "Z");
    path.append(closedPath);
  }
  if (path.count() && !m_noShow)
  {
    m_shapeOutputDrawing->addStyle(m_styleProps, WPXPropertyListVector());
    m_shapeOutputDrawing->addPath(path);
  }
  m_currentGeometry.clear();
}

void libvisio::VSDXContentCollector::_flushText()
{
  m_fieldIndex = 0;
  if (!m_textStream.size()) return;
  double angle = 0.0;
  transformAngle(angle, m_txtxform);

  double x = 0.0;
  double y = m_txtxform ? m_txtxform->height: m_xform.height;

  transformPoint(x,y, m_txtxform);

  WPXPropertyList textBlockProps;
  textBlockProps.insert("svg:x", m_scale * x);
  textBlockProps.insert("svg:y", m_scale * y);
  textBlockProps.insert("svg:height", m_scale * (m_txtxform ? m_txtxform->height : m_xform.height));
  textBlockProps.insert("svg:width", m_scale * (m_txtxform ? m_txtxform->width : m_xform.width));
  textBlockProps.insert("fo:padding-top", m_textBlockStyle.topMargin);
  textBlockProps.insert("fo:padding-bottom", m_textBlockStyle.bottomMargin);
  textBlockProps.insert("fo:padding-left", m_textBlockStyle.leftMargin);
  textBlockProps.insert("fo:padding-right", m_textBlockStyle.rightMargin);
  textBlockProps.insert("libwpg:rotate", -angle*180/M_PI, WPX_GENERIC);

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

  for (std::vector<VSDXParaStyle>::iterator paraIt = m_paraFormats.begin();
       paraIt < m_paraFormats.end() || charIndex < m_charFormats.size(); paraIt++)
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
      paraProps.insert("fo:text-align", "left");
      break;
    case 2: // right
      paraProps.insert("fo:text-align", "end");
      break;
    case 4: // justify
      paraProps.insert("fo:text-align", "justify");
      break;
    case 5: // full
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
    // Find char format that overlaps
    while (charIndex < m_charFormats.size() && paraCharCount)
    {
      paraCharCount -= m_charFormats[charIndex].charCount;

      WPXString text;

      if (m_textFormat == VSD_TEXT_UTF16)
      {
        unsigned long max = m_charFormats[charIndex].charCount <= (m_textStream.size()/2) ? m_charFormats[charIndex].charCount : (m_textStream.size()/2);
        VSD_DEBUG_MSG(("Charcount: %d, max: %lu, stream size: %lu\n", m_charFormats[charIndex].charCount, max, (unsigned long)m_textStream.size()));
        max = (m_charFormats[charIndex].charCount == 0 && m_textStream.size()) ? m_textStream.size()/2 : max;
        VSD_DEBUG_MSG(("Charcount: %d, max: %lu, stream size: %lu\n", m_charFormats[charIndex].charCount, max, (unsigned long)m_textStream.size()));
        VSDInternalStream tmpStream(&pTextBuffer[textBufferPosition], max*2);
        _appendUTF16LE(text, &tmpStream);
        textBufferPosition += max*2;
      }
      else
      {
        unsigned long max = m_charFormats[charIndex].charCount <= m_textStream.size() ? m_charFormats[charIndex].charCount : m_textStream.size();
        max = (m_charFormats[charIndex].charCount == 0 && m_textStream.size()) ? m_textStream.size() : max;
        VSDInternalStream tmpStream(&pTextBuffer[textBufferPosition], max);
        while (!tmpStream.atEOS())
        {
          unsigned char character = readU8(&tmpStream);
          if (character <= 0x20)
            _appendUCS4(text, (unsigned) 0x20);
          else
            _appendUCS4(text, (unsigned) character);
        }
        textBufferPosition += max;
      }
      WPXPropertyList textProps;
      if (m_fonts[m_charFormats[charIndex].faceID] == "")
        textProps.insert("style:font-name", m_charFormats[charIndex].face);
      else
        textProps.insert("style:font-name", m_fonts[m_charFormats[charIndex].faceID]);

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
      if (m_textBlockStyle.textBkgndColourId)
      {
        textProps.insert("fo:background-color", getColourString(m_textBlockStyle.textBkgndColour));
#if 0
        if (m_textBlockStyle.textBkgndColour.a)
          textProps.insert("fo:background-opacity", 1.0 - m_textBlockStyle.textBkgndColour.a/255.0, WPX_PERCENT);
#endif
      }

      VSD_DEBUG_MSG(("Text: %s\n", text.cstr()));
      m_shapeOutputText->addStartTextSpan(textProps);
      m_shapeOutputText->addInsertText(text);
      m_shapeOutputText->addEndTextSpan();

      charIndex++;
      if (charIndex < m_charFormats.size() && paraCharCount && m_charFormats[charIndex].charCount > paraCharCount)
      {
        // Insert duplicate
        std::vector<VSDXCharStyle>::iterator charIt = m_charFormats.begin() + charIndex;
        m_charFormats.insert(charIt, m_charFormats[charIndex]);
        m_charFormats[charIndex].charCount = paraCharCount;
        m_charFormats[charIndex+1].charCount -= paraCharCount;
      }
    }
    m_shapeOutputText->addEndTextLine();
  }

  m_shapeOutputText->addEndTextObject();
}

void libvisio::VSDXContentCollector::_flushCurrentForeignData()
{
  m_currentForeignProps.insert("svg:width", m_scale*m_xform.width);
  m_currentForeignProps.insert("svg:height", m_scale*m_xform.height);
  double x = 0.0;
  double y = 0.0;
  transformPoint(x,y);

  m_currentForeignProps.insert("svg:x", m_scale*x);
  // Y axis starts at the bottom not top
  m_currentForeignProps.insert("svg:y", m_scale*(y - m_xform.height));

  if (m_currentForeignData.size() && m_currentForeignProps["libwpg:mime-type"] && !m_noShow)
  {
    m_shapeOutputDrawing->addStyle(m_styleProps, WPXPropertyListVector());
    m_shapeOutputDrawing->addGraphicObject(m_currentForeignProps, m_currentForeignData);
  }
  m_currentForeignData.clear();
  m_currentForeignProps.clear();
}

void libvisio::VSDXContentCollector::_flushCurrentPage()
{
  if (m_pageShapeOrder.size())
  {
    std::stack<std::pair<unsigned, VSDXOutputElementList> > groupTextStack;
    for (std::list<unsigned>::iterator iterList = m_pageShapeOrder.begin(); iterList != m_pageShapeOrder.end(); iterList++)
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
      else if (iterGroup->second != groupTextStack.top().first)
      {
        while (groupTextStack.top().first != iterGroup->second && !groupTextStack.empty())
        {
          m_currentPage.append(groupTextStack.top().second);
          groupTextStack.pop();
        }
      }

      std::map<unsigned, VSDXOutputElementList>::iterator iter;
      iter = m_pageOutputDrawing.find(*iterList);
      if (iter != m_pageOutputDrawing.end())
        m_currentPage.append(iter->second);
      iter = m_pageOutputText.find(*iterList);
      if (iter != m_pageOutputText.end())
        groupTextStack.push(std::make_pair(*iterList, iter->second));
      else
        groupTextStack.push(std::make_pair(*iterList, VSDXOutputElementList()));
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
void libvisio::VSDXContentCollector::collectEllipticalArcTo(unsigned /* id */, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc)
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
    m_currentGeometry.push_back(end);
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
  m_currentGeometry.push_back(arc);
}

void libvisio::VSDXContentCollector::collectEllipse(unsigned /* id */, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop)
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
  m_currentGeometry.push_back(ellipse);
  ellipse.insert("svg:rx",m_scale*rx);
  ellipse.insert("svg:ry",m_scale*ry);
  ellipse.insert("svg:x",m_scale*xtop);
  ellipse.insert("svg:y",m_scale*ytop);
  ellipse.insert("libwpg:large-arc", largeArc?1:0);
  ellipse.insert("libwpg:path-action", "A");
  ellipse.insert("libwpg:rotate", angle * 180/M_PI, WPX_GENERIC);
  m_currentGeometry.push_back(ellipse);
  ellipse.insert("svg:x",m_scale*xleft);
  ellipse.insert("svg:y",m_scale*yleft);
  ellipse.insert("libwpg:large-arc", largeArc?0:1);
  m_currentGeometry.push_back(ellipse);

}

void libvisio::VSDXContentCollector::collectInfiniteLine(unsigned /* id */, unsigned level, double x1, double y1, double x2, double y2)
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

    xmove = points.begin()->first;
    ymove = points.begin()->second;
    for (std::map<double, double>::iterator iter = points.begin(); iter != points.end(); iter++)
    {
      if (iter->first != xmove || iter->second != ymove)
      {
        xline = iter->first;
        yline = iter->second;
      }
    }
  }

  WPXPropertyList infLine;
  infLine.insert("svg:x",m_scale*xmove);
  infLine.insert("svg:y",m_scale*ymove);
  infLine.insert("libwpg:path-action", "M");
  m_currentGeometry.push_back(infLine);
  infLine.insert("svg:x",m_scale*xline);
  infLine.insert("svg:y",m_scale*yline);
  infLine.insert("libwpg:path-action", "L");
  m_currentGeometry.push_back(infLine);
}

void libvisio::VSDXContentCollector::collectLine(unsigned /* id */, unsigned level, double strokeWidth, Colour c, unsigned linePattern, unsigned char startMarker, unsigned char endMarker, unsigned lineCap)
{
  _handleLevelChange(level);
  _lineProperties(strokeWidth, c, linePattern, startMarker, endMarker, lineCap);
}

void libvisio::VSDXContentCollector::collectFillAndShadow(unsigned /* id */, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG,
    unsigned fillPattern, unsigned fillFGTransparency, unsigned fillBGTransparency,
    unsigned shadowPattern, Colour shfgc, double shadowOffsetX, double shadowOffsetY)
{
  _handleLevelChange(level);
  _fillAndShadowProperties(colourIndexFG, colourIndexBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc, shadowOffsetX, shadowOffsetY);
}

void libvisio::VSDXContentCollector::collectFillAndShadow(unsigned id, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern, unsigned fillFGTransparency, unsigned fillBGTransparency, unsigned shadowPattern, Colour shfgc)
{
  collectFillAndShadow(id, level, colourIndexFG, colourIndexBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc, m_shadowOffsetX, m_shadowOffsetY);
}

void libvisio::VSDXContentCollector::collectForeignData(unsigned /* id */, unsigned level, const WPXBinaryData &binaryData)
{
  _handleLevelChange(level);
  _handleForeignData(binaryData);
}

void libvisio::VSDXContentCollector::_handleForeignData(const WPXBinaryData &binaryData)
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
}

void libvisio::VSDXContentCollector::collectGeometry(unsigned /* id */, unsigned level, unsigned char geomFlags)
{
  _handleLevelChange(level);
  m_x = 0.0;
  m_y = 0.0;
  m_originalX = 0.0;
  m_originalY = 0.0;
  bool noFill = ((geomFlags & 1) == 1);
  bool noLine = ((geomFlags & 2) == 2);
  bool noShow = ((geomFlags & 4) == 4);

  if ((m_noFill != noFill) || (m_noLine != noLine) || (m_noShow != noShow) || m_isFirstGeometry)
    _flushCurrentPath();

  m_isFirstGeometry = false;
  m_noFill = noFill;
  m_noLine = noLine;
  m_noShow = noShow;
  if (m_noLine || m_linePattern == 0)
    m_styleProps.insert("draw:stroke", "none");
  else
    m_styleProps.insert("svg:stroke-color", m_lineColour);
  if (m_noFill || m_fillPattern == 0)
    m_styleProps.insert("draw:fill", "none");
  else
  {
    m_styleProps.insert("draw:fill", m_fillType);
    m_styleProps.insert("svg:fill-rule", "evenodd");
  }
  VSD_DEBUG_MSG(("Flag: %d NoFill: %d NoLine: %d NoShow: %d\n", geomFlags, m_noFill, m_noLine, m_noShow));
  m_currentGeometryCount++;
}

void libvisio::VSDXContentCollector::collectMoveTo(unsigned /* id */, unsigned level, double x, double y)
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
  m_currentGeometry.push_back(end);
}

void libvisio::VSDXContentCollector::collectLineTo(unsigned /* id */, unsigned level, double x, double y)
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
  m_currentGeometry.push_back(end);
}

void libvisio::VSDXContentCollector::collectArcTo(unsigned /* id */, unsigned level, double x2, double y2, double bow)
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
    m_currentGeometry.push_back(end);
  }
  else
  {
    WPXPropertyList arc;
    double chord = sqrt(pow((y2 - m_y),2) + pow((x2 - m_x),2));
    double radius = (4 * bow * bow + chord * chord) / (8 * fabs(bow));
    int largeArc = fabs(bow) > radius ? 1 : 0;
    int sweep = bow < 0 ? 1 : 0;

    // If any parent group is flipped, invert sweep
    unsigned shapeId = m_currentShapeId;
    while (true)
    {
      std::map<unsigned, XForm>::iterator iterX = m_groupXForms.find(shapeId);
      if (iterX != m_groupXForms.end())
      {
        XForm xform = iterX->second;
        if (xform.flipX) sweep = sweep == 0 ? 1 : 0;
        if (xform.flipY) sweep = sweep == 0 ? 1 : 0;
      }
      else
        break;
      std::map<unsigned, unsigned>::iterator iter = m_groupMemberships.find(shapeId);
      if (iter != m_groupMemberships.end())
        shapeId = iter->second;
      else
        break;
    }
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
    m_currentGeometry.push_back(arc);
  }
}

#define VSD_NUM_POLYLINES_PER_NURBS 200

void libvisio::VSDXContentCollector::collectNURBSTo(unsigned /* id */, unsigned level, double x2, double y2, unsigned char xType, unsigned char yType, unsigned degree, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights)
{
  _handleLevelChange(level);

  if (!knotVector.size() || !controlPoints.size() || !weights.size())
    // Here, maybe we should just draw line to (x2,y2)
    return;

  // Fill in end knots
  while (knotVector.size() < (controlPoints.size() + degree + 2))
    knotVector.push_back(knotVector.back());

  // Convert control points to static co-ordinates
  for (std::vector<std::pair<double, double> >::iterator it = controlPoints.begin();
       it != controlPoints.end(); it++)
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
    double denominator = 0.0000001;

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
    m_currentGeometry.push_back(NURBS);
  }

  m_originalX = x2;
  m_originalY = y2;
  m_x = x2;
  m_y = y2;
  transformPoint(m_x, m_y);
}

double libvisio::VSDXContentCollector::_NURBSBasis(unsigned knot, unsigned degree, double point, const std::vector<double> &knotVector)
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
void libvisio::VSDXContentCollector::collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, unsigned dataID)
{
  std::map<unsigned, NURBSData>::const_iterator iter;
  NURBSData data;
  if (dataID == 0xFFFFFFFE) // Use stencil NURBS data
  {
    if (!m_stencilShape || m_stencilShape->m_geometries.size() < m_currentGeometryCount)
    {
      _handleLevelChange(level);
      return;
    }

    // Get stencil geometry so as to find stencil NURBS data ID
    VSDXGeometryListElement *element = m_stencilShape->m_geometries[m_currentGeometryCount-1].getElement(id);
    VSDXNURBSTo2 *tmpElement = dynamic_cast<VSDXNURBSTo2 *>(element);
    if (!tmpElement)
    {
      _handleLevelChange(level);
      return;
    }
    dataID = tmpElement->m_dataID;
    iter = m_stencilShape->m_nurbsData.find(dataID);
  }
  else // No stencils involved, directly get dataID and fill in missing parts
  {
    iter = m_NURBSData.find(dataID);
  }

  if (iter != m_NURBSData.end())
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

void libvisio::VSDXContentCollector::collectPolylineTo(unsigned /* id */ , unsigned level, double x, double y, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > &points)
{
  _handleLevelChange(level);

  WPXPropertyList polyline;
  for (unsigned i = 0; i< points.size(); i++)
  {
    polyline.clear();
    if (xType == 0)
      points[i].first *= m_xform.width;
    if (yType == 0)
      points[i].second *= m_xform.height;

    transformPoint(points[i].first, points[i].second);
    polyline.insert("libwpg:path-action", "L");
    polyline.insert("svg:x", m_scale*points[i].first);
    polyline.insert("svg:y", m_scale*points[i].second);
    m_currentGeometry.push_back(polyline);
  }

  m_originalX = x;
  m_originalY = y;
  m_x = x;
  m_y = y;
  transformPoint(m_x, m_y);
  polyline.insert("libwpg:path-action", "L");
  polyline.insert("svg:x", m_scale*m_x);
  polyline.insert("svg:y", m_scale*m_y);
  m_currentGeometry.push_back(polyline);
}

/* Polyline with incomplete data */
void libvisio::VSDXContentCollector::collectPolylineTo(unsigned id, unsigned level, double x, double y, unsigned dataID)
{
  std::map<unsigned, PolylineData>::const_iterator iter;
  if (dataID == 0xFFFFFFFE) // Use stencil polyline data
  {
    if (!m_stencilShape || m_stencilShape->m_geometries.size() < m_currentGeometryCount)
    {
      _handleLevelChange(level);
      return;
    }

    // Get stencil geometry so as to find stencil polyline data ID
    VSDXGeometryListElement *element = m_stencilShape->m_geometries[m_currentGeometryCount-1].getElement(id);
    dataID = dynamic_cast<VSDXPolylineTo2 *>(element)->m_dataID;
    iter = m_stencilShape->m_polylineData.find(dataID);
  }
  else // No stencils involved, directly get dataID
  {
    iter = m_polylineData.find(dataID);
  }

  if (iter != m_polylineData.end())
  {
    PolylineData data = iter->second;
    collectPolylineTo(id, level, x, y, data.xType, data.yType, data.points);
  }
  else
    _handleLevelChange(level);
}

/* NURBS shape data */
void libvisio::VSDXContentCollector::collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, unsigned degree, double lastKnot, std::vector<std::pair<double, double> > controlPoints, std::vector<double> knotVector, std::vector<double> weights)
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
void libvisio::VSDXContentCollector::collectShapeData(unsigned id, unsigned level, unsigned char xType, unsigned char yType, std::vector<std::pair<double, double> > points)
{
  _handleLevelChange(level);
  PolylineData data;
  data.xType = xType;
  data.yType = yType;
  data.points = points;
  m_polylineData[id] = data;
}

void libvisio::VSDXContentCollector::collectXFormData(unsigned /* id */, unsigned level, const XForm &xform)
{
  _handleLevelChange(level);
  m_xform = xform;
}

void libvisio::VSDXContentCollector::collectTxtXForm(unsigned /* id */, unsigned level, const XForm &txtxform)
{
  _handleLevelChange(level);
  if (m_txtxform)
    delete(m_txtxform);
  m_txtxform = new XForm(txtxform);
  m_txtxform->x = m_txtxform->pinX - m_txtxform->pinLocX;
  m_txtxform->y = m_txtxform->pinY - m_txtxform->pinLocY;
}

void libvisio::VSDXContentCollector::applyXForm(double &x, double &y, const XForm &xform)
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

void libvisio::VSDXContentCollector::transformPoint(double &x, double &y, XForm *txtxform)
{
  // We are interested for the while in shapes xforms only
  if (!m_isShapeStarted)
    return;

  if (!m_currentShapeId)
    return;

  unsigned shapeId = m_currentShapeId;

  if (txtxform)
    applyXForm(x, y, *txtxform);

  while (true)
  {
    std::map<unsigned, XForm>::iterator iterX = m_groupXForms.find(shapeId);
    if (iterX != m_groupXForms.end())
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

void libvisio::VSDXContentCollector::transformAngle(double &angle, XForm *txtxform)
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

void libvisio::VSDXContentCollector::collectShapeId(unsigned /* id */, unsigned level, unsigned /* shapeId */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::collectForeignDataType(unsigned /* id */, unsigned level, unsigned foreignType, unsigned foreignFormat)
{
  _handleLevelChange(level);
  m_foreignType = foreignType;
  m_foreignFormat = foreignFormat;
}

void libvisio::VSDXContentCollector::collectPageProps(unsigned /* id */, unsigned level, double pageWidth, double pageHeight, double shadowOffsetX, double shadowOffsetY, double scale)
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

void libvisio::VSDXContentCollector::collectPage(unsigned /* id */, unsigned level, unsigned backgroundPageID, unsigned currentPageID)
{
  _handleLevelChange(level);
  m_currentPage.m_backgroundPageID = backgroundPageID;
  m_currentPage.m_currentPageID = currentPageID;
}

void libvisio::VSDXContentCollector::collectShape(unsigned id, unsigned level, unsigned masterPage, unsigned masterShape, unsigned lineStyleId, unsigned fillStyleId, unsigned textStyleId)
{
  _handleLevelChange(level);

  m_foreignType = 0; // Tracks current foreign data type
  m_foreignFormat = 0; // Tracks foreign data format

  m_originalX = 0.0;
  m_originalY = 0.0;
  m_x = 0;
  m_y = 0;

  // Geometry flags
  m_noLine = false;
  m_noFill = false;
  m_noShow = false;

  // Save line colour and pattern, fill type and pattern
  m_fillType = "none";
  m_fillPattern = 1; // same as "solid"
  m_fillFGTransparency = 0;
  m_fillBGTransparency = 0;

  // Reset style
  m_styleProps.clear();
  m_styleProps.insert("draw:fill", m_fillType);
  m_styleProps.insert("draw:stroke", "solid");

  m_textStream.clear();
  m_charFormats.clear();
  m_paraFormats.clear();

  m_defaultCharStyle = VSDXCharStyle();
  if (m_styles.getCharStyle(0))
    m_defaultCharStyle = *(m_styles.getCharStyle(0));

  m_defaultParaStyle = VSDXParaStyle();
  if (m_styles.getParaStyle(0))
    m_defaultParaStyle = *(m_styles.getParaStyle(0));

  m_textBlockStyle = VSDXTextBlockStyle();
  if (m_styles.getTextBlockStyle(0))
    m_textBlockStyle = *(m_styles.getTextBlockStyle(0));

  m_currentShapeId = id;
  m_pageOutputDrawing[m_currentShapeId] = VSDXOutputElementList();
  m_pageOutputText[m_currentShapeId] = VSDXOutputElementList();
  m_shapeOutputDrawing = &m_pageOutputDrawing[m_currentShapeId];
  m_shapeOutputText = &m_pageOutputText[m_currentShapeId];
  m_isShapeStarted = true;
  m_isFirstGeometry = true;

  // Get stencil shape
  m_stencilShape = 0;
  if (masterPage != 0xffffffff && masterShape != 0xffffffff)
  {
    const VSDXStencil *stencil = m_stencils.getStencil(masterPage);
    if (stencil)
      m_stencilShape = stencil->getStencilShape(masterShape);
    // Initialize the shape from stencil content
    if (m_stencilShape)
    {
      if (m_stencilShape->m_foreign)
      {
        m_foreignType = m_stencilShape->m_foreign->type;
        m_foreignFormat = m_stencilShape->m_foreign->format;
        _handleForeignData(m_stencilShape->m_foreign->data);
      }

      m_textStream = m_stencilShape->m_text;
      m_textFormat = m_stencilShape->m_textFormat;

      m_names.clear();
      for (std::vector<WPXBinaryData>::const_iterator iterData = m_stencilShape->m_names.begin(); iterData != m_stencilShape->m_names.end(); ++iterData)
      {
        WPXString nameString;
        _convertDataToString(nameString, *(iterData), m_stencilShape->m_textFormat);
        m_names.push_back(nameString);
      }

      m_fields = m_stencilShape->m_fields;

      if (m_stencilShape->m_lineStyleId)
        lineStyleFromStyleSheet(m_stencilShape->m_lineStyleId);
      if (m_stencilShape->m_lineStyle)
        lineStyleFromStyleSheet(m_stencilShape->m_lineStyle);

      if (m_stencilShape->m_fillStyleId)
        fillStyleFromStyleSheet(m_stencilShape->m_fillStyleId);
      if (m_stencilShape->m_fillStyle)
        fillStyleFromStyleSheet(m_stencilShape->m_fillStyle);

      if (m_stencilShape->m_textStyleId)
      {
        if (m_styles.getCharStyle(m_stencilShape->m_textStyleId))
          m_defaultCharStyle = *(m_styles.getCharStyle(m_stencilShape->m_textStyleId));
        if (m_styles.getParaStyle(m_stencilShape->m_textStyleId))
          m_defaultParaStyle = *(m_styles.getParaStyle(m_stencilShape->m_textStyleId));
        if (m_styles.getTextBlockStyle(m_stencilShape->m_textStyleId))
          m_textBlockStyle = *(m_styles.getTextBlockStyle(m_stencilShape->m_textStyleId));
      }
      if (m_stencilShape->m_textBlockStyle)
        m_textBlockStyle = *(m_stencilShape->m_textBlockStyle);
      if (m_stencilShape->m_charStyle)
        m_defaultCharStyle = *(m_stencilShape->m_charStyle);
      if (m_stencilShape->m_paraStyle)
        m_defaultParaStyle = *(m_stencilShape->m_paraStyle);
    }
  }

  if (lineStyleId != 0xffffffff)
    lineStyleFromStyleSheet(lineStyleId);
  if (fillStyleId != 0xffffffff)
    fillStyleFromStyleSheet(fillStyleId);
  if (textStyleId != 0xffffffff)
  {
    if (m_styles.getCharStyle(textStyleId))
      m_defaultCharStyle = *(m_styles.getCharStyle(textStyleId));
    if (m_styles.getParaStyle(textStyleId))
      m_defaultParaStyle = *(m_styles.getParaStyle(textStyleId));
    if (m_styles.getTextBlockStyle(textStyleId))
      m_textBlockStyle = *(m_styles.getTextBlockStyle(textStyleId));
  }

  m_currentGeometryCount = 0;
  m_fieldIndex = 0;
}

void libvisio::VSDXContentCollector::collectUnhandledChunk(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::collectColours(const std::vector<Colour> &colours)
{
  m_colours.clear();
  m_colours.reserve(colours.size());
  for (unsigned i = 0; i < colours.size(); i++)
    m_colours.push_back(colours[i]);
}

void libvisio::VSDXContentCollector::collectFont(unsigned short fontID, const WPXBinaryData &textStream, TextFormat format)
{
  WPXString fontname;
  _convertDataToString(fontname, textStream, format);
  m_fonts[fontID] = fontname;
}


void libvisio::VSDXContentCollector::collectSplineStart(unsigned /* id */, unsigned level, double x, double y, double secondKnot, double firstKnot, double lastKnot, unsigned degree)
{
  m_splineLevel = level;
  m_splineKnotVector.push_back(firstKnot);
  m_splineKnotVector.push_back(secondKnot);
  m_splineLastKnot = lastKnot;
  m_splineX = x;
  m_splineY = y;
  m_splineDegree = degree;
}


void libvisio::VSDXContentCollector::collectSplineKnot(unsigned /* id */, unsigned /* level */, double x, double y, double knot)
{
  m_splineKnotVector.push_back(knot);
  m_splineControlPoints.push_back(std::pair<double,double>(m_splineX,m_splineY));
  m_splineX = x;
  m_splineY = y;
}


void libvisio::VSDXContentCollector::collectSplineEnd()
{
  if (!m_splineKnotVector.size() || !m_splineControlPoints.size())
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


void libvisio::VSDXContentCollector::collectText(unsigned /*id*/, unsigned level, const WPXBinaryData &textStream, TextFormat format)
{
  _handleLevelChange(level);

  m_textStream = textStream;
  m_textFormat = format;
}

void libvisio::VSDXContentCollector::collectVSDXParaStyle(unsigned /* id */ , unsigned level, unsigned charCount, double indFirst, double indLeft, double indRight,
    double spLine, double spBefore, double spAfter, unsigned char align)
{
  _handleLevelChange(level);
  VSDXParaStyle format(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align);
  m_paraFormats.push_back(format);
}

void libvisio::VSDXContentCollector::collectVSDXCharStyle(unsigned /*id*/ , unsigned level, unsigned charCount, unsigned short fontID,
    Colour fontColour, unsigned langId, double fontSize, bool bold, bool italic,
    bool underline, bool doubleunderline, bool strikeout, bool doublestrikeout,
    bool allcaps, bool initcaps, bool smallcaps, bool superscript, bool subscript, WPXString fontFace)
{
  _handleLevelChange(level);
  VSDXCharStyle format(charCount, fontID, fontColour, langId, fontSize, bold, italic,
                       underline, doubleunderline, strikeout, doublestrikeout,
                       allcaps, initcaps, smallcaps, superscript, subscript, fontFace);
  m_charFormats.push_back(format);
}

void libvisio::VSDXContentCollector::collectTextBlock(unsigned /* id */, unsigned level, double leftMargin, double rightMargin,
    double topMargin, double bottomMargin,  unsigned char verticalAlign, unsigned char bgClrId,
    const Colour &bgColour, double defaultTabStop,  unsigned char textDirection)
{
  _handleLevelChange(level);
  m_textBlockStyle = VSDXTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin, verticalAlign, bgClrId, bgColour, defaultTabStop, textDirection);
}

void libvisio::VSDXContentCollector::collectNameList(unsigned /*id*/, unsigned level)
{
  _handleLevelChange(level);

  m_names.clear();
}

void libvisio::VSDXContentCollector::_convertDataToString(WPXString &result, const WPXBinaryData &data, TextFormat format)
{
  m_fieldIndex = 0;
  WPXInputStream *pStream = const_cast<WPXInputStream *>(data.getDataStream());
  if (format == VSD_TEXT_ANSI)
  {
    while (!pStream->atEOS())
    {
      unsigned char character = readU8(pStream);
      if (character <= 0x20)
        _appendUCS4(result, (unsigned) 0x20);
      else
        _appendUCS4(result, (unsigned) character);
    }
  }
  else if (format == VSD_TEXT_UTF16)
  {
    _appendUTF16LE(result, pStream);
  }
}

void libvisio::VSDXContentCollector::collectName(unsigned /*id*/, unsigned level, const WPXBinaryData &name, TextFormat format)
{
  _handleLevelChange(level);

  WPXString nameString;
  _convertDataToString(nameString, name, format);
  m_names.push_back(nameString);
}

void libvisio::VSDXContentCollector::collectStyleSheet(unsigned /* id */, unsigned level, unsigned /* parentLineStyle */, unsigned /* parentFillStyle */, unsigned /* parentTextStyle */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::collectLineStyle(unsigned /* id */, unsigned level, double /* strokeWidth */, Colour /* c */, unsigned char /* linePattern */,  unsigned char /*startMarker*/, unsigned char /*endMarker*/, unsigned char /* lineCap */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::collectFillStyle(unsigned /*id*/, unsigned level, unsigned char /*colourIndexFG*/, unsigned char /*colourIndexBG*/, unsigned char /*fillPattern*/, unsigned char /*fillFGTransparency*/, unsigned char /*fillBGTransparency*/, unsigned char /*shadowPattern*/, Colour /*shfgc*/, double /*shadowOffsetX*/, double /*shadowOffsetY*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::collectFillStyle(unsigned /*id*/, unsigned level, unsigned char /*colourIndexFG*/, unsigned char /*colourIndexBG*/, unsigned char /*fillPattern*/, unsigned char /*fillFGTransparency*/, unsigned char /*fillBGTransparency*/, unsigned char /*shadowPattern*/, Colour /*shfgc*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::collectCharIXStyle(unsigned /*id*/ , unsigned level, unsigned /*charCount*/, unsigned short /*fontID*/, Colour /*fontColour*/, unsigned /*langId*/, double /*fontSize*/,
    bool /*bold*/, bool /*italic*/, bool /*underline*/, bool /* doubleunderline */, bool /* strikeout */, bool /* doublestrikeout */,
    bool /* allcaps */, bool /* initcaps */, bool /* smallcaps */, bool /* superscript */, bool /* subscript */, WPXString /*fontFace*/)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::collectParaIXStyle(unsigned /* id */, unsigned level, unsigned /* charCount */, double /* indFirst */, double /* indLeft */, double /* indRight */,
    double /* spLine */, double /* spBefore */, double /* spAfter */, unsigned char /* align */)
{
  _handleLevelChange(level);
}


void libvisio::VSDXContentCollector::collectTextBlockStyle(unsigned /* id */, unsigned level, double /* leftMargin */, double /* rightMargin */,
    double /* topMargin */, double /* bottomMargin */,  unsigned char /* verticalAlign */, unsigned char /* bgClrId */,
    const Colour & /* colour */, double /* defaultTabStop */,  unsigned char /* textDirection */)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::lineStyleFromStyleSheet(unsigned styleId)
{
  lineStyleFromStyleSheet(m_styles.getLineStyle(styleId));
}

void libvisio::VSDXContentCollector::lineStyleFromStyleSheet(const VSDXLineStyle *style)
{
  if (style)
    _lineProperties(style->width, style->colour, style->pattern, style->startMarker, style->endMarker, style->cap);
}

void libvisio::VSDXContentCollector::fillStyleFromStyleSheet(unsigned styleId)
{
  fillStyleFromStyleSheet(m_styles.getFillStyle(styleId));
}

void libvisio::VSDXContentCollector::fillStyleFromStyleSheet(const VSDXFillStyle *style)
{
  if (style)
    _fillAndShadowProperties(style->fgColourId, style->bgColourId, style->pattern, style->fgTransparency, style->bgTransparency,
                             style->shadowPattern, style->shadowFgColour, style->shadowOffsetX, style->shadowOffsetY);
}

void libvisio::VSDXContentCollector::collectFieldList(unsigned /* id */, unsigned level, const std::vector<unsigned> &fieldsOrder)
{
  _handleLevelChange(level);
  m_fields.clear();
  m_fields.setElementsOrder(fieldsOrder);
}

void libvisio::VSDXContentCollector::collectTextField(unsigned id, unsigned level, unsigned nameId)
{
  _handleLevelChange(level);
  m_fields.addTextField(id, level, nameId);
}

void libvisio::VSDXContentCollector::collectNumericField(unsigned id, unsigned level, unsigned format, double number)
{
  _handleLevelChange(level);
  m_fields.addNumericField(id, level, format, number);
}

void libvisio::VSDXContentCollector::collectDatetimeField(unsigned id, unsigned level, unsigned format, unsigned long timeValue)
{
  _handleLevelChange(level);
  m_fields.addDatetimeField(id, level, format, timeValue);
}

void libvisio::VSDXContentCollector::collectEmptyField(unsigned id, unsigned level)
{
  _handleLevelChange(level);
  m_fields.addEmptyField(id, level);
}


void libvisio::VSDXContentCollector::_handleLevelChange(unsigned level)
{
  if (m_currentLevel == level)
    return;
  if (level < 2)
  {
    if (m_isShapeStarted)
    {
      if (m_stencilShape != 0 && !m_isStencilStarted)
      {
        m_isStencilStarted = true;
        m_NURBSData = m_stencilShape->m_nurbsData;
        m_polylineData = m_stencilShape->m_polylineData;

        if (m_currentGeometry.size() == 0)
        {
          for (unsigned i = 0; i < m_stencilShape->m_geometries.size(); i++)
          {
            m_x = 0.0;
            m_y = 0.0;
            m_stencilShape->m_geometries[i].handle(this);
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


void libvisio::VSDXContentCollector::startPage()
{
  if (m_isShapeStarted)
  {
    _flushCurrentPath();
    _flushCurrentForeignData();
    m_isShapeStarted = false;
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
    m_groupXForms = m_groupXFormsSequence[m_currentPageNumber-1];
  if (m_groupMembershipsSequence.size() >= m_currentPageNumber)
    m_groupMemberships = m_groupMembershipsSequence[m_currentPageNumber-1];
  if (m_documentPageShapeOrders.size() >= m_currentPageNumber)
    m_pageShapeOrder = m_documentPageShapeOrders[m_currentPageNumber-1];
  m_currentPage = libvisio::VSDXPage();
  m_isPageStarted = true;
}

void libvisio::VSDXContentCollector::endPage()
{
  if (m_isPageStarted)
  {
    _handleLevelChange(0);
    _flushCurrentPage();
    m_pages.addPage(m_currentPage);
    m_isPageStarted = false;
  }
}

void libvisio::VSDXContentCollector::endPages()
{
  m_pages.draw(m_painter);
}

#define SURROGATE_VALUE(h,l) (((h) - 0xd800) * 0x400 + (l) - 0xdc00 + 0x10000)

void libvisio::VSDXContentCollector::_appendUTF16LE(WPXString &text, WPXInputStream *input)
{
  while (!input->atEOS())
  {
    uint16_t high_surrogate = 0;
    bool fail = false;
    uint32_t ucs4Character = 0;
    uint16_t character = 0;
    while (true)
    {
      if (input->atEOS())
      {
        fail = true;
        break;
      }
      character = readU16(input);
      if (character == 0xfffc)
        text.append(m_fields.getElement(m_fieldIndex++)->getString(m_names).cstr());
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
      throw GenericException();

    _appendUCS4(text, ucs4Character);
  }
}

void libvisio::VSDXContentCollector::_appendUCS4(WPXString &text, unsigned ucs4Character)
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

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
