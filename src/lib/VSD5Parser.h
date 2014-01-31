/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
