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

#include "VSDSVGGenerator.h"
#include <locale.h>
#include <sstream>
#include <string>

static std::string doubleToString(const double value)
{
  std::ostringstream tempStream;
  tempStream << value;
#ifndef __ANDROID__
  std::string decimalPoint(localeconv()->decimal_point);
#else
  std::string decimalPoint(".");
#endif
  if ((decimalPoint.size() == 0) || (decimalPoint == "."))
    return tempStream.str();
  std::string stringValue(tempStream.str());
  if (!stringValue.empty())
  {
    std::string::size_type pos;
    while ((pos = stringValue.find(decimalPoint)) != std::string::npos)
      stringValue.replace(pos,decimalPoint.size(),".");
  }
  return stringValue;
}

static unsigned stringToColour(const ::WPXString &s)
{
  std::string str(s.cstr());
  if (str[0] == '#')
  {
    if (str.length() != 7)
      return 0;
    else
      str.erase(str.begin());
  }
  else
    return 0;

  std::istringstream istr(str);
  unsigned val = 0;
  istr >> std::hex >> val;
  return val;
}

libvisio::VSDSVGGenerator::VSDSVGGenerator(libvisio::VSDStringVector &vec): m_gradient(), m_style(), m_gradientIndex(1), m_shadowIndex(1), m_outputSink(), m_vec(vec)
{
}

libvisio::VSDSVGGenerator::~VSDSVGGenerator()
{
}

void libvisio::VSDSVGGenerator::startGraphics(const WPXPropertyList &propList)
{
  m_outputSink << "<svg:svg version=\"1.1\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" ";
  if (propList["svg:width"])
    m_outputSink << "width=\"" << doubleToString(72*(propList["svg:width"]->getDouble())) << "\" ";
  if (propList["svg:height"])
    m_outputSink << "height=\"" << doubleToString(72*(propList["svg:height"]->getDouble())) << "\"";
  m_outputSink << " >\n";
}

void libvisio::VSDSVGGenerator::endGraphics()
{
  m_outputSink << "</svg:svg>\n";
  m_vec.append(m_outputSink.str().c_str());
  m_outputSink.clear();
}

