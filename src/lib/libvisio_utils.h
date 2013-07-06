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

#ifndef __LIBVISIO_UTILS_H__
#define __LIBVISIO_UTILS_H__

#include <stdio.h>
#include "VSDTypes.h"

#ifdef _MSC_VER

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned uint32_t;
typedef int int32_t;
typedef unsigned __int64 uint64_t;

#else /* !defined _MSC_VER */

#ifdef HAVE_CONFIG_H

#include <config.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#else /* !defined HAVE_CONFIG_H */

#include <stdint.h>
#include <inttypes.h>

#endif /* HAVE_CONFIG_H */

#endif /* _MSC_VER */

#include <libwpd/libwpd.h>
#include <libwpd-stream/libwpd-stream.h>

// debug message includes source file and line number
//#define VERBOSE_DEBUG 1

// do nothing with debug messages in a release compile
#ifdef DEBUG
#ifdef VERBOSE_DEBUG
#define VSD_DEBUG_MSG(M) printf("%15s:%5d: ", __FILE__, __LINE__); printf M
#define VSD_DEBUG(M) M
#else
#define VSD_DEBUG_MSG(M) printf M
#define VSD_DEBUG(M) M
#endif
#else
#define VSD_DEBUG_MSG(M)
#define VSD_DEBUG(M)
#endif

namespace libvisio
{

uint8_t readU8(WPXInputStream *input);
uint16_t readU16(WPXInputStream *input);
int16_t readS16(WPXInputStream *input);
uint32_t readU32(WPXInputStream *input);
int32_t readS32(WPXInputStream *input);
uint64_t readU64(WPXInputStream *input);

double readDouble(WPXInputStream *input);

void appendFromBase64(WPXBinaryData &data, const unsigned char *base64Data, size_t base64DataLength);

const ::WPXString getColourString(const Colour &c);

class EndOfStreamException
{
};

class XmlParserException
{
};

class GenericException
{
};

} // namespace libvisio

#endif // __LIBVISIO_UTILS_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
