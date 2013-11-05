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

#ifndef __VSD6PARSER_H__
#define __VSD6PARSER_H__

#include <stdio.h>
#include <iostream>
#include <librevenge/librevenge.h>
#include "VSDParser.h"
#include "VSDInternalStream.h"

namespace libvisio
{

class VSD6Parser : public VSDParser
{
public:
  explicit VSD6Parser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter);
  ~VSD6Parser();
protected:
  virtual bool getChunkHeader(librevenge::RVNGInputStream *input);
private:
  void readText(librevenge::RVNGInputStream *input);
  virtual void readCharIX(librevenge::RVNGInputStream *input);
  virtual void readParaIX(librevenge::RVNGInputStream *input);
  virtual void readFillAndShadow(librevenge::RVNGInputStream *input);
  virtual void readName(librevenge::RVNGInputStream *input);
  virtual void readName2(librevenge::RVNGInputStream *input);
  virtual void readTextField(librevenge::RVNGInputStream *input);

  VSD6Parser();
  VSD6Parser(const VSDParser &);
  VSD6Parser &operator=(const VSDParser &);
};

} // namespace libvisio

#endif // __VSD6PARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
