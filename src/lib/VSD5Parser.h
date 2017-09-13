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
  ~VSD5Parser() override;

protected:
  void readPointer(librevenge::RVNGInputStream *input, Pointer &ptr) override;
  bool getChunkHeader(librevenge::RVNGInputStream *input) override;
  void readPointerInfo(librevenge::RVNGInputStream *input, unsigned ptrType, unsigned shift, unsigned &listSize, int &pointerCount) override;

  void readGeomList(librevenge::RVNGInputStream *input) override;
  void readCharList(librevenge::RVNGInputStream *input) override;
  void readParaList(librevenge::RVNGInputStream *input) override;
  void readShapeList(librevenge::RVNGInputStream *input) override;
  void readPropList(librevenge::RVNGInputStream *input) override;
  void readFieldList(librevenge::RVNGInputStream *input) override;
  void readNameList2(librevenge::RVNGInputStream *input) override;
  void readTabsDataList(librevenge::RVNGInputStream *input) override;

  void readLine(librevenge::RVNGInputStream *input) override;
  void readFillAndShadow(librevenge::RVNGInputStream *input) override;
  void readTextBlock(librevenge::RVNGInputStream *input) override;
  void readCharIX(librevenge::RVNGInputStream *input) override;
  void readParaIX(librevenge::RVNGInputStream *input) override;
  void readTextField(librevenge::RVNGInputStream *input) override;

  void readShape(librevenge::RVNGInputStream *input) override;
  void readPage(librevenge::RVNGInputStream *input) override;

  virtual void handleChunkRecords(librevenge::RVNGInputStream *input);

  void readStyleSheet(librevenge::RVNGInputStream *input) override;

  void readNameIDX(librevenge::RVNGInputStream *input) override;

  void readMisc(librevenge::RVNGInputStream *input) override;

  void readXForm1D(librevenge::RVNGInputStream *input) override;

  unsigned getUInt(librevenge::RVNGInputStream *input) override;
  int getInt(librevenge::RVNGInputStream *input) override;

private:
  VSD5Parser();
  VSD5Parser(const VSDParser &);
  VSD5Parser &operator=(const VSDParser &);

  void readList(librevenge::RVNGInputStream *input);
};

} // namespace libvisio

#endif // __VSD5PARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
