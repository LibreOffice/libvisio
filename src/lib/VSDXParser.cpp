/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
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

#include <libwpd-stream/libwpd-stream.h>
#include <locale.h>
#include <sstream>
#include <string>
#include <cmath>
#include <set>
#include "libvisio_utils.h"
#include "VSDXParser.h"
#include "VSDInternalStream.h"
#include "VSDXDocumentStructure.h"
#include "VSDXCollector.h"

#define DUMP_BITMAP 0

#if DUMP_BITMAP
static unsigned bitmapId = 0;
#include <sstream>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

libvisio::VSDXParser::VSDXParser(WPXInputStream *input, libwpg::WPGPaintInterface *painter)
  : m_input(input), m_painter(painter), m_isPageStarted(false), m_pageWidth(0.0), 
    m_pageHeight(0.0), m_scale(1.0), m_x(0.0), m_y(0.0), m_xform(), m_header(),
    m_currentGeometryOrder(), m_currentGeometry(), m_currentComplexGeometry(),
    m_groupXForms(), m_currentForeignData(), m_currentForeignProps(), m_currentShapeId(0),
    m_foreignType(0), m_foreignFormat(0), m_styleProps(), m_lineColour("black"),
    m_fillType("none"), m_linePattern(1), m_fillPattern(1), m_gradientProps(), m_noLine(false),
    m_noFill(false), m_noShow(false)
{}

libvisio::VSDXParser::~VSDXParser()
{}


const ::WPXString libvisio::VSDXParser::getColourString(const struct Colour &c) const
{
    ::WPXString sColour;
    sColour.sprintf("#%.2x%.2x%.2x", c.r, c.g, c.b);
    return sColour;
}

void libvisio::VSDXParser::rotatePoint(double &x, double &y, const XForm &xform)
{
  if (xform.angle == 0.0) return;

  // Calculate co-ordinates using pin position as origin
  double tmpX = x - xform.pinX;
  double tmpY = (m_pageHeight - y) - xform.pinY; // Start from bottom left

  // Rotate around pin and move back to bottom left as origin
  x = (tmpX * cos(xform.angle)) - (tmpY * sin(xform.angle)) + xform.pinX;
  y = (tmpX * sin(xform.angle)) + (tmpY * cos(xform.angle)) + xform.pinY;
  y = m_pageHeight - y; // Flip Y for screen co-ordinate
}

void libvisio::VSDXParser::flipPoint(double &x, double &y, const XForm &xform)
{
  if (!xform.flipX && !xform.flipY) return;

  double tmpX = x - xform.x;
  double tmpY = y - xform.y;

  if (xform.flipX)
    tmpX = xform.width - tmpX;
  if (xform.flipY)
    tmpY = xform.height - tmpY;
  x = tmpX + xform.x;
  y = tmpY + xform.y;
}

