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
#include "VSDXParser.h"
#include <locale.h>
#include <sstream>
#include <string>

libvisio::VSDXParser::VSDXParser(WPXInputStream *input)
  : m_input(input)
{}

libvisio::VSDXParser::~VSDXParser()
{}


/**************************************************
 * Visio 2000 parser implementation
 **************************************************/

libvisio::VSD6Parser::VSD6Parser(WPXInputStream *input)
  : VSDXParser(input)
{}

libvisio::VSD6Parser::~VSD6Parser()
{}

/** Parses VSD 2000 input stream content, making callbacks to functions provided
by WPGPaintInterface class implementation as needed.
\param iface A WPGPaintInterface implementation
\return A value indicating whether parsing was successful
*/
bool libvisio::VSD6Parser::parse(libwpg::WPGPaintInterface *iface)
{
  return false;
}


/**************************************************
 * Visio 2003 parser implementation
 **************************************************/

libvisio::VSD11Parser::VSD11Parser(WPXInputStream *input)
  : VSDXParser(input)
{}

libvisio::VSD11Parser::~VSD11Parser()
{}

/** Parses VSD 2003 input stream content, making callbacks to functions provided
by WPGPaintInterface class implementation as needed.
\param iface A WPGPaintInterface implementation
\return A value indicating whether parsing was successful
*/
bool libvisio::VSD11Parser::parse(libwpg::WPGPaintInterface *iface)
{
  return false;
}
