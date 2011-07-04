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

#include "VSDXContentCollector.h"
#include "VSDXParser.h"

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
  std::vector<std::list<unsigned> > &documentPageShapeOrders
  ) :
    m_painter(painter), m_isPageStarted(false), m_pageWidth(0.0), m_pageHeight(0.0),
    m_scale(1.0), m_x(0.0), m_y(0.0), m_xform(), m_currentGeometry(),
    m_groupXForms(groupXFormsSequence[0]), m_currentForeignData(), m_currentForeignProps(),
    m_currentShapeId(0), m_foreignType(0), m_foreignFormat(0), m_styleProps(),
    m_lineColour("black"), m_fillType("none"), m_linePattern(1), m_fillPattern(1),
    m_gradientProps(), m_noLine(false), m_noFill(false), m_noShow(false), m_currentLevel(0),
    m_isShapeStarted(false), m_groupMemberships(groupMembershipsSequence[0]),
    m_groupXFormsSequence(groupXFormsSequence),
    m_groupMembershipsSequence(groupMembershipsSequence), m_currentPageNumber(0),
    m_shapeList(), m_shapeOutput(0), m_documentPageShapeOrders(documentPageShapeOrders),
    m_pageShapeOrder(documentPageShapeOrders[0]), m_isFirstGeometry(true)
{
}

const ::WPXString libvisio::VSDXContentCollector::getColourString(const struct Colour &c) const
{
  ::WPXString sColour;
  sColour.sprintf("#%.2x%.2x%.2x", c.r, c.g, c.b);
  return sColour;
}

void libvisio::VSDXContentCollector::_flushCurrentPath()
{
  WPXPropertyListVector path;
  double startX = 0; double startY = 0;
  double x = 0; double y = 0;
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
      if (startX == x && startY == y)
      {
         WPXPropertyList closedPath;
         closedPath.insert("libwpg:path-action", "Z");
         path.append(closedPath);
      }
#if 0
      if (path.count() && !m_noShow)
      {
         m_shapeOutput->addStyle(m_styleProps, m_gradientProps);
         m_shapeOutput->addPath(path);
      }

      path = WPXPropertyListVector();
#endif
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
  if (startX == x && startY == y && path.count())
  {
     WPXPropertyList closedPath;
     closedPath.insert("libwpg:path-action", "Z");
     path.append(closedPath);
  }
  if (path.count() && !m_noShow)
  {
    m_shapeOutput->addStyle(m_styleProps, m_gradientProps);
    m_shapeOutput->addPath(path);
  }
  m_currentGeometry.clear();
}

void libvisio::VSDXContentCollector::_flushCurrentForeignData()
{
  if (m_currentForeignData.size() && m_currentForeignProps["libwpg:mime-type"] && !m_noShow)
  {
    m_shapeOutput->addStyle(m_styleProps, m_gradientProps);
    m_shapeOutput->addGraphicObject(m_currentForeignProps, m_currentForeignData);
  }
  m_currentForeignData.clear();
  m_currentForeignProps.clear();
}

void libvisio::VSDXContentCollector::_flushCurrentPage()
{
  std::map<unsigned, VSDXOutputElementList>::iterator iter;
  if (m_pageShapeOrder.size())
  {
    for (std::list<unsigned>::iterator iterList = m_pageShapeOrder.begin(); iterList != m_pageShapeOrder.end(); iterList++)
    {
      iter = m_pageOutput.find(*iterList);
      if (iter != m_pageOutput.end())
        iter->second.draw(m_painter);
    }
  }
  m_pageOutput.clear();
}