void libvisio::VSDXParser::_flushCurrentPath()
{
  double startX = 0; double startY = 0;
  double x = 0; double y = 0;
  bool firstPoint = true;

  WPXPropertyListVector path;
  std::map<unsigned int, WPXPropertyList>::iterator iter;
  std::map<unsigned int, WPXPropertyListVector>::iterator itervec;
  if (m_currentGeometryOrder.size())
  {
    for (unsigned i = 0; i < m_currentGeometryOrder.size(); i++)
    {
      iter = m_currentGeometry.find(m_currentGeometryOrder[i]);
      if (iter != m_currentGeometry.end())
      {
        if (firstPoint)
        {
          x = (iter->second)["svg:x"]->getDouble();
          y = (iter->second)["svg:y"]->getDouble();
          startX = x;
          startY = y;
          firstPoint = false;
        }
        else if ((iter->second)["libwpg:path-action"]->getStr() == "M")
        {
          if (startX == x && startY == y)
          {
             WPXPropertyList closedPath;
             closedPath.insert("libwpg:path-action", "Z");
             path.append(closedPath);
          }
          if (path.count() && !m_noShow)
          {
            m_painter->setStyle(m_styleProps, m_gradientProps);
            m_painter->drawPath(path);
          }

          path = WPXPropertyListVector();
          x = (iter->second)["svg:x"]->getDouble();
          y = (iter->second)["svg:y"]->getDouble();
          startX = x;
          startY = y;
        }
        else
        {
          x = (iter->second)["svg:x"]->getDouble();
          y = (iter->second)["svg:y"]->getDouble();
        }
        path.append(iter->second);
      }
      else
      {
        itervec = m_currentComplexGeometry.find(m_currentGeometryOrder[i]);
        if (itervec != m_currentComplexGeometry.end())
        {
          WPXPropertyListVector::Iter iter2(itervec->second);
          for (; iter2.next();)
          {
            if (firstPoint)
            {
              x = (iter2())["svg:x"]->getDouble();
              y = (iter2())["svg:y"]->getDouble();
              startX = x;
              startY = y;
              firstPoint = false;
            }
            else if ((iter2())["libwpg:path-action"]->getStr() == "M")
            {
              if (startX == x && startY == y)
              {
                WPXPropertyList closedPath;
                closedPath.insert("libwpg:path-action", "Z");
                path.append(closedPath);
              }
              if (path.count() && !m_noShow)
              {
                m_painter->setStyle(m_styleProps, m_gradientProps);
                m_painter->drawPath(path);
              }

              path = WPXPropertyListVector();
              x = (iter2())["svg:x"]->getDouble();
              y = (iter2())["svg:y"]->getDouble();
              startX = x;
              startY = y;
            }
            else
            {
              x = (iter2())["svg:x"]->getDouble();
              y = (iter2())["svg:y"]->getDouble();
            }
          path.append(iter2());
          }
        }
      }
    }

    if (startX == x && startY == y && path.count())
    {
      WPXPropertyList closedPath;
      closedPath.insert("libwpg:path-action", "Z");
      path.append(closedPath);
    }
    if (path.count() && !m_noShow)
    {
      m_painter->setStyle(m_styleProps, m_gradientProps);
      m_painter->drawPath(path);
    }
  }
  else
  {
    for (iter=m_currentGeometry.begin(); iter != m_currentGeometry.end(); iter++)
      path.append(iter->second);
    for (itervec=m_currentComplexGeometry.begin(); itervec != m_currentComplexGeometry.end(); itervec++)
    {
      WPXPropertyListVector::Iter iter2(itervec->second);
      for (; iter2.next();)
        path.append(iter2());
    }
    if (path.count() && !m_noShow)
    {
      m_painter->setStyle(m_styleProps, m_gradientProps);
      m_painter->drawPath(path);
    }
  }
  m_currentGeometry.clear();
  m_currentComplexGeometry.clear();
  m_currentGeometryOrder.clear();
}

void libvisio::VSDXParser::_flushCurrentForeignData()
{
  if (m_currentForeignData.size() && m_currentForeignProps["libwpg:mime-type"] && !m_noShow)
  {
    m_painter->setStyle(m_styleProps, m_gradientProps);
    m_painter->drawGraphicObject(m_currentForeignProps, m_currentForeignData);
  }
  m_currentForeignData.clear();
  m_currentForeignProps.clear();
}



// --- READERS ---

void libvisio::VSDXParser::readEllipticalArcTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x3 = readDouble(input); // End x
  input->seek(1, WPX_SEEK_CUR);
  double y3 = readDouble(input); // End y
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input); // Mid x
  input->seek(1, WPX_SEEK_CUR);
  double y2 = readDouble(input); // Mid y
  input->seek(1, WPX_SEEK_CUR);
  double angle = readDouble(input); // Angle
  input->seek(1, WPX_SEEK_CUR);
  double ecc = readDouble(input); // Eccentricity

#if 0
  m_collector->collectEllipticalArcTo(x3, y3, x2, y2, angle, ecc, m_header.id);
