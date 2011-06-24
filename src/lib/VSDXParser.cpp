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
#include "libvisio_utils.h"
#include "VSDXParser.h"
#include "VSDInternalStream.h"

libvisio::VSDXParser::VSDXParser(WPXInputStream *input)
  : m_input(input), m_isPageStarted(false), m_pageWidth(0.0), 
    m_pageHeight(0.0), m_scale(1.0), m_x(0.0), m_y(0.0),
    m_currentShapeId(0), m_foreignType(0), m_foreignFormat(0)
{}

libvisio::VSDXParser::~VSDXParser()
{}




