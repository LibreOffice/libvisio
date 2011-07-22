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


libvisio::VSDSVGGenerator::VSDSVGGenerator(std::ostream & output_sink): m_gradient(), m_style(), m_gradientIndex(1), m_shadowIndex(1), m_isFirstPage(true), m_outputSink(output_sink)
{
}

libvisio::VSDSVGGenerator::~VSDSVGGenerator()
{
}

void libvisio::VSDSVGGenerator::startGraphics(const WPXPropertyList &propList)
{
  if (m_isFirstPage)
    m_isFirstPage = false;
  else
    m_outputSink << "<hr/>\n";

  m_outputSink << "<!-- \n";
  m_outputSink << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
  m_outputSink << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"";
  m_outputSink << " \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
  m_outputSink << " -->\n";

  m_outputSink << "<svg:svg version=\"1.1\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" ";
  if (propList["svg:width"])
    m_outputSink << "width=\"" << doubleToString(72*(propList["svg:width"]->getDouble())) << "\" ";
  if (propList["svg:height"])
    m_outputSink << "height=\"" << doubleToString(72*(propList["svg:height"]->getDouble())) << "\"";
  m_outputSink << " >\n";

  m_gradientIndex = 1;
}

void libvisio::VSDSVGGenerator::endGraphics()
{
  m_outputSink << "</svg:svg>\n";
}

