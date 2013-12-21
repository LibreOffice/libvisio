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


#ifndef __VSDINTERNALSTREAM_H__
#define __VSDINTERNALSTREAM_H__

#include <stddef.h>
#include <vector>
#include <librevenge-stream/librevenge-stream.h>

class VSDInternalStream : public librevenge::RVNGInputStream
{
public:
  VSDInternalStream(librevenge::RVNGInputStream *input, unsigned long size, bool compressed=false);
  ~VSDInternalStream() {}

  bool isStructured()
  {
    return false;
  }
  unsigned subStreamCount()
  {
    return 0;
  }
  const char *subStreamName(unsigned)
  {
    return 0;
  }
  bool existsSubStream(const char *)
  {
    return false;
  }
  librevenge::RVNGInputStream *getSubStreamByName(const char *)
  {
    return 0;
  }
  librevenge::RVNGInputStream *getSubStreamById(unsigned)
  {
    return 0;
  }
  const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
  int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType);
  long tell();
  bool isEnd();
  unsigned long getSize() const
  {
    return m_buffer.size();
  };

private:
  volatile long m_offset;
  std::vector<unsigned char> m_buffer;
  VSDInternalStream(const VSDInternalStream &);
  VSDInternalStream &operator=(const VSDInternalStream &);
};

#endif
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
