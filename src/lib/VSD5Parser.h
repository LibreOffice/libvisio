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

#ifndef __VSD5PARSER_H__
#define __VSD5PARSER_H__

#include <stdio.h>
#include <iostream>
#include <librevenge/librevenge.h>
#include "VSD6Parser.h"
#include "VSDInternalStream.h"

namespace libvisio
{

class VSD5Parser : public VSD6Parser
{
public:
  explicit VSD5Parser(librevenge::RVNGInputStream *input, librevenge::RVNGDrawingInterface *painter);
  ~VSD5Parser();

protected:
  virtual void readPointer(librevenge::RVNGInputStream *input, Pointer &ptr);
  virtual bool getChunkHeader(librevenge::RVNGInputStream *input);
  virtual void readPointerInfo(librevenge::RVNGInputStream *input, unsigned ptrType, unsigned shift, unsigned &listSize, int &pointerCount);

  virtual void readGeomList(librevenge::RVNGInputStream *input);
  virtual void readCharList(librevenge::RVNGInputStream *input);
  virtual void readParaList(librevenge::RVNGInputStream *input);
  virtual void readShapeList(librevenge::RVNGInputStream *input);
  virtual void readPropList(librevenge::RVNGInputStream *input);
  virtual void readFieldList(librevenge::RVNGInputStream *input);
  virtual void readNameList2(librevenge::RVNGInputStream *input);

  virtual void readLine(librevenge::RVNGInputStream *input);
  virtual void readFillAndShadow(librevenge::RVNGInputStream *input);
  virtual void readTextBlock(librevenge::RVNGInputStream *input);
  virtual void readCharIX(librevenge::RVNGInputStream *input);
  virtual void readTextField(librevenge::RVNGInputStream *input);

  virtual void readShape(librevenge::RVNGInputStream *input);
  virtual void readPage(librevenge::RVNGInputStream *input);

  virtual void handleChunkRecords(librevenge::RVNGInputStream *input);

  virtual void readStyleSheet(librevenge::RVNGInputStream *input);

  virtual void readNameIDX(librevenge::RVNGInputStream *input);

  virtual unsigned getUInt(librevenge::RVNGInputStream *input);
  virtual int getInt(librevenge::RVNGInputStream *input);

private:
  VSD5Parser();
  VSD5Parser(const VSDParser &);
  VSD5Parser &operator=(const VSDParser &);

  void readList(librevenge::RVNGInputStream *input);
};

} // namespace libvisio

#endif // __VSD5PARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