void libvisio::VSDSVGGenerator::setStyle(const ::WPXPropertyList &propList, const ::WPXPropertyListVector& gradient)
{
  m_style.clear();
  m_style = propList;

  m_gradient = gradient;
  if(propList["draw:shadow"] && propList["draw:shadow"]->getStr() == "visible")
  {
    m_outputSink << "<svg:defs>\n";
    m_outputSink << "<svg:filter filterUnits=\"userSpaceOnUse\" id=\"shadow" << m_shadowIndex++ << "\">";
    m_outputSink << "<svg:feOffset in=\"SourceGraphic\" result=\"offset\" ";
    m_outputSink << "dx=\"" << doubleToString(72*propList["draw:shadow-offset-x"]->getDouble()) << "\" ";
    m_outputSink << "dy=\"" << doubleToString(-72*propList["draw:shadow-offset-y"]->getDouble()) << "\"/>";
    m_outputSink << "<svg:feColorMatrix in=\"offset\" result=\"offset-color\" type=\"matrix\" values=\"";
    m_outputSink << "0 0 0 0 " << doubleToString(propList["libwpg:shadow-color-r"]->getDouble());
    m_outputSink << " 0 0 0 0 " << doubleToString(propList["libwpg:shadow-color-g"]->getDouble());
    m_outputSink << " 0 0 0 0 " << doubleToString(propList["libwpg:shadow-color-b"]->getDouble());
    if(m_style["draw:opacity"] && m_style["draw:opacity"]->getDouble() < 1)
      m_outputSink << " 0 0 0 "   << doubleToString(propList["draw:shadow-opacity"]->getDouble()/propList["draw:opacity"]->getDouble()) << " 0\"/>";
    else
      m_outputSink << " 0 0 0 "   << doubleToString(propList["draw:shadow-opacity"]->getDouble()) << " 0\"/>";
    m_outputSink << "<svg:feMerge><svg:feMergeNode in=\"offset-color\" /><svg:feMergeNode in=\"SourceGraphic\" /></svg:feMerge></svg:filter></svg:defs>";
  }

  if(propList["draw:fill"] && propList["draw:fill"]->getStr() == "gradient")
  {
    double angle = (m_style["draw:angle"] ? m_style["draw:angle"]->getDouble() : 0.0);
	angle *= -1.0;
    while(angle < 0)
      angle += 360;
    while(angle > 360)
      angle -= 360;

    if (!gradient.count())
	{
	  if (propList["draw:style"] &&
	     (propList["draw:style"]->getStr() == "radial" ||
		  propList["draw:style"]->getStr() == "rectangular" ||
		  propList["draw:style"]->getStr() == "square" ||
		  propList["draw:style"]->getStr() == "ellipsoid"))
      {
        m_outputSink << "<svg:defs>\n";
        m_outputSink << "  <svg:radialGradient id=\"grad" << m_gradientIndex++ << "\"";

		if (propList["svg:cx"])
		  m_outputSink << " cx=\"" << propList["svg:cx"]->getStr().cstr() << "\"";
		else if (propList["draw:cx"])
		  m_outputSink << " cx=\"" << propList["draw:cx"]->getStr().cstr() << "\"";

		if (propList["svg:cy"])
		  m_outputSink << " cy=\"" << propList["svg:cy"]->getStr().cstr() << "\"";
		else if (propList["draw:cy"])
		  m_outputSink << " cy=\"" << propList["draw:cy"]->getStr().cstr() << "\"";
        m_outputSink << " r=\"" << (1 - (propList["draw:border"] ? propList["draw:border"]->getDouble() : 0))*100.0 << "%\" >\n";
		m_outputSink << " >\n";
		
        if (propList["draw:start-color"] && propList["draw:end-color"])
		{
		  m_outputSink << "    <svg:stop offset=\"0%\"";
          m_outputSink << " stop-color=\"" << propList["draw:end-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (propList["libwpg:end-opacity"] ? propList["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;

		  m_outputSink << "    <svg:stop offset=\"100%\"";
          m_outputSink << " stop-color=\"" << propList["draw:start-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (propList["libwpg:start-opacity"] ? propList["libwpg:start-opacity"]->getDouble() : 1) << "\" />" << std::endl;
        }
        m_outputSink << "  </svg:radialGradient>\n";
        m_outputSink << "</svg:defs>\n";
      }
	  else if (propList["draw:style"] && propList["draw:style"]->getStr() == "linear")
	  {
        m_outputSink << "<svg:defs>\n";
        m_outputSink << "  <svg:linearGradient id=\"grad" << m_gradientIndex++ << "\" >\n";

        if (propList["draw:start-color"] && propList["draw:end-color"])
		{
		  m_outputSink << "    <svg:stop offset=\"0%\"";
          m_outputSink << " stop-color=\"" << propList["draw:start-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (propList["libwpg:start-opacity"] ? propList["libwpg:start-opacity"]->getDouble() : 1) << "\" />" << std::endl;

		  m_outputSink << "    <svg:stop offset=\"100%\"";
          m_outputSink << " stop-color=\"" << propList["draw:end-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (propList["libwpg:end-opacity"] ? propList["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;
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
	  else if (propList["draw:style"] && propList["draw:style"]->getStr() == "axial")
	  {
        m_outputSink << "<svg:defs>\n";
        m_outputSink << "  <svg:linearGradient id=\"grad" << m_gradientIndex++ << "\" >\n";

        if (propList["draw:start-color"] && propList["draw:end-color"])
		{
		  m_outputSink << "    <svg:stop offset=\"0%\"";
          m_outputSink << " stop-color=\"" << propList["draw:end-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (propList["libwpg:end-opacity"] ? propList["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;

		  m_outputSink << "    <svg:stop offset=\"50%\"";
          m_outputSink << " stop-color=\"" << propList["draw:start-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (propList["libwpg:start-opacity"] ? propList["libwpg:start-opacity"]->getDouble() : 1) << "\" />" << std::endl;

		  m_outputSink << "    <svg:stop offset=\"100%\"";
          m_outputSink << " stop-color=\"" << propList["draw:end-color"]->getStr().cstr() << "\"";
          m_outputSink << " stop-opacity=\"" << (propList["libwpg:end-opacity"] ? propList["libwpg:end-opacity"]->getDouble() : 1) << "\" />" << std::endl;
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
	  if (propList["draw:style"] && propList["draw:style"]->getStr() == "radial")
      {
        m_outputSink << "<svg:defs>\n";
        m_outputSink << "  <svg:radialGradient id=\"grad" << m_gradientIndex++ << "\" cx=\"" << propList["svg:cx"]->getStr().cstr() << "\" cy=\"" << propList["svg:cy"]->getStr().cstr() << "\" r=\"" << propList["svg:r"]->getStr().cstr() << "\" >\n";
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

void libvisio::VSDSVGGenerator::startLayer(const ::WPXPropertyList& propList)
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

void libvisio::VSDSVGGenerator::drawRectangle(const ::WPXPropertyList& propList)
{
  m_outputSink << "<svg:rect ";
  m_outputSink << "x=\"" << doubleToString(72*propList["svg:x"]->getDouble()) << "\" y=\"" << doubleToString(72*propList["svg:y"]->getDouble()) << "\" ";
  m_outputSink << "width=\"" << doubleToString(72*propList["svg:width"]->getDouble()) << "\" height=\"" << doubleToString(72*propList["svg:height"]->getDouble()) << "\" ";
  if((propList["svg:rx"] && propList["svg:rx"]->getInt() !=0) || (propList["svg:ry"] && propList["svg:ry"]->getInt() !=0))
    m_outputSink << "rx=\"" << doubleToString(72*propList["svg:rx"]->getDouble()) << "\" ry=\"" << doubleToString(72*propList["svg:ry"]->getDouble()) << "\" ";
  writeStyle();
  m_outputSink << "/>\n";
}

void libvisio::VSDSVGGenerator::drawEllipse(const WPXPropertyList& propList)
{
  m_outputSink << "<svg:ellipse ";
  m_outputSink << "cx=\"" << doubleToString(72*propList["svg:cx"]->getDouble()) << "\" cy=\"" << doubleToString(72*propList["svg:cy"]->getDouble()) << "\" ";
  m_outputSink << "rx=\"" << doubleToString(72*propList["svg:rx"]->getDouble()) << "\" ry=\"" << doubleToString(72*propList["svg:ry"]->getDouble()) << "\" ";
  writeStyle();
  if (propList["libwpg:rotate"] && propList["libwpg:rotate"]->getDouble() != 0.0)
    m_outputSink << " transform=\" translate(" << doubleToString(72*propList["svg:cx"]->getDouble()) << ", " << doubleToString(72*propList["svg:cy"]->getDouble())
    << ") rotate(" << doubleToString(-propList["libwpg:rotate"]->getDouble())
    << ") translate(" << doubleToString(-72*propList["svg:cx"]->getDouble())
    << ", " << doubleToString(-72*propList["svg:cy"]->getDouble())
    << ")\" ";
  m_outputSink << "/>\n";
}

void libvisio::VSDSVGGenerator::drawPolyline(const ::WPXPropertyListVector& vertices)
{
  drawPolySomething(vertices, false);
}

void libvisio::VSDSVGGenerator::drawPolygon(const ::WPXPropertyListVector& vertices)
{
  drawPolySomething(vertices, true);
}

void libvisio::VSDSVGGenerator::drawPolySomething(const ::WPXPropertyListVector& vertices, bool isClosed)
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

void libvisio::VSDSVGGenerator::drawPath(const ::WPXPropertyListVector& path)
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

void libvisio::VSDSVGGenerator::drawGraphicObject(const ::WPXPropertyList &propList, const ::WPXBinaryData& binaryData)
{
  if (!propList["libwpg:mime-type"] || propList["libwpg:mime-type"]->getStr().len() <= 0)
    return;
  WPXString base64 = binaryData.getBase64Data();
  m_outputSink << "<svg:image ";
  if (propList["svg:x"] && propList["svg:y"] && propList["svg:width"] && propList["svg:height"])
  m_outputSink << "x=\"" << doubleToString(72*(propList["svg:x"]->getDouble())) << "\" y=\"" << doubleToString(72*(propList["svg:y"]->getDouble())) << "\" ";
  m_outputSink << "width=\"" << doubleToString(72*(propList["svg:width"]->getDouble())) << "\" height=\"" << doubleToString(72*(propList["svg:height"]->getDouble())) << "\" ";
  m_outputSink << "xlink:href=\"data:" << propList["libwpg:mime-type"]->getStr().cstr() << ";base64,";
  m_outputSink << base64.cstr();
  m_outputSink << "\" />\n";
}

void libvisio::VSDSVGGenerator::startTextObject(const ::WPXPropertyList &propList, const ::WPXPropertyListVector & /* path */)
{
  m_outputSink << "<svg:text ";
  if (propList["svg:x"] && propList["svg:y"])
  m_outputSink << "x=\"" << doubleToString(72*(propList["svg:x"]->getDouble())) << "\" y=\"" << doubleToString(72*(propList["svg:y"]->getDouble())) << "\"";
  if (propList["libwpg:rotate"] && propList["libwpg:rotate"]->getDouble() != 0.0)
    m_outputSink << " transform=\"translate(" << doubleToString(72*propList["svg:cx"]->getDouble()) << ", " << doubleToString(72*propList["svg:cy"]->getDouble())
    << ") rotate(" << doubleToString(-propList["libwpg:rotate"]->getDouble())
    << ") translate(" << doubleToString(-72*propList["svg:cx"]->getDouble())
    << ", " << doubleToString(-72*propList["svg:cy"]->getDouble())
    << ")\"";
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
    m_outputSink << "font-variant=\"" << propList["fo:font-weight"]->getStr().cstr() << "\" ";
  if (propList["fo:font-size"])
    m_outputSink << "font-size=\"" << propList["fo:font-size"]->getStr().cstr() << "\" ";
  if (propList["fo:color"])
    m_outputSink << "fill=\"" << propList["fo:color"]->getStr().cstr() << "\" ";
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

  if((m_style["svg:stroke-width"] && m_style["svg:stroke-width"]->getDouble() > 0.0) || (m_style["draw:stroke"] && m_style["draw:stroke"]->getStr() == "solid"))
  {
    if (m_style["svg:stroke-color"])
      m_outputSink << "stroke: " << m_style["svg:stroke-color"]->getStr().cstr()  << "; ";
    if(m_style["svg:stroke-opacity"] &&  m_style["svg:stroke-opacity"]->getInt()!= 1)
      m_outputSink << "stroke-opacity: " << doubleToString(m_style["svg:stroke-opacity"]->getDouble()) << "; ";
  }

  if (m_style["svg:stroke-dasharray"])
    m_outputSink << "stroke-dasharray: " << m_style["svg:stroke-dasharray"]->getStr().cstr() <<"; ";

  if (m_style["svg:stroke-linecap"])
     m_outputSink << "stroke-linecap: " << m_style["svg:stroke-linecap"]->getStr().cstr() << "; ";

  if (m_style["svg:stroke-linejoin"])
     m_outputSink << "stroke-linejoin: " << m_style["svg:stroke-linejoin"]->getStr().cstr() << "; ";

  if(m_style["draw:fill"] && m_style["draw:fill"]->getStr() == "none")
    m_outputSink << "fill: none; ";
  else
    if(m_style["svg:fill-rule"])
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
