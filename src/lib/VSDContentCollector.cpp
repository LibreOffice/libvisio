/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string.h> // for memcpy
#include <stack>
#include <boost/spirit/include/classic.hpp>
#include <unicode/ucnv.h>
#include <unicode/utypes.h>
#include <unicode/utf8.h>

#include "VSDContentCollector.h"
#include "VSDParser.h"
#include "VSDInternalStream.h"

#ifndef DUMP_BITMAP
#define DUMP_BITMAP 0
#endif

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

static void _appendUCS4(librevenge::RVNGString &text, UChar32 ucs4Character)
{
  // Convert carriage returns to new line characters
  // Writerperfect/LibreOffice will replace them by <text:line-break>
  if (ucs4Character == (UChar32) 0x0d || ucs4Character == (UChar32) 0x0e)
    ucs4Character = (UChar32) '\n';

  unsigned char outbuf[U8_MAX_LENGTH+1];
  int i = 0;
  U8_APPEND_UNSAFE(&outbuf[0], i, ucs4Character);
  outbuf[i] = 0;

  text.append((char *)outbuf);
}

} // anonymous namespace


libvisio::VSDContentCollector::VSDContentCollector(
  librevenge::RVNGDrawingInterface *painter,
  std::vector<std::map<unsigned, XForm> > &groupXFormsSequence,
  std::vector<std::map<unsigned, unsigned> > &groupMembershipsSequence,
  std::vector<std::list<unsigned> > &documentPageShapeOrders,
  VSDStyles &styles, VSDStencils &stencils
) :
  m_painter(painter), m_isPageStarted(false), m_pageWidth(0.0), m_pageHeight(0.0),
  m_shadowOffsetX(0.0), m_shadowOffsetY(0.0),
  m_scale(1.0), m_x(0.0), m_y(0.0), m_originalX(0.0), m_originalY(0.0), m_xform(), m_txtxform(0), m_misc(),
  m_currentFillGeometry(), m_currentLineGeometry(), m_groupXForms(groupXFormsSequence.empty() ? 0 : &groupXFormsSequence[0]),
  m_currentForeignData(), m_currentOLEData(), m_currentForeignProps(), m_currentShapeId(0), m_foreignType((unsigned)-1),
  m_foreignFormat(0), m_foreignOffsetX(0.0), m_foreignOffsetY(0.0), m_foreignWidth(0.0), m_foreignHeight(0.0),
  m_noLine(false), m_noFill(false), m_noShow(false), m_fonts(),
  m_currentLevel(0), m_isShapeStarted(false),
  m_groupXFormsSequence(groupXFormsSequence), m_groupMembershipsSequence(groupMembershipsSequence),
  m_groupMemberships(m_groupMembershipsSequence.begin()),
  m_currentPageNumber(0), m_shapeOutputDrawing(0), m_shapeOutputText(0),
  m_pageOutputDrawing(), m_pageOutputText(), m_documentPageShapeOrders(documentPageShapeOrders),
  m_pageShapeOrder(m_documentPageShapeOrders.begin()), m_isFirstGeometry(true), m_NURBSData(), m_polylineData(),
  m_textStream(), m_names(), m_stencilNames(), m_fields(), m_stencilFields(), m_fieldIndex(0),
  m_textFormat(VSD_TEXT_ANSI), m_charFormats(), m_paraFormats(), m_lineStyle(), m_fillStyle(), m_textBlockStyle(),
  m_themeReference(), m_defaultCharStyle(), m_defaultParaStyle(), m_currentStyleSheet(0), m_styles(styles),
  m_stencils(stencils), m_stencilShape(0), m_isStencilStarted(false), m_currentGeometryCount(0),
  m_backgroundPageID(MINUS_ONE), m_currentPageID(0), m_currentPage(), m_pages(),
  m_splineControlPoints(), m_splineKnotVector(), m_splineX(0.0), m_splineY(0.0),
  m_splineLastKnot(0.0), m_splineDegree(0), m_splineLevel(0), m_currentShapeLevel(0),
  m_isBackgroundPage(false)
{
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

void libvisio::VSDContentCollector::_flushShape()
{
  unsigned numPathElements = 0;
  unsigned numForeignElements = 0;
  unsigned numTextElements = 0;
  if (m_fillStyle.pattern && !m_currentFillGeometry.empty())
    numPathElements++;
  if (m_lineStyle.pattern && !m_currentLineGeometry.empty())
    numPathElements++;
  if (m_currentForeignData.size() && m_currentForeignProps["librevenge:mime-type"] && m_foreignWidth != 0.0 && m_foreignHeight != 0.0)
    numForeignElements++;
  if (m_textStream.size())
    numTextElements++;

  if (numPathElements+numForeignElements+numTextElements > 1)
    m_shapeOutputDrawing->addStartLayer(librevenge::RVNGPropertyList());

  if (numPathElements > 1 && (numForeignElements || numTextElements))
    m_shapeOutputDrawing->addStartLayer(librevenge::RVNGPropertyList());
  _flushCurrentPath();
  if (numPathElements > 1 && (numForeignElements || numTextElements))
    m_shapeOutputDrawing->addEndLayer();
  _flushCurrentForeignData();
  _flushText();

  if (numPathElements+numForeignElements+numTextElements > 1)
  {
    if (numTextElements)
      m_shapeOutputText->addEndLayer();
    else
      m_shapeOutputDrawing->addEndLayer();
  }

  m_isShapeStarted = false;
}

void libvisio::VSDContentCollector::_flushCurrentPath()
{
  librevenge::RVNGPropertyList styleProps;
  _lineProperties(m_lineStyle, styleProps);
  _fillAndShadowProperties(m_fillStyle, styleProps);
  librevenge::RVNGPropertyList fillPathProps(styleProps);
  fillPathProps.insert("draw:stroke", "none");
  librevenge::RVNGPropertyList linePathProps(styleProps);
  linePathProps.insert("draw:fill", "none");

  std::vector<librevenge::RVNGPropertyList> tmpPath;
  if (m_fillStyle.pattern && !m_currentFillGeometry.empty())
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
      else if (m_currentFillGeometry[i]["librevenge:path-action"]->getStr() == "M")
      {
        if (!tmpPath.empty())
        {
          if (!wasMove)
          {
            if (tmpPath.back()["librevenge:path-action"]->getStr() != "Z")
            {
              librevenge::RVNGPropertyList closedPath;
              closedPath.insert("librevenge:path-action", "Z");
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
        if (tmpPath.back()["librevenge:path-action"]->getStr() != "Z")
        {
          librevenge::RVNGPropertyList closedPath;
          closedPath.insert("librevenge:path-action", "Z");
          tmpPath.push_back(closedPath);
        }
      }
      else
        tmpPath.pop_back();
    }
    if (!tmpPath.empty())
    {
      librevenge::RVNGPropertyListVector path;
      for (unsigned i = 0; i < tmpPath.size(); ++i)
        path.append(tmpPath[i]);
      m_shapeOutputDrawing->addStyle(fillPathProps);
      librevenge::RVNGPropertyList propList;
      propList.insert("svg:d", path);
      m_shapeOutputDrawing->addPath(propList);
    }
  }
  m_currentFillGeometry.clear();
  tmpPath.clear();

  if (m_lineStyle.pattern && !m_currentLineGeometry.empty())
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
      else if (m_currentLineGeometry[i]["librevenge:path-action"]->getStr() == "M")
      {
        if (!tmpPath.empty())
        {
          if (!wasMove)
          {
            if ((x == prevX) && (y == prevY))
            {
              if (tmpPath.back()["librevenge:path-action"]->getStr() != "Z")
              {
                librevenge::RVNGPropertyList closedPath;
                closedPath.insert("librevenge:path-action", "Z");
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
          if (tmpPath.back()["librevenge:path-action"]->getStr() != "Z")
          {
            librevenge::RVNGPropertyList closedPath;
            closedPath.insert("librevenge:path-action", "Z");
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
      librevenge::RVNGPropertyListVector path;
      for (unsigned i = 0; i < tmpPath.size(); ++i)
        path.append(tmpPath[i]);
      m_shapeOutputDrawing->addStyle(linePathProps);
      librevenge::RVNGPropertyList propList;
      propList.insert("svg:d", path);
      m_shapeOutputDrawing->addPath(propList);
    }
  }
  m_currentLineGeometry.clear();
}

void libvisio::VSDContentCollector::_flushText()
{
  if (!m_textStream.size() || m_misc.m_hideText)
    return;

  double xmiddle = m_txtxform ? m_txtxform->width / 2.0 : m_xform.width / 2.0;
  double ymiddle = m_txtxform ? m_txtxform->height / 2.0 : m_xform.height / 2.0;

  transformPoint(xmiddle,ymiddle, m_txtxform);

  double x = xmiddle - (m_txtxform ? m_txtxform->width / 2.0 : m_xform.width / 2.0);
  double y = ymiddle - (m_txtxform ? m_txtxform->height / 2.0 : m_xform.height / 2.0);

  double angle = 0.0;
  transformAngle(angle, m_txtxform);

  librevenge::RVNGPropertyList textBlockProps;

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
  textBlockProps.insert("librevenge:rotate", angle*180/M_PI, librevenge::RVNG_GENERIC);

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

  unsigned numCharsInText = (unsigned)(m_textFormat == VSD_TEXT_UTF16 ? m_textStream.size() / 2 : m_textStream.size());

  for (unsigned iChar = 0; iChar < m_charFormats.size(); iChar++)
  {
    if (m_charFormats[iChar].charCount)
      numCharsInText -= m_charFormats[iChar].charCount;
    else
      m_charFormats[iChar].charCount = numCharsInText;
  }

  numCharsInText = (unsigned)(m_textFormat == VSD_TEXT_UTF16 ? m_textStream.size() / 2 : m_textStream.size());

  for (unsigned iPara = 0; iPara < m_paraFormats.size(); iPara++)
  {
    if (m_paraFormats[iPara].charCount)
      numCharsInText -= m_paraFormats[iPara].charCount;
    else
      m_paraFormats[iPara].charCount = numCharsInText;
  }

  m_shapeOutputText->addStartTextObject(textBlockProps);

  unsigned int charIndex = 0;
  unsigned int paraCharCount = 0;
  unsigned long textBufferPosition = 0;
  const unsigned char *pTextBuffer = m_textStream.getDataBuffer();
  const unsigned long nTextBufferLength = m_textStream.size();

  for (std::vector<VSDParaStyle>::iterator paraIt = m_paraFormats.begin();
       paraIt != m_paraFormats.end() && charIndex < m_charFormats.size(); ++paraIt)
  {
    librevenge::RVNGPropertyList paraProps;

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
      paraProps.insert("fo:line-height", (*paraIt).spLine);
    else
      paraProps.insert("fo:line-height", -(*paraIt).spLine, librevenge::RVNG_PERCENT);

    m_shapeOutputText->addOpenParagraph(paraProps);

    paraCharCount = (*paraIt).charCount;

    // Find char format that overlaps
    while (charIndex < m_charFormats.size() && paraCharCount)
    {
      paraCharCount -= m_charFormats[charIndex].charCount;

      librevenge::RVNGPropertyList textProps;

      librevenge::RVNGString fontName;
      if (m_charFormats[charIndex].font.m_data.size())
        _convertDataToString(fontName, m_charFormats[charIndex].font.m_data, m_charFormats[charIndex].font.m_format);
      else
        fontName = "Arial";

      textProps.insert("style:font-name", fontName);

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
      textProps.insert("fo:font-size", m_charFormats[charIndex].size*72.0, librevenge::RVNG_POINT);
      textProps.insert("fo:color", getColourString(m_charFormats[charIndex].colour));
      double opacity = 1.0;
      if (m_charFormats[charIndex].colour.a)
        opacity -= (double)(m_charFormats[charIndex].colour.a)/255.0;
      textProps.insert("svg:stroke-opacity", opacity, librevenge::RVNG_PERCENT);
      textProps.insert("svg:fill-opacity", opacity, librevenge::RVNG_PERCENT);
      // TODO: In draw, text span background cannot be specified the same way as in writer span
      if (m_textBlockStyle.isTextBkgndFilled)
      {
        textProps.insert("fo:background-color", getColourString(m_textBlockStyle.textBkgndColour));
#if 0
        if (m_textBlockStyle.textBkgndColour.a)
          textProps.insert("fo:background-opacity", 1.0 - m_textBlockStyle.textBkgndColour.a/255.0, librevenge::RVNG_PERCENT);
#endif
      }

      librevenge::RVNGString text;

      if (m_textFormat == VSD_TEXT_UTF16)
      {
        unsigned long max = m_charFormats[charIndex].charCount <= (m_textStream.size()/2) ? m_charFormats[charIndex].charCount : (m_textStream.size()/2);
        VSD_DEBUG_MSG(("Charcount: %d, max: %lu, stream size: %lu\n", m_charFormats[charIndex].charCount, max, (unsigned long)m_textStream.size()));
        max = (m_charFormats[charIndex].charCount == 0 && m_textStream.size()) ? m_textStream.size()/2 : max;
        VSD_DEBUG_MSG(("Charcount: %d, max: %lu, stream size: %lu\n", m_charFormats[charIndex].charCount, max, (unsigned long)m_textStream.size()));
        std::vector<unsigned char> tmpBuffer;
        unsigned i = 0;
        for (; i < max*2 && textBufferPosition+i <nTextBufferLength; ++i)
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
            if (tmpBuffer[tmpBuffer.size() - 1] == 0 && (tmpBuffer[tmpBuffer.size() - 2] == 0x0a ||
                                                         tmpBuffer[tmpBuffer.size() - 2] == '\n' || tmpBuffer[tmpBuffer.size() - 2] == 0x0e))
            {
              tmpBuffer.pop_back();
              tmpBuffer.pop_back();
            }
          }
          else
            tmpBuffer.clear();
        }

        if (!tmpBuffer.empty())
          appendCharacters(text, tmpBuffer);
        textBufferPosition += i;
      }
      else if (m_textFormat == VSD_TEXT_UTF8)
      {
        unsigned long max = m_charFormats[charIndex].charCount <= m_textStream.size() ? m_charFormats[charIndex].charCount : m_textStream.size();
        std::vector<unsigned char> tmpBuffer;
        unsigned i = 0;
        for (; i < max && textBufferPosition+i <nTextBufferLength; ++i)
          tmpBuffer.push_back(pTextBuffer[textBufferPosition+i]);
        if (!paraCharCount && !tmpBuffer.empty())
        {
          while (!tmpBuffer.empty() && tmpBuffer.back() == 0)
            tmpBuffer.pop_back();
          if (!tmpBuffer.empty() && (tmpBuffer.back() == 0x0a || tmpBuffer.back() == '\n' || tmpBuffer.back() == 0x0e))
            tmpBuffer.back() = 0;
        }
        if (!tmpBuffer.empty() && tmpBuffer[0])
          appendCharacters(text, tmpBuffer, VSD_TEXT_UTF8);
        textBufferPosition += i;
      }
      else
      {
        unsigned long max = m_charFormats[charIndex].charCount <= m_textStream.size() ? m_charFormats[charIndex].charCount : m_textStream.size();
        max = (m_charFormats[charIndex].charCount == 0 && m_textStream.size()) ? m_textStream.size() : max;
        std::vector<unsigned char> tmpBuffer;
        unsigned i = 0;
        for (; i < max && textBufferPosition+i <nTextBufferLength; ++i)
          tmpBuffer.push_back(pTextBuffer[textBufferPosition+i]);
        if (!paraCharCount && !tmpBuffer.empty())
        {
          while (!tmpBuffer.empty() && tmpBuffer.back() == 0)
            tmpBuffer.pop_back();
          if (!tmpBuffer.empty() && (tmpBuffer.back() == 0x0a || tmpBuffer.back() == '\n' || tmpBuffer.back() == 0x0e))
            tmpBuffer.back() = 0;
        }
        if (!tmpBuffer.empty())
          appendCharacters(text, tmpBuffer, m_charFormats[charIndex].font.m_format);
        textBufferPosition += i;
      }

      VSD_DEBUG_MSG(("Text: %s\n", text.cstr()));
      m_shapeOutputText->addOpenSpan(textProps);
      m_shapeOutputText->addInsertText(text);
      m_shapeOutputText->addCloseSpan();

      charIndex++;
      if (charIndex < m_charFormats.size() && paraCharCount && m_charFormats[charIndex].charCount > paraCharCount)
      {
        // Insert duplicate
        std::vector<VSDCharStyle>::iterator charIt = m_charFormats.begin() + charIndex;
        VSDCharStyle tmpCharFormat = m_charFormats[charIndex];
        m_charFormats.insert(charIt, tmpCharFormat);
        m_charFormats[charIndex].charCount = paraCharCount;
        m_charFormats[charIndex+1].charCount -= paraCharCount;
      }
    }
    m_shapeOutputText->addCloseParagraph();
  }

  m_shapeOutputText->addEndTextObject();
  m_textStream.clear();
}

void libvisio::VSDContentCollector::_flushCurrentForeignData()
{
  double xmiddle = m_foreignOffsetX + m_foreignWidth / 2.0;
  double ymiddle = m_foreignOffsetY + m_foreignHeight / 2.0;

  transformPoint(xmiddle, ymiddle);

  bool flipX = false;
  bool flipY = false;

  transformFlips(flipX, flipY);

  librevenge::RVNGPropertyList styleProps;

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
    m_currentForeignProps.insert("librevenge:rotate", angle * 180 / M_PI, librevenge::RVNG_GENERIC);

  if (m_currentForeignData.size() && m_currentForeignProps["librevenge:mime-type"] && m_foreignWidth != 0.0 && m_foreignHeight != 0.0)
  {
    m_shapeOutputDrawing->addStyle(styleProps);
    m_currentForeignProps.insert("office:binary-data", m_currentForeignData);
    m_shapeOutputDrawing->addGraphicObject(m_currentForeignProps);
  }
  m_currentForeignData.clear();
  m_currentForeignProps.clear();
}

void libvisio::VSDContentCollector::_flushCurrentPage()
{
  if (m_pageShapeOrder != m_documentPageShapeOrders.end() && !m_pageShapeOrder->empty() &&
      m_groupMemberships != m_groupMembershipsSequence.end())
  {
    std::stack<std::pair<unsigned, VSDOutputElementList> > groupTextStack;
    for (std::list<unsigned>::iterator iterList = m_pageShapeOrder->begin(); iterList != m_pageShapeOrder->end(); ++iterList)
    {
      std::map<unsigned, unsigned>::iterator iterGroup = m_groupMemberships->find(*iterList);
      if (iterGroup == m_groupMemberships->end())
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
    librevenge::RVNGPropertyList end;
    end.insert("svg:x", m_scale*m_x);
    end.insert("svg:y", m_scale*m_y);
    end.insert("librevenge:path-action", "L");
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
  librevenge::RVNGPropertyList arc;
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
  arc.insert("librevenge:rotate", angle * 180 / M_PI, librevenge::RVNG_GENERIC);
  arc.insert("librevenge:large-arc", largeArc);
  arc.insert("librevenge:sweep", sweep);
  arc.insert("svg:x", m_scale*m_x);
  arc.insert("svg:y", m_scale*m_y);
  arc.insert("librevenge:path-action", "A");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(arc);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(arc);
}

void libvisio::VSDContentCollector::collectEllipse(unsigned /* id */, unsigned level, double cx, double cy, double xleft, double yleft, double xtop, double ytop)
{
  _handleLevelChange(level);
  librevenge::RVNGPropertyList ellipse;
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
  ellipse.insert("librevenge:path-action", "M");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(ellipse);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(ellipse);
  ellipse.insert("svg:rx",m_scale*rx);
  ellipse.insert("svg:ry",m_scale*ry);
  ellipse.insert("svg:x",m_scale*xtop);
  ellipse.insert("svg:y",m_scale*ytop);
  ellipse.insert("librevenge:large-arc", largeArc?1:0);
  ellipse.insert("librevenge:path-action", "A");
  ellipse.insert("librevenge:rotate", angle * 180/M_PI, librevenge::RVNG_GENERIC);
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(ellipse);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(ellipse);
  ellipse.insert("svg:x",m_scale*xleft);
  ellipse.insert("svg:y",m_scale*yleft);
  ellipse.insert("librevenge:large-arc", largeArc?0:1);
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(ellipse);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(ellipse);
  ellipse.clear();
  ellipse.insert("librevenge:path-action", "Z");
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

  librevenge::RVNGPropertyList infLine;
  infLine.insert("svg:x",m_scale*xmove);
  infLine.insert("svg:y",m_scale*ymove);
  infLine.insert("librevenge:path-action", "M");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(infLine);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(infLine);
  infLine.insert("svg:x",m_scale*xline);
  infLine.insert("svg:y",m_scale*yline);
  infLine.insert("librevenge:path-action", "L");
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(infLine);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(infLine);
}

void libvisio::VSDContentCollector::collectRelCubBezTo(unsigned /* id */, unsigned level, double x, double y, double x1, double y1, double x2, double y2)
{
  _handleLevelChange(level);
  x *= m_xform.width;
  y *= m_xform.height;
  x1 *= m_xform.width;
  y1 *= m_xform.height;
  x2 *= m_xform.width;
  y2 *= m_xform.height;
  transformPoint(x1, y1);
  transformPoint(x2, y2);
  m_originalX = x;
  m_originalY = y;
  transformPoint(x, y);
  m_x = x;
  m_y = y;
  librevenge::RVNGPropertyList node;
  node.insert("librevenge:path-action", "C");
  node.insert("svg:x",m_scale*x);
  node.insert("svg:y",m_scale*y);
  node.insert("svg:x1",m_scale*x1);
  node.insert("svg:y1",m_scale*y1);
  node.insert("svg:x2",m_scale*x2);
  node.insert("svg:y2",m_scale*y2);
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

void libvisio::VSDContentCollector::collectRelQuadBezTo(unsigned /* id */, unsigned level, double x, double y, double x1, double y1)
{
  _handleLevelChange(level);
  x *= m_xform.width;
  y *= m_xform.height;
  x1 *= m_xform.width;
  y1 *= m_xform.height;
  transformPoint(x1, y1);
  m_originalX = x;
  m_originalY = y;
  transformPoint(x, y);
  m_x = x;
  m_y = y;
  librevenge::RVNGPropertyList node;
  node.insert("librevenge:path-action", "Q");
  node.insert("svg:x",m_scale*x);
  node.insert("svg:y",m_scale*y);
  node.insert("svg:x1",m_scale*x1);
  node.insert("svg:y1",m_scale*y1);
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
                                                         const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                                                         const boost::optional<double> &fillBGTransparency,
                                                         const boost::optional<unsigned char> &shadowPattern, const boost::optional<Colour> &shfgc)
{
  collectFillAndShadow(level, colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc, m_shadowOffsetX, m_shadowOffsetY);
}

void libvisio::VSDContentCollector::collectThemeReference(unsigned level, const boost::optional<long> &lineColour, const boost::optional<long> &fillColour,
                                                          const boost::optional<long> &shadowColour, const boost::optional<long> &fontColour)
{
  _handleLevelChange(level);
  m_themeReference.override(VSDOptionalThemeReference(lineColour, fillColour, shadowColour, fontColour));
}

void libvisio::VSDContentCollector::collectForeignData(unsigned level, const librevenge::RVNGBinaryData &binaryData)
{
  _handleLevelChange(level);
  _handleForeignData(binaryData);
}

void libvisio::VSDContentCollector::collectOLEList(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
  m_currentForeignData.clear();
  librevenge::RVNGBinaryData binaryData;
  _handleForeignData(binaryData);
}

void libvisio::VSDContentCollector::collectOLEData(unsigned /* id */, unsigned level, const librevenge::RVNGBinaryData &oleData)
{
  _handleLevelChange(level);
  m_currentForeignData.append(oleData);
}

void libvisio::VSDContentCollector::_handleForeignData(const librevenge::RVNGBinaryData &binaryData)
{
  if (m_foreignType == 0 || m_foreignType == 1 || m_foreignType == 4) // Image
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

      m_currentForeignData.append((unsigned char)0x00);
      m_currentForeignData.append((unsigned char)0x00);
      m_currentForeignData.append((unsigned char)0x00);
      m_currentForeignData.append((unsigned char)0x00);

      m_currentForeignData.append((unsigned char)0x36);
      m_currentForeignData.append((unsigned char)0x00);
      m_currentForeignData.append((unsigned char)0x00);
      m_currentForeignData.append((unsigned char)0x00);
    }
    m_currentForeignData.append(binaryData);

    if (m_foreignType == 1)
    {
      switch (m_foreignFormat)
      {
      case 0:
      case 255:
        m_currentForeignProps.insert("librevenge:mime-type", "image/bmp");
        break;
      case 1:
        m_currentForeignProps.insert("librevenge:mime-type", "image/jpeg");
        break;
      case 2:
        m_currentForeignProps.insert("librevenge:mime-type", "image/gif");
        break;
      case 3:
        m_currentForeignProps.insert("librevenge:mime-type", "image/tiff");
        break;
      case 4:
        m_currentForeignProps.insert("librevenge:mime-type", "image/png");
        break;
      }
    }
    else if (m_foreignType == 0 || m_foreignType == 4)
    {
      const unsigned char *tmpBinData = m_currentForeignData.getDataBuffer();
      // Check for EMF signature
      if (m_currentForeignData.size() > 0x2B && tmpBinData[0x28] == 0x20 && tmpBinData[0x29] == 0x45 && tmpBinData[0x2A] == 0x4D && tmpBinData[0x2B] == 0x46)
        m_currentForeignProps.insert("librevenge:mime-type", "image/emf");
      else
        m_currentForeignProps.insert("librevenge:mime-type", "image/wmf");
    }
  }
  else if (m_foreignType == 2)
  {
    m_currentForeignProps.insert("librevenge:mime-type", "object/ole");
    m_currentForeignData.append(binaryData);
  }

#if DUMP_BITMAP
  librevenge::RVNGString filename;
  if (m_foreignType == 1)
  {
    switch (m_foreignFormat)
    {
    case 0:
    case 255:
      filename.sprintf("binarydump%08u.bmp", bitmapId++);
      break;
    case 1:
      filename.sprintf("binarydump%08u.jpeg", bitmapId++);
      break;
    case 2:
      filename.sprintf("binarydump%08u.gif", bitmapId++);
      break;
    case 3:
      filename.sprintf("binarydump%08u.tiff", bitmapId++);
      break;
    case 4:
      filename.sprintf("binarydump%08u.png", bitmapId++);
      break;
    default:
      filename.sprintf("binarydump%08u.bin", bitmapId++);
      break;
    }
  }
  else if (m_foreignType == 0 || m_foreignType == 4)
  {
    const unsigned char *tmpBinData = m_currentForeignData.getDataBuffer();
    // Check for EMF signature
    if (m_currentForeignData.size() > 0x2B && tmpBinData[0x28] == 0x20 && tmpBinData[0x29] == 0x45 && tmpBinData[0x2A] == 0x4D && tmpBinData[0x2B] == 0x46)
      filename.sprintf("binarydump%08u.emf", bitmapId++);
    else
      filename.sprintf("binarydump%08u.wmf", bitmapId++);
  }
  else if (m_foreignType == 2)
    filename.sprintf("binarydump%08u.ole", bitmapId++);
  else
    filename.sprintf("binarydump%08u.bin", bitmapId++);

  FILE *f = fopen(filename.cstr(), "wb");
  if (f)
  {
    const unsigned char *tmpBuffer = m_currentForeignData.getDataBuffer();
    for (unsigned long k = 0; k < m_currentForeignData.size(); k++)
      fprintf(f, "%c",tmpBuffer[k]);
    fclose(f);
  }
#endif
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
  librevenge::RVNGPropertyList end;
  end.insert("svg:x", m_scale*m_x);
  end.insert("svg:y", m_scale*m_y);
  end.insert("librevenge:path-action", "M");
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
  librevenge::RVNGPropertyList end;
  end.insert("svg:x", m_scale*m_x);
  end.insert("svg:y", m_scale*m_y);
  end.insert("librevenge:path-action", "L");
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
    librevenge::RVNGPropertyList end;
    end.insert("svg:x", m_scale*m_x);
    end.insert("svg:y", m_scale*m_y);
    end.insert("librevenge:path-action", "L");
    if (!m_noFill && !m_noShow)
      m_currentFillGeometry.push_back(end);
    if (!m_noLine && !m_noShow)
      m_currentLineGeometry.push_back(end);
  }
  else
  {
    librevenge::RVNGPropertyList arc;
    double chord = sqrt(pow((y2 - m_y),2) + pow((x2 - m_x),2));
    double radius = (4 * bow * bow + chord * chord) / (8 * fabs(bow));
    int largeArc = fabs(bow) > radius ? 1 : 0;
    bool sweep = (bow < 0);
    transformFlips(sweep, sweep);

    m_x = x2;
    m_y = y2;
    arc.insert("svg:rx", m_scale*radius);
    arc.insert("svg:ry", m_scale*radius);
    arc.insert("librevenge:rotate", angle*180/M_PI, librevenge::RVNG_GENERIC);
    arc.insert("librevenge:large-arc", largeArc);
    arc.insert("librevenge:sweep", sweep);
    arc.insert("svg:x", m_scale*m_x);
    arc.insert("svg:y", m_scale*m_y);
    arc.insert("librevenge:path-action", "A");
    if (!m_noFill && !m_noShow)
      m_currentFillGeometry.push_back(arc);
    if (!m_noLine && !m_noShow)
      m_currentLineGeometry.push_back(arc);
  }
}


void libvisio::VSDContentCollector::_outputCubicBezierSegment(const std::vector<std::pair<double, double> > &points)
{
  if (points.size() < 4)
    return;
  librevenge::RVNGPropertyList node;
  node.insert("librevenge:path-action", "C");
  double x = points[1].first;
  double y = points[1].second;
  transformPoint(x, y);
  node.insert("svg:x1", m_scale*x);
  node.insert("svg:y1", m_scale*y);
  x = points[2].first;
  y = points[2].second;
  transformPoint(x, y);
  node.insert("svg:x2", m_scale*x);
  node.insert("svg:y2", m_scale*y);
  x = points[3].first;
  y = points[3].second;
  transformPoint(x, y);
  node.insert("svg:x", m_scale*x);
  node.insert("svg:y", m_scale*y);

  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(node);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(node);
}

void libvisio::VSDContentCollector::_outputQuadraticBezierSegment(const std::vector<std::pair<double, double> > &points)
{
  if (points.size() < 3)
    return;
  librevenge::RVNGPropertyList node;
  node.insert("librevenge:path-action", "Q");
  double x = points[1].first;
  double y = points[1].second;
  transformPoint(x, y);
  node.insert("svg:x1", m_scale*x);
  node.insert("svg:y1", m_scale*y);
  x = points[2].first;
  y = points[2].second;
  transformPoint(x, y);
  node.insert("svg:x", m_scale*x);
  node.insert("svg:y", m_scale*y);

  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(node);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(node);
}

void libvisio::VSDContentCollector::_outputLinearBezierSegment(const std::vector<std::pair<double, double> > &points)
{
  if (points.size() < 2)
    return;
  librevenge::RVNGPropertyList node;
  node.insert("librevenge:path-action", "L");
  double x = points[1].first;
  double y = points[1].second;
  transformPoint(x, y);
  node.insert("svg:x", m_scale*x);
  node.insert("svg:y", m_scale*y);

  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(node);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(node);
}

void libvisio::VSDContentCollector::_generateBezierSegmentsFromNURBS(unsigned degree,
                                                                     const std::vector<std::pair<double, double> > &controlPoints, const std::vector<double> &knotVector)
{
  if (controlPoints.empty() || knotVector.empty() || !degree)
    return;

  /* Decomposition of a uniform spline of a given degree into Bezier segments
   * adapted from the algorithm DecomposeCurve (Les Piegl, Wayne Tiller:
   * The NURBS Book, 2nd Edition, 1997
   */

  unsigned a = degree;
  unsigned b = degree + 1;
  std::vector< std::pair<double, double> > points(degree + 1), nextPoints(degree + 1);
  unsigned i = 0;
  for (; i <= degree; i++)
    points[i] = controlPoints[i];
  while (b < knotVector.size() - 1)
  {
    i = b;
    while (b < knotVector.size() - 1 && knotVector[b+1] == knotVector[b])
      b++;
    unsigned mult = b - i + 1;
    if (mult < degree)
    {
      double numer = (double)(knotVector[b] - knotVector[a]);
      unsigned j = degree;
      std::vector<double> alphas(degree - 1, 0.0);
      for (; j >mult; j--)
        alphas[j-mult-1] = numer/double(knotVector[a+j]-knotVector[a]);
      unsigned r = degree - mult;
      for (j=1; j<=r; j++)
      {
        unsigned save = r - j;
        unsigned s = mult+j;
        for (unsigned k = degree; k>=s; k--)
        {
          double alpha = alphas[k-s];
          points[k].first = alpha*points[k].first + (1.0-alpha)*points[k-1].first;
          points[k].second = alpha*points[k].second + (1.0-alpha)*points[k-1].second;
        }
        if (b < knotVector.size() - 1)
        {
          nextPoints[save].first = points[degree].first;
          nextPoints[save].second = points[degree].second;
        }
      }
    }
    // Pass the segment to the path

    switch (degree)
    {
    case 1:
      _outputLinearBezierSegment(points);
      break;
    case 2:
      _outputQuadraticBezierSegment(points);
      break;
    case 3:
      _outputCubicBezierSegment(points);
      break;
    default:
      break;
    }

    std::swap(points, nextPoints);

    if (b < knotVector.size() - 1)
    {
      for (i=degree-mult; i <= degree; i++)
      {
        points[i].first = controlPoints[b-degree+i].first;
        points[i].second = controlPoints[b-degree+i].second;
      }
      a = b;
      b++;
    }
  }
}

double libvisio::VSDContentCollector::_NURBSBasis(unsigned knot, unsigned degree, double point, const std::vector<double> &knotVector)
{
  double basis = 0;
  if (knotVector.empty())
    return basis;
  if (degree == 0)
  {
    if (knotVector[knot] <= point && point < knotVector[knot+1])
      return 1;
    else
      return 0;
  }
  if (knotVector.size() > knot+degree && fabs(knotVector[knot+degree]-knotVector[knot]) > LIBVISIO_EPSILON)
    basis = (point-knotVector[knot])/(knotVector[knot+degree]-knotVector[knot]) * _NURBSBasis(knot, degree-1, point, knotVector);

  if (knotVector.size() > knot+degree+1 && fabs(knotVector[knot+degree+1] - knotVector[knot+1]) > LIBVISIO_EPSILON)
    basis += (knotVector[knot+degree+1]-point)/(knotVector[knot+degree+1]-knotVector[knot+1]) * _NURBSBasis(knot+1, degree-1, point, knotVector);

  return basis;
}

#define VSD_NUM_POLYLINES_PER_KNOT 100

void libvisio::VSDContentCollector::_generatePolylineFromNURBS(unsigned degree, const std::vector<std::pair<double, double> > &controlPoints,
                                                               const std::vector<double> &knotVector, const std::vector<double> &weights)
{
  if (m_noShow)
    return;

  librevenge::RVNGPropertyList node;

  for (unsigned i = 0; i < VSD_NUM_POLYLINES_PER_KNOT * knotVector.size(); i++)
  {
    node.clear();
    node.insert("librevenge:path-action", "L");
    double x = 0;
    double y = 0;
    double denominator = LIBVISIO_EPSILON;

    for (unsigned p = 0; p < controlPoints.size() && p < weights.size(); p++)
    {
      double basis = _NURBSBasis(p, degree, (double)i / (VSD_NUM_POLYLINES_PER_KNOT * knotVector.size()), knotVector);
      x += basis * controlPoints[p].first * weights[p];
      y += basis * controlPoints[p].second * weights[p];
      denominator += weights[p] * basis;
    }
    x /= denominator;
    y /= denominator;
    transformPoint(x, y);
    node.insert("svg:x", m_scale*x);
    node.insert("svg:y", m_scale*y);

    if (!m_noFill && !m_noShow)
      m_currentFillGeometry.push_back(node);
    if (!m_noLine && !m_noShow)
      m_currentLineGeometry.push_back(node);
  }
}

bool libvisio::VSDContentCollector::_isUniform(const std::vector<double> &weights) const
{
  if (weights.empty())
    return true;
  double previousValue = weights[0];
  for (std::vector<double>::size_type i = 0; i < weights.size(); ++i)
  {
    if (fabs(weights[i] - previousValue) < LIBVISIO_EPSILON)
      previousValue = weights[i];
    else
      return false;
  }
  return true;
}

void libvisio::VSDContentCollector::collectNURBSTo(unsigned /* id */, unsigned level, double x2, double y2,
                                                   unsigned char xType, unsigned char yType, unsigned degree, const std::vector<std::pair<double, double> > &ctrlPnts,
                                                   const std::vector<double> &kntVec, const std::vector<double> &weights)
{
  _handleLevelChange(level);

  if (kntVec.empty() || ctrlPnts.empty() || weights.empty())
    // Here, maybe we should just draw line to (x2,y2)
    return;

  std::vector<std::pair<double, double> > controlPoints(ctrlPnts);

  // Convert control points to static co-ordinates
  for (std::vector<std::pair<double, double> >::iterator iter = controlPoints.begin(); iter != controlPoints.end(); ++iter)
  {
    if (xType == 0) // Percentage
      iter->first *= m_xform.width;
    if (yType == 0) // Percentage
      iter->second *= m_xform.height;
  }

  controlPoints.push_back(std::pair<double,double>(x2, y2));
  controlPoints.insert(controlPoints.begin(), std::pair<double, double>(m_originalX, m_originalY));

  std::vector<double> knotVector(kntVec);

  // Fill in end knots
  while (knotVector.size() < (controlPoints.size() + degree + 1))
  {
    double tmpBack = knotVector.back();
    knotVector.push_back(tmpBack);
  }

  // Let knotVector run from 0 to 1
  double firstKnot = knotVector[0];
  double lastKnot = knotVector.back()-knotVector[0];
  for (std::vector<double>::iterator knot = knotVector.begin(); knot != knotVector.end(); ++knot)
  {
    *knot -= firstKnot;
    *knot /= lastKnot;
  }

  if (degree <= 3 && _isUniform(weights))
    _generateBezierSegmentsFromNURBS(degree, controlPoints, knotVector);
  else
    _generatePolylineFromNURBS(degree, controlPoints, knotVector, weights);

  m_originalX = x2;
  m_originalY = y2;
  m_x = x2;
  m_y = y2;
  transformPoint(m_x, m_y);
#if 1
  librevenge::RVNGPropertyList node;
  node.insert("librevenge:path-action", "L");
  node.insert("svg:x", m_scale*m_x);
  node.insert("svg:y", m_scale*m_y);
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(node);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(node);
#endif
}

void libvisio::VSDContentCollector::collectNURBSTo(unsigned id, unsigned level, double x2, double y2, double knot, double knotPrev, double weight, double weightPrev, const NURBSData &data)
{
  NURBSData newData(data);
  newData.knots.push_back(knot);
  newData.knots.push_back(newData.lastKnot);
  newData.knots.insert(newData.knots.begin(), knotPrev);
  newData.weights.push_back(weight);
  newData.weights.insert(newData.weights.begin(), weightPrev);
  collectNURBSTo(id, level, x2, y2, newData.xType, newData.yType, newData.degree, newData.points, newData.knots, newData.weights);
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
    collectNURBSTo(id, level, x2, y2, knot, knotPrev, weight, weightPrev, iter->second);
  else
    _handleLevelChange(level);
}

void libvisio::VSDContentCollector::collectPolylineTo(unsigned /* id */ , unsigned level, double x, double y, unsigned char xType, unsigned char yType, const std::vector<std::pair<double, double> > &points)
{
  _handleLevelChange(level);

  librevenge::RVNGPropertyList polyline;
  std::vector<std::pair<double, double> > tmpPoints(points);
  for (unsigned i = 0; i< points.size(); i++)
  {
    polyline.clear();
    if (xType == 0)
      tmpPoints[i].first *= m_xform.width;
    if (yType == 0)
      tmpPoints[i].second *= m_xform.height;

    transformPoint(tmpPoints[i].first, tmpPoints[i].second);
    polyline.insert("librevenge:path-action", "L");
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
  polyline.insert("librevenge:path-action", "L");
  polyline.insert("svg:x", m_scale*m_x);
  polyline.insert("svg:y", m_scale*m_y);
  if (!m_noFill && !m_noShow)
    m_currentFillGeometry.push_back(polyline);
  if (!m_noLine && !m_noShow)
    m_currentLineGeometry.push_back(polyline);
}

void libvisio::VSDContentCollector::collectPolylineTo(unsigned id, unsigned level, double x, double y, const PolylineData &data)
{
  collectPolylineTo(id, level, x, y, data.xType, data.yType, data.points);
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
    collectPolylineTo(id, level, x, y, iter->second);
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
    bool shapeFound = false;
    if (m_groupMemberships != m_groupMembershipsSequence.end())
    {
      std::map<unsigned, unsigned>::iterator iter = m_groupMemberships->find(shapeId);
      if (iter != m_groupMemberships->end() && shapeId != iter->second)
      {
        shapeId = iter->second;
        shapeFound = true;
      }
    }
    if (!shapeFound)
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
    bool shapeFound = false;
    if (m_groupMemberships != m_groupMembershipsSequence.end())
    {
      std::map<unsigned, unsigned>::iterator iter = m_groupMemberships->find(shapeId);
      if (iter != m_groupMemberships->end() && shapeId != iter->second)
      {
        shapeId = iter->second;
        shapeFound = true;
      }
    }
    if (!shapeFound)
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

void libvisio::VSDContentCollector::collectPage(unsigned /* id */, unsigned level, unsigned backgroundPageID, bool isBackgroundPage, const VSDName &pageName)
{
  _handleLevelChange(level);
  m_currentPage.m_backgroundPageID = backgroundPageID;
  m_currentPage.m_pageName.clear();
  if (!pageName.empty())
    _convertDataToString(m_currentPage.m_pageName, pageName.m_data, pageName.m_format);
  m_isBackgroundPage = isBackgroundPage;
}

void libvisio::VSDContentCollector::collectShape(unsigned id, unsigned level, unsigned /*parent*/, unsigned masterPage, unsigned masterShape, unsigned lineStyleId, unsigned fillStyleId, unsigned textStyleId)
{
  _handleLevelChange(level);
  m_currentShapeLevel = level;

  m_foreignType = (unsigned)-1; // Tracks current foreign data type
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

  m_misc = VSDMisc();

  // Save line colour and pattern, fill type and pattern
  m_textStream.clear();
  m_charFormats.clear();
  m_paraFormats.clear();

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
  m_textBlockStyle = VSDTextBlockStyle();
  m_defaultCharStyle = VSDCharStyle();
  m_defaultParaStyle = VSDParaStyle();
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
      librevenge::RVNGString nameString;
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
        m_fields.push_back(librevenge::RVNGString());
    }

    if (m_stencilShape->m_lineStyleId != MINUS_ONE)
      m_lineStyle.override(m_styles.getOptionalLineStyle(m_stencilShape->m_lineStyleId));

    m_lineStyle.override(m_stencilShape->m_lineStyle);

    if (m_stencilShape->m_fillStyleId != MINUS_ONE)
      m_fillStyle.override(m_styles.getOptionalFillStyle(m_stencilShape->m_fillStyleId));

    m_fillStyle.override(m_stencilShape->m_fillStyle);

    if (m_stencilShape->m_textStyleId != MINUS_ONE)
    {
      m_defaultCharStyle.override(m_styles.getOptionalCharStyle(m_stencilShape->m_textStyleId));
      m_defaultParaStyle.override(m_styles.getOptionalParaStyle(m_stencilShape->m_textStyleId));
      m_textBlockStyle.override(m_styles.getOptionalTextBlockStyle(m_stencilShape->m_textStyleId));
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
    m_defaultCharStyle.override(m_styles.getOptionalCharStyle(textStyleId));
    m_defaultParaStyle.override(m_styles.getOptionalParaStyle(textStyleId));
    m_textBlockStyle.override(m_styles.getOptionalTextBlockStyle(textStyleId));
  }

  m_currentGeometryCount = 0;
  m_fieldIndex = 0;
}

void libvisio::VSDContentCollector::collectUnhandledChunk(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
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
  std::vector<double> weights(m_splineControlPoints.size()+2);
  for (unsigned i=0; i < m_splineControlPoints.size()+2; i++)
    weights[i] = 1.0;
  collectNURBSTo(0, m_splineLevel, m_splineX, m_splineY, 1, 1, m_splineDegree, m_splineControlPoints, m_splineKnotVector, weights);
  m_splineKnotVector.clear();
  m_splineControlPoints.clear();
}


void libvisio::VSDContentCollector::collectText(unsigned level, const librevenge::RVNGBinaryData &textStream, TextFormat format)
{
  _handleLevelChange(level);

  m_textStream = textStream;
  m_textFormat = format;
}

void libvisio::VSDContentCollector::collectParaIX(unsigned /* id */ , unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                                                  const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine, const boost::optional<double> &spBefore,
                                                  const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align, const boost::optional<unsigned> &flags)
{
  _handleLevelChange(level);
  VSDParaStyle format(m_defaultParaStyle);
  format.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align, flags));
  format.charCount = charCount;
  m_paraFormats.push_back(format);
}

void libvisio::VSDContentCollector::collectDefaultParaStyle(unsigned charCount, const boost::optional<double> &indFirst,
                                                            const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine, const boost::optional<double> &spBefore,
                                                            const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align, const boost::optional<unsigned> &flags)
{
  m_defaultParaStyle.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align, flags));
}

void libvisio::VSDContentCollector::collectCharIX(unsigned /* id */ , unsigned level, unsigned charCount,
                                                  const boost::optional<VSDName> &font, const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                                                  const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline, const boost::optional<bool> &strikeout,
                                                  const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps,
                                                  const boost::optional<bool> &superscript, const boost::optional<bool> &subscript)
{
  _handleLevelChange(level);
  VSDCharStyle format(m_defaultCharStyle);
  format.override(VSDOptionalCharStyle(charCount, font, fontColour, fontSize, bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                       allcaps, initcaps, smallcaps, superscript, subscript));
  format.charCount = charCount;
  m_charFormats.push_back(format);
}

void libvisio::VSDContentCollector::collectDefaultCharStyle(unsigned charCount,
                                                            const boost::optional<VSDName> &font, const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                                                            const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline, const boost::optional<bool> &strikeout,
                                                            const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps,
                                                            const boost::optional<bool> &superscript, const boost::optional<bool> &subscript)
{
  m_defaultCharStyle.override(VSDOptionalCharStyle(charCount, font, fontColour, fontSize, bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                                   allcaps, initcaps, smallcaps, superscript, subscript));
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

void libvisio::VSDContentCollector::_convertDataToString(librevenge::RVNGString &result, const librevenge::RVNGBinaryData &data, TextFormat format)
{
  if (!data.size())
    return;
  std::vector<unsigned char> tmpData(data.size());
  memcpy(&tmpData[0], data.getDataBuffer(), data.size());
  appendCharacters(result, tmpData, format);
}

void libvisio::VSDContentCollector::collectName(unsigned id, unsigned level, const librevenge::RVNGBinaryData &name, TextFormat format)
{
  _handleLevelChange(level);

  librevenge::RVNGString nameString;
  _convertDataToString(nameString, name, format);
  m_names[id] = nameString;
}

void libvisio::VSDContentCollector::collectPageSheet(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
  m_currentShapeLevel = level;
}

void libvisio::VSDContentCollector::collectStyleSheet(unsigned id, unsigned level, unsigned lineStyleParent, unsigned fillStyleParent, unsigned textStyleParent)
{
  _handleLevelChange(level);
  // reusing the shape level for style sheet to avoid another variable
  m_currentShapeLevel = level;
  m_currentStyleSheet = id;
  m_styles.addLineStyleMaster(m_currentStyleSheet, lineStyleParent);
  m_styles.addFillStyleMaster(m_currentStyleSheet, fillStyleParent);
  m_styles.addTextStyleMaster(m_currentStyleSheet, textStyleParent);
}

void libvisio::VSDContentCollector::collectLineStyle(unsigned /* level */, const boost::optional<double> &strokeWidth, const boost::optional<Colour> &c,
                                                     const boost::optional<unsigned char> &linePattern, const boost::optional<unsigned char> &startMarker, const boost::optional<unsigned char> &endMarker,
                                                     const boost::optional<unsigned char> &lineCap)
{
  VSDOptionalLineStyle lineStyle(strokeWidth, c, linePattern, startMarker, endMarker, lineCap);
  m_styles.addLineStyle(m_currentStyleSheet, lineStyle);
}

void libvisio::VSDContentCollector::collectFillStyle(unsigned /* level */, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                                     const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency, const boost::optional<double> &fillBGTransparency,
                                                     const boost::optional<unsigned char> &shadowPattern, const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX,
                                                     const boost::optional<double> &shadowOffsetY)
{
  VSDOptionalFillStyle fillStyle(colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shfgc, shadowPattern, shadowOffsetX, shadowOffsetY);
  m_styles.addFillStyle(m_currentStyleSheet, fillStyle);

}

void libvisio::VSDContentCollector::collectFillStyle(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                                     const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency, const boost::optional<double> &fillBGTransparency,
                                                     const boost::optional<unsigned char> &shadowPattern, const boost::optional<Colour> &shfgc)
{
  collectFillStyle(level, colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc, m_shadowOffsetX, m_shadowOffsetY);
}

void libvisio::VSDContentCollector::collectParaIXStyle(unsigned /* id */, unsigned /* level */, unsigned charCount,
                                                       const boost::optional<double> &indFirst, const boost::optional<double> &indLeft, const boost::optional<double> &indRight,
                                                       const boost::optional<double> &spLine, const boost::optional<double> &spBefore, const boost::optional<double> &spAfter,
                                                       const boost::optional<unsigned char> &align, const boost::optional<unsigned> &flags)
{
  VSDOptionalParaStyle paraStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align, flags);
  m_styles.addParaStyle(m_currentStyleSheet, paraStyle);
}


void libvisio::VSDContentCollector::collectCharIXStyle(unsigned /* id */, unsigned /* level */, unsigned charCount,
                                                       const boost::optional<VSDName> &font, const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize,
                                                       const boost::optional<bool> &bold, const boost::optional<bool> &italic, const boost::optional<bool> &underline,
                                                       const boost::optional<bool> &doubleunderline, const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout,
                                                       const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps,
                                                       const boost::optional<bool> &superscript, const boost::optional<bool> &subscript)
{
  VSDOptionalCharStyle charStyle(charCount, font, fontColour, fontSize, bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                 allcaps, initcaps, smallcaps, superscript, subscript);
  m_styles.addCharStyle(m_currentStyleSheet, charStyle);
}

void libvisio::VSDContentCollector::collectTextBlockStyle(unsigned /* level */, const boost::optional<double> &leftMargin, const boost::optional<double> &rightMargin,
                                                          const boost::optional<double> &topMargin, const boost::optional<double> &bottomMargin, const boost::optional<unsigned char> &verticalAlign,
                                                          const boost::optional<bool> &isBgFilled, const boost::optional<Colour> &bgColour, const boost::optional<double> &defaultTabStop,
                                                          const boost::optional<unsigned char> &textDirection)
{
  VSDOptionalTextBlockStyle textBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin, verticalAlign, isBgFilled, bgColour, defaultTabStop, textDirection);
  m_styles.addTextBlockStyle(m_currentStyleSheet, textBlockStyle);
}

