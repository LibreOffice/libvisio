/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

#include <librevenge/librevenge.h>
#include <librevenge-stream/librevenge-stream.h>

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

uint8_t readU8(librevenge::RVNGInputStream *input);
uint16_t readU16(librevenge::RVNGInputStream *input);
int16_t readS16(librevenge::RVNGInputStream *input);
uint32_t readU32(librevenge::RVNGInputStream *input);
int32_t readS32(librevenge::RVNGInputStream *input);
uint64_t readU64(librevenge::RVNGInputStream *input);

double readDouble(librevenge::RVNGInputStream *input);

const librevenge::RVNGString getColourString(const Colour &c);

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
