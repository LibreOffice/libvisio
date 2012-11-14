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

#include <string>
#include "VSDInternalStream.h"
#include "libvisio_utils.h"

#define VSD_NUM_ELEMENTS(array) sizeof(array)/sizeof(array[0])

uint8_t libvisio::readU8(WPXInputStream *input)
{
  if (!input || input->atEOS())
  {
    VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
    throw EndOfStreamException();
  }
  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint8_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint8_t))
    return *(uint8_t const *)(p);
  VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
  throw EndOfStreamException();
}

uint16_t libvisio::readU16(WPXInputStream *input)
{
  if (!input || input->atEOS())
  {
    VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
    throw EndOfStreamException();
  }
  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint16_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint16_t))
    return (uint16_t)p[0]|((uint16_t)p[1]<<8);
  VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
  throw EndOfStreamException();
}

int16_t libvisio::readS16(WPXInputStream *input)
{
  return (int16_t)readU16(input);
}

uint32_t libvisio::readU32(WPXInputStream *input)
{
  if (!input || input->atEOS())
  {
    VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
    throw EndOfStreamException();
  }
  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint32_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint32_t))
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
  VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
  throw EndOfStreamException();
}

int32_t libvisio::readS32(WPXInputStream *input)
{
  return (int32_t)readU32(input);
}

uint64_t libvisio::readU64(WPXInputStream *input)
{
  if (!input || input->atEOS())
  {
    VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
    throw EndOfStreamException();
  }
  unsigned long numBytesRead;
  uint8_t const *p = input->read(sizeof(uint64_t), numBytesRead);

  if (p && numBytesRead == sizeof(uint64_t))
    return (uint64_t)p[0]|((uint64_t)p[1]<<8)|((uint64_t)p[2]<<16)|((uint64_t)p[3]<<24)|((uint64_t)p[4]<<32)|((uint64_t)p[5]<<40)|((uint64_t)p[6]<<48)|((uint64_t)p[7]<<56);
  VSD_DEBUG_MSG(("Throwing EndOfStreamException\n"));
  throw EndOfStreamException();
}

double libvisio::readDouble(WPXInputStream *input)
{
  union
  {
    uint64_t u;
    double d;
  } tmpUnion;

  tmpUnion.u = readU64(input);

  return tmpUnion.d;
}

void libvisio::appendFromBase64(WPXBinaryData &data, const unsigned char *base64String, size_t base64StringLength)
{
  static const std::string base64Chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";


  VSDInternalStream tmpStream(base64String, base64StringLength);

  unsigned i = 0;
  char tmpCharsToDecode[4];
  while (!tmpStream.atEOS())
  {
    const char tmpChar = (char)readU8(&tmpStream);
    if (std::string::npos == base64Chars.find(tmpChar) && (tmpChar != '='))
      continue;
    if (tmpChar == '=')
      break;
    tmpCharsToDecode[i++] = tmpChar;
    i %= 4;
    if (!i)
    {
      for (unsigned k = 0; k < 4; k++)
        tmpCharsToDecode[k] = base64Chars.find(tmpCharsToDecode[k]);

      data.append((unsigned char)((tmpCharsToDecode[0] << 2) | ((tmpCharsToDecode[1] & 0x30) >> 4)));
      data.append((unsigned char)(((tmpCharsToDecode[1] & 0xf) << 4) | ((tmpCharsToDecode[2] & 0x3c) >> 2)));
      data.append((unsigned char)(((tmpCharsToDecode[2] & 0x3) << 6) | tmpCharsToDecode[3]));
    }
  }

  if (i)
  {
    for (unsigned j = i; j < 4; j++)
      tmpCharsToDecode[j] = 0;
    for (unsigned k = 0; k < 4; k++)
      tmpCharsToDecode[k] = base64Chars.find(tmpCharsToDecode[k]);

    data.append((unsigned char)((tmpCharsToDecode[0] << 2) | ((tmpCharsToDecode[1] & 0x30) >> 4)));
    if (i > 1)
    {
      data.append((unsigned char)(((tmpCharsToDecode[1] & 0xf) << 4) | ((tmpCharsToDecode[2] & 0x3c) >> 2)));
      if (i > 2)
        data.append((unsigned char)(((tmpCharsToDecode[2] & 0x3) << 6) | tmpCharsToDecode[3]));
    }
  }
}

const ::WPXString libvisio::getColourString(const Colour &c)
{
  ::WPXString sColour;
  sColour.sprintf("#%.2x%.2x%.2x", c.r, c.g, c.b);
  return sColour;
}



/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