void libvisio::VSDContentCollector::_lineProperties(const VSDLineStyle &style, librevenge::RVNGPropertyList &styleProps)
{
  if (!style.pattern)
  {
    styleProps.insert("draw:stroke", "none");
    return;
  }

  styleProps.insert("svg:stroke-width", m_scale*style.width);
  styleProps.insert("svg:stroke-color", getColourString(style.colour));
  if (style.colour.a)
    styleProps.insert("svg:stroke-opacity", (1 - style.colour.a/255.0), librevenge::RVNG_PERCENT);
  else
    styleProps.insert("svg:stroke-opacity", 1.0, librevenge::RVNG_PERCENT);
  switch (style.cap)
  {
  case 0:
    styleProps.insert("svg:stroke-linecap", "round");
    styleProps.insert("svg:stroke-linejoin", "round");
    break;
  case 2:
    styleProps.insert("svg:stroke-linecap", "square");
    styleProps.insert("svg:stroke-linejoin", "miter");
    break;
  default:
    styleProps.insert("svg:stroke-linecap", "butt");
    styleProps.insert("svg:stroke-linejoin", "miter");
    break;
  }

  // Deal with line markers (arrows, etc.)
  if (style.startMarker > 0)
  {
    styleProps.insert("draw:marker-start-viewbox", _linePropertiesMarkerViewbox(style.startMarker));
    styleProps.insert("draw:marker-start-path", _linePropertiesMarkerPath(style.startMarker));
    styleProps.insert("draw:marker-start-width", m_scale*_linePropertiesMarkerScale(style.startMarker)*(0.1/(style.width*style.width+1)+2.54*style.width));
  }
  if (style.endMarker > 0)
  {
    styleProps.insert("draw:marker-end-viewbox", _linePropertiesMarkerViewbox(style.endMarker));
    styleProps.insert("draw:marker-end-path", _linePropertiesMarkerPath(style.endMarker));
    styleProps.insert("draw:marker-end-width", m_scale*_linePropertiesMarkerScale(style.endMarker)*(0.1/(style.width*style.width+1)+2.54*style.width));
  }

  int dots1 = 0;
  int dots2 = 0;
  double dots1len = 0.0;
  double dots2len = 0.0;
  double gap = 0.0;

  styleProps.remove("draw:stroke");
  switch (style.pattern)
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

  if (style.pattern == 0)
    styleProps.insert("draw:stroke", "none");
  else if (style.pattern == 1)
    styleProps.insert("draw:stroke", "solid");
  else if (style.pattern > 1 && style.pattern <= 23)
  {
    styleProps.insert("draw:stroke", "dash");
    styleProps.insert("draw:dots1", dots1);
    styleProps.insert("draw:dots1-length", dots1len, librevenge::RVNG_PERCENT);
    styleProps.insert("draw:dots2", dots2);
    styleProps.insert("draw:dots2-length", dots2len, librevenge::RVNG_PERCENT);
    styleProps.insert("draw:distance", gap, librevenge::RVNG_PERCENT);
  }
  else
    // FIXME: later it will require special treatment for custom line patterns
    // patt ID is 0xfe, link to stencil name is in 'Line' blocks
    styleProps.insert("draw:stroke", "solid");
}

