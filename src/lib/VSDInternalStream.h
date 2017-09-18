/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
  ~VSDInternalStream() override {}

  bool isStructured() override
  {
    return false;
  }
  unsigned subStreamCount() override
  {
    return 0;
  }
  const char *subStreamName(unsigned) override
  {
    return nullptr;
  }
  bool existsSubStream(const char *) override
  {
    return false;
  }
  librevenge::RVNGInputStream *getSubStreamByName(const char *) override
  {
    return nullptr;
  }
  librevenge::RVNGInputStream *getSubStreamById(unsigned) override
  {
    return nullptr;
  }
  const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead) override;
  int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType) override;
  long tell() override;
  bool isEnd() override;
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
