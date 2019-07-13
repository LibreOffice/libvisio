/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDContentCollector.h"

#include <algorithm>
#include <cassert>
#include <string.h> // for memcpy
#include <limits>
#include <set>
#include <stack>
#include <boost/spirit/include/qi.hpp>
#include <unicode/ucnv.h>
#include <unicode/utf8.h>

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

namespace
{

void computeRounding(double &prevX, double &prevY, double x0, double y0, double x, double y, double &rounding,
                     double &newX0, double &newY0, double &newX, double &newY, bool &sweep)
{
  double prevHalfLength = hypot(y0 - prevY, x0 - prevX) / 2.0;
  double halfLength = hypot(y - y0, x - x0) / 2.0;
  double lambda1 = atan2(y0-prevY, x0-prevX);
  double lambda2 = atan2(y-y0, x-x0);
  double angle = M_PI - lambda2 + lambda1;
  if (angle < 0.0)
    angle += 2.0*M_PI;
  if (angle > M_PI)
  {
    angle -= M_PI;
    sweep = !sweep;
  }
  double t = tan(angle / 2.0);
  if (!(t < 0 || t > 0))
    t = std::numeric_limits<double>::epsilon();
  double q = fabs(rounding / t);
  if (q > prevHalfLength)
  {
    q = prevHalfLength;
    rounding = fabs(q * t);
  }
  if (q > halfLength)
  {
    q = halfLength;
    rounding = fabs(q * t);
  }
  newX0 = x0-q*cos(lambda1);
  newY0 = y0-q*sin(lambda1);
  newX = x0+q*cos(lambda2);
  newY = y0+q*sin(lambda2);
  prevX = x0;
  prevY = y0;
}

unsigned computeBMPDataOffset(librevenge::RVNGInputStream *const input, const unsigned long maxLength)
{
  assert(input);

  using namespace libvisio;

  // determine header size
  unsigned headerSize = readU32(input);
  if (headerSize > maxLength)
    headerSize = 40; // assume v.3 bitmap header size
  unsigned off = headerSize;

  // determine palette size
  input->seek(10, librevenge::RVNG_SEEK_CUR);
  unsigned bpp = readU16(input);
  // sanitize bpp - limit to the allowed range and then round up to one
  // of the allowed values
  if (bpp > 32)
    bpp = 32;
  const unsigned allowedBpp[] = {1, 4, 8, 16, 24, 32};
  size_t bppIdx = 0;
  while (bppIdx < VSD_NUM_ELEMENTS(allowedBpp) && bpp > allowedBpp[bppIdx])
    ++bppIdx;
  if (bpp < allowedBpp[bppIdx])
    bpp = allowedBpp[bppIdx];
  input->seek(16, librevenge::RVNG_SEEK_CUR);
  unsigned paletteColors = readU32(input);
  if (bpp < 16 && paletteColors == 0)
    paletteColors = 1 << bpp;
  assert(maxLength >= off);
  if (paletteColors > 0 && (paletteColors < (maxLength - off) / 4))
    off += 4 * paletteColors;

  off += 14; // file header size

  return off;
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
  m_scale(1.0), m_x(0.0), m_y(0.0), m_originalX(0.0), m_originalY(0.0), m_xform(), m_txtxform(), m_misc(),
  m_currentFillGeometry(), m_currentLineGeometry(), m_groupXForms(groupXFormsSequence.empty() ? nullptr : &groupXFormsSequence[0]),
  m_currentForeignData(), m_currentOLEData(), m_currentForeignProps(), m_currentShapeId(0), m_foreignType((unsigned)-1),
  m_foreignFormat(0), m_foreignOffsetX(0.0), m_foreignOffsetY(0.0), m_foreignWidth(0.0), m_foreignHeight(0.0),
  m_noLine(false), m_noFill(false), m_noShow(false), m_fonts(),
  m_currentLevel(0), m_isShapeStarted(false),
  m_groupXFormsSequence(groupXFormsSequence), m_groupMembershipsSequence(groupMembershipsSequence),
  m_groupMemberships(m_groupMembershipsSequence.begin()),
  m_currentPageNumber(0), m_shapeOutputDrawing(nullptr), m_shapeOutputText(nullptr),
  m_pageOutputDrawing(), m_pageOutputText(), m_documentPageShapeOrders(documentPageShapeOrders),
  m_pageShapeOrder(m_documentPageShapeOrders.begin()), m_isFirstGeometry(true), m_NURBSData(), m_polylineData(),
  m_currentText(), m_names(), m_stencilNames(), m_fields(), m_stencilFields(), m_fieldIndex(0),
  m_charFormats(), m_paraFormats(), m_lineStyle(), m_fillStyle(), m_textBlockStyle(),
  m_defaultCharStyle(), m_defaultParaStyle(), m_currentStyleSheet(0), m_styles(styles),
  m_stencils(stencils), m_stencilShape(nullptr), m_isStencilStarted(false), m_currentGeometryCount(0),
  m_backgroundPageID(MINUS_ONE), m_currentPageID(0), m_currentPage(), m_pages(), m_layerList(),
  m_splineControlPoints(), m_splineKnotVector(), m_splineX(0.0), m_splineY(0.0),
  m_splineLastKnot(0.0), m_splineDegree(0), m_splineLevel(0), m_currentShapeLevel(0),
  m_isBackgroundPage(false), m_currentLayerList(), m_currentLayerMem(), m_tabSets(), m_documentTheme(nullptr)
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
  case 3: // Short line Arrow
    return "0 0 3000 3000";
  case 4:
  case 5:
  case 6:
  case 16:
  case 17:
  case 18:
    return "0 0 20 20";
  case 11: // Centered square filled
    return "0 0 10 10";
  case 12:
  case 13:
  case 14:
    return "0 0 20 30";
  case 22:
  case 39:
    return "0 0 20 40";
  case 21: // Centered square unfilled
    return "0 0 300 300";
  case 10:
    return "0 0 1131 1131";
  default:
    return "0 0 20 30";
  }
}

