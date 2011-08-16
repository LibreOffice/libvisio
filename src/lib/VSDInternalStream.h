/* libvisio
 * Copyright (C) 2011 Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#ifndef __VSDINTERNALSTREAM_H__
#define __VSDINTERNALSTREAM_H__

#include <vector>

#include <libwpd-stream/libwpd-stream.h>

class VSDInternalStream : public WPXInputStream
{
public:
  VSDInternalStream(WPXInputStream *input, unsigned long size, bool compressed);
  VSDInternalStream(std::vector<unsigned char> buffer, unsigned long size);
  ~VSDInternalStream() {}

  bool isOLEStream() { return false; }
  WPXInputStream * getDocumentOLEStream(const char*) { return 0; }

  const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
  int seek(long offset, WPX_SEEK_TYPE seekType);
  long tell();
  bool atEOS();
  unsigned long getSize() const { return m_buffer.size(); };

private:
  long m_offset;
  std::vector<unsigned char> m_buffer;
  VSDInternalStream(const VSDInternalStream&);
  VSDInternalStream& operator=(const VSDInternalStream&);
};

#endif