void libvisio::VSDXContentCollector::collectEllipticalArcTo(unsigned /* id */, unsigned level, double x3, double y3, double x2, double y2, double angle, double ecc)
{
  _handleLevelChange(level);

  transformPoint(x2, y2);
  transformPoint(x3, y3);
  transformAngle(angle);

  double x1 = m_x;
  double y1 = m_y;
  double x0 = ((x1-x2)*(x1+x2)*(y2-y3) - (x2-x3)*(x2+x3)*(y1-y2) +
               ecc*ecc*(y1-y2)*(y2-y3)*(y1-y3)) /
               (2*((x1-x2)*(y2-y3) - (x2-x3)*(y1-y2)));
  double y0 = ((x1-x2)*(x2-x3)*(x1-x3) + ecc*ecc*(x2-x3)*(y1-y2)*(y1+y2) -
               ecc*ecc*(x1-x2)*(y2-y3)*(y2+y3)) /
               (2*ecc*ecc*((x2-x3)*(y1-y2) - (x1-x2)*(y2-y3)));
  VSD_DEBUG_MSG(("Centre: (%f,%f), angle %f\n", x0, y0, angle));
  double rx = sqrt(pow(x1-x0, 2) + ecc*ecc*pow(y1-y0, 2));
  double ry = rx / ecc;

  m_x = x3; m_y = y3;
  WPXPropertyList arc;
  int largeArc = 0;
  int sweep = 1;

  // Calculate side of chord that ellipse centre and control point fall on
  double centreSide = (x3-x1)*(y0-y1) - (y3-y1)*(x0-x1);
  double midSide = (x3-x1)*(y2-y1) - (y3-y1)*(x2-x1);
  // Large arc if centre and control point are on the same side
  if ((centreSide > 0 && midSide > 0) || (centreSide < 0 && midSide < 0))
    largeArc = 1;
  // Change direction depending of side of control point
  if (midSide > 0)
    sweep = 0;

  arc.insert("svg:rx", m_scale*rx);
  arc.insert("svg:ry", m_scale*ry);
  arc.insert("libwpg:rotate", angle * 180 / M_PI);
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
  transformPoint(cx, cy);
  transformPoint(xleft, yleft);
  transformPoint(xtop, ytop);
  double rx = sqrt((xleft - cx)*(xleft - cx) + (yleft - cy)*(yleft - cy));
  double ry = sqrt((xtop - cx)*(xtop - cx) + (ytop - cy)*(ytop - cy));
  ellipse.insert("svg:rx", m_scale*rx);
  ellipse.insert("svg:ry", m_scale*ry);
  ellipse.insert("svg:cx", m_scale*cx);
  ellipse.insert("svg:cy", m_scale*cy);
  double angle = 0.0;
  transformAngle(angle);
  ellipse.insert("libwpg:rotate", angle * 180/M_PI);
  if (!m_noShow)
  {
    // Here we want to maintain drawing order even though we might lose some evenodd goodness
    _flushCurrentPath();
    m_shapeOutput->addStyle(m_styleProps, m_gradientProps);
    m_shapeOutput->addEllipse(ellipse);
  }
}

void libvisio::VSDXContentCollector::collectLine(unsigned /* id */, unsigned level, double strokeWidth, Colour c, unsigned linePattern)
{
  _handleLevelChange(level);
  m_linePattern = linePattern;
  m_styleProps.insert("svg:stroke-width", m_scale*strokeWidth);
  m_lineColour = getColourString(c);
  m_styleProps.insert("svg:stroke-color", m_lineColour);
  const char* patterns[] = {
    /*  0 */  "none",
    /*  1 */  "solid",
    /*  2 */  "6, 3",
    /*  3 */  "1, 3",
    /*  4 */  "6, 3, 1, 3",
    /*  5 */  "6, 3, 1, 3, 1, 3",
    /*  6 */  "6, 3, 6, 3, 1, 3",
    /*  7 */  "14, 2, 6, 2",
    /*  8 */  "14, 2, 6, 2, 6, 2",
    /*  9 */  "3, 1",
    /* 10 */  "1, 1",
    /* 11 */  "3, 1, 1, 1",
    /* 12 */  "3, 1, 1, 1, 1, 1",
    /* 13 */  "3, 1, 3, 1, 1, 1",
    /* 14 */  "7, 1, 3, 1",
    /* 15 */  "7, 1, 3, 1, 3, 1",
    /* 16 */  "11, 5",
    /* 17 */  "1, 5",
    /* 18 */  "11, 5, 1, 5",
    /* 19 */  "11, 5, 1, 5, 1, 5",
    /* 20 */  "11, 5, 11, 5, 1, 5",
    /* 21 */  "27, 5, 11, 5",
    /* 22 */  "27, 5, 11, 5, 11, 5",
    /* 23 */  "2, 1"
  };
  if (m_linePattern > 0 && m_linePattern < sizeof(patterns)/sizeof(patterns[0]))
    m_styleProps.insert("svg:stroke-dasharray", patterns[m_linePattern]);
  // FIXME: later it will require special treatment for custom line patterns
  // patt ID is 0xfe, link to stencil name is in 'Line' blocks
}

