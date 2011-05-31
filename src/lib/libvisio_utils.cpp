/* libvisio
 * Copyright (C) 2011 Fridrich Strba <fridrich.strba@bluewin.ch>
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
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02111-1301 USA
 */

#include "libvisio_utils.h"

#define VSD_NUM_ELEMENTS(array) sizeof(array)/sizeof(array[0])

uint8_t readU8(WPXInputStream *input)
{
	if (!input || input->atEOS())
		return (uint8_t)0;
	unsigned long numBytesRead;
	uint8_t const * p = input->read(sizeof(uint8_t), numBytesRead);
	
	if (p && numBytesRead == sizeof(uint8_t))
		return *(uint8_t const *)(p);
	return (uint8_t)0;
}

uint16_t readU16(WPXInputStream *input)
{
	uint16_t p0 = (uint16_t)readU8(input);
	uint16_t p1 = (uint16_t)readU8(input);
	return (uint16_t)(p0|(p1<<8));
}

uint32_t readU32(WPXInputStream *input)
{
	uint32_t p0 = (uint32_t)readU8(input);
	uint32_t p1 = (uint32_t)readU8(input);
	uint32_t p2 = (uint32_t)readU8(input);
	uint32_t p3 = (uint32_t)readU8(input);
	return (uint32_t)(p0|(p1<<8)|(p2<<16)|(p3<<24));
}

uint64_t readU64(WPXInputStream *input)
{
	uint64_t p0 = (uint64_t)readU8(input);
	uint64_t p1 = (uint64_t)readU8(input);
	uint64_t p2 = (uint64_t)readU8(input);
	uint64_t p3 = (uint64_t)readU8(input);
	uint64_t p4 = (uint64_t)readU8(input);
	uint64_t p5 = (uint64_t)readU8(input);
	uint64_t p6 = (uint64_t)readU8(input);
	uint64_t p7 = (uint64_t)readU8(input);
	return (uint64_t)(p0|(p1<<8)|(p2<<16)|(p3<<24)|(p4<<32)|(p5<<40)|(p6<<48)|(p7<<56));
}

double readDouble(WPXInputStream *input)
{
#if defined (_MSC_VER) && (_MSC_VER <= 1200)
    return static_cast<double>(static_cast<__int64>(readU64(input)));
#else
	return static_cast<double>(readU64(input));
#endif
}