void libvisio::VSDSVGGenerator::setStyle(const ::WPXPropertyList &propList, const ::WPXPropertyListVector &gradient)
{
  m_style.clear();
  m_style = propList;

  m_gradient = gradient;
  if(m_style["draw:shadow"] && m_style["draw:shadow"]->getStr() == "visible")
  {
    double shadowRed = 0.0;
    double shadowGreen = 0.0;
    double shadowBlue = 0.0;
    if (m_style["draw:shadow-color"])
    {
      unsigned shadowColour = stringToColour(m_style["draw:shadow-color"]->getStr());
      shadowRed = (double)((shadowColour & 0x00ff0000) >> 16)/255.0;
      shadowGreen = (double)((shadowColour & 0x0000ff00) >> 8)/255.0;
      shadowBlue = (double)(shadowColour & 0x000000ff)/255.0;
    }
    m_outputSink << "<svg:defs>\n";
    m_outputSink << "<svg:filter filterUnits=\"userSpaceOnUse\" id=\"shadow" << m_shadowIndex++ << "\">";
    m_outputSink << "<svg:feOffset in=\"SourceGraphic\" result=\"offset\" ";
    m_outputSink << "dx=\"" << doubleToString(72*m_style["draw:shadow-offset-x"]->getDouble()) << "\" ";
    m_outputSink << "dy=\"" << doubleToString(72*m_style["draw:shadow-offset-y"]->getDouble()) << "\"/>";
    m_outputSink << "<svg:feColorMatrix in=\"offset\" result=\"offset-color\" type=\"matrix\" values=\"";
    m_outputSink << "0 0 0 0 " << doubleToString(shadowRed) ;
    m_outputSink << " 0 0 0 0 " << doubleToString(shadowGreen);
    m_outputSink << " 0 0 0 0 " << doubleToString(shadowBlue);
    if(m_style["draw:opacity"] && m_style["draw:opacity"]->getDouble() < 1)
      m_outputSink << " 0 0 0 "   << doubleToString(m_style["draw:shadow-opacity"]->getDouble()/m_style["draw:opacity"]->getDouble()) << " 0\"/>";
    else
      m_outputSink << " 0 0 0 "   << doubleToString(m_style["draw:shadow-opacity"]->getDouble()) << " 0\"/>";
    m_outputSink << "<svg:feMerge><svg:feMergeNode in=\"offset-color\" /><svg:feMergeNode in=\"SourceGraphic\" /></svg:feMerge></svg:filter></svg:defs>";
  }

  if(m_style["draw:fill"] && m_style["draw:fill"]->getStr() == "gradient")
  {
    double angle = (m_style["draw:angle"] ? m_style["draw:angle"]->getDouble() : 0.0);
    angle *= -1.0;
    while(angle < 0)
      angle += 360;
    while(angle > 360)
      angle -= 360;

    if (!m_gradient.count())
    {
      if (m_style["draw:style"] &&
          (m_style["draw:style"]->getStr() == "radial" ||
           m_style["draw:style"]->getStr() == "rectangular" ||
           m_style["draw:style"]->getStr() == "square" ||
           m_style["draw:style"]->getStr() == "ellipsoid"))
      {
        m_outputSink << "<svg:defs>\n";
        m_outputSink << "  <svg:radialGradient id=\"grad" << m_gradientIndex++ << "\"";

        if (m_style["svg:cx"])
          m_outputSink << " cx=\"" << m_style["svg:cx"]->getStr().cstr() << "\"";
        else if (m_style["draw:cx"])
          m_outputSink << " cx=\"" << m_style["draw:cx"]->getStr().cstr() << "\"";

        if (m_style["svg:cy"])
          m_outputSink << " cy=\"" << m_style["svg:cy"]->getStr().cstr() << "\"";
        else if (m_style["draw:cy"])
          m_outputSink << " cy=\"" << m_style["draw:cy"]->getStr().cstr() << "\"";
        m_outputSink << " r=\"" << (1 - (m_style["draw:border"] ? m_style["draw:border"]->getDouble() : 0))*100.0 << "%\" >\n";
        m_outputSink << " >\n";

        if (m_style["draw:start-color"] && m_style["draw:end-color"])
        {
          m_outputSink << "    <svg:stop offset=\"0%\"";
          m_outputSink << " stop-color=\"" << m_style["draw:end-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (m_style["libwpg:end-opacity"] ? m_style["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;

          m_outputSink << "    <svg:stop offset=\"100%\"";
          m_outputSink << " stop-color=\"" << m_style["draw:start-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (m_style["libwpg:start-opacity"] ? m_style["libwpg:start-opacity"]->getDouble() : 1) << "\" />" << std::endl;
        }
        m_outputSink << "  </svg:radialGradient>\n";
        m_outputSink << "</svg:defs>\n";
      }
      else if (m_style["draw:style"] && m_style["draw:style"]->getStr() == "linear")
      {
        m_outputSink << "<svg:defs>\n";
        m_outputSink << "  <svg:linearGradient id=\"grad" << m_gradientIndex++ << "\" >\n";

        if (m_style["draw:start-color"] && m_style["draw:end-color"])
        {
          m_outputSink << "    <svg:stop offset=\"0%\"";
          m_outputSink << " stop-color=\"" << m_style["draw:start-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (m_style["libwpg:start-opacity"] ? m_style["libwpg:start-opacity"]->getDouble() : 1) << "\" />" << std::endl;

          m_outputSink << "    <svg:stop offset=\"100%\"";
          m_outputSink << " stop-color=\"" << m_style["draw:end-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (m_style["libwpg:end-opacity"] ? m_style["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;
        }
        m_outputSink << "  </svg:linearGradient>\n";

        // not a simple horizontal gradient
        if(angle != 270)
        {
          m_outputSink << "  <svg:linearGradient xlink:href=\"#grad" << m_gradientIndex-1 << "\"";
          m_outputSink << " id=\"grad" << m_gradientIndex++ << "\" ";
          m_outputSink << "x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\" ";
          m_outputSink << "gradientTransform=\"rotate(" << angle << " .5 .5)\" ";
          m_outputSink << "gradientUnits=\"objectBoundingBox\" >\n";
          m_outputSink << "  </svg:linearGradient>\n";
        }

        m_outputSink << "</svg:defs>\n";
      }
      else if (m_style["draw:style"] && m_style["draw:style"]->getStr() == "axial")
      {
        m_outputSink << "<svg:defs>\n";
        m_outputSink << "  <svg:linearGradient id=\"grad" << m_gradientIndex++ << "\" >\n";

        if (m_style["draw:start-color"] && m_style["draw:end-color"])
        {
          m_outputSink << "    <svg:stop offset=\"0%\"";
          m_outputSink << " stop-color=\"" << m_style["draw:end-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (m_style["libwpg:end-opacity"] ? m_style["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;

          m_outputSink << "    <svg:stop offset=\"50%\"";
          m_outputSink << " stop-color=\"" << m_style["draw:start-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (m_style["libwpg:start-opacity"] ? m_style["libwpg:start-opacity"]->getDouble() : 1) << "\" />" << std::endl;

          m_outputSink << "    <svg:stop offset=\"100%\"";
          m_outputSink << " stop-color=\"" << m_style["draw:end-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (m_style["libwpg:end-opacity"] ? m_style["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;
        }
        m_outputSink << "  </svg:linearGradient>\n";

        // not a simple horizontal gradient
        if(angle != 270)
        {
          m_outputSink << "  <svg:linearGradient xlink:href=\"#grad" << m_gradientIndex-1 << "\"";
          m_outputSink << " id=\"grad" << m_gradientIndex++ << "\" ";
          m_outputSink << "x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\" ";
          m_outputSink << "gradientTransform=\"rotate(" << angle << " .5 .5)\" ";
          m_outputSink << "gradientUnits=\"objectBoundingBox\" >\n";
          m_outputSink << "  </svg:linearGradient>\n";
        }

        m_outputSink << "</svg:defs>\n";
      }
    }
    else
    {
      if (m_style["draw:style"] && m_style["draw:style"]->getStr() == "radial")
      {
        m_outputSink << "<svg:defs>\n";
        m_outputSink << "  <svg:radialGradient id=\"grad" << m_gradientIndex++ << "\" cx=\"" << m_style["svg:cx"]->getStr().cstr() << "\" cy=\"" << m_style["svg:cy"]->getStr().cstr() << "\" r=\"" << m_style["svg:r"]->getStr().cstr() << "\" >\n";
        for(unsigned c = 0; c < m_gradient.count(); c++)
        {
          m_outputSink << "    <svg:stop offset=\"" << m_gradient[c]["svg:offset"]->getStr().cstr() << "\"";

          m_outputSink << " stop-color=\"" << m_gradient[c]["svg:stop-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << m_gradient[c]["svg:stop-opacity"]->getDouble() << "\" />" << std::endl;

        }
        m_outputSink << "  </svg:radialGradient>\n";
        m_outputSink << "</svg:defs>\n";
      }
      else
      {
        m_outputSink << "<svg:defs>\n";
        m_outputSink << "  <svg:linearGradient id=\"grad" << m_gradientIndex++ << "\" >\n";
        for(unsigned c = 0; c < m_gradient.count(); c++)
        {
          m_outputSink << "    <svg:stop offset=\"" << m_gradient[c]["svg:offset"]->getStr().cstr() << "\"";

          m_outputSink << " stop-color=\"" << m_gradient[c]["svg:stop-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << m_gradient[c]["svg:stop-opacity"]->getDouble() << "\" />" << std::endl;

        }
        m_outputSink << "  </svg:linearGradient>\n";

        // not a simple horizontal gradient
        if(angle != 270)
        {
          m_outputSink << "  <svg:linearGradient xlink:href=\"#grad" << m_gradientIndex-1 << "\"";
          m_outputSink << " id=\"grad" << m_gradientIndex++ << "\" ";
          m_outputSink << "x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\" ";
          m_outputSink << "gradientTransform=\"rotate(" << angle << " .5 .5)\" ";
          m_outputSink << "gradientUnits=\"objectBoundingBox\" >\n";
          m_outputSink << "  </svg:linearGradient>\n";
        }

        m_outputSink << "</svg:defs>\n";
      }
    }
  }
}

void libvisio::VSDSVGGenerator::startLayer(const ::WPXPropertyList &propList)
{
  m_outputSink << "<svg:g id=\"Layer" << propList["svg:id"]->getInt() << "\"";
  if (propList["svg:fill-rule"])
    m_outputSink << " fill-rule=\"" << propList["svg:fill-rule"]->getStr().cstr() << "\"";
  m_outputSink << " >\n";
}

void libvisio::VSDSVGGenerator::endLayer()
{
  m_outputSink << "</svg:g>\n";
}

void libvisio::VSDSVGGenerator::drawRectangle(const ::WPXPropertyList &propList)
{
  m_outputSink << "<svg:rect ";
  m_outputSink << "x=\"" << doubleToString(72*propList["svg:x"]->getDouble()) << "\" y=\"" << doubleToString(72*propList["svg:y"]->getDouble()) << "\" ";
  m_outputSink << "width=\"" << doubleToString(72*propList["svg:width"]->getDouble()) << "\" height=\"" << doubleToString(72*propList["svg:height"]->getDouble()) << "\" ";
  if((propList["svg:rx"] && propList["svg:rx"]->getInt() !=0) || (propList["svg:ry"] && propList["svg:ry"]->getInt() !=0))
    m_outputSink << "rx=\"" << doubleToString(72*propList["svg:rx"]->getDouble()) << "\" ry=\"" << doubleToString(72*propList["svg:ry"]->getDouble()) << "\" ";
  writeStyle();
  m_outputSink << "/>\n";
}

void libvisio::VSDSVGGenerator::drawEllipse(const WPXPropertyList &propList)
{
  m_outputSink << "<svg:ellipse ";
  m_outputSink << "cx=\"" << doubleToString(72*propList["svg:cx"]->getDouble()) << "\" cy=\"" << doubleToString(72*propList["svg:cy"]->getDouble()) << "\" ";
  m_outputSink << "rx=\"" << doubleToString(72*propList["svg:rx"]->getDouble()) << "\" ry=\"" << doubleToString(72*propList["svg:ry"]->getDouble()) << "\" ";
  writeStyle();
  if (propList["libwpg:rotate"] && propList["libwpg:rotate"]->getDouble() != 0.0)
    m_outputSink << " transform=\" rotate(" << doubleToString(-propList["libwpg:rotate"]->getDouble())
                 << ", " << doubleToString(72*propList["svg:cy"]->getDouble())
                 << ", " << doubleToString(72*propList["svg:cy"]->getDouble())
                 << ")\" ";
  m_outputSink << "/>\n";
}

void libvisio::VSDSVGGenerator::drawPolyline(const ::WPXPropertyListVector &vertices)
{
  drawPolySomething(vertices, false);
}

void libvisio::VSDSVGGenerator::drawPolygon(const ::WPXPropertyListVector &vertices)
{
  drawPolySomething(vertices, true);
}

void libvisio::VSDSVGGenerator::drawPolySomething(const ::WPXPropertyListVector &vertices, bool isClosed)
{
  if(vertices.count() < 2)
    return;

  if(vertices.count() == 2)
  {
    m_outputSink << "<svg:line ";
    m_outputSink << "x1=\"" << doubleToString(72*(vertices[0]["svg:x"]->getDouble())) << "\"  y1=\"" << doubleToString(72*(vertices[0]["svg:y"]->getDouble())) << "\" ";
    m_outputSink << "x2=\"" << doubleToString(72*(vertices[1]["svg:x"]->getDouble())) << "\"  y2=\"" << doubleToString(72*(vertices[1]["svg:y"]->getDouble())) << "\"\n";
    writeStyle();
    m_outputSink << "/>\n";
  }
  else
  {
    if (isClosed)
      m_outputSink << "<svg:polygon ";
    else
      m_outputSink << "<svg:polyline ";

    m_outputSink << "points=\"";
    for(unsigned i = 0; i < vertices.count(); i++)
    {
      m_outputSink << doubleToString(72*(vertices[i]["svg:x"]->getDouble())) << " " << doubleToString(72*(vertices[i]["svg:y"]->getDouble()));
      if (i < vertices.count()-1)
        m_outputSink << ", ";
    }
    m_outputSink << "\"\n";
    writeStyle(isClosed);
    m_outputSink << "/>\n";
  }
}

void libvisio::VSDSVGGenerator::drawPath(const ::WPXPropertyListVector &path)
{
  m_outputSink << "<svg:path d=\" ";
  bool isClosed = false;
  unsigned i=0;
  for(i=0; i < path.count(); i++)
  {
    WPXPropertyList propList = path[i];
    if (propList["libwpg:path-action"] && propList["libwpg:path-action"]->getStr() == "M")
    {
      m_outputSink << "\nM";
      m_outputSink << doubleToString(72*(propList["svg:x"]->getDouble())) << "," << doubleToString(72*(propList["svg:y"]->getDouble()));
    }
    else if (propList["libwpg:path-action"] && propList["libwpg:path-action"]->getStr() == "L")
    {
      m_outputSink << "\nL";
      m_outputSink << doubleToString(72*(propList["svg:x"]->getDouble())) << "," << doubleToString(72*(propList["svg:y"]->getDouble()));
    }
    else if (propList["libwpg:path-action"] && propList["libwpg:path-action"]->getStr() == "C")
    {
      m_outputSink << "\nC";
      m_outputSink << doubleToString(72*(propList["svg:x1"]->getDouble())) << "," << doubleToString(72*(propList["svg:y1"]->getDouble())) << " ";
      m_outputSink << doubleToString(72*(propList["svg:x2"]->getDouble())) << "," << doubleToString(72*(propList["svg:y2"]->getDouble())) << " ";
      m_outputSink << doubleToString(72*(propList["svg:x"]->getDouble())) << "," << doubleToString(72*(propList["svg:y"]->getDouble()));
    }
    else if (propList["libwpg:path-action"] && propList["libwpg:path-action"]->getStr() == "A")
    {
      m_outputSink << "\nA";
      m_outputSink << doubleToString(72*(propList["svg:rx"]->getDouble())) << "," << doubleToString(72*(propList["svg:ry"]->getDouble())) << " ";
      m_outputSink << doubleToString(propList["libwpg:rotate"] ? propList["libwpg:rotate"]->getDouble() : 0) << " ";
      m_outputSink << (propList["libwpg:large-arc"] ? propList["libwpg:large-arc"]->getInt() : 1) << ",";
      m_outputSink << (propList["libwpg:sweep"] ? propList["libwpg:sweep"]->getInt() : 1) << " ";
      m_outputSink << doubleToString(72*(propList["svg:x"]->getDouble())) << "," << doubleToString(72*(propList["svg:y"]->getDouble()));
    }
    else if ((i >= path.count()-1 && i > 2) && propList["libwpg:path-action"] && propList["libwpg:path-action"]->getStr() == "Z" )
    {
      isClosed = true;
      m_outputSink << "\nZ";
    }
  }

  m_outputSink << "\" \n";
  writeStyle(isClosed);
  m_outputSink << "/>\n";
}

void libvisio::VSDSVGGenerator::drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData &binaryData)
{
  if (!propList["libwpg:mime-type"] || propList["libwpg:mime-type"]->getStr().len() <= 0)
    return;
  WPXString base64 = binaryData.getBase64Data();
  m_outputSink << "<svg:image ";
  if (propList["svg:x"] && propList["svg:y"] && propList["svg:width"] && propList["svg:height"])
  {
    double x(propList["svg:x"]->getDouble());
    double y(propList["svg:y"]->getDouble());
    double width(propList["svg:width"]->getDouble());
    double height(propList["svg:height"]->getDouble());
    bool flipX(propList["draw:mirror-horizontal"] && propList["draw:mirror-horizontal"]->getInt());
    bool flipY(propList["draw:mirror-vertical"] && propList["draw:mirror-vertical"]->getInt());

    double xmiddle = x + width / 2.0;
    double ymiddle = y + height / 2.0;
    m_outputSink << "x=\"" << doubleToString(72*x) << "\" y=\"" << doubleToString(72*y) << "\" ";
    m_outputSink << "width=\"" << doubleToString(72*width) << "\" height=\"" << doubleToString(72*height) << "\" ";
    m_outputSink << "transform=\"";
    m_outputSink << " translate(" << doubleToString(72*xmiddle) << ", " << doubleToString (72*ymiddle) << ") ";
    m_outputSink << " scale(" << (flipX ? "-1" : "1") << ", " << (flipY ? "-1" : "1") << ") ";
    // rotation is around the center of the object's bounding box
    if (propList["libwpg:rotate"])
    {
      double angle(propList["libwpg:rotate"]->getDouble());
      while (angle > 180.0)
        angle -= 360.0;
      while (angle < -180.0)
        angle += 360.0;
      m_outputSink << " rotate(" << doubleToString(angle) << ") ";
    }
    m_outputSink << " translate(" << doubleToString(-72*xmiddle) << ", " << doubleToString (-72*ymiddle) << ") ";
    m_outputSink << "\" ";
  }
  m_outputSink << "xlink:href=\"data:" << propList["libwpg:mime-type"]->getStr().cstr() << ";base64,";
  m_outputSink << base64.cstr();
  m_outputSink << "\" />\n";
}

void libvisio::VSDSVGGenerator::startTextObject(const ::WPXPropertyList &propList, const ::WPXPropertyListVector & /* path */)
{
  double x = 0.0;
  double y = 0.0;
  double width = 0.0;
  double height = 0.0;
  m_outputSink << "<svg:text ";
  if (propList["svg:x"] && propList["svg:y"])
  {
    x = propList["svg:x"]->getDouble();
    y = propList["svg:y"]->getDouble();
  }

  double xmiddle = x;
  double ymiddle = y;

  if (propList["svg:width"])
  {
    width = propList["svg:width"]->getDouble();
    xmiddle += width / 2.0;
  }

  if (propList["svg:height"])
  {
    height = propList["svg:height"]->getDouble();
    ymiddle += height / 2.0;
  }

  if (propList["draw:textarea-vertical-align"])
  {
    if (propList["draw:textarea-vertical-align"]->getStr() == "middle")
      y = ymiddle;
    if (propList["draw:textarea-vertical-align"]->getStr() == "bottom")
    {
      y += height;
      if (propList["fo:padding-bottom"])
        y -= propList["fo:padding-bottom"]->getDouble();
    }
  }
  else
    y += height;

  if (propList["fo:padding-left"])
    x += propList["fo:padding-left"]->getDouble();

  m_outputSink << "x=\"" << doubleToString(72*x) << "\" y=\"" << doubleToString(72*y) << "\"";

  // rotation is around the center of the object's bounding box
  if (propList["libwpg:rotate"] && propList["libwpg:rotate"]->getDouble() != 0.0)
  {
    double angle(propList["libwpg:rotate"]->getDouble());
    while (angle > 180.0)
      angle -= 360.0;
    while (angle < -180.0)
      angle += 360.0;
    m_outputSink << " transform=\"rotate(" << doubleToString(angle) << ", " << doubleToString(72*xmiddle) << ", " << doubleToString(72*ymiddle) << ")\" ";
  }
  m_outputSink << ">\n";

}

void libvisio::VSDSVGGenerator::endTextObject()
{
  m_outputSink << "</svg:text>\n";
}

void libvisio::VSDSVGGenerator::startTextSpan(const ::WPXPropertyList &propList)
{
  m_outputSink << "<svg:tspan ";
  if (propList["style:font-name"])
    m_outputSink << "font-family=\"" << propList["style:font-name"]->getStr().cstr() << "\" ";
  if (propList["fo:font-style"])
    m_outputSink << "font-style=\"" << propList["fo:font-style"]->getStr().cstr() << "\" ";
  if (propList["fo:font-weight"])
    m_outputSink << "font-weight=\"" << propList["fo:font-weight"]->getStr().cstr() << "\" ";
  if (propList["fo:font-variant"])
    m_outputSink << "font-variant=\"" << propList["fo:font-variant"]->getStr().cstr() << "\" ";
  if (propList["fo:font-size"])
    m_outputSink << "font-size=\"" << doubleToString(propList["fo:font-size"]->getDouble()) << "\" ";
  if (propList["fo:color"])
    m_outputSink << "fill=\"" << propList["fo:color"]->getStr().cstr() << "\" ";
  if (propList["fo:text-transform"])
    m_outputSink << "text-transform=\"" << propList["fo:text-transform"]->getStr().cstr() << "\" ";
  if (propList["svg:fill-opacity"])
    m_outputSink << "fill-opacity=\"" << doubleToString(propList["svg:fill-opacity"]->getDouble()) << "\" ";
  if (propList["svg:stroke-opacity"])
    m_outputSink << "stroke-opacity=\"" << doubleToString(propList["svg:stroke-opacity"]->getDouble()) << "\" ";
  m_outputSink << ">\n";
}

void libvisio::VSDSVGGenerator::endTextSpan()
{
  m_outputSink << "</svg:tspan>\n";
}

void libvisio::VSDSVGGenerator::insertText(const ::WPXString &str)
{
  WPXString tempUTF8(str, true);
  m_outputSink << tempUTF8.cstr() << "\n";
}

// create "style" attribute based on current pen and brush
void libvisio::VSDSVGGenerator::writeStyle(bool /* isClosed */)
{
  m_outputSink << "style=\"";

  if (m_style["svg:stroke-width"])
    m_outputSink << "stroke-width: " << doubleToString(72*m_style["svg:stroke-width"]->getDouble()) << "; ";

  if ((m_style["draw:stroke"] && m_style["draw:stroke"]->getStr() != "none"))
  {
    if (m_style["svg:stroke-color"])
      m_outputSink << "stroke: " << m_style["svg:stroke-color"]->getStr().cstr()  << "; ";
    if(m_style["svg:stroke-opacity"] &&  m_style["svg:stroke-opacity"]->getInt()!= 1)
      m_outputSink << "stroke-opacity: " << doubleToString(m_style["svg:stroke-opacity"]->getDouble()) << "; ";
  }

  if (m_style["draw:stroke"] && m_style["draw:stroke"]->getStr() == "solid")
    m_outputSink << "stroke-dasharray:  solid; ";
  else if (m_style["draw:stroke"] && m_style["draw:stroke"]->getStr() == "dash")
  {
    int dots1 = m_style["draw:dots1"]->getInt();
    int dots2 = m_style["draw:dots2"]->getInt();
    double dots1len = m_style["draw:dots1-length"]->getDouble();
    double dots2len = m_style["draw:dots2-length"]->getDouble();
    double gap = m_style["draw:distance"]->getDouble();
    m_outputSink << "stroke-dasharray: ";
    for (int i = 0; i < dots1; i++)
    {
      if (i)
        m_outputSink << ", ";
      m_outputSink << (int)dots1len;
      m_outputSink << ", ";
      m_outputSink << (int)gap;
    }
    for (int j = 0; j < dots2; j++)
    {
      m_outputSink << ", ";
      m_outputSink << (int)dots2len;
      m_outputSink << ", ";
      m_outputSink << (int)gap;
    }
    m_outputSink << "; ";
  }

  if (m_style["svg:stroke-linecap"])
    m_outputSink << "stroke-linecap: " << m_style["svg:stroke-linecap"]->getStr().cstr() << "; ";

  if (m_style["svg:stroke-linejoin"])
    m_outputSink << "stroke-linejoin: " << m_style["svg:stroke-linejoin"]->getStr().cstr() << "; ";

  if(m_style["draw:fill"] && m_style["draw:fill"]->getStr() == "none")
    m_outputSink << "fill: none; ";
  else if(m_style["svg:fill-rule"])
    m_outputSink << "fill-rule: " << m_style["svg:fill-rule"]->getStr().cstr() << "; ";

  if(m_style["draw:fill"] && m_style["draw:fill"]->getStr() == "gradient")
    m_outputSink << "fill: url(#grad" << m_gradientIndex-1 << "); ";

  if(m_style["draw:shadow"] && m_style["draw:shadow"]->getStr() == "visible")
    m_outputSink << "filter:url(#shadow" << m_shadowIndex-1 << "); ";

  if(m_style["draw:fill"] && m_style["draw:fill"]->getStr() == "solid")
    if (m_style["draw:fill-color"])
      m_outputSink << "fill: " << m_style["draw:fill-color"]->getStr().cstr() << "; ";
  if(m_style["draw:opacity"] && m_style["draw:opacity"]->getDouble() < 1)
    m_outputSink << "fill-opacity: " << doubleToString(m_style["draw:opacity"]->getDouble()) << "; ";
  m_outputSink << "\""; // style
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
