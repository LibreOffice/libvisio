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

#ifndef __LIBVISIO_UTILS_H__
#define __LIBVISIO_UTILS_H__

#include <stdio.h>

//#define DEBUG // FIXME !

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

#endif // __LIBVISIO_UTILS_H__
