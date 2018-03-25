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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <memory>

#include <boost/cstdint.hpp>

#include "VSDTypes.h"

#define VSD_EPSILON 1E-6
#define VSD_ALMOST_ZERO(m) (fabs(m) <= VSD_EPSILON)
#define VSD_APPROX_EQUAL(x, y) VSD_ALMOST_ZERO((x) - (y))

#include <librevenge/librevenge.h>
#include <librevenge-stream/librevenge-stream.h>
#include <unicode/utypes.h>

#if defined(HAVE_FUNC_ATTRIBUTE_FORMAT)
#define VSD_ATTRIBUTE_PRINTF(fmt, arg) __attribute__((format(printf, fmt, arg)))
#else
#define VSD_ATTRIBUTE_PRINTF(fmt, arg)
#endif

// do nothing with debug messages in a release compile
#ifdef DEBUG
#define VSD_DEBUG_MSG(M) libvisio::debugPrint M
#define VSD_DEBUG(M) M
#else
#define VSD_DEBUG_MSG(M)
#define VSD_DEBUG(M)
#endif

#define VSD_NUM_ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

namespace libvisio
{

typedef std::shared_ptr<librevenge::RVNGInputStream> RVNGInputStreamPtr_t;

struct VSDDummyDeleter
{
  void operator()(void *) {}
};

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
std::unique_ptr<T> clone(const std::unique_ptr<T> &other)
{
  return std::unique_ptr<T>(other->clone());
}

uint8_t readU8(librevenge::RVNGInputStream *input);
uint16_t readU16(librevenge::RVNGInputStream *input);
int16_t readS16(librevenge::RVNGInputStream *input);
uint32_t readU32(librevenge::RVNGInputStream *input);
int32_t readS32(librevenge::RVNGInputStream *input);
uint64_t readU64(librevenge::RVNGInputStream *input);

double readDouble(librevenge::RVNGInputStream *input);

const librevenge::RVNGString getColourString(const Colour &c);

unsigned long getRemainingLength(librevenge::RVNGInputStream *input);

void appendUCS4(librevenge::RVNGString &text, UChar32 ucs4Character);

void debugPrint(const char *format, ...) VSD_ATTRIBUTE_PRINTF(1, 2);

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
