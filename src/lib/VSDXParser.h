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

#ifndef __VSDXPARSER_H__
#define __VSDXPARSER_H__

#include <stdio.h>
#include <iostream>
#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>

namespace libvisio
{

class VSDXParser
{
public:
  explicit VSDXParser(WPXInputStream *input);
  virtual ~VSDXParser();
  virtual bool parse(libwpg::WPGPaintInterface *iface) = 0;
protected:
  WPXInputStream *m_input;
};

/* Parser for Visio 2000 */
class VSD6Parser : protected VSDXParser
{
public:
  explicit VSD6Parser(WPXInputStream *input);
  ~VSD6Parser();
  bool parse(libwpg::WPGPaintInterface *iface);
};

/* Parser for Visio 2003 */
class VSD11Parser : protected VSDXParser
{
public:
  explicit VSD11Parser(WPXInputStream *input);
  ~VSD11Parser();
  bool parse(libwpg::WPGPaintInterface *iface);
};

} // namespace libvisio

#endif // __VSDXPARSER_H__
