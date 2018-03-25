/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VSDInternalStream.h"

#include <string.h>

VSDInternalStream::VSDInternalStream(librevenge::RVNGInputStream *input, unsigned long size, bool compressed) :
  librevenge::RVNGInputStream(),
  m_offset(0),
  m_buffer()
{
  unsigned long tmpNumBytesRead = 0;

  const unsigned char *tmpBuffer = input->read(size, tmpNumBytesRead);

  if (tmpNumBytesRead < 2)
    return;

  if (!compressed)
  {
    m_buffer.assign(tmpBuffer, tmpBuffer + tmpNumBytesRead);
  }
  else
  {
    unsigned char buffer[4096] = { 0 };
    unsigned pos = 0;
    unsigned offset = 0;

    while (offset < tmpNumBytesRead)
    {
      unsigned flag = tmpBuffer[offset++];
      if (offset > tmpNumBytesRead-1)
        break;

      unsigned mask = 1;
      for (unsigned bit = 0; bit < 8 && offset < tmpNumBytesRead; ++bit)
      {
        if (flag & mask)
        {
          buffer[pos&4095] = tmpBuffer[offset++];
          m_buffer.push_back(buffer[pos&4095]);
          pos++;
        }
        else
        {
          if (offset > tmpNumBytesRead-2)
            break;
          unsigned char addr1 = tmpBuffer[offset++];
          unsigned char addr2 = tmpBuffer[offset++];

          unsigned length = (addr2&15) + 3;
          unsigned pointer = (((unsigned)addr2 & 0xF0) << 4) | addr1;
          if (pointer > 4078)
            pointer -= 4078;
          else
            pointer += 18;

          for (unsigned j = 0; j < length; ++j)
          {
            buffer[(pos+j) & 4095] = buffer[(pointer+j) & 4095];
            m_buffer.push_back(buffer[(pointer+j) & 4095]);
          }
          pos += length;
        }
        mask = mask << 1;
      }
    }
  }
}

const unsigned char *VSDInternalStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
  numBytesRead = 0;

  if (numBytes == 0)
    return nullptr;

  int numBytesToRead;

  if (numBytes < m_buffer.size() - m_offset)
    numBytesToRead = numBytes;
  else
    numBytesToRead = m_buffer.size() - m_offset;

  numBytesRead = numBytesToRead; // about as paranoid as we can be..

  if (numBytesToRead == 0)
    return nullptr;

  long oldOffset = m_offset;
  m_offset += numBytesToRead;

  return &m_buffer[oldOffset];
}

int VSDInternalStream::seek(long offset, librevenge::RVNG_SEEK_TYPE seekType)
{
  if (seekType == librevenge::RVNG_SEEK_CUR)
    m_offset += offset;
  else if (seekType == librevenge::RVNG_SEEK_SET)
    m_offset = offset;
  else if (seekType == librevenge::RVNG_SEEK_END)
    m_offset = long(static_cast<unsigned long>(m_buffer.size())) + offset;

  if (m_offset < 0)
  {
    m_offset = 0;
    return 1;
  }
  if ((long)m_offset > (long)m_buffer.size())
  {
    m_offset = m_buffer.size();
    return 1;
  }

  return 0;
}

long VSDInternalStream::tell()
{
  return m_offset;
}

bool VSDInternalStream::isEnd()
{
  if ((long)m_offset >= (long)m_buffer.size())
    return true;

  return false;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
