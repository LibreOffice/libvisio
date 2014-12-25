/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDXMETADATA_H__
#define __VSDXMETADATA_H__

#include <librevenge-stream/librevenge-stream.h>
#include "VSDXMLHelper.h"

namespace libvisio
{

/// Parses docProps/core.xml stream of a VSDX file.
class VSDXMetaData
{
public:
  VSDXMetaData();
  ~VSDXMetaData();
  bool parse(librevenge::RVNGInputStream *input);
  const librevenge::RVNGPropertyList &getMetaData();

private:
  VSDXMetaData(const VSDXMetaData &);
  VSDXMetaData &operator=(const VSDXMetaData &);

  int getElementToken(xmlTextReaderPtr reader);
  void readCoreProperties(xmlTextReaderPtr reader);
  librevenge::RVNGString readString(xmlTextReaderPtr reader, int stringTokenId);

  librevenge::RVNGPropertyList m_metaData;
};

} // namespace libvisio

#endif // __VSDXMETADATA_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
