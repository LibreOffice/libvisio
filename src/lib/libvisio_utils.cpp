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

#define VSD_GET_UINT8(p) (*(unsigned char const *)(p))
#define VSD_GET_UINT16(p)				  \
        (unsigned short)((((unsigned char const *)(p))[0] << 0)  |    \
                  (((unsigned char const *)(p))[1] << 8))
#define VSD_GET_UINT32(p) \
        (unsigned)((((unsigned char const *)(p))[0] << 0)  |    \
                  (((unsigned char const *)(p))[1] << 8)  |    \
                  (((unsigned char const *)(p))[2] << 16) |    \
                  (((unsigned char const *)(p))[3] << 24))

#define VSD_NUM_ELEMENTS(array) sizeof(array)/sizeof(array[0])

unsigned char readU8(WPXInputStream *input)
{
	unsigned long numBytesRead;
	unsigned char const * p = input->read(sizeof(unsigned char), numBytesRead);

  	if (!p || numBytesRead != sizeof(unsigned char))
		return 0;

	return VSD_GET_UINT8(p);
}

unsigned short readU16(WPXInputStream *input)
{
	unsigned long numBytesRead;
	unsigned short const *val = (unsigned short const *)input->read(sizeof(unsigned short), numBytesRead);

	if (!val || numBytesRead != sizeof(unsigned short))
  		return 0;

	return VSD_GET_UINT16(val);
}

unsigned readU32(WPXInputStream *input)
{
	unsigned long numBytesRead;
	unsigned const *val = (unsigned const *)input->read(sizeof(unsigned), numBytesRead);

	if (!val || numBytesRead != sizeof(unsigned))
  		return 0;

	return VSD_GET_UINT32(val);
}
