/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __VSDMETADATA_H__
#define __VSDMETADATA_H__

#include <vector>
#include <utility>
#include <map>
#include <librevenge-stream/librevenge-stream.h>
#include <librevenge/librevenge.h>
#include "libvisio_utils.h"

namespace libvisio
{

class VSDMetaData
{
public:
  VSDMetaData();
  ~VSDMetaData();
  bool parse(librevenge::RVNGInputStream *input);
  bool parseTimes(librevenge::RVNGInputStream *input);
  const librevenge::RVNGPropertyList &getMetaData();

private:
  VSDMetaData(const VSDMetaData &);
  VSDMetaData &operator=(const VSDMetaData &);

  void readPropertySetStream(librevenge::RVNGInputStream *input);
  void readPropertySet(librevenge::RVNGInputStream *input, uint32_t offset, char *FMTID);
  void readPropertyIdentifierAndOffset(librevenge::RVNGInputStream *input);
  void readTypedPropertyValue(librevenge::RVNGInputStream *input, uint32_t index, uint32_t offset, char *FMTID);
  librevenge::RVNGString readCodePageString(librevenge::RVNGInputStream *input);

  uint32_t getCodePage();

  std::vector< std::pair<uint32_t, uint32_t> > m_idsAndOffsets;
  std::map<uint16_t, uint16_t> m_typedPropertyValues;
  librevenge::RVNGPropertyList m_metaData;
};

} // namespace libvisio

#endif // __VSDMETADATA_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