void libvisio::VSDXContentCollector::collectFillAndShadow(unsigned /* id */, unsigned level, unsigned colourIndexFG, unsigned colourIndexBG, unsigned fillPattern)
{
  _handleLevelChange(level);
  m_fillPattern = fillPattern;
  if (m_fillPattern == 0)
  {
    m_fillType = "none";
    m_styleProps.insert("draw:fill", "none");
  }
  else if (m_fillPattern == 1)
  {
    m_fillType = "solid";
    m_styleProps.insert("draw:fill", "solid");
    m_styleProps.insert("draw:fill-color", getColourString(m_colours[colourIndexFG]));
  }
  else if (m_fillPattern >= 25 && m_fillPattern <= 34)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:fill", "gradient");
    m_styleProps.insert("draw:style", "linear");
    WPXPropertyList startColour;
    startColour.insert("svg:stop-color",
                       getColourString(m_colours[colourIndexFG]));
    startColour.insert("svg:offset", 0, WPX_PERCENT);
    startColour.insert("svg:stop-opacity", 1, WPX_PERCENT);
    WPXPropertyList endColour;
    endColour.insert("svg:stop-color",
                     getColourString(m_colours[colourIndexBG]));
    endColour.insert("svg:offset", 1, WPX_PERCENT);
    endColour.insert("svg:stop-opacity", 1, WPX_PERCENT);

    switch(m_fillPattern)
    {
    case 25:
      m_styleProps.insert("draw:angle", -90);
      break;
    case 26:
      m_styleProps.insert("draw:angle", -90);
      endColour.insert("svg:offset", 0, WPX_PERCENT);
      m_gradientProps.append(endColour);
      endColour.insert("svg:offset", 1, WPX_PERCENT);
      startColour.insert("svg:offset", 0.5, WPX_PERCENT);
      break;
    case 27:
      m_styleProps.insert("draw:angle", 90);
      break;
    case 28:
      m_styleProps.insert("draw:angle", 0);
      break;
    case 29:
      m_styleProps.insert("draw:angle", 0);
      endColour.insert("svg:offset", 0, WPX_PERCENT);
      m_gradientProps.append(endColour);
      endColour.insert("svg:offset", 1, WPX_PERCENT);
      startColour.insert("svg:offset", 0.5, WPX_PERCENT);
      break;
    case 30:
      m_styleProps.insert("draw:angle", 180);
      break;
    case 31:
      m_styleProps.insert("draw:angle", -45);
      break;
    case 32:
      m_styleProps.insert("draw:angle", 45);
      break;
    case 33:
      m_styleProps.insert("draw:angle", 225);
      break;
    case 34:
      m_styleProps.insert("draw:angle", 135);
      break;
    }
    m_gradientProps.append(startColour);
    m_gradientProps.append(endColour);
  }
  else if (m_fillPattern >= 35 && m_fillPattern <= 40)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:fill", "gradient");
    m_styleProps.insert("draw:style", "radial");
    m_styleProps.insert("svg:r", 1, WPX_PERCENT);
    WPXPropertyList startColour;
    startColour.insert("svg:stop-color",
                       getColourString(m_colours[colourIndexFG]));
    startColour.insert("svg:offset", 0, WPX_PERCENT);
    startColour.insert("svg:stop-opacity", 1, WPX_PERCENT);
    WPXPropertyList endColour;
    endColour.insert("svg:stop-color",
                     getColourString(m_colours[colourIndexBG]));
    endColour.insert("svg:offset", 1, WPX_PERCENT);
    endColour.insert("svg:stop-opacity", 1, WPX_PERCENT);
    m_gradientProps.append(startColour);
    m_gradientProps.append(endColour);

    switch(m_fillPattern)
    {
    case 35:
    case 40:
      m_styleProps.insert("svg:cx", 0.5, WPX_PERCENT);
      m_styleProps.insert("svg:cy", 0.5, WPX_PERCENT);
      m_styleProps.insert("svg:r", 0.5, WPX_PERCENT);
      break;
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
    }
  }
  else
  // fill types we don't handle right, but let us approximate with solid fill
  {
    m_fillType = "solid";
    m_styleProps.insert("draw:fill", "solid");
    m_styleProps.insert("draw:fill-color", getColourString(m_colours[colourIndexBG]));
  }
}


