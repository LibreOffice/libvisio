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

#ifndef __VSD6PARSER_H__
#define __VSD6PARSER_H__

#include <stdio.h>
#include <iostream>
#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>
#include "VSDXParser.h"
#include "VSDInternalStream.h"

namespace libvisio
{

class VSD6Parser : public VSDXParser
{
public:
  explicit VSD6Parser(WPXInputStream *input);
  ~VSD6Parser();
  bool parse(libwpg::WPGPaintInterface *iface);
private:

  typedef void (VSD6Parser::*Method)(VSDInternalStream&, libwpg::WPGPaintInterface*);
  struct StreamHandler { unsigned int type; const char *name; Method handler;};
  static const struct StreamHandler handlers[32];
  void handlePages(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);
  void handlePage(VSDInternalStream &stream, libwpg::WPGPaintInterface *painter);
};

} // namespace libvisio

#endif // __VSD6PARSER_H__
