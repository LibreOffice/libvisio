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
  explicit VSD5Parser(RVNGInputStream *input, RVNGDrawingInterface *painter);
  ~VSD5Parser();

protected:
  virtual void readPointer(RVNGInputStream *input, Pointer &ptr);
  virtual bool getChunkHeader(RVNGInputStream *input);
  virtual void readPointerInfo(RVNGInputStream *input, unsigned ptrType, unsigned shift, unsigned &listSize, int &pointerCount);

  virtual void readGeomList(RVNGInputStream *input);
  virtual void readCharList(RVNGInputStream *input);
  virtual void readParaList(RVNGInputStream *input);
  virtual void readShapeList(RVNGInputStream *input);
  virtual void readPropList(RVNGInputStream *input);
  virtual void readFieldList(RVNGInputStream *input);
  virtual void readNameList2(RVNGInputStream *input);

  virtual void readLine(RVNGInputStream *input);
  virtual void readFillAndShadow(RVNGInputStream *input);
  virtual void readTextBlock(RVNGInputStream *input);
  virtual void readCharIX(RVNGInputStream *input);
  virtual void readTextField(RVNGInputStream *input);

  virtual void readShape(RVNGInputStream *input);
  virtual void readPage(RVNGInputStream *input);

  virtual void handleChunkRecords(RVNGInputStream *input);

  virtual void readStyleSheet(RVNGInputStream *input);

  virtual void readNameIDX(RVNGInputStream *input);

  virtual unsigned getUInt(RVNGInputStream *input);
  virtual int getInt(RVNGInputStream *input);

private:
  VSD5Parser();
  VSD5Parser(const VSDParser &);
  VSD5Parser &operator=(const VSDParser &);

  void readList(RVNGInputStream *input);
};

} // namespace libvisio

#endif // __VSD5PARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
