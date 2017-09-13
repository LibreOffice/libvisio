/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
  ~VSD6Parser() override;
protected:
  bool getChunkHeader(librevenge::RVNGInputStream *input) override;
private:
  void readText(librevenge::RVNGInputStream *input) override;
  void readCharIX(librevenge::RVNGInputStream *input) override;
  void readParaIX(librevenge::RVNGInputStream *input) override;
  void readFillAndShadow(librevenge::RVNGInputStream *input) override;
  void readName(librevenge::RVNGInputStream *input) override;
  void readName2(librevenge::RVNGInputStream *input) override;
  void readTextField(librevenge::RVNGInputStream *input) override;
  void readLayerMem(librevenge::RVNGInputStream *input) override;
  void readMisc(librevenge::RVNGInputStream *input) override;


  VSD6Parser();
  VSD6Parser(const VSDParser &);
  VSD6Parser &operator=(const VSDParser &);
};

} // namespace libvisio

#endif // __VSD6PARSER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