void libvisio::VSDContentCollector::_fillAndShadowProperties(const VSDFillStyle &style, librevenge::RVNGPropertyList &styleProps)
{
  if (style.pattern)
    styleProps.insert("svg:fill-rule", "evenodd");

  if (!style.pattern)
    styleProps.insert("draw:fill", "none");
  else if (style.pattern == 1)
  {
    styleProps.insert("draw:fill", "solid");
    styleProps.insert("draw:fill-color", getColourString(style.fgColour));
    if (style.fgTransparency > 0)
      styleProps.insert("draw:opacity", 1 - style.fgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.remove("draw:opacity");
  }
  else if (style.pattern == 26 || style.pattern == 29)
  {
    styleProps.insert("draw:fill", "gradient");
    styleProps.insert("draw:style", "axial");
    styleProps.insert("draw:start-color", getColourString(style.fgColour));
    styleProps.insert("draw:end-color", getColourString(style.bgColour));
    styleProps.remove("draw:opacity");
    if (style.bgTransparency > 0.0)
      styleProps.insert("librevenge:start-opacity", 1 - style.bgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.insert("librevenge:start-opacity", 1, librevenge::RVNG_PERCENT);
    if (style.fgTransparency > 0.0)
      styleProps.insert("librevenge:end-opacity", 1 - style.fgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.insert("librevenge:end-opacity", 1, librevenge::RVNG_PERCENT);
    styleProps.insert("draw:border", 0, librevenge::RVNG_PERCENT);

    if (style.pattern == 26)
      styleProps.insert("draw:angle", 90);
    else
      styleProps.insert("draw:angle", 0);
  }
  else if (style.pattern >= 25 && style.pattern <= 34)
  {
    styleProps.insert("draw:fill", "gradient");
    styleProps.insert("draw:style", "linear");
    styleProps.insert("draw:start-color", getColourString(style.bgColour));
    styleProps.insert("draw:end-color", getColourString(style.fgColour));
    styleProps.remove("draw:opacity");
    if (style.bgTransparency > 0)
      styleProps.insert("librevenge:start-opacity", 1 - style.bgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.insert("librevenge:start-opacity", 1, librevenge::RVNG_PERCENT);
    if (style.fgTransparency > 0)
      styleProps.insert("librevenge:end-opacity", 1 - style.fgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.insert("librevenge:end-opacity", 1, librevenge::RVNG_PERCENT);
    styleProps.insert("draw:border", 0, librevenge::RVNG_PERCENT);

    switch (style.pattern)
    {
    case 25:
      styleProps.insert("draw:angle", 270);
      break;
    case 27:
      styleProps.insert("draw:angle", 90);
      break;
    case 28:
      styleProps.insert("draw:angle", 180);
      break;
    case 30:
      styleProps.insert("draw:angle", 0);
      break;
    case 31:
      styleProps.insert("draw:angle", 225);
      break;
    case 32:
      styleProps.insert("draw:angle", 135);
      break;
    case 33:
      styleProps.insert("draw:angle", 315);
      break;
    case 34:
      styleProps.insert("draw:angle", 45);
      break;
    }
  }
  else if (style.pattern == 35)
  {
    styleProps.insert("draw:fill", "gradient");
    styleProps.insert("draw:style", "rectangular");
    styleProps.insert("svg:cx", 0.5, librevenge::RVNG_PERCENT);
    styleProps.insert("svg:cy", 0.5, librevenge::RVNG_PERCENT);
    styleProps.insert("draw:start-color", getColourString(style.bgColour));
    styleProps.insert("draw:end-color", getColourString(style.fgColour));
    styleProps.remove("draw:opacity");
    if (style.bgTransparency > 0)
      styleProps.insert("librevenge:start-opacity", 1 - style.bgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.insert("librevenge:start-opacity", 1, librevenge::RVNG_PERCENT);
    if (style.fgTransparency > 0)
      styleProps.insert("librevenge:end-opacity", 1 - style.fgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.insert("librevenge:end-opacity", 1, librevenge::RVNG_PERCENT);
    styleProps.insert("draw:angle", 0);
    styleProps.insert("draw:border", 0, librevenge::RVNG_PERCENT);
  }
  else if (style.pattern >= 36 && style.pattern <= 40)
  {
    styleProps.insert("draw:fill", "gradient");
    styleProps.insert("draw:style", "radial");
    styleProps.insert("draw:start-color", getColourString(style.bgColour));
    styleProps.insert("draw:end-color", getColourString(style.fgColour));
    styleProps.remove("draw:opacity");
    if (style.bgTransparency > 0)
      styleProps.insert("librevenge:start-opacity", 1 - style.bgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.insert("librevenge:start-opacity", 1, librevenge::RVNG_PERCENT);
    if (style.fgTransparency > 0)
      styleProps.insert("librevenge:end-opacity", 1 - style.fgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.insert("librevenge:end-opacity", 1, librevenge::RVNG_PERCENT);
    styleProps.insert("draw:border", 0, librevenge::RVNG_PERCENT);

    switch (style.pattern)
    {
    case 36:
      styleProps.insert("svg:cx", 0, librevenge::RVNG_PERCENT);
      styleProps.insert("svg:cy", 0, librevenge::RVNG_PERCENT);
      break;
    case 37:
      styleProps.insert("svg:cx", 1, librevenge::RVNG_PERCENT);
      styleProps.insert("svg:cy", 0, librevenge::RVNG_PERCENT);
      break;
    case 38:
      styleProps.insert("svg:cx", 0, librevenge::RVNG_PERCENT);
      styleProps.insert("svg:cy", 1, librevenge::RVNG_PERCENT);
      break;
    case 39:
      styleProps.insert("svg:cx", 1, librevenge::RVNG_PERCENT);
      styleProps.insert("svg:cy", 1, librevenge::RVNG_PERCENT);
      break;
    case 40:
      styleProps.insert("svg:cx", 0.5, librevenge::RVNG_PERCENT);
      styleProps.insert("svg:cy", 0.5, librevenge::RVNG_PERCENT);
      break;
    }
  }
  else
    // fill types we don't handle right, but let us approximate with solid fill
  {
    styleProps.insert("draw:fill", "solid");
    styleProps.insert("draw:fill-color", getColourString(style.bgColour));
  }

  if (style.shadowPattern)
  {
    styleProps.insert("draw:shadow","visible"); // for ODG
    styleProps.insert("draw:shadow-offset-x",style.shadowOffsetX != 0.0 ? style.shadowOffsetX : m_shadowOffsetX);
    styleProps.insert("draw:shadow-offset-y",style.shadowOffsetY != 0.0 ? -style.shadowOffsetY : -m_shadowOffsetY);
    styleProps.insert("draw:shadow-color",getColourString(style.shadowFgColour));
    styleProps.insert("draw:shadow-opacity",(double)(1 - style.shadowFgColour.a/255.), librevenge::RVNG_PERCENT);
  }
}

void libvisio::VSDContentCollector::collectStyleThemeReference(unsigned /* level */, const boost::optional<long> &lineColour,
                                                               const boost::optional<long> &fillColour, const boost::optional<long> &shadowColour,
                                                               const boost::optional<long> &fontColour)
{
  VSDOptionalThemeReference themeReference(lineColour, fillColour, shadowColour, fontColour);
  m_styles.addStyleThemeReference(m_currentStyleSheet, themeReference);
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
        m_fields.push_back(librevenge::RVNGString());
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
        std::map<unsigned, librevenge::RVNGString>::const_iterator iter = m_names.find(formatStringId);
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
      if (m_stencilShape && !m_isStencilStarted)
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
      _flushShape();
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
    _flushShape();
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
    m_groupMemberships = m_groupMembershipsSequence.begin() + (m_currentPageNumber-1);
  if (m_documentPageShapeOrders.size() >= m_currentPageNumber)
    m_pageShapeOrder = m_documentPageShapeOrders.begin() + (m_currentPageNumber-1);
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

bool libvisio::VSDContentCollector::parseFormatId(const char *formatString, unsigned short &result)
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
            space_p).full)
  {
    return true;
  }
  return false;
}

void libvisio::VSDContentCollector::appendCharacters(librevenge::RVNGString &text, const std::vector<unsigned char> &characters, TextFormat format)
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

  static const UChar32 symbolmap [] =
  {
    0x0020, 0x0021, 0x2200, 0x0023, 0x2203, 0x0025, 0x0026, 0x220D, // 0x20 ..
    0x0028, 0x0029, 0x2217, 0x002B, 0x002C, 0x2212, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x2245, 0x0391, 0x0392, 0x03A7, 0x0394, 0x0395, 0x03A6, 0x0393,
    0x0397, 0x0399, 0x03D1, 0x039A, 0x039B, 0x039C, 0x039D, 0x039F,
    0x03A0, 0x0398, 0x03A1, 0x03A3, 0x03A4, 0x03A5, 0x03C2, 0x03A9,
    0x039E, 0x03A8, 0x0396, 0x005B, 0x2234, 0x005D, 0x22A5, 0x005F,
    0xF8E5, 0x03B1, 0x03B2, 0x03C7, 0x03B4, 0x03B5, 0x03C6, 0x03B3,
    0x03B7, 0x03B9, 0x03D5, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BF,
    0x03C0, 0x03B8, 0x03C1, 0x03C3, 0x03C4, 0x03C5, 0x03D6, 0x03C9,
    0x03BE, 0x03C8, 0x03B6, 0x007B, 0x007C, 0x007D, 0x223C, 0x0020, // .. 0x7F
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009E, 0x009f,
    0x20AC, 0x03D2, 0x2032, 0x2264, 0x2044, 0x221E, 0x0192, 0x2663, // 0xA0 ..
    0x2666, 0x2665, 0x2660, 0x2194, 0x2190, 0x2191, 0x2192, 0x2193,
    0x00B0, 0x00B1, 0x2033, 0x2265, 0x00D7, 0x221D, 0x2202, 0x2022,
    0x00F7, 0x2260, 0x2261, 0x2248, 0x2026, 0x23D0, 0x23AF, 0x21B5,
    0x2135, 0x2111, 0x211C, 0x2118, 0x2297, 0x2295, 0x2205, 0x2229,
    0x222A, 0x2283, 0x2287, 0x2284, 0x2282, 0x2286, 0x2208, 0x2209,
    0x2220, 0x2207, 0x00AE, 0x00A9, 0x2122, 0x220F, 0x221A, 0x22C5,
    0x00AC, 0x2227, 0x2228, 0x21D4, 0x21D0, 0x21D1, 0x21D2, 0x21D3,
    0x25CA, 0x3008, 0x00AE, 0x00A9, 0x2122, 0x2211, 0x239B, 0x239C,
    0x239D, 0x23A1, 0x23A2, 0x23A3, 0x23A7, 0x23A8, 0x23A9, 0x23AA,
    0xF8FF, 0x3009, 0x222B, 0x2320, 0x23AE, 0x2321, 0x239E, 0x239F,
    0x23A0, 0x23A4, 0x23A5, 0x23A6, 0x23AB, 0x23AC, 0x23AD, 0x0020  // .. 0xFE
  };

  UChar32  ucs4Character = 0;
  if (format == VSD_TEXT_SYMBOL) // SYMBOL
  {
    for (std::vector<unsigned char>::const_iterator iter = characters.begin();
         iter != characters.end(); ++iter)
    {
      if (0x1e == ucs4Character)
      {
        _appendField(text);
        continue;
      }
      else if (*iter < 0x20)
        ucs4Character = 0x20;
      else
        ucs4Character = symbolmap[*iter - 0x20];
      _appendUCS4(text, ucs4Character);
    }
  }
  else
  {
    UErrorCode status = U_ZERO_ERROR;
    UConverter *conv = NULL;
    switch (format)
    {
    case VSD_TEXT_JAPANESE:
      conv = ucnv_open("windows-932", &status);
      break;
    case VSD_TEXT_KOREAN:
      conv = ucnv_open("windows-949", &status);
      break;
    case VSD_TEXT_CHINESE_SIMPLIFIED:
      conv = ucnv_open("windows-936", &status);
      break;
    case VSD_TEXT_CHINESE_TRADITIONAL:
      conv = ucnv_open("windows-950", &status);
      break;
    case VSD_TEXT_GREEK:
      conv = ucnv_open("windows-1253", &status);
      break;
    case VSD_TEXT_TURKISH:
      conv = ucnv_open("windows-1254", &status);
      break;
    case VSD_TEXT_VIETNAMESE:
      conv = ucnv_open("windows-1258", &status);
      break;
    case VSD_TEXT_HEBREW:
      conv = ucnv_open("windows-1255", &status);
      break;
    case VSD_TEXT_ARABIC:
      conv = ucnv_open("windows-1256", &status);
      break;
    case VSD_TEXT_BALTIC:
      conv = ucnv_open("windows-1257", &status);
      break;
    case VSD_TEXT_RUSSIAN:
      conv = ucnv_open("windows-1251", &status);
      break;
    case VSD_TEXT_THAI:
      conv = ucnv_open("windows-874", &status);
      break;
    case VSD_TEXT_CENTRAL_EUROPE:
      conv = ucnv_open("windows-1250", &status);
      break;
    default:
      conv = ucnv_open("windows-1252", &status);
      break;
    }
    if (U_SUCCESS(status) && conv)
    {
      const char *src = (const char *)&characters[0];
      const char *srcLimit = (const char *)src + characters.size();
      while (src < srcLimit)
      {
        ucs4Character = ucnv_getNextUChar(conv, &src, srcLimit, &status);
        if (U_SUCCESS(status) && U_IS_UNICODE_CHAR(ucs4Character))
        {
          if (0x1e == ucs4Character)
            _appendField(text);
          else
            _appendUCS4(text, ucs4Character);
        }
      }
    }
    if (conv)
      ucnv_close(conv);
  }
}

void libvisio::VSDContentCollector::appendCharacters(librevenge::RVNGString &text, const std::vector<unsigned char> &characters)
{
  UErrorCode status = U_ZERO_ERROR;
  UConverter *conv = ucnv_open("UTF-16LE", &status);

  if (U_SUCCESS(status) && conv)
  {
    const char *src = (const char *)&characters[0];
    const char *srcLimit = (const char *)src + characters.size();
    while (src < srcLimit)
    {
      UChar32 ucs4Character = ucnv_getNextUChar(conv, &src, srcLimit, &status);
      if (U_SUCCESS(status) && U_IS_UNICODE_CHAR(ucs4Character))
      {
        if (0xfffc == ucs4Character)
          _appendField(text);
        else
          _appendUCS4(text, ucs4Character);
      }
    }
  }
  if (conv)
    ucnv_close(conv);
}

void libvisio::VSDContentCollector::_appendField(librevenge::RVNGString &text)
{
  if (m_fieldIndex < m_fields.size())
    text.append(m_fields[m_fieldIndex++].cstr());
  else
    m_fieldIndex++;
}

void libvisio::VSDContentCollector::collectMisc(unsigned level, const VSDMisc &misc)
{
  _handleLevelChange(level);
  m_misc = misc;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