const char *libvisio::VSDContentCollector::_linePropertiesMarkerPath(unsigned marker)
{
  // Information how to draw path
  // https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/d

  /*
  SVG defines 6 types of path commands, for a total of 20 commands:

  MoveTo: M, m
  LineTo: L, l, H, h, V, v
  Cubic Bézier Curve: C, c, S, s
  Quadratic Bézier Curve: Q, q, T, t
  Elliptical Arc Curve: A, a
  ClosePath: Z, z
  */
  switch (marker)
  {
  case 1: // TODO
    return "M1500 0l1500 2789v211h-114l-1286-2392v2392h-200v-2392l-1286 2392h-114v-211z";
  case 2: //
    return "m10 0-10 10h20z";
  case 3: // Short line arrow, Copied from LO
    return "M1500 0l1500 2789v211h-114l-1286-2392v2392h-200v-2392l-1286 2392h-114v-211z";
  case 4: // Filled equilateral triangle
    return "m10 0-10 20h20z";
  case 5: // Arrow concave
    return "m10 0-10 20q10,-5 20,0z";
  case 6: //
    return "m10 0-10 20q10,5 20,0z";
  case 7: // TODO
    return "m10 0q-2.6,13.4 -10,18q10,-5 20,0q-7.4,-4.6 -10,-18";
  case 8: // filled arrow
    return "m10 0q-2.6,13.4 -10,18q10,-5 20,0q-7.4,-4.6 -10,-18";
  case 9:  // Centered line
    return "M1 2l1 -1l20 20l-1 1zM11 11v12h1v-10z";//
  case 10: // Centered Filled circle
  case 42: // Filled circle, Copied from LO
    return "m462 1118-102-29-102-51-93-72-72-93-51-102-29-102-13-105 13-102 29-106 51-102 72-89 93-72 102-50 102-34 106-9 101 9 106 34 98 50 93 72 72 89 51 102 29 106 13 102-13 105-29 102-51 102-72 93-93 72-98 51-106 29-101 13z";//
  case 11: // Centered square filled, Copied from LO
    return "M0 0h10v10h-10z";
  case 12: // TODO
    return "M1500 0l1500 2789v211h-114l-1286-2392v2392h-200v-2392l-1286 2392h-114v-211z";
  case 13:
    return "m10 0-10 30h20z";
  case 14:
    return "m10 0-10 30h20z m0 12l-5 15h10z";
  case 15:
    return "m10 0-10 10h20z m0 1l-8 8h16z";
  case 16:
    return "m10 0-10 20h20z m0 7l-5 10h10z";
  case 17:
    return "m10 0-10 20q10,-5 20,0z m0 7l-4 8q4,-2 8,0z";
  case 18:
    return "m10 0-10 20q10,5 20,0z m0 7l-5 10q5,2 10,0z";
  case 19: // TODO
    return "m10 0q-2.6,13.4 -10,18q10,-5 20,0q-7.4,-4.6 -10,-18";
  case 20: // Centered unfilled circle,
  case 41: // unfilled circle, Copied from LO
    return "M1500 3000c-276 0-511-63-750-201s-411-310-549-549-201-474-201-750 63-511 201-750 310-411 549-549 474-201 750-201 511 63 750 201 411 310 549 549 201 474 201 750-63 511-201 750-310 411-549 549-474 201-750 201zM1500 2800c-239 0-443-55-650-174s-356-269-476-476-174-411-174-650 55-443 174-650 269-356 476-476c207-119 411-174 650-174s443 55 650 174c207 120 356 269 476 476s174 411 174 650-55 443-174 650-269 356-476 476c-207 119-411 174-650 174z";
  case 21: // Centered unfilled square, Copied from LO
    return "M0 0h300v300h-300zM20 20h260v260h-260z";
  case 22: // Unfilled diamond, Copied from LO
    return "M1500 0l1500 3000-1500 3000-1500-3000zM1500 447l-1276 2553 1276 2553 1276-2553z";
  case 23:
    return "M1 32l1 1l19 -19l-1 -1zM11 0v33h1v-33z";
  case 24: // CF One, Copied from LO
    return "M0 0h1v-40h-2v40zM1 0h-20v-2h20zM-1 0h20v-2h-20z";
  case 25: // CF Only One, Copied from LO
    return "M0 0h1v-40h-2v40zM1 0h-20v-2h20zM-1 0h20v-2h-20zM1-18h-20v-2h20zM-1-18h20v-2h-20z";
  case 26: // TODO CF Only One with three lines
    return "M0 0h1v-40h-2v40zM1 0h-20v-2h20zM-1 0h20v-2h-20zM1-18h-20v-2h20zM-1-18h20v-2h-20z";
  case 27: // CF Many, Copied from LO
    return "M1500 0l1500-2789v-211h-114l-1286 2392v-2392h-200v2392l-1286-2392h-114v211z";
  case 28: // CF Many One, Copied from LO
    return "M1500 3200h1500v-200h-3000v200zM1500 3000l1500-2789v-211h-114l-1286 2392v-2392h-200v2392l-1286-2392h-114v211z";
  case 29: // CF Zero Many, Copied from LO
    return "M-1500 0c0-276 63-511 201-749 138-240 310-411 549-550 239-138 474-201 750-201s511 63 750 201c239 139 411 310 549 549 138 240 201 474 201 750 0 277-63 511-201 750-138 240-310 411-549 550-239 138-474 201-750 201s-511-63-750-201c-239-139-411-310-549-549s-201-474-201-750zM-1350 0c0-248 57-459 181-674 124-216 279-370 494-495 215-124 426-181 675-181s460 57 675 181c215 125 370 279 494 494 124 216 181 427 181 675 0 249-57 460-181 675-124 216-279 370-494 495-215 124-426 181-675 181s-460-57-675-181c-215-125-370-279-494-494-124-216-181-427-181-675zM0-1500l1500-2789v-211h-114l-1286 2392v-2392h-200v2392l-1286-2392h-114v211z";
  case 30: // CF Zero One, Copied from LO
    return "M100 4300c0-276 63-511 201-749 138-240 310-411 549-550 239-138 474-201 750-201s511 63 750 201c239 139 411 310 549 549 138 240 201 474 201 750 0 277-63 511-201 750-138 240-310 411-549 550-239 138-474 201-750 201s-511-63-750-201c-239-139-411-310-549-549s-201-474-201-750zM250 4300c0-248 57-459 181-674 124-216 279-370 494-495 215-124 426-181 675-181s460 57 675 181c215 125 370 279 494 494 124 216 181 427 181 675 0 249-57 460-181 675-124 216-279 370-494 495-215 124-426 181-675 181s-460-57-675-181c-215-125-370-279-494-494-124-216-181-427-181-675zM1600 2800h100v-2800h-200v2800zM1700 1400v100h1500v-200h-1500zM1500 1400v100h-1500v-200h1500z";
  case 31: // TODO unfilled circle and line
  case 32: // TODO unfilled circle and two lines
  case 33: // TODO unfilled circle and three lines
    return "M1500 3000c-276 0-511-63-750-201s-411-310-549-549-201-474-201-750 63-511 201-750 310-411 549-549 474-201 750-201 511 63 750 201 411 310 549 549 201 474 201 750-63 511-201 750-310 411-549 549-474 201-750 201zM1500 2800c-239 0-443-55-650-174s-356-269-476-476-174-411-174-650 55-443 174-650 269-356 476-476c207-119 411-174 650-174s443 55 650 174c207 120 356 269 476 476s174 411 174 650-55 443-174 650-269 356-476 476c-207 119-411 174-650 174z";
  case 34: // TODO unfilled circle and diamond
    return "M1500 3000c-276 0-511-63-750-201s-411-310-549-549-201-474-201-750 63-511 201-750 310-411 549-549 474-201 750-201 511 63 750 201 411 310 549 549 201 474 201 750-63 511-201 750-310 411-549 549-474 201-750 201zM1500 2800c-239 0-443-55-650-174s-356-269-476-476-174-411-174-650 55-443 174-650 269-356 476-476c207-119 411-174 650-174s443 55 650 174c207 120 356 269 476 476s174 411 174 650-55 443-174 650-269 356-476 476c-207 119-411 174-650 174z";
  case 35: // TODO Filled circle with line,
  case 36: // TODO Filled circle with two lines,
  case 37: // TODO Filled circle with three lines,
    return "m462 1118-102-29-102-51-93-72-72-93-51-102-29-102-13-105 13-102 29-106 51-102 72-89 93-72 102-50 102-34 106-9 101 9 106 34 98 50 93 72 72 89 51 102 29 106 13 102-13 105-29 102-51 102-72 93-93 72-98 51-106 29-101 13z";//
  case 38: // TODO Filled circle with diamond,
    return "m462 1118-102-29-102-51-93-72-72-93-51-102-29-102-13-105 13-102 29-106 51-102 72-89 93-72 102-50 102-34 106-9 101 9 106 34 98 50 93 72 72 89 51 102 29 106 13 102-13 105-29 102-51 102-72 93-93 72-98 51-106 29-101 13z";//
  case 39: // double filled equilateral triangle arrow, Copied from LO
    return "M737 1131h394l-564-1131-567 1131h398l-398 787h1131z";
  case 40: // TODO double unfilled equilateral triangle arrow
    return "M737 1131h394l-564-1131-567 1131h398l-398 787h1131z";
  case 43: // TODO double Short line arrow
    return "M1500 0l1500 2789v211h-114l-1286-2392v2392h-200v-2392l-1286 2392h-114v-211z";
  case 44: // TODO Short line arrow with line
    return "M1500 0l1500 2789v211h-114l-1286-2392v2392h-200v-2392l-1286 2392h-114v-211z";
  case 45: // TODO double Short line arrow with line
    return "M1500 0l1500 2789v211h-114l-1286-2392v2392h-200v-2392l-1286 2392h-114v-211z";

  default: // default arrow
    return "m10 0-10 30h20z";//
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
  unsigned shapeId = m_currentShapeId;
  if (m_fillStyle.pattern && !m_currentFillGeometry.empty())
    numPathElements++;
  if (m_lineStyle.pattern && !m_currentLineGeometry.empty())
    numPathElements++;
  if (m_currentForeignData.size() && m_currentForeignProps["librevenge:mime-type"] && m_foreignWidth != 0.0 && m_foreignHeight != 0.0)
    numForeignElements++;
  if (!m_currentText.empty())
  {
    if ((m_currentText.m_format == VSD_TEXT_UTF16
         && (m_currentText.m_data.size() >= 2 && (m_currentText.m_data.getDataBuffer()[0] || m_currentText.m_data.getDataBuffer()[1])))
        || m_currentText.m_data.getDataBuffer()[0])
    {
      numTextElements++;
    }
  }

  if (numPathElements+numForeignElements+numTextElements > 1)
  {
    librevenge::RVNGPropertyList propList;
    if (shapeId && shapeId != MINUS_ONE)
    {
      librevenge::RVNGString stringId;
      stringId.sprintf("id%u", shapeId);
      propList.insert("draw:id", stringId);
      shapeId = MINUS_ONE;
    }
    m_shapeOutputDrawing->addStartLayer(propList);
  }

  if (numPathElements > 1 && (numForeignElements || numTextElements))
  {
    librevenge::RVNGPropertyList propList;
    if (shapeId && shapeId != MINUS_ONE)
    {
      librevenge::RVNGString stringId;
      stringId.sprintf("id%u", shapeId);
      propList.insert("draw:id", stringId);
      shapeId = MINUS_ONE;
    }
    m_shapeOutputDrawing->addStartLayer(librevenge::RVNGPropertyList());
  }
  _flushCurrentPath(shapeId);
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

void libvisio::VSDContentCollector::_flushCurrentPath(unsigned shapeId)
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
    for (auto &i : m_currentFillGeometry)
    {
      if (firstPoint)
      {
        firstPoint = false;
        wasMove = true;
      }
      else if (i["librevenge:path-action"]->getStr() == "M")
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
      tmpPath.push_back(i);
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
      _convertToPath(tmpPath, path, m_scale*m_lineStyle.rounding);
      m_shapeOutputDrawing->addStyle(fillPathProps);
      librevenge::RVNGPropertyList propList;
      propList.insert("svg:d", path);
      if (shapeId && shapeId != MINUS_ONE)
      {
        librevenge::RVNGString stringId;
        stringId.sprintf("id%u", shapeId);
        propList.insert("draw:id", stringId);
        shapeId = MINUS_ONE;
      }
      _appendVisibleAndPrintable(propList);
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
    for (auto &i : m_currentLineGeometry)
    {
      if (firstPoint)
      {
        firstPoint = false;
        wasMove = true;
        x = i["svg:x"]->getDouble();
        y = i["svg:y"]->getDouble();
      }
      else if (i["librevenge:path-action"]->getStr() == "M")
      {
        if (!tmpPath.empty())
        {
          if (!wasMove)
          {
            if (VSD_ALMOST_ZERO(x - prevX) && VSD_ALMOST_ZERO(y - prevY))
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
        x = i["svg:x"]->getDouble();
        y = i["svg:y"]->getDouble();
        wasMove = true;
      }
      else
        wasMove = false;
      tmpPath.push_back(i);
      if (i["svg:x"])
        prevX = i["svg:x"]->getDouble();
      if (i["svg:y"])
        prevY = i["svg:y"]->getDouble();
    }
    if (!tmpPath.empty())
    {
      if (!wasMove)
      {
        if (VSD_ALMOST_ZERO(x - prevX) && VSD_ALMOST_ZERO(y - prevY))
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
      _convertToPath(tmpPath, path, m_scale*m_lineStyle.rounding);
      m_shapeOutputDrawing->addStyle(linePathProps);
      librevenge::RVNGPropertyList propList;
      propList.insert("svg:d", path);
      if (shapeId && shapeId != MINUS_ONE)
      {
        librevenge::RVNGString stringId;
        stringId.sprintf("id%u", shapeId);
        propList.insert("draw:id", stringId);
      }
      _appendVisibleAndPrintable(propList);
      m_shapeOutputDrawing->addPath(propList);
    }
  }
  m_currentLineGeometry.clear();
}

void libvisio::VSDContentCollector::_convertToPath(const std::vector<librevenge::RVNGPropertyList> &segmentVector,
                                                   librevenge::RVNGPropertyListVector &path, double rounding)
{
  if (segmentVector.empty())
    return;
  if (rounding > 0.0)
  {
    double prevX = segmentVector[0]["svg:x"] ? segmentVector[0]["svg:x"]->getDouble() : 0.0;
    double prevY = segmentVector[0]["svg:y"] ? segmentVector[0]["svg:y"]->getDouble() : 0.0;
    unsigned moveIndex = 0;
    std::vector<librevenge::RVNGPropertyList> tmpSegment;
    for (size_t i = 0; i < segmentVector.size(); ++i)
    {
      if (segmentVector[i]["librevenge:path-action"] && segmentVector[i]["librevenge:path-action"]->getStr() == "M")
      {
        _convertToPath(tmpSegment, path, 0.0);
        tmpSegment.clear();
      }
      tmpSegment.push_back(segmentVector[i]);
      if (segmentVector[i]["librevenge:path-action"] && segmentVector[i]["librevenge:path-action"]->getStr() == "M")
      {
        prevX = segmentVector[i]["svg:x"] ? segmentVector[i]["svg:x"]->getDouble() : 0.0;
        prevY = segmentVector[i]["svg:y"] ? segmentVector[i]["svg:y"]->getDouble() : 0.0;
        moveIndex = i;
      }
      else if (segmentVector[i]["librevenge:path-action"] && segmentVector[i]["librevenge:path-action"]->getStr() == "L")
      {
        double x0 = segmentVector[i]["svg:x"] ? segmentVector[i]["svg:x"]->getDouble() : 0.0;
        double y0 = segmentVector[i]["svg:y"] ? segmentVector[i]["svg:y"]->getDouble() : 0.0;
        if (i+1 < segmentVector.size() && segmentVector[i+1]["librevenge:path-action"] && segmentVector[i+1]["librevenge:path-action"]->getStr() == "L")
        {
          double x = segmentVector[i+1]["svg:x"] ? segmentVector[i+1]["svg:x"]->getDouble() : 0.0;
          double y = segmentVector[i+1]["svg:y"] ? segmentVector[i+1]["svg:y"]->getDouble() : 0.0;
          double newX0, newY0, newX, newY;
          double tmpRounding(rounding);
          bool sweep(true);
          computeRounding(prevX, prevY, x0, y0, x, y, tmpRounding, newX0, newY0, newX, newY, sweep);
          tmpSegment.back().insert("svg:x", newX0);
          tmpSegment.back().insert("svg:y", newY0);
          librevenge::RVNGPropertyList q;
          q.insert("librevenge:path-action", "Q");
          q.insert("svg:x1", x0);
          q.insert("svg:y1", y0);
          q.insert("svg:x", newX);
          q.insert("svg:y", newY);
          tmpSegment.push_back(q);
        }
        else if (i+1 < segmentVector.size() && segmentVector[i+1]["librevenge:path-action"] && segmentVector[i+1]["librevenge:path-action"]->getStr() == "Z")
        {
          if (tmpSegment.size() >= 2 &&
              segmentVector[moveIndex]["librevenge:path-action"] &&
              segmentVector[moveIndex]["librevenge:path-action"]->getStr() == "M" &&
              segmentVector[moveIndex+1]["librevenge:path-action"] &&
              segmentVector[moveIndex+1]["librevenge:path-action"]->getStr() == "L")
          {
            double x = segmentVector[moveIndex+1]["svg:x"] ? segmentVector[moveIndex+1]["svg:x"]->getDouble() : 0.0;
            double y = segmentVector[moveIndex+1]["svg:y"] ? segmentVector[moveIndex+1]["svg:y"]->getDouble() : 0.0;
            double newX0, newY0, newX, newY;
            double tmpRounding(rounding);
            bool sweep(true);
            computeRounding(prevX, prevY, x0, y0, x, y, tmpRounding, newX0, newY0, newX, newY, sweep);
            tmpSegment.back().insert("svg:x", newX0);
            tmpSegment.back().insert("svg:y", newY0);
            librevenge::RVNGPropertyList q;
            q.insert("librevenge:path-action", "Q");
            q.insert("svg:x1", x0);
            q.insert("svg:y1", y0);
            q.insert("svg:x", newX);
            q.insert("svg:y", newY);
            tmpSegment.push_back(q);
            tmpSegment[0].insert("svg:x", newX) ;
            tmpSegment[0].insert("svg:y", newY);
          }
        }
      }
      else if (segmentVector[i]["librevenge:path-action"] && segmentVector[i]["librevenge:path-action"]->getStr() == "Z")
      {
        prevX = segmentVector[moveIndex]["svg:x"] ? segmentVector[moveIndex]["svg:x"]->getDouble() : 0.0;
        prevY = segmentVector[moveIndex]["svg:y"] ? segmentVector[moveIndex]["svg:y"]->getDouble() : 0.0;
      }
      else
      {
        prevX = segmentVector[i]["svg:x"] ? segmentVector[i]["svg:x"]->getDouble() : 0.0;
        prevY = segmentVector[i]["svg:y"] ? segmentVector[i]["svg:y"]->getDouble() : 0.0;
      }
    }
    _convertToPath(tmpSegment, path, 0.0);
  }
  else
  {
    double prevX = DBL_MAX;
    double prevY = DBL_MAX;
    for (const auto &i : segmentVector)
    {
      if (!i["librevenge:path-action"])
        continue;
      double x = DBL_MAX;
      double y = DBL_MAX;
      if (i["svg:x"] && i["svg:y"])

      {
        x = i["svg:x"]->getDouble();
        y = i["svg:y"]->getDouble();
      }
      // skip segment that have length 0.0
      if (!VSD_ALMOST_ZERO(x-prevX) || !VSD_ALMOST_ZERO(y-prevY))
      {
        path.append(i);
        prevX = x;
        prevY = y;
      }
    }
  }
}

void libvisio::VSDContentCollector::_flushText()
{
  /* Do not output empty text objects. */
  if (m_currentText.empty() || m_misc.m_hideText)
    return;
  else
    // Check whether the buffer contains only the terminating NULL character
  {
    if (m_currentText.m_format == VSD_TEXT_UTF16)
    {
      if (m_currentText.m_data.size() < 2)
        return;
      else if (!(m_currentText.m_data.getDataBuffer()[0]) && !(m_currentText.m_data.getDataBuffer()[1]))
        return;
    }
    else if (!(m_currentText.m_data.getDataBuffer()[0]))
      return;
  }

  /* Fill the text object/frame properties */
  double xmiddle = m_txtxform ? m_txtxform->width / 2.0 : m_xform.width / 2.0;
  double ymiddle = m_txtxform ? m_txtxform->height / 2.0 : m_xform.height / 2.0;

  transformPoint(xmiddle,ymiddle, m_txtxform.get());

  double x = xmiddle - (m_txtxform ? m_txtxform->width / 2.0 : m_xform.width / 2.0);
  double y = ymiddle - (m_txtxform ? m_txtxform->height / 2.0 : m_xform.height / 2.0);

  double angle = 0.0;
  transformAngle(angle, m_txtxform.get());

  librevenge::RVNGPropertyList textBlockProps;

  bool flipX = false;
  bool flipY = false;
  transformFlips(flipX, flipY);

  if (flipX)
    angle -= M_PI;

  angle = std::fmod(angle, 2 * M_PI);
  if (angle < 0)
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

  _appendVisibleAndPrintable(textBlockProps);

  /* Start the text object. */
  m_shapeOutputText->addStartTextObject(textBlockProps);

  /* Assure that the different styles have at least one element,
   * corresponding to the default style. */
  if (m_charFormats.empty())
  {
    m_charFormats.push_back(m_defaultCharStyle);
    m_charFormats.back().charCount = 0;
  }
  if (m_paraFormats.empty())
  {
    m_paraFormats.push_back(m_defaultParaStyle);
    m_paraFormats.back().charCount = 0;
  }
  if (m_tabSets.empty())
  {
    m_tabSets.push_back(VSDTabSet());
    m_tabSets.back().m_numChars = 0;
  }

  /* Helper variables used to iterate over the text buffer */
  std::vector<VSDParaStyle>::const_iterator paraIt = m_paraFormats.begin();
  std::vector<VSDCharStyle>::const_iterator charIt = m_charFormats.begin();
  std::vector<VSDTabSet>::const_iterator tabIt = m_tabSets.begin();

  bool isParagraphOpened(false);
  bool isSpanOpened(false);
  bool isParagraphWithoutSpan(false);

  VSDBullet currentBullet;

  unsigned paraNumRemaining(paraIt->charCount);
  unsigned charNumRemaining(charIt->charCount);
  unsigned tabNumRemaining(tabIt->m_numChars);

  std::vector<unsigned char> sOutputVector;
  librevenge::RVNGString sOutputText;

  // Unfortunately, we have to handle the unicode formats differently then the 8-bit formats
  if (m_currentText.m_format == VSD_TEXT_UTF8 || m_currentText.m_format == VSD_TEXT_UTF16)
  {
    std::vector<unsigned char> tmpBuffer(m_currentText.m_data.size());
    memcpy(&tmpBuffer[0], m_currentText.m_data.getDataBuffer(), m_currentText.m_data.size());
    librevenge::RVNGString textString;
    appendCharacters(textString, tmpBuffer, m_currentText.m_format);
    /* Iterate over the text character by character */
    librevenge::RVNGString::Iter textIt(textString);
    for (textIt.rewind(); textIt.next();)
    {
      /* Any character will cause a paragraph to open if it is not yet opened. */
      if (!isParagraphOpened)
      {
        librevenge::RVNGPropertyList paraProps;
        _fillParagraphProperties(paraProps, *paraIt);

        if (m_textBlockStyle.defaultTabStop > 0.0)
          paraProps.insert("style:tab-stop-distance", m_textBlockStyle.defaultTabStop);

        _fillTabSet(paraProps, *tabIt);

        VSDBullet bullet;
        _bulletFromParaFormat(bullet, *paraIt);

        /* Bullet definition changed with regard to the last paragraph style. */
        if (bullet != currentBullet)
        {
          /* If the previous paragraph style had a bullet, close the list level. */
          if (!!currentBullet)
            m_shapeOutputText->addCloseUnorderedListLevel();

          currentBullet = bullet;
          /* If the current paragraph style has a bullet, open a new list level. */
          if (!!currentBullet)
          {
            librevenge::RVNGPropertyList bulletList;
            _listLevelFromBullet(bulletList, currentBullet);
            m_shapeOutputText->addOpenUnorderedListLevel(bulletList);
          }
        }

        if (!currentBullet)
          m_shapeOutputText->addOpenParagraph(paraProps);
        else
          m_shapeOutputText->addOpenListElement(paraProps);
        isParagraphOpened = true;
        isParagraphWithoutSpan = true;
      }

      /* Any character will cause a span to open if it is not yet opened.
       * The additional conditions aim to avoid superfluous empty span but
       * also a paragraph without span at all. */
      if (!isSpanOpened && ((*(textIt()) != '\n') || isParagraphWithoutSpan))
      {
        librevenge::RVNGPropertyList textProps;
        _fillCharProperties(textProps, *charIt);

        // TODO: In draw, text span background cannot be specified the same way as in writer span
        if (m_textBlockStyle.isTextBkgndFilled)
        {
          textProps.insert("fo:background-color", getColourString(m_textBlockStyle.textBkgndColour));
#if 0
          if (m_textBlockStyle.textBkgndColour.a)
            textProps.insert("fo:background-opacity", 1.0 - m_textBlockStyle.textBkgndColour.a/255.0, librevenge::RVNG_PERCENT);
#endif
        }
        m_shapeOutputText->addOpenSpan(textProps);
        isSpanOpened = true;
        isParagraphWithoutSpan = false;
      }

      /* Current character is a paragraph break,
       * which will cause the paragraph to close. */
      if (*(textIt()) == '\n')
      {
        if (!sOutputText.empty())
          m_shapeOutputText->addInsertText(sOutputText);
        sOutputText.clear();
        if (isSpanOpened)
        {
          m_shapeOutputText->addCloseSpan();
          isSpanOpened = false;
        }

        if (isParagraphOpened)
        {
          if (!currentBullet)
            m_shapeOutputText->addCloseParagraph();
          else
            m_shapeOutputText->addCloseListElement();
          isParagraphOpened = false;
        }
      }
      /* Current character is a tabulator. We have to output
       * the current text buffer and insert the tab. */
      else if (*(textIt()) == '\t')
      {
        if (!sOutputText.empty())
          m_shapeOutputText->addInsertText(sOutputText);
        sOutputText.clear();
        m_shapeOutputText->addInsertTab();
      }
      /* Current character is a field placeholder. We append
       * to the current text buffer a text representation
       * of the field. */
      else if (strlen(textIt()) == 3 &&
               textIt()[0] == '\xef' &&
               textIt()[1] == '\xbf' &&
               textIt()[2] == '\xbc')
        _appendField(sOutputText);
      /* We have a normal UTF8 character and we append it
       * to the current text buffer. */
      else
        sOutputText.append(textIt());

      /* Decrease the count of remaining characters in the same paragraph,
       * if it is possible. */
      if (paraNumRemaining)
        paraNumRemaining--;
      /* Fetch next paragraph style if it exists. If not, just use the
       * last one. */
      if (!paraNumRemaining)
      {
        ++paraIt;
        if (paraIt != m_paraFormats.end())
          paraNumRemaining = paraIt->charCount;
        else
          --paraIt;
      }

      /* Decrease the count of remaining characters in the same span,
       * if it is possible. */
      if (charNumRemaining)
        charNumRemaining--;
      /* Fetch next character style if it exists and close span, since
       * the next span will have to use the new character style.
       * If there is no more character style to fetch, just finish using
       * the last one. */
      if (!charNumRemaining)
      {
        ++charIt;
        if (charIt != m_charFormats.end())
        {
          charNumRemaining = charIt->charCount;
          if (isSpanOpened)
          {
            if (!sOutputText.empty())
              m_shapeOutputText->addInsertText(sOutputText);
            sOutputText.clear();
            m_shapeOutputText->addCloseSpan();
            isSpanOpened = false;
          }
        }
        else
          --charIt;
      }

      /* Decrease the count of remaining characters using the same
       * tab-set definition, if it is possible. */
      if (tabNumRemaining)
        tabNumRemaining--;
      /* Fetch next tab-set definition if it exists. If not, just use the
       * last one. */
      if (!tabNumRemaining)
      {
        ++tabIt;
        if (tabIt != m_tabSets.end())
          tabNumRemaining = tabIt->m_numChars;
        else
          --tabIt;
      }
    }
  }
  else // 8-bit charsets
  {
    /* Iterate over the text character by character */
    const unsigned char *tmpBuffer = m_currentText.m_data.getDataBuffer();
    unsigned long tmpBufferLength = m_currentText.m_data.size();
    // Remove the terminating \0 character from the buffer
    while (tmpBufferLength > 1 &&!tmpBuffer[tmpBufferLength-1])
    {
      --tmpBufferLength;
    }
    for (unsigned long i = 0; i < tmpBufferLength; ++i)
    {
      /* Any character will cause a paragraph to open if it is not yet opened. */
      if (!isParagraphOpened)
      {
        librevenge::RVNGPropertyList paraProps;
        _fillParagraphProperties(paraProps, *paraIt);

        if (m_textBlockStyle.defaultTabStop > 0.0)
          paraProps.insert("style:tab-stop-distance", m_textBlockStyle.defaultTabStop);

        _fillTabSet(paraProps, *tabIt);

        VSDBullet bullet;
        _bulletFromParaFormat(bullet, *paraIt);

        /* Bullet definition changed with regard to the last paragraph style. */
        if (bullet != currentBullet)
        {
          /* If the previous paragraph style had a bullet, close the list level. */
          if (!!currentBullet)
            m_shapeOutputText->addCloseUnorderedListLevel();

          currentBullet = bullet;
          /* If the current paragraph style has a bullet, open a new list level. */
          if (!!currentBullet)
          {
            librevenge::RVNGPropertyList bulletList;
            _listLevelFromBullet(bulletList, currentBullet);
            m_shapeOutputText->addOpenUnorderedListLevel(bulletList);
          }
        }

        if (!currentBullet)
          m_shapeOutputText->addOpenParagraph(paraProps);
        else
          m_shapeOutputText->addOpenListElement(paraProps);
        isParagraphOpened = true;
        isParagraphWithoutSpan = true;
      }

      /* Any character will cause a span to open if it is not yet opened.
       * The additional conditions aim to avoid superfluous empty span but
       * also a paragraph without span at all. */
      if (!isSpanOpened && ((tmpBuffer[i] != (unsigned char)'\n' && tmpBuffer[i] != 0x0d && tmpBuffer[i] != 0x0e) || isParagraphWithoutSpan))
      {
        librevenge::RVNGPropertyList textProps;
        _fillCharProperties(textProps, *charIt);

        // TODO: In draw, text span background cannot be specified the same way as in writer span
        if (m_textBlockStyle.isTextBkgndFilled)
        {
          textProps.insert("fo:background-color", getColourString(m_textBlockStyle.textBkgndColour));
#if 0
          if (m_textBlockStyle.textBkgndColour.a)
            textProps.insert("fo:background-opacity", 1.0 - m_textBlockStyle.textBkgndColour.a/255.0, librevenge::RVNG_PERCENT);
#endif
        }
        m_shapeOutputText->addOpenSpan(textProps);
        isSpanOpened = true;
        isParagraphWithoutSpan = false;
      }

      /* Current character is a paragraph break,
       * which will cause the paragraph to close. */
      if (tmpBuffer[i] == (unsigned char)'\n' || tmpBuffer[i] == 0x0d || tmpBuffer[i] == 0x0e)
      {
        if (!sOutputVector.empty())
        {
          appendCharacters(sOutputText, sOutputVector, charIt->font.m_format);
          sOutputVector.clear();
        }
        if (!sOutputText.empty())
        {
          m_shapeOutputText->addInsertText(sOutputText);
          sOutputText.clear();
        }
        if (isSpanOpened)
        {
          m_shapeOutputText->addCloseSpan();
          isSpanOpened = false;
        }

        if (isParagraphOpened)
        {
          if (!currentBullet)
            m_shapeOutputText->addCloseParagraph();
          else
            m_shapeOutputText->addCloseListElement();
          isParagraphOpened = false;
        }
      }
      /* Current character is a tabulator. We have to output
       * the current text buffer and insert the tab. */
      else if (tmpBuffer[i] == (unsigned char)'\t')
      {
        if (!sOutputVector.empty())
        {
          appendCharacters(sOutputText, sOutputVector, charIt->font.m_format);
          sOutputVector.clear();
        }
        if (!sOutputText.empty())
        {
          m_shapeOutputText->addInsertText(sOutputText);
          sOutputText.clear();
        }
        m_shapeOutputText->addInsertTab();
      }
      /* Current character is a field placeholder. We append
       * to the current text buffer a text representation
       * of the field. */
      else if (tmpBuffer[i] == 0x1e)
      {
        if (!sOutputVector.empty())
        {
          appendCharacters(sOutputText, sOutputVector, charIt->font.m_format);
          sOutputVector.clear();
        }
        _appendField(sOutputText);
      }
      /* We have a normal UTF8 character and we append it
       * to the current text buffer. */
      else
        sOutputVector.push_back(tmpBuffer[i]);

      /* Decrease the count of remaining characters in the same paragraph,
       * if it is possible. */
      if (paraNumRemaining)
        paraNumRemaining--;
      /* Fetch next paragraph style if it exists. If not, just use the
       * last one. */
      if (!paraNumRemaining)
      {
        ++paraIt;
        if (paraIt != m_paraFormats.end())
          paraNumRemaining = paraIt->charCount;
        else
          --paraIt;
      }

      /* Decrease the count of remaining characters in the same span,
       * if it is possible. */
      if (charNumRemaining)
        charNumRemaining--;
      /* Fetch next character style if it exists and close span, since
       * the next span will have to use the new character style.
       * If there is no more character style to fetch, just finish using
       * the last one. */
      if (!charNumRemaining)
      {
        ++charIt;
        if (charIt != m_charFormats.end())
        {
          charNumRemaining = charIt->charCount;
          if (isSpanOpened)
          {
            if (!sOutputVector.empty())
            {
              appendCharacters(sOutputText, sOutputVector, charIt->font.m_format);
              sOutputVector.clear();
            }
            if (!sOutputText.empty())
            {
              m_shapeOutputText->addInsertText(sOutputText);
              sOutputText.clear();
            }
            m_shapeOutputText->addCloseSpan();
            isSpanOpened = false;
          }
        }
        else
          --charIt;
      }

      /* Decrease the count of remaining characters using the same
       * tab-set definition, if it is possible. */
      if (tabNumRemaining)
        tabNumRemaining--;
      /* Fetch next tab-set definition if it exists. If not, just use the
       * last one. */
      if (!tabNumRemaining)
      {
        ++tabIt;
        if (tabIt != m_tabSets.end())
          tabNumRemaining = tabIt->m_numChars;
        else
          --tabIt;
      }
    }
  }

  // Clean up the elements that remained opened
  if (isParagraphOpened)
  {
    if (isSpanOpened)
    {
      if (!sOutputVector.empty())
      {
        appendCharacters(sOutputText, sOutputVector, charIt->font.m_format);
        sOutputVector.clear();
      }
      if (!sOutputText.empty())
      {
        m_shapeOutputText->addInsertText(sOutputText);
        sOutputText.clear();
      }
      m_shapeOutputText->addCloseSpan();
    }

    if (!currentBullet)
      m_shapeOutputText->addCloseParagraph();
    else
      m_shapeOutputText->addCloseListElement();
  }

  /* Last paragraph style had a bullet and we have to close
   * the corresponding list level. */
  if (!!currentBullet)
    m_shapeOutputText->addCloseUnorderedListLevel();

  m_shapeOutputText->addEndTextObject();
  m_currentText.clear();
}

void libvisio::VSDContentCollector::_fillCharProperties(librevenge::RVNGPropertyList &propList, const VSDCharStyle &style)
{
  librevenge::RVNGString fontName;
  if (style.font.m_data.size())
    _convertDataToString(fontName, style.font.m_data, style.font.m_format);
  else
    fontName = "Arial";

  propList.insert("style:font-name", fontName);

  if (style.bold) propList.insert("fo:font-weight", "bold");
  if (style.italic) propList.insert("fo:font-style", "italic");
  if (style.underline) propList.insert("style:text-underline-type", "single");
  if (style.doubleunderline) propList.insert("style:text-underline-type", "double");
  if (style.strikeout) propList.insert("style:text-line-through-type", "single");
  if (style.doublestrikeout) propList.insert("style:text-line-through-type", "double");
  if (style.allcaps) propList.insert("fo:text-transform", "uppercase");
  if (style.initcaps) propList.insert("fo:text-transform", "capitalize");
  if (style.smallcaps) propList.insert("fo:font-variant", "small-caps");
  if (style.superscript) propList.insert("style:text-position", "super");
  if (style.subscript) propList.insert("style:text-position", "sub");
  if (style.scaleWidth != 1.0) propList.insert("style:text-scale", style.scaleWidth, librevenge::RVNG_PERCENT);
  propList.insert("fo:font-size", style.size*72.0, librevenge::RVNG_POINT);
  Colour colour = style.colour;
  const Colour *pColour = m_currentLayerList.getColour(m_currentLayerMem);
  if (pColour)
    colour = *pColour;
  propList.insert("fo:color", getColourString(colour));
  double opacity = 1.0;
  if (style.colour.a)
    opacity -= (double)(style.colour.a)/255.0;
  propList.insert("svg:stroke-opacity", opacity, librevenge::RVNG_PERCENT);
  propList.insert("svg:fill-opacity", opacity, librevenge::RVNG_PERCENT);
}

void libvisio::VSDContentCollector::_fillParagraphProperties(librevenge::RVNGPropertyList &propList, const VSDParaStyle &style)
{
  propList.insert("fo:text-indent", style.indFirst);
  propList.insert("fo:margin-left", style.indLeft);
  propList.insert("fo:margin-right", style.indRight);
  propList.insert("fo:margin-top", style.spBefore);
  propList.insert("fo:margin-bottom", style.spAfter);

  switch (style.align)
  {
  case 0: // left
    if (!style.flags)
      propList.insert("fo:text-align", "left");
    else
      propList.insert("fo:text-align", "end");
    break;
  case 2: // right
    if (!style.flags)
      propList.insert("fo:text-align", "end");
    else
      propList.insert("fo:text-align", "left");
    break;
  case 3: // justify
    propList.insert("fo:text-align", "justify");
    break;
  case 4: // full
    propList.insert("fo:text-align", "full");
    break;
  default: // center
    propList.insert("fo:text-align", "center");
    break;
  }
  if (style.spLine > 0)
    propList.insert("fo:line-height", style.spLine);
  else
    propList.insert("fo:line-height", -style.spLine, librevenge::RVNG_PERCENT);

}

void libvisio::VSDContentCollector::_fillTabSet(librevenge::RVNGPropertyList &propList, const VSDTabSet &tabSet)
{
  librevenge::RVNGPropertyListVector tmpTabSet;
  for (const auto &tabStop : tabSet.m_tabStops)
  {
    librevenge::RVNGPropertyList tmpTabStop;
    tmpTabStop.insert("style:position", tabStop.second.m_position);
    switch (tabStop.second.m_alignment)
    {
    case 0:
      tmpTabStop.insert("style:type", "left");
      break;
    case 1:
      tmpTabStop.insert("style:type", "center");
      break;
    case 2:
      tmpTabStop.insert("style:type", "right");
      break;
    default:
      tmpTabStop.insert("style:type", "char");
      tmpTabStop.insert("style:char", ".");
      break;
    }
    tmpTabSet.append(tmpTabStop);
  }
  if (!tmpTabSet.empty())
    propList.insert("style:tab-stops", tmpTabSet);
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

  _appendVisibleAndPrintable(m_currentForeignProps);

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
    for (unsigned int &iterList : *m_pageShapeOrder)
    {
      auto iterGroup = m_groupMemberships->find(iterList);
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
      iter = m_pageOutputDrawing.find(iterList);
      if (iter != m_pageOutputDrawing.end())
        m_currentPage.append(iter->second);
      iter = m_pageOutputText.find(iterList);
      if (iter != m_pageOutputText.end())
        groupTextStack.push(std::make_pair(iterList, iter->second));
      else
        groupTextStack.push(std::make_pair(iterList, VSDOutputElementList()));
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

void libvisio::VSDContentCollector::collectDocumentTheme(const VSDXTheme *theme)
{
  if (theme)
    m_documentTheme = theme;
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

  double rx = hypot(x1 - x0, y1 - y0);
  double ry = ecc != 0 ? rx / ecc : rx;
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
  double h = hypot(xleft - cx, yleft - cy);
  double angle = h != 0 ? fmod(2.0*M_PI + (cy > yleft ? 1.0 : -1.0)*acos((cx-xleft) / h), 2.0*M_PI) : 0;
  transformPoint(cx, cy);
  transformPoint(xleft, yleft);
  transformPoint(xtop, ytop);
  transformAngle(angle);

  double rx = hypot(xleft - cx, yleft - cy);
  double ry = hypot(xtop - cx, ytop - cy);

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

  if (VSD_APPROX_EQUAL(x1, x2))
  {
    xmove = x1;
    ymove = 0;
    xline = x1;
    yline = m_pageHeight;
  }
  else if (VSD_APPROX_EQUAL(y1, y2))
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
      for (auto &point : points)
      {
        if (point.first != xmove || point.second != ymove)
        {
          xline = point.first;
          yline = point.second;
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
                                                const boost::optional<unsigned char> &startMarker, const boost::optional<unsigned char> &endMarker, const boost::optional<unsigned char> &lineCap,
                                                const boost::optional<double> &rounding, const boost::optional<long> &qsLineColour, const boost::optional<long> &qsLineMatrix)
{
  _handleLevelChange(level);
  m_lineStyle.override(VSDOptionalLineStyle(strokeWidth, c, linePattern, startMarker, endMarker, lineCap, rounding, qsLineColour, qsLineMatrix), m_documentTheme);
}

void libvisio::VSDContentCollector::collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                                         const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency, const boost::optional<double> &fillBGTransparency,
                                                         const boost::optional<unsigned char> &shadowPattern, const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX,
                                                         const boost::optional<double> &shadowOffsetY, const boost::optional<long> &qsFillColour, const boost::optional<long> &qsShadowColour,
                                                         const boost::optional<long> &qsFillMatrix)
{
  _handleLevelChange(level);
  m_fillStyle.override(VSDOptionalFillStyle(colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shfgc,
                                            shadowPattern, shadowOffsetX, shadowOffsetY, qsFillColour, qsShadowColour, qsFillMatrix), m_documentTheme);
}

void libvisio::VSDContentCollector::collectFillAndShadow(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                                         const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                                                         const boost::optional<double> &fillBGTransparency,
                                                         const boost::optional<unsigned char> &shadowPattern, const boost::optional<Colour> &shfgc)
{
  collectFillAndShadow(level, colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc, m_shadowOffsetX, m_shadowOffsetY, -1, -1, -1);
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

      const unsigned dataOff = computeBMPDataOffset(binaryData.getDataStream(), binaryData.size());
      m_currentForeignData.append((unsigned char)(dataOff & 0xff));
      m_currentForeignData.append((unsigned char)((dataOff >> 8) & 0xff));
      m_currentForeignData.append((unsigned char)((dataOff >> 16) & 0xff));
      m_currentForeignData.append((unsigned char)((dataOff >> 24) & 0xff));
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
    double chord = hypot(y2 - m_y, x2 - m_x);
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
  if (controlPoints.size() <= degree || knotVector.empty() || degree == 0)
    return;

  /* Decomposition of a uniform spline of a given degree into Bezier segments
   * adapted from the algorithm DecomposeCurve (Les Piegl, Wayne Tiller:
   * The NURBS Book, 2nd Edition, 1997
   */

  unsigned a = degree;
  unsigned b = degree + 1;
  unsigned m = (controlPoints.size() - 1) + degree + 1;
  if (m > knotVector.size() - 1)
    m = knotVector.size() - 1;
  std::vector< std::pair<double, double> > points(degree + 1), nextPoints(degree + 1);
  unsigned i = 0;
  for (; i <= degree; i++)
    points[i] = controlPoints[i];
  while (b < m)
  {
    i = b;
    while (b < m && VSD_APPROX_EQUAL(knotVector[b+1], knotVector[i]))
      b++;
    unsigned mult = b - i + 1;
    if (mult > degree) // it doesn't make sense to have knot multiplicity greater than the curve degree
      mult = degree;
    if (mult < degree)
    {
      double numer = knotVector[b] - knotVector[a];
      unsigned j = degree;
      std::vector<double> alphas(degree - 1, 0.0);
      // TODO: The second part of the end condition should not be
      // necessary, but for some reason it can happen. I'm still
      // convinced there is an inherent off-by-one bug in our impl. of
      // the algorithm somewhere...
      for (; j >mult && j < knotVector.size() - a; j--)
        alphas[j-mult-1] = numer/(knotVector[a+j]-knotVector[a]);
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
    }

    std::swap(points, nextPoints);

    if (b < m)
    {
      for (i=degree-mult; i <= degree; i++)
      {
        // FIXME: I've absolutely no idea how this can happen, but it can...
        if (b-degree+i >= controlPoints.size())
          break;
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

  if (!m_noFill)
    m_currentFillGeometry.reserve(VSD_NUM_POLYLINES_PER_KNOT * knotVector.size());
  if (!m_noLine)
    m_currentLineGeometry.reserve(VSD_NUM_POLYLINES_PER_KNOT * knotVector.size());

  for (size_t i = 0; i < VSD_NUM_POLYLINES_PER_KNOT * knotVector.size(); i++)
  {
    librevenge::RVNGPropertyList node;

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

    if (!m_noFill)
      m_currentFillGeometry.push_back(node);
    if (!m_noLine)
      m_currentLineGeometry.push_back(node);
  }
}

bool libvisio::VSDContentCollector::_isUniform(const std::vector<double> &weights) const
{
  if (weights.empty())
    return true;
  double previousValue = weights[0];
  for (double weight : weights)
  {
    if (fabs(weight - previousValue) < LIBVISIO_EPSILON)
      previousValue = weight;
    else
      return false;
  }
  return true;
}

#define MAX_ALLOWED_NURBS_DEGREE 8

void libvisio::VSDContentCollector::collectNURBSTo(unsigned /* id */, unsigned level, double x2, double y2,
                                                   unsigned char xType, unsigned char yType, unsigned degree, const std::vector<std::pair<double, double> > &ctrlPnts,
                                                   const std::vector<double> &kntVec, const std::vector<double> &weights)
{
  _handleLevelChange(level);

  if (kntVec.empty() || ctrlPnts.empty() || weights.empty())
    // Here, maybe we should just draw line to (x2,y2)
    return;

  if (degree > MAX_ALLOWED_NURBS_DEGREE)
    degree = MAX_ALLOWED_NURBS_DEGREE;

  std::vector<std::pair<double, double> > controlPoints(ctrlPnts);

  // Convert control points to static co-ordinates
  for (auto &controlPoint : controlPoints)
  {
    if (xType == 0) // Percentage
      controlPoint.first *= m_xform.width;
    if (yType == 0) // Percentage
      controlPoint.second *= m_xform.height;
  }

  controlPoints.push_back(std::pair<double,double>(x2, y2));
  controlPoints.insert(controlPoints.begin(), std::pair<double, double>(m_originalX, m_originalY));

  std::vector<double> knotVector(kntVec);

  // Ensure knots are sorted in non-decreasing order
  for (size_t i = 1; i < knotVector.size(); ++i)
  {
    if (knotVector[i] < knotVector[i - 1])
      knotVector[i] = knotVector[i - 1];
  }

  // Fill in end knots
  knotVector.reserve(controlPoints.size() + degree + 1);
  while (knotVector.size() < (controlPoints.size() + degree + 1))
    knotVector.push_back(knotVector.back());

  // Let knotVector run from 0 to 1
  double firstKnot = knotVector[0];
  double lastKnot = knotVector.back()-knotVector[0];
  if (VSD_ALMOST_ZERO(lastKnot))
    lastKnot = VSD_EPSILON;
  for (double &knot : knotVector)
  {
    knot -= firstKnot;
    knot /= lastKnot;
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
    auto cstiter = m_stencilShape->m_geometries.find(m_currentGeometryCount-1);
    VSDGeometryListElement *element = nullptr;
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

void libvisio::VSDContentCollector::collectPolylineTo(unsigned /* id */, unsigned level, double x, double y, unsigned char xType, unsigned char yType, const std::vector<std::pair<double, double> > &points)
{
  _handleLevelChange(level);

  librevenge::RVNGPropertyList polyline;
  std::vector<std::pair<double, double> > tmpPoints(points);
  for (size_t i = 0; i< points.size(); i++)
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
    auto cstiter = m_stencilShape->m_geometries.find(m_currentGeometryCount-1);
    VSDGeometryListElement *element = nullptr;
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
  m_txtxform.reset(new XForm(txtxform));
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

  std::set<unsigned> visitedShapes; // avoid mutually nested shapes in broken files
  visitedShapes.insert(shapeId);

  if (txtxform)
    applyXForm(x, y, *txtxform);

  while (true && m_groupXForms)
  {
    auto iterX = m_groupXForms->find(shapeId);
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
      auto iter = m_groupMemberships->find(shapeId);
      if (iter != m_groupMemberships->end() && shapeId != iter->second)
      {
        shapeId = iter->second;
        shapeFound = visitedShapes.insert(shapeId).second;
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
  const double h = hypot(x1-x0, y1-y0);
  angle = h != 0 ? fmod(2.0*M_PI + (y1 > y0 ? 1.0 : -1.0)*acos((x1-x0) / h), 2.0*M_PI) : 0;
}

void libvisio::VSDContentCollector::transformFlips(bool &flipX, bool &flipY)
{
  if (!m_isShapeStarted)
    return;

  if (!m_currentShapeId)
    return;

  unsigned shapeId = m_currentShapeId;

  std::set<unsigned> visitedShapes; // avoid mutually nested shapes in broken files
  visitedShapes.insert(shapeId);

  while (true && m_groupXForms)
  {
    auto iterX = m_groupXForms->find(shapeId);
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
      auto iter = m_groupMemberships->find(shapeId);
      if (iter != m_groupMemberships->end() && shapeId != iter->second)
      {
        shapeId = iter->second;
        shapeFound = visitedShapes.insert(shapeId).second;
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
  m_currentText.clear();
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

    for (const auto &name : m_stencilShape->m_names)
    {
      librevenge::RVNGString nameString;
      _convertDataToString(nameString, name.second.m_data, name.second.m_format);
      m_stencilNames[name.first] = nameString;
    }

    if (m_stencilShape->m_txtxform)
      m_txtxform.reset(new XForm(*(m_stencilShape->m_txtxform)));

    m_stencilFields = m_stencilShape->m_fields;
    for (size_t i = 0; i < m_stencilFields.size(); i++)
    {
      VSDFieldListElement *elem = m_stencilFields.getElement(i);
      if (elem)
        m_fields.push_back(elem->getString(m_stencilNames));
      else
        m_fields.push_back(librevenge::RVNGString());
    }

    if (m_stencilShape->m_lineStyleId != MINUS_ONE)
      m_lineStyle.override(m_styles.getOptionalLineStyle(m_stencilShape->m_lineStyleId), m_documentTheme);

    m_lineStyle.override(m_stencilShape->m_lineStyle, m_documentTheme);

    if (m_stencilShape->m_fillStyleId != MINUS_ONE)
      m_fillStyle.override(m_styles.getOptionalFillStyle(m_stencilShape->m_fillStyleId), m_documentTheme);

    m_fillStyle.override(m_stencilShape->m_fillStyle, m_documentTheme);

    if (m_stencilShape->m_textStyleId != MINUS_ONE)
    {
      m_defaultCharStyle.override(m_styles.getOptionalCharStyle(m_stencilShape->m_textStyleId), m_documentTheme);
      m_defaultParaStyle.override(m_styles.getOptionalParaStyle(m_stencilShape->m_textStyleId), m_documentTheme);
      m_textBlockStyle.override(m_styles.getOptionalTextBlockStyle(m_stencilShape->m_textStyleId), m_documentTheme);
    }

    m_textBlockStyle.override(m_stencilShape->m_textBlockStyle, m_documentTheme);
    m_defaultCharStyle.override(m_stencilShape->m_charStyle, m_documentTheme);
    m_defaultParaStyle.override(m_stencilShape->m_paraStyle, m_documentTheme);
  }

  if (lineStyleId != MINUS_ONE)
    m_lineStyle.override(m_styles.getOptionalLineStyle(lineStyleId), m_documentTheme);

  if (fillStyleId != MINUS_ONE)
    m_fillStyle = m_styles.getFillStyle(fillStyleId, m_documentTheme);

  if (textStyleId != MINUS_ONE)
  {
    m_defaultCharStyle.override(m_styles.getOptionalCharStyle(textStyleId), m_documentTheme);
    m_defaultParaStyle.override(m_styles.getOptionalParaStyle(textStyleId), m_documentTheme);
    m_textBlockStyle.override(m_styles.getOptionalTextBlockStyle(textStyleId), m_documentTheme);
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
  for (size_t i=0; i < m_splineControlPoints.size()+2; i++)
    weights[i] = 1.0;
  collectNURBSTo(0, m_splineLevel, m_splineX, m_splineY, 1, 1, m_splineDegree, m_splineControlPoints, m_splineKnotVector, weights);
  m_splineKnotVector.clear();
  m_splineControlPoints.clear();
}


void libvisio::VSDContentCollector::collectText(unsigned level, const librevenge::RVNGBinaryData &textStream, TextFormat format)
{
  _handleLevelChange(level);

  m_currentText.clear();
  if (!textStream.empty())
    m_currentText = libvisio::VSDName(textStream, format);
}

void libvisio::VSDContentCollector::collectParaIX(unsigned /* id */, unsigned level, unsigned charCount, const boost::optional<double> &indFirst,
                                                  const boost::optional<double> &indLeft, const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                                                  const boost::optional<double> &spBefore, const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                                                  const boost::optional<unsigned char> &bullet, const boost::optional<VSDName> &bulletStr,
                                                  const boost::optional<VSDName> &bulletFont, const boost::optional<double> &bulletFontSize,
                                                  const boost::optional<double> &textPosAfterBullet,  const boost::optional<unsigned> &flags)
{
  _handleLevelChange(level);
  VSDParaStyle format(m_defaultParaStyle);
  format.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align,
                                       bullet, bulletStr, bulletFont, bulletFontSize, textPosAfterBullet, flags), m_documentTheme);
  format.charCount = charCount;
  m_paraFormats.push_back(format);
}

void libvisio::VSDContentCollector::collectDefaultParaStyle(unsigned charCount, const boost::optional<double> &indFirst,
                                                            const boost::optional<double> &indLeft, const boost::optional<double> &indRight,
                                                            const boost::optional<double> &spLine, const boost::optional<double> &spBefore,
                                                            const boost::optional<double> &spAfter, const boost::optional<unsigned char> &align,
                                                            const boost::optional<unsigned char> &bullet, const boost::optional<VSDName> &bulletStr,
                                                            const boost::optional<VSDName> &bulletFont, const boost::optional<double> &bulletFontSize,
                                                            const boost::optional<double> &textPosAfterBullet, const boost::optional<unsigned> &flags)
{
  m_defaultParaStyle.override(VSDOptionalParaStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align,
                                                   bullet, bulletStr, bulletFont, bulletFontSize, textPosAfterBullet, flags), m_documentTheme);
}

void libvisio::VSDContentCollector::collectCharIX(unsigned /* id */, unsigned level, unsigned charCount,
                                                  const boost::optional<VSDName> &font, const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize, const boost::optional<bool> &bold,
                                                  const boost::optional<bool> &italic, const boost::optional<bool> &underline, const boost::optional<bool> &doubleunderline, const boost::optional<bool> &strikeout,
                                                  const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps,
                                                  const boost::optional<bool> &superscript, const boost::optional<bool> &subscript, const boost::optional<double> &scaleWidth)
{
  _handleLevelChange(level);
  VSDCharStyle format(m_defaultCharStyle);
  format.override(VSDOptionalCharStyle(charCount, font, fontColour, fontSize, bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                       allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth), m_documentTheme);
  format.charCount = charCount;
  m_charFormats.push_back(format);
}

void libvisio::VSDContentCollector::collectTabsDataList(unsigned level, const std::map<unsigned, VSDTabSet> &tabSets)
{
  _handleLevelChange(level);

  m_tabSets.clear();
  for (auto iter = tabSets.begin(); iter != tabSets.end(); ++iter)
    if (tabSets.begin() == iter || iter->second.m_numChars)
      m_tabSets.push_back(iter->second);
}

void libvisio::VSDContentCollector::collectDefaultCharStyle(unsigned charCount,
                                                            const boost::optional<VSDName> &font, const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize,
                                                            const boost::optional<bool> &bold, const boost::optional<bool> &italic, const boost::optional<bool> &underline,
                                                            const boost::optional<bool> &doubleunderline, const boost::optional<bool> &strikeout,
                                                            const boost::optional<bool> &doublestrikeout, const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps,
                                                            const boost::optional<bool> &smallcaps, const boost::optional<bool> &superscript, const boost::optional<bool> &subscript,
                                                            const boost::optional<double> &scaleWidth)
{
  m_defaultCharStyle.override(VSDOptionalCharStyle(charCount, font, fontColour, fontSize, bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                                   allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth), m_documentTheme);
}

void libvisio::VSDContentCollector::collectTextBlock(unsigned level, const boost::optional<double> &leftMargin, const boost::optional<double> &rightMargin,
                                                     const boost::optional<double> &topMargin, const boost::optional<double> &bottomMargin, const boost::optional<unsigned char> &verticalAlign,
                                                     const boost::optional<bool> &isBgFilled, const boost::optional<Colour> &bgColour, const boost::optional<double> &defaultTabStop,
                                                     const boost::optional<unsigned char> &textDirection)
{
  _handleLevelChange(level);
  m_textBlockStyle.override(VSDOptionalTextBlockStyle(leftMargin, rightMargin, topMargin, bottomMargin, verticalAlign, isBgFilled, bgColour, defaultTabStop, textDirection), m_documentTheme);
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
  m_currentLayerList.clear();
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
                                                     const boost::optional<unsigned char> &linePattern, const boost::optional<unsigned char> &startMarker,
                                                     const boost::optional<unsigned char> &endMarker, const boost::optional<unsigned char> &lineCap,
                                                     const boost::optional<double> &rounding, const boost::optional<long> &qsLineColour,
                                                     const boost::optional<long> &qsLineMatrix)
{
  VSDOptionalLineStyle lineStyle(strokeWidth, c, linePattern, startMarker, endMarker, lineCap, rounding, qsLineColour, qsLineMatrix);
  m_styles.addLineStyle(m_currentStyleSheet, lineStyle);
}

void libvisio::VSDContentCollector::collectFillStyle(unsigned /* level */, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                                     const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                                                     const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                                                     const boost::optional<Colour> &shfgc, const boost::optional<double> &shadowOffsetX,
                                                     const boost::optional<double> &shadowOffsetY, const boost::optional<long> &qsFillColour,
                                                     const boost::optional<long> &qsShadowColour, const boost::optional<long> &qsFillMatrix)
{
  VSDOptionalFillStyle fillStyle(colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shfgc, shadowPattern,
                                 shadowOffsetX, shadowOffsetY, qsFillColour, qsShadowColour, qsFillMatrix);
  m_styles.addFillStyle(m_currentStyleSheet, fillStyle);

}

void libvisio::VSDContentCollector::collectFillStyle(unsigned level, const boost::optional<Colour> &colourFG, const boost::optional<Colour> &colourBG,
                                                     const boost::optional<unsigned char> &fillPattern, const boost::optional<double> &fillFGTransparency,
                                                     const boost::optional<double> &fillBGTransparency, const boost::optional<unsigned char> &shadowPattern,
                                                     const boost::optional<Colour> &shfgc)
{
  collectFillStyle(level, colourFG, colourBG, fillPattern, fillFGTransparency, fillBGTransparency, shadowPattern, shfgc,
                   m_shadowOffsetX, m_shadowOffsetY, -1, -1, -1);
}

void libvisio::VSDContentCollector::collectParaIXStyle(unsigned /* id */, unsigned /* level */, unsigned charCount,
                                                       const boost::optional<double> &indFirst, const boost::optional<double> &indLeft,
                                                       const boost::optional<double> &indRight, const boost::optional<double> &spLine,
                                                       const boost::optional<double> &spBefore, const boost::optional<double> &spAfter,
                                                       const boost::optional<unsigned char> &align, const boost::optional<unsigned char> &bullet,
                                                       const boost::optional<VSDName> &bulletStr, const boost::optional<VSDName> &bulletFont,
                                                       const boost::optional<double> &bulletFontSize, const boost::optional<double> &textPosAfterBullet,
                                                       const boost::optional<unsigned> &flags)
{
  VSDOptionalParaStyle paraStyle(charCount, indFirst, indLeft, indRight, spLine, spBefore, spAfter, align,
                                 bullet, bulletStr, bulletFont, bulletFontSize, textPosAfterBullet, flags);
  m_styles.addParaStyle(m_currentStyleSheet, paraStyle);
}


void libvisio::VSDContentCollector::collectCharIXStyle(unsigned /* id */, unsigned /* level */, unsigned charCount,
                                                       const boost::optional<VSDName> &font, const boost::optional<Colour> &fontColour, const boost::optional<double> &fontSize,
                                                       const boost::optional<bool> &bold, const boost::optional<bool> &italic, const boost::optional<bool> &underline,
                                                       const boost::optional<bool> &doubleunderline, const boost::optional<bool> &strikeout, const boost::optional<bool> &doublestrikeout,
                                                       const boost::optional<bool> &allcaps, const boost::optional<bool> &initcaps, const boost::optional<bool> &smallcaps,
                                                       const boost::optional<bool> &superscript, const boost::optional<bool> &subscript, const boost::optional<double> &scaleWidth)
{
  VSDOptionalCharStyle charStyle(charCount, font, fontColour, fontSize, bold, italic, underline, doubleunderline, strikeout, doublestrikeout,
                                 allcaps, initcaps, smallcaps, superscript, subscript, scaleWidth);
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
  libvisio::Colour colour = style.colour;
  const Colour *pColour = m_currentLayerList.getColour(m_currentLayerMem);
  if (pColour)
    colour = *pColour;
  styleProps.insert("svg:stroke-color", getColourString(colour));
  if (style.colour.a)
    styleProps.insert("svg:stroke-opacity", (1 - style.colour.a/255.0), librevenge::RVNG_PERCENT);
  else
    styleProps.insert("svg:stroke-opacity", 1.0, librevenge::RVNG_PERCENT);
  switch (style.cap)
  {
  case 0:
    // https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/stroke-linecap
    styleProps.insert("svg:stroke-linecap", "round");
    // https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/stroke-linejoin
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
    if ((style.startMarker == 9) || (style.startMarker == 10) || (style.startMarker == 11) || (style.startMarker == 20) || (style.startMarker == 21))
      styleProps.insert("draw:marker-start-center", "true");
    styleProps.insert("draw:marker-start-path", _linePropertiesMarkerPath(style.startMarker));
    double w =  m_scale*_linePropertiesMarkerScale(style.startMarker)*(0.1/(style.width*style.width+1)+2.54*style.width);
    styleProps.insert("draw:marker-start-width", (std::max)(w, 0.05));
  }
  if (style.endMarker > 0)
  {
    styleProps.insert("draw:marker-end-viewbox", _linePropertiesMarkerViewbox(style.endMarker));
    if ((style.endMarker == 9) || (style.endMarker == 10) || (style.endMarker == 11) || (style.endMarker == 20) || (style.endMarker == 21))
      styleProps.insert("draw:marker-end-center", "true");
    styleProps.insert("draw:marker-end-path", _linePropertiesMarkerPath(style.endMarker));
    double w =  m_scale*_linePropertiesMarkerScale(style.endMarker)*(0.1/(style.width*style.width+1)+2.54*style.width);
    styleProps.insert("draw:marker-end-width", (std::max)(w, 0.05));
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
  else if (style.pattern >= 2 && style.pattern <= 24)
  {
    styleProps.insert("draw:fill", "hatch");
    if (style.bgTransparency == 1)
      styleProps.insert("draw:fill-hatch-solid", "false");
    else
    {
      styleProps.insert("draw:fill-hatch-solid", "true");
      styleProps.insert("draw:opacity", (1 - (std::max)(style.fgTransparency, style.bgTransparency)), librevenge::RVNG_PERCENT);
      styleProps.insert("draw:fill-color", getColourString(style.bgColour));
    }

    styleProps.insert("draw:color", getColourString(style.fgColour));
    if (style.pattern == 2)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:rotation", 45);
      styleProps.insert("draw:distance", 0.1, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 3)
    {
      styleProps.insert("draw:style", "double");
      styleProps.insert("draw:distance", 0.1, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 4)
    {
      styleProps.insert("draw:style", "double");
      styleProps.insert("draw:rotation", 45);
      styleProps.insert("draw:distance", 0.1, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 5)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:rotation", 315);
      styleProps.insert("draw:distance", 0.1, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 6)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:distance", 0.1, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 7)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:rotation", 90);
      styleProps.insert("draw:distance", 0.1, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 8)
    {
      styleProps.insert("draw:style", "triple");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 9)
    {
      styleProps.insert("draw:style", "triple");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 10)
    {
      styleProps.insert("draw:style", "triple");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 11)
    {
      styleProps.insert("draw:style", "triple");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 12)
    {
      styleProps.insert("draw:style", "triple");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 13)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 14)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:rotation", 90);
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 15)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:rotation", 315);
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 16)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:rotation", 45);
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 17)
    {
      styleProps.insert("draw:style", "triple");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 18)
    {
      styleProps.insert("draw:style", "triple");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 19)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 20)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:rotation", 90);
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 21)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:rotation", 315);
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 22)
    {
      styleProps.insert("draw:style", "single");
      styleProps.insert("draw:rotation", 45);
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 23)
    {
      styleProps.insert("draw:style", "double");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
    else if (style.pattern == 24)
    {
      styleProps.insert("draw:style", "triple");
      styleProps.insert("draw:distance", 0.05, librevenge::RVNG_INCH);
    }
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
    // fill types we don't handle right, but let us approximate with solid fill of background colour
  {
    styleProps.insert("draw:fill", "solid");
    styleProps.insert("draw:fill-color", getColourString(style.bgColour));
    if (style.bgTransparency > 0)
      styleProps.insert("draw:opacity", 1 - style.bgTransparency, librevenge::RVNG_PERCENT);
    else
      styleProps.remove("draw:opacity");
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

void libvisio::VSDContentCollector::collectNumericField(unsigned id, unsigned level, unsigned short format, unsigned short cellType, double number, int formatStringId)
{
  _handleLevelChange(level);
  VSDFieldListElement *pElement = m_stencilFields.getElement(m_fields.size());
  if (pElement)
  {
    std::unique_ptr<VSDFieldListElement> element{pElement->clone()};
    if (element)
    {
      element->setValue(number);
      element->setCellType(cellType);
      if (format == VSD_FIELD_FORMAT_Unknown)
      {
        std::map<unsigned, librevenge::RVNGString>::const_iterator iter = m_names.find(formatStringId);
        if (iter != m_names.end())
          parseFormatId(iter->second.cstr(), format);
      }
      if (format != VSD_FIELD_FORMAT_Unknown)
        element->setFormat(format);

      m_fields.push_back(element->getString(m_names));
    }
  }
  else
  {
    VSDNumericField tmpField(id, level, format, cellType, number, formatStringId);
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
          for (const auto &geometry : m_stencilShape->m_geometries)
          {
            m_x = 0.0;
            m_y = 0.0;
            geometry.second.handle(this);
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
    m_txtxform.reset();
    m_xform = XForm();
    m_NURBSData.clear();
    m_polylineData.clear();
  }

  m_currentLevel = level;
}

void libvisio::VSDContentCollector::collectMetaData(const librevenge::RVNGPropertyList &metaData)
{
  m_pages.setMetaData(metaData);
}

void libvisio::VSDContentCollector::startPage(unsigned pageId)
{
  if (m_isShapeStarted)
    _flushShape();
  m_originalX = 0.0;
  m_originalY = 0.0;
  m_txtxform.reset();
  m_xform = XForm();
  m_x = 0;
  m_y = 0;
  m_currentPageNumber++;
  if (m_groupXFormsSequence.size() >= m_currentPageNumber)
    m_groupXForms = m_groupXFormsSequence.size() > m_currentPageNumber-1 ? &m_groupXFormsSequence[m_currentPageNumber-1] : nullptr;
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
    // TODO: this check does not prevent two pages mutually referencing themselves
    // as their background pages. Or even longer cycle of pages.
    if (m_currentPage.m_backgroundPageID == m_currentPage.m_currentPageID)
      m_currentPage.m_backgroundPageID = MINUS_ONE;
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
  using namespace boost::spirit::qi;

  result = 0xffff;

  uint_parser<unsigned short,10,1,5> ushort5;
  auto first = formatString;
  const auto last = first + strlen(formatString);
  if (phrase_parse(first, last,
                   (
                     "{<" >> ushort5 >> ">}"
                     | "esc(" >> ushort5 >> ')'
                   ),
                   space, result))
  {
    return first == last;
  }
  return false;
}

void libvisio::VSDContentCollector::appendCharacters(librevenge::RVNGString &text, const std::vector<unsigned char> &characters, TextFormat format)
{
  if (format == VSD_TEXT_UTF16)
    return appendCharacters(text, characters);
  if (format == VSD_TEXT_UTF8)
  {
    // TODO: revisit for librevenge 0.1
    std::vector<unsigned char> buf;
    buf.reserve(characters.size() + 1);
    buf.assign(characters.begin(), characters.end());
    buf.push_back(0);
    text.append(reinterpret_cast<const char *>(buf.data()));
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
    for (unsigned char character : characters)
    {
      if (0x1e == ucs4Character)
        ucs4Character = 0xfffc;
      else if (character < 0x20)
        ucs4Character = 0x20;
      else
        ucs4Character = symbolmap[character - 0x20];
      appendUCS4(text, ucs4Character);
    }
  }
  else
  {
    UErrorCode status = U_ZERO_ERROR;
    UConverter *conv = nullptr;
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
      const auto *src = (const char *)&characters[0];
      const char *srcLimit = (const char *)src + characters.size();
      while (src < srcLimit)
      {
        ucs4Character = ucnv_getNextUChar(conv, &src, srcLimit, &status);
        if (U_SUCCESS(status) && U_IS_UNICODE_CHAR(ucs4Character))
        {
          if (0x1e == ucs4Character)
            appendUCS4(text, 0xfffc);
          else
            appendUCS4(text, ucs4Character);
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
    const auto *src = (const char *)&characters[0];
    const char *srcLimit = (const char *)src + characters.size();
    while (src < srcLimit)
    {
      UChar32 ucs4Character = ucnv_getNextUChar(conv, &src, srcLimit, &status);
      if (U_SUCCESS(status) && U_IS_UNICODE_CHAR(ucs4Character))
        appendUCS4(text, ucs4Character);
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

void libvisio::VSDContentCollector::collectLayerMem(unsigned level, const VSDName &layerMem)
{
  _handleLevelChange(level);
  m_currentLayerMem.clear();

  if (layerMem.m_data.empty())
    return;

  librevenge::RVNGString text;
  std::vector<unsigned char> tmpData(layerMem.m_data.size());
  memcpy(&tmpData[0], layerMem.m_data.getDataBuffer(), layerMem.m_data.size());
  appendCharacters(text, tmpData, layerMem.m_format);

  using namespace boost::spirit::qi;
  auto first = text.cstr();
  const auto last = first + strlen(first);
  bool bRes = phrase_parse(first, last, int_ % ';', space, m_currentLayerMem) && first == last;

  if (!bRes)
    m_currentLayerMem.clear();
}

void libvisio::VSDContentCollector::collectLayer(unsigned id, unsigned level, const VSDLayer &layer)
{
  _handleLevelChange(level);
  m_currentLayerList.addLayer(id, layer);
}

void libvisio::VSDContentCollector::_appendVisibleAndPrintable(librevenge::RVNGPropertyList &propList)
{
  bool visible = m_currentLayerList.getVisible(m_currentLayerMem);
  bool printable = m_currentLayerList.getPrintable(m_currentLayerMem);

  if (visible && printable)
    return;
  else if (!visible && !printable)
    propList.insert("draw:display", "none");
  else if (!visible && printable)
    propList.insert("draw:display", "printer");
  else if (visible && !printable)
    propList.insert("draw:display", "screen");
}

void libvisio::VSDContentCollector::_bulletFromParaFormat(libvisio::VSDBullet &bullet, const libvisio::VSDParaStyle &paraStyle)
{
  bullet.m_textPosAfterBullet = paraStyle.textPosAfterBullet;
  bullet.m_bulletFontSize = paraStyle.bulletFontSize;
  VSDName name = paraStyle.bulletFont;
  if (name.m_data.empty())
    bullet.m_bulletFont.clear();
  else
    _convertDataToString(bullet.m_bulletFont, name.m_data, name.m_format);
  if (!paraStyle.bullet)
  {
    bullet.m_bulletStr.clear();
    bullet.m_bulletFont.clear();
    bullet.m_bulletFontSize = 0.0;
    bullet.m_textPosAfterBullet = 0.0;
  }
  else
  {
    name = paraStyle.bulletStr;
    if (name.m_data.empty())
      bullet.m_bulletStr.clear();
    else
      _convertDataToString(bullet.m_bulletStr, name.m_data, name.m_format);
    if (bullet.m_bulletStr.empty())
    {
      switch (paraStyle.bullet)
      {
      case 1: // U+2022
        appendUCS4(bullet.m_bulletStr, 0x2022);
        break;
      case 2: // U+25CB
        appendUCS4(bullet.m_bulletStr, 0x25cb);
        break;
      case 3: // U+25A0
        appendUCS4(bullet.m_bulletStr, 0x25a0);
        break;
      case 4: // U+25A1
        appendUCS4(bullet.m_bulletStr, 0x25a1);
        break;
      case 5: // U+2756
        appendUCS4(bullet.m_bulletStr, 0x2756);
        break;
      case 6: // U+27A2
        appendUCS4(bullet.m_bulletStr, 0x27a2);
        break;
      case 7: // U+2714
        appendUCS4(bullet.m_bulletStr, 0x2714);
        break;
      default:
        appendUCS4(bullet.m_bulletStr, 0x2022);
        break;
      }
    }
  }
}

void libvisio::VSDContentCollector::_listLevelFromBullet(librevenge::RVNGPropertyList &propList, const libvisio::VSDBullet &bullet)
{
  if (!bullet)
    return;
  propList.insert("librevenge:level", 1);
  propList.insert("text:bullet-char", bullet.m_bulletStr);
  if (!(bullet.m_bulletFont.empty()))
    propList.insert("fo:font-family", bullet.m_bulletFont);
  if (bullet.m_bulletFontSize > 0.0)
    propList.insert("fo:font-size", bullet.m_bulletFontSize*72.0, librevenge::RVNG_POINT);
  else if (bullet.m_bulletFontSize < 0.0)
    propList.insert("fo:font-size", -bullet.m_bulletFontSize, librevenge::RVNG_PERCENT);
  else
    propList.insert("fo:font-size", 1.0, librevenge::RVNG_PERCENT);
  if (bullet.m_textPosAfterBullet > 0.0)
    propList.insert("text:min-label-width", bullet.m_textPosAfterBullet);
  else
    propList.insert("text:min-label-width", 0.25);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