#else
  x3 += m_xform.x;
  y3 = m_xform.height - y3 + m_xform.y;
  x2 += m_xform.x;
  y2 = m_xform.height - y2 + m_xform.y;

  rotatePoint(x2, y2, m_xform);
  rotatePoint(x3, y3, m_xform);
  flipPoint(x2, y2, m_xform);
  flipPoint(x3, y3, m_xform);

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
  arc.insert("libwpg:rotate", -(angle * (180 / M_PI) + m_xform.angle * (180 / M_PI)));
  arc.insert("libwpg:large-arc", largeArc);
  arc.insert("libwpg:sweep", sweep);
  arc.insert("svg:x", m_scale*m_x);
  arc.insert("svg:y", m_scale*m_y);
  arc.insert("libwpg:path-action", "A");
  m_currentGeometry[m_header.id] = arc;
#endif
}


void libvisio::VSDXParser::readForeignData(WPXInputStream *input)
{
  if (m_foreignType == 1 || m_foreignType == 4) // Image
  {
    unsigned long tmpBytesRead = 0;
    const unsigned char *buffer = input->read(m_header.dataLength, tmpBytesRead);
    // If bmp data found, reconstruct header
    if (m_foreignType == 1 && m_foreignFormat == 0)
    {
      m_currentForeignData.append(0x42);
      m_currentForeignData.append(0x4d);

      m_currentForeignData.append((unsigned char)((tmpBytesRead + 14) & 0x000000ff));
      m_currentForeignData.append((unsigned char)(((tmpBytesRead + 14) & 0x0000ff00) >> 8));
      m_currentForeignData.append((unsigned char)(((tmpBytesRead + 14) & 0x00ff0000) >> 16));
      m_currentForeignData.append((unsigned char)(((tmpBytesRead + 14) & 0xff000000) >> 24));

      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);

      m_currentForeignData.append(0x36);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
      m_currentForeignData.append(0x00);
    }
    m_currentForeignData.append(buffer, tmpBytesRead);

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
    m_currentForeignProps.insert("svg:x", m_scale*(m_xform.pinX - m_xform.pinLocX));
    // Y axis starts at the bottom not top
    m_currentForeignProps.insert("svg:y", m_scale*(m_pageHeight -
                        m_xform.pinY + m_xform.pinLocY - m_xform.height));

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

void libvisio::VSDXParser::readEllipse(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double cx = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double cy = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double aa = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  /* double bb = */ readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  /* double cc = */ readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  double dd = readDouble(input);

#if 0
  m_collector->collectEllipse(cx, cy, aa, bb, cc, dd);
#else
  WPXPropertyList ellipse;
  ellipse.insert("svg:rx", m_scale*(aa-cx));
  ellipse.insert("svg:ry", m_scale*(dd-cy));
  ellipse.insert("svg:cx", m_scale*(m_xform.x+cx));
  ellipse.insert("svg:cy", m_scale*(m_xform.y+cy));
  ellipse.insert("libwpg:rotate", m_xform.angle * (180/M_PI));
  if (!m_noShow)
  {
    m_painter->setStyle(m_styleProps, m_gradientProps);
    m_painter->drawEllipse(ellipse);
  }
#endif
}

void libvisio::VSDXParser::readLine(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double strokeWidth = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  Colour c;
  c.r = readU8(input);
  c.g = readU8(input);
  c.b = readU8(input);
  c.a = readU8(input);
  unsigned linePattern = readU8(input);

#if 0
  m_collector->collectLine(strokeWidth, c, linePattern);
#else
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
#endif
}

void libvisio::VSDXParser::readFillAndShadow(WPXInputStream *input)
{
  unsigned int colourIndexFG = readU8(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned int colourIndexBG = readU8(input);
  input->seek(4, WPX_SEEK_CUR);
  unsigned fillPattern = readU8(input);

#if 0
  m_collector->collectFillAndShadow(colourIndexFG, colourIndexBG, fillPattern);
#else
  m_fillPattern = fillPattern;
  if (m_fillPattern == 1)
  {
    m_fillType = "solid";
    m_styleProps.insert("draw:fill", "solid");
    m_styleProps.insert("draw:fill-color", getColourString(m_colours[colourIndexFG]));
  }
  else if (m_fillPattern >= 25 && m_fillPattern <= 34)
  {
    m_fillType = "gradient";
    m_styleProps.insert("draw:fill", "gradient");
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
#endif
}


void libvisio::VSDXParser::readGeomList(WPXInputStream *input)
{
  uint32_t subHeaderLength = readU32(input);
  uint32_t childrenListLength = readU32(input);
  input->seek(subHeaderLength, WPX_SEEK_CUR);
  _flushCurrentPath();
  for (unsigned i = 0; i < (childrenListLength / sizeof(uint32_t)); i++)
    m_currentGeometryOrder.push_back(readU32(input));
  m_noShow = false;
}

void libvisio::VSDXParser::readGeometry(WPXInputStream *input)
{
  m_x = 0.0; m_x = 0.0;
  unsigned int geomFlags = readU8(input);
  m_noFill = ((geomFlags & 1) == 1);
  m_noLine = ((geomFlags & 2) == 2);
  m_noShow = ((geomFlags & 4) == 4);
  if (m_noLine || m_linePattern == 0)
    m_styleProps.insert("svg:stroke-color", "none");
  else
    m_styleProps.insert("svg:stroke-color", m_lineColour);
  if (m_noFill || m_fillPattern == 0)
    m_styleProps.insert("svg:fill", "none");
  else
    m_styleProps.insert("svg:fill", m_fillType);
  VSD_DEBUG_MSG(("Flag: %d NoFill: %d NoLine: %d NoShow: %d\n", geomFlags, m_noFill, m_noLine, m_noShow));
}

void libvisio::VSDXParser::readMoveTo(WPXInputStream *input)
{
  WPXPropertyList end;
  input->seek(1, WPX_SEEK_CUR);
  m_x = readDouble(input) + m_xform.x;
  input->seek(1, WPX_SEEK_CUR);
  m_y = (m_xform.height - readDouble(input)) + m_xform.y;
  rotatePoint(m_x, m_y, m_xform);
  flipPoint(m_x, m_y, m_xform);

  end.insert("svg:x", m_scale*m_x);
  end.insert("svg:y", m_scale*m_y);
  end.insert("libwpg:path-action", "M");
  m_currentGeometry[m_header.id] = end;
}

void libvisio::VSDXParser::readLineTo(WPXInputStream *input)
{
  WPXPropertyList end;
  input->seek(1, WPX_SEEK_CUR);
  m_x = readDouble(input) + m_xform.x;
  input->seek(1, WPX_SEEK_CUR);
  m_y = (m_xform.height - readDouble(input)) + m_xform.y;
  rotatePoint(m_x, m_y, m_xform);
  flipPoint(m_x, m_y, m_xform);

  end.insert("svg:x", m_scale*m_x);
  end.insert("svg:y", m_scale*m_y);
  end.insert("libwpg:path-action", "L");
  m_currentGeometry[m_header.id] = end;
}

void libvisio::VSDXParser::readArcTo(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  double x2 = readDouble(input) + m_xform.x;
  input->seek(1, WPX_SEEK_CUR);
  double y2 = (m_xform.height - readDouble(input)) + m_xform.y;
  input->seek(1, WPX_SEEK_CUR);
  double bow = readDouble(input);

  rotatePoint(x2, y2, m_xform);
  flipPoint(x2, y2, m_xform);

  if (bow == 0)
  {
    m_x = x2; m_y = y2;
    WPXPropertyList end;
    end.insert("svg:x", m_scale*m_x);
    end.insert("svg:y", m_scale*m_y);
    end.insert("libwpg:path-action", "L");
    m_currentGeometry[m_header.id] = end;
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
    arc.insert("libwpg:rotate", m_xform.angle * (180/M_PI));
    arc.insert("libwpg:large-arc", largeArc);
    arc.insert("libwpg:sweep", sweep);
    arc.insert("svg:x", m_scale*m_x);
    arc.insert("svg:y", m_scale*m_y);
    arc.insert("libwpg:path-action", "A");
    m_currentGeometry[m_header.id] = arc;
  }
}

void libvisio::VSDXParser::readXFormData(WPXInputStream *input)
{
  input->seek(1, WPX_SEEK_CUR);
  m_xform.pinX = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  m_xform.pinY = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  m_xform.width = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  m_xform.height = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  m_xform.pinLocX = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  m_xform.pinLocY = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  m_xform.angle = readDouble(input);
  m_xform.flipX = (readU8(input) != 0);
  m_xform.flipY = (readU8(input) != 0);

  std::map<unsigned int, XForm>::iterator iter = m_groupXForms.find(m_currentShapeId);
  if  (iter != m_groupXForms.end()) {
    m_xform.pinX += iter->second.pinX;
    m_xform.pinY += iter->second.pinY;
    m_xform.pinLocX += iter->second.pinLocX;
    m_xform.pinLocY += iter->second.pinLocY;
  }
  m_xform.x = m_xform.pinX - m_xform.pinLocX;
  m_xform.y = m_pageHeight - m_xform.pinY + m_xform.pinLocY - m_xform.height;
}

void libvisio::VSDXParser::readShapeID(WPXInputStream *input)
{
  m_groupXForms[readU32(input)] = m_xform;
}

void libvisio::VSDXParser::readForeignDataType(WPXInputStream *input)
{
  input->seek(0x24, WPX_SEEK_CUR);
  m_foreignType = readU16(input);
  input->seek(0xb, WPX_SEEK_CUR);
  m_foreignFormat = readU32(input);

  VSD_DEBUG_MSG(("Found foreign data, type %d format %d\n", m_foreignType, m_foreignFormat));
}

void libvisio::VSDXParser::readPageProps(WPXInputStream *input)
{
  // Skip bytes representing unit to *display* (value is always inches)
  input->seek(1, WPX_SEEK_CUR);
  m_pageWidth = readDouble(input);
  input->seek(1, WPX_SEEK_CUR);
  m_pageHeight = readDouble(input);
  input->seek(19, WPX_SEEK_CUR);
  /* m_scale = */ readDouble(input);

  WPXPropertyList pageProps;
  pageProps.insert("svg:width", m_scale*m_pageWidth);
  pageProps.insert("svg:height", m_scale*m_pageHeight);

  if (m_isPageStarted)
    m_painter->endGraphics();
  m_painter->startGraphics(pageProps);
  m_isPageStarted = true;
}

void libvisio::VSDXParser::shapeChunk(WPXInputStream *input)
{
  long endPos = 0;

  m_gradientProps = WPXPropertyListVector();
  m_foreignType = 0; // Tracks current foreign data type
  m_foreignFormat = 0; // Tracks foreign data format

  _flushCurrentPath();
  _flushCurrentForeignData();
  m_x = 0; m_y = 0;
  m_xform = XForm();
  m_header = ChunkHeader();

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

  while (!input->atEOS())
  {
    if (!getChunkHeader(input))
      break;
    endPos = m_header.dataLength+m_header.trailer+input->tell();

    // Break once a chunk that is not nested in the shape is found
    if (m_header.level < 2)
    {
      input->seek(-19, WPX_SEEK_CUR);
      break;
    }

    VSD_DEBUG_MSG(("Shape: parsing chunk type %x\n", m_header.chunkType));
    switch (m_header.chunkType)
    {
    case VSD_XFORM_DATA: // XForm data
      readXFormData(input);
      break;
    case VSD_SHAPE_ID:
      readShapeID(input);
      break;
    case VSD_LINE:
      readLine(input);
      break;
    case VSD_FILL_AND_SHADOW:
      readFillAndShadow(input);
      break;
    case VSD_GEOM_LIST:
      readGeomList(input);
      break;
    case VSD_GEOMETRY:
      readGeometry(input);
      break;
    case VSD_MOVE_TO:
      readMoveTo(input);
      break;
    case VSD_LINE_TO:
      readLineTo(input);
      break;
    case VSD_ARC_TO:
      readArcTo(input);
      break;
    case VSD_ELLIPSE:
      readEllipse(input);
      break;
    case VSD_ELLIPTICAL_ARC_TO:
      readEllipticalArcTo(input);
      break;
    case VSD_FOREIGN_DATA_TYPE:
      readForeignDataType(input);
      break;
    case VSD_FOREIGN_DATA:
      readForeignData(input);
      break;
    }

    input->seek(endPos, WPX_SEEK_SET);
  }
  _flushCurrentPath();
  _flushCurrentForeignData();
  m_x = 0; m_y = 0;
}