void libvisio::VSDXContentCollector::collectForeignData(unsigned /* id */, unsigned level, const WPXBinaryData &binaryData)
{
  _handleLevelChange(level);
  if (m_foreignType == 1 || m_foreignType == 4) // Image
  {
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
        filename.sprintf("binarydump%i.bmp", bitmapId++); break;
      case 1:
        filename.sprintf("binarydump%i.jpeg", bitmapId++); break;
      case 2:
        filename.sprintf("binarydump%i.gif", bitmapId++); break;
      case 3:
        filename.sprintf("binarydump%i.tiff", bitmapId++); break;
      case 4:
        filename.sprintf("binarydump%i.png", bitmapId++); break;
      default:
        filename.sprintf("binarydump%i.bin", bitmapId++); break;
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

    m_currentForeignProps.insert("svg:width", m_scale*m_xform.width);
    m_currentForeignProps.insert("svg:height", m_scale*m_xform.height);
	double x = 0.0; double y = 0.0;
	transformPoint(x,y);
    m_currentForeignProps.insert("svg:x", m_scale*x);
    // Y axis starts at the bottom not top
    m_currentForeignProps.insert("svg:y", m_scale*(y - m_xform.height));

    if (m_foreignType == 1)
    {
      switch(m_foreignFormat)
      {
      case 0:
        m_currentForeignProps.insert("libwpg:mime-type", "image/bmp"); break;
      case 1:
        m_currentForeignProps.insert("libwpg:mime-type", "image/jpeg"); break;
      case 2:
        m_currentForeignProps.insert("libwpg:mime-type", "image/gif"); break;
      case 3:
        m_currentForeignProps.insert("libwpg:mime-type", "image/tiff"); break;
      case 4:
        m_currentForeignProps.insert("libwpg:mime-type", "image/png"); break;
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

void libvisio::VSDXContentCollector::collectGeomList(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::collectGeometry(unsigned /* id */, unsigned level, unsigned geomFlags)
{
  _handleLevelChange(level);
  m_x = 0.0; m_x = 0.0;
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
    m_styleProps.insert("svg:stroke-color", "none");
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
}

void libvisio::VSDXContentCollector::collectMoveTo(unsigned /* id */, unsigned level, double x, double y)
{
  _handleLevelChange(level);
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
  transformPoint(x2, y2);
  double angle = 0.0;
  transformAngle(angle);

  if (bow == 0)
  {
    m_x = x2; m_y = y2;
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
    VSD_DEBUG_MSG(("ArcTo with bow %f radius %f and chord %f\n", bow, radius, chord));
    int largeArc = fabs(bow) > radius ? 1 : 0;
    int sweep = bow < 0 ? 1 : 0;
    m_x = x2; m_y = y2;
    arc.insert("svg:rx", m_scale*radius);
    arc.insert("svg:ry", m_scale*radius);
    arc.insert("libwpg:rotate", angle*180/M_PI);
    arc.insert("libwpg:large-arc", largeArc);
    arc.insert("libwpg:sweep", sweep);
    arc.insert("svg:x", m_scale*m_x);
    arc.insert("svg:y", m_scale*m_y);
    arc.insert("libwpg:path-action", "A");
    m_currentGeometry.push_back(arc);
  }
}

void libvisio::VSDXContentCollector::collectXFormData(unsigned /* id */, unsigned level, const XForm &xform)
{
  _handleLevelChange(level);
  m_xform = xform;
}

void libvisio::VSDXContentCollector::transformPoint(double &x, double &y)
{
  // We are interested for the while in shapes xforms only
  if (!m_isShapeStarted)
    return;

  if (!m_currentShapeId)
    return;

  unsigned shapeId = m_currentShapeId;

  while (true)
  {
    std::map<unsigned, XForm>::iterator iterX = m_groupXForms.find(shapeId);
    if (iterX != m_groupXForms.end())
    {
      XForm xform = iterX->second;
      x -= xform.pinLocX;
      y -= xform.pinLocY;
      if (xform.flipX)
        x = xform.width - x - 2.0*xform.pinLocX;
      if (xform.flipY)
        y = xform.height -y - 2.0*xform.pinLocY;
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

void libvisio::VSDXContentCollector::transformAngle(double &angle)
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
  transformPoint(x0, y0);
  transformPoint(x1, y1);
  angle = fmod(2.0*M_PI + (y1 > y0 ? 1.0 : -1.0)*acos((x1-x0) / sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0))), 2.0*M_PI);
}

void libvisio::VSDXContentCollector::collectShapeId(unsigned /* id */, unsigned level, unsigned /* shapeId */)
{
  _handleLevelChange(level);
}


void libvisio::VSDXContentCollector::collectShapeList(unsigned /* id */, unsigned level)
{
  _handleLevelChange(level);
}

void libvisio::VSDXContentCollector::collectForeignDataType(unsigned /* id */, unsigned level, unsigned foreignType, unsigned foreignFormat)
{
  _handleLevelChange(level);
  m_foreignType = foreignType;
  m_foreignFormat = foreignFormat;
}

void libvisio::VSDXContentCollector::collectPageProps(unsigned /* id */, unsigned level, double pageWidth, double pageHeight)
{
  _handleLevelChange(level);
  m_pageWidth = pageWidth;
  m_pageHeight = pageHeight;
  WPXPropertyList pageProps;
  pageProps.insert("svg:width", m_scale*m_pageWidth);
  pageProps.insert("svg:height", m_scale*m_pageHeight);

  if (m_isPageStarted)
    m_painter->endGraphics();
  m_painter->startGraphics(pageProps);
  m_isPageStarted = true;
}

void libvisio::VSDXContentCollector::collectShape(unsigned id, unsigned level)
{
  _handleLevelChange(level);

  m_gradientProps = WPXPropertyListVector();
  m_foreignType = 0; // Tracks current foreign data type
  m_foreignFormat = 0; // Tracks foreign data format

  m_x = 0; m_y = 0;

  // Geometry flags
  m_noLine = false;
  m_noFill = false;
  m_noShow = false;

  // Save line colour and pattern, fill type and pattern
  m_lineColour = "black";
  m_fillType = "none";
  m_linePattern = 1; // same as "solid"
  m_fillPattern = 1; // same as "solid"

  // Reset style
  m_styleProps.clear();
  m_styleProps.insert("svg:stroke-width", m_scale*0.0138889);
  m_styleProps.insert("svg:stroke-color", m_lineColour);
  m_styleProps.insert("draw:fill", m_fillType);
  m_styleProps.insert("svg:stroke-dasharray", "solid");

  m_currentShapeId = id;
  m_pageOutput[m_currentShapeId] = VSDXOutputElementList();
  m_shapeOutput = &m_pageOutput[m_currentShapeId];
  m_isShapeStarted = true;
  m_isFirstGeometry = true;
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

void libvisio::VSDXContentCollector::_handleLevelChange(unsigned level)
{
  if (m_currentLevel == level)
    return;
  if (level < 2)
  {
    if (m_isShapeStarted)
    {
      _flushCurrentPath();
      _flushCurrentForeignData();
      m_isShapeStarted = false;
    }
    m_x = 0; m_y = 0;
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
  m_x = 0; m_y = 0;
  m_currentPageNumber++;
  if (m_groupXFormsSequence.size() >= m_currentPageNumber)
    m_groupXForms = m_groupXFormsSequence[m_currentPageNumber-1];
  if (m_groupMembershipsSequence.size() >= m_currentPageNumber)
    m_groupMemberships = m_groupMembershipsSequence[m_currentPageNumber-1];
  if (m_documentPageShapeOrders.size() >= m_currentPageNumber)
    m_pageShapeOrder = m_documentPageShapeOrders[m_currentPageNumber-1];
}

void libvisio::VSDXContentCollector::endPage()
{
  // End page if one is started
  if (m_isPageStarted)
  {
    _handleLevelChange(0);
    _flushCurrentPage();

    m_painter->endGraphics();
    m_isPageStarted = false;
  }
}
